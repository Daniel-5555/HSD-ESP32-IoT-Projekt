//================================================================================
//| DATEI: AsyncWebServer.cpp                                                    |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| High-Performance Wrapper um die native ESP-IDF "esp_http_server" API.        |
//|                                                                              |
//| WARUM DIESER WRAPPER?                                                        |
//| 1. KEIN LGPL: Im Gegensatz zur beliebten "ESPAsyncWebServer" Bibliothek      |
//|    nutzt dieser Code keine externen LGPL-Abhängigkeiten (wie AsyncTCP).      |
//|    Er basiert rein auf dem offiziellen Espressif SDK (Apache 2.0).           |
//| 2. STABILITÄT & PERFORMANCE: Nutzt den im ROM/SDK integrierten Server.       |
//|    Das spart Flash-Speicher und ist extrem robust.                           |
//| 3. EINFACHHEIT: Bietet eine API, die der Arduino-Syntax (.on, .send) ähnelt, |
//|    aber intern die mächtige C-API von Espressif verwendet.                   |
//================================================================================

#include "AsyncWebServer.h"
#include <SPIFFS.h>

// =============================================================
// === AsyncWebServerRequest Implementation                  ===
// =============================================================

/**
 * @brief Sendet eine einfache Text/HTML Antwort.
 * Konvertiert C++ Strings in die C-Strukturen der ESP-IDF.
 */
void AsyncWebServerRequest::send(int code, const String& contentType, const String& content) {
    httpd_resp_set_type(_req, contentType.c_str());
    
    // Status Code setzen (String-Konvertierung nötig für native API)
    char statusStr[16];
    sprintf(statusStr, "%d", code);
    
    // Gängige Statuscodes mappen
    if(code == 200) httpd_resp_set_status(_req, "200 OK");
    else if(code == 400) httpd_resp_set_status(_req, "400 Bad Request");
    else if(code == 404) httpd_resp_set_status(_req, "404 Not Found");
    else if(code == 500) httpd_resp_set_status(_req, "500 Internal Server Error");
    else httpd_resp_set_status(_req, statusStr);

    httpd_resp_send(_req, content.c_str(), content.length());
}

/**
 * @brief Streamt eine Datei aus dem Dateisystem (SPIFFS/LittleFS).
 * Nutzt "Chunked Transfer Encoding", um RAM zu sparen. Große Dateien werden
 * in kleinen Häppchen (Chunks) gesendet.
 */
void AsyncWebServerRequest::send(fs::FS &fs, const String& path, const String& contentType, bool download) {
    File file = fs.open(path, "r");
    if(!file) { 
        send(404, "text/plain", "File not found"); 
        return; 
    }
    httpd_resp_set_type(_req, contentType.c_str());
    
    // Falls Download gewünscht ist, Header setzen
    if(download) {
        String filename = path;
        int slash = filename.lastIndexOf('/');
        if(slash >= 0) filename = filename.substring(slash+1);
        String cd = "attachment; filename=\"" + filename + "\"";
        httpd_resp_set_hdr(_req, "Content-Disposition", cd.c_str());
    }

    // Chunked Response Loop
    char buf[1024];
    while(file.available()) {
        size_t len = file.readBytes(buf, sizeof(buf));
        httpd_resp_send_chunk(_req, buf, len);
    }
    httpd_resp_send_chunk(_req, NULL, 0); // Ende signalisieren (0-Byte Chunk)
    file.close();
}

/**
 * @brief Liest einen GET-Parameter aus der URL.
 * Beispiel: /save?ssid=Test -> arg("ssid") gibt "Test" zurück.
 * Hinweis: Funktioniert aktuell nur für URL-Parameter, nicht für POST-Body-Formulare.
 */
String AsyncWebServerRequest::arg(const String& name) {
    size_t len = httpd_req_get_url_query_len(_req) + 1;
    if (len > 1) {
        char* buf = (char*)malloc(len);
        if (buf) {
            if (httpd_req_get_url_query_str(_req, buf, len) == ESP_OK) {
                char val[128];
                if (httpd_query_key_value(buf, name.c_str(), val, sizeof(val)) == ESP_OK) {
                    free(buf); 
                    return String(val);
                }
            }
            free(buf);
        }
    }
    return "";
}

bool AsyncWebServerRequest::hasParam(const String& name, bool post) { 
    return arg(name) != ""; 
}

void AsyncWebServerRequest::addHeader(const String& name, const String& value) {
    httpd_resp_set_hdr(_req, name.c_str(), value.c_str());
}

String AsyncWebServerRequest::url() {
    return String(_req->uri);
}

// =============================================================
// === AsyncWebServer Implementation                         ===
// =============================================================

AsyncWebServer::AsyncWebServer(int port) : _port(port) {}

AsyncWebServer::~AsyncWebServer() { 
    if(_server) httpd_stop(_server); 
}

/**
 * @brief Startet den nativen ESP-IDF Webserver.
 * Konfiguriert Speicherlimits und Handler-Anzahl.
 */
void AsyncWebServer::begin() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = _port;
    config.stack_size = 8192; // Erhöhter Stack für JSON/String Operationen
    config.lru_purge_enable = true;
    
    // WICHTIG: Standard ist 8. Das reicht nicht für API + Statische Dateien!
    // Wir setzen es hoch auf 200, um sicherzustellen, dass alle Routen registriert werden.
    config.max_uri_handlers = 200; 
    
    // Erlaubt Wildcards wie /api/* oder /*
    config.uri_match_fn = httpd_uri_match_wildcard;

    if (httpd_start(&_server, &config) == ESP_OK) {
        Serial.printf("HTTP Server gestartet auf Port %d\n", _port);
    } else {
        Serial.println("Fehler: HTTP Server konnte nicht starten!");
    }
}

/**
 * @brief Statische Dispatcher-Funktion (Bridge C -> C++).
 * Die native C-API kann keine C++ Member-Funktionen aufrufen.
 * Diese Funktion nimmt den "user_ctx" (unseren RouteContext), castet ihn zurück
 * und ruft die eigentliche C++ Lambda-Funktion auf.
 */
esp_err_t AsyncWebServer::_dispatcher(httpd_req_t *req) {
    RouteContext* ctx = (RouteContext*)req->user_ctx;
    AsyncWebServerRequest wrappedReq(req);
    
    // --- Upload / Body Handling ---
    // Hier wird der eingehende Datenstrom (z.B. Firmware-Update) verarbeitet.
    if (ctx->bodyHandler || ctx->uploadHandler) {
        size_t total = req->content_len;
        size_t cur = 0;
        const size_t bufSize = 4096; 
        char* buf = (char*)malloc(bufSize);
        
        if (!buf) return ESP_ERR_NO_MEM;

        int ret;
        
        // Upload Loop: Liest Daten in Chunks vom Netzwerk
        if (ctx->uploadHandler && total > 0) {
             while ((ret = httpd_req_recv(req, buf, bufSize)) > 0) {
                if (ret == HTTPD_SOCK_ERR_TIMEOUT) continue;
                
                // WICHTIG: Wir simulieren hier einen "Raw Upload".
                // Multipart-Parsing ist komplex und fehleranfällig.
                // Durch das Senden als "application/octet-stream" im Frontend
                // landen die reinen Binärdaten hier.
                ctx->uploadHandler(&wrappedReq, "upload.bin", cur, (uint8_t*)buf, ret, (cur + ret) >= total);
                cur += ret;
            }
        }
        free(buf);
        
        // Nach dem Upload den Request-Handler aufrufen (z.B. "Update Success" senden)
        if (ctx->handler) ctx->handler(&wrappedReq);
        return ESP_OK;
    }

    // --- Normaler Request (GET/POST ohne Body) ---
    if (ctx->handler) {
        ctx->handler(&wrappedReq);
        return ESP_OK;
    }
    return ESP_FAIL;
}

void AsyncWebServer::on(const char* uri, int method, std::function<void(AsyncWebServerRequest*)> handler) {
    on(uri, method, handler, nullptr, nullptr);
}

/**
 * @brief Registriert eine neue Route im nativen Server.
 */
void AsyncWebServer::on(const char* uri, int method, 
        std::function<void(AsyncWebServerRequest*)> onRequest,
        std::function<void(AsyncWebServerRequest*, String, size_t, uint8_t*, size_t, bool)> onUpload,
        std::function<void(AsyncWebServerRequest*, uint8_t*, size_t, size_t, size_t)> onBody) {
            
    if (!_server) return;
    
    // Context muss auf dem Heap bleiben, da der Server asynchron darauf zugreift.
    // (Wird aktuell nicht gelöscht -> Memory Leak bei Server-Neustart, aber okay für Embedded Dauerlauf)
    RouteContext* ctx = new RouteContext{onRequest, onBody, onUpload};
    
    httpd_uri_t route = {
        .uri = uri,
        .method = (httpd_method_t)method,
        .handler = _dispatcher,
        .user_ctx = ctx
    };
    
    // Workaround: HTTP_ANY mappen wir auf GET, da native API explizit sein will.
    if (method == HTTP_ANY) route.method = HTTP_GET;
    
    // WICHTIG: Rückgabewert prüfen! Hilft beim Debuggen von "URI does not exist".
    esp_err_t err = httpd_register_uri_handler(_server, &route);
    if (err != ESP_OK) {
        Serial.printf("FEHLER: Konnte Route '%s' nicht registrieren! Fehlercode: %d\n", uri, err);
        delete ctx; 
    } else {
        Serial.printf("Route registriert: %s\n", uri);
    }
}

void AsyncWebServer::onNotFound(std::function<void(AsyncWebServerRequest*)> handler) {
    // Native API hat keinen globalen 404 Handler im gleichen Sinne.
    // Dies wird meist über den Wildcard-Handler in serveStatic gelöst.
}

/**
 * @brief Bedient statische Dateien aus dem Dateisystem.
 * Registriert "/" und "/*" um alle Anfragen abzufangen, die keine API-Route getroffen haben.
 */
void AsyncWebServer::serveStatic(const char* uri, fs::FS& fs, const char* path) {
    // 1. Der Handler-Code (wird für beide Routen verwendet)
    auto handlerFunc = [path](AsyncWebServerRequest* req) {
        String url = req->url();
        
        // --- FIX: Query-Parameter (alles ab ?) entfernen ---
        // Sonst wird "/index.html?foo=bar" nicht als "/index.html" gefunden.
        int qIndex = url.indexOf('?');
        if (qIndex >= 0) {
            url = url.substring(0, qIndex);
        }
        // ---------------------------------------------------

        if(url.endsWith("/")) url += "index.html";
        
        String filePath = String(path) + url;
        // Doppelte Slashes entfernen
        while(filePath.indexOf("//") >= 0) filePath.replace("//", "/");
        
        if(SPIFFS.exists(filePath)) {
            // MIME-Type Erkennung
            String contentType = "text/plain";
            if(filePath.endsWith(".html")) contentType = "text/html";
            else if(filePath.endsWith(".css")) contentType = "text/css";
            else if(filePath.endsWith(".js")) contentType = "application/javascript";
            else if(filePath.endsWith(".png")) contentType = "image/png";
            else if(filePath.endsWith(".ico")) contentType = "image/x-icon";
            else if(filePath.endsWith(".json")) contentType = "application/json";
            
            req->send(SPIFFS, filePath, contentType);
        } else {
            req->send(404, "text/plain", "File not found");
        }
    };

    // 2. Context erstellen
    RouteContext* ctx = new RouteContext();
    ctx->handler = handlerFunc;

    // 3. Route für GENAU "/" registrieren (Wichtig für Root-Aufruf)
    httpd_uri_t rootRoute = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = _dispatcher,
        .user_ctx = ctx
    };
    httpd_register_uri_handler(_server, &rootRoute);

    // 4. Route für ALLES ANDERE "/*" registrieren (Catch-All)
    httpd_uri_t wildcardRoute = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = _dispatcher,
        .user_ctx = ctx
    };
    httpd_register_uri_handler(_server, &wildcardRoute);
}