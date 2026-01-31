//================================================================================
//| DATEI: AsyncWebServer.h                                                      |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| Definition der High-Performance Wrapper-Klassen für den ESP32 Webserver.     |
//|                                                                              |
//| WARUM DIESER WRAPPER?                                                        |
//| 1. KEIN LGPL: Dieser Code ist frei von Copyleft-Lizenzen (wie LGPL), die bei |
//|    der Nutzung von "ESPAsyncWebServer" oft problematisch für kommerzielle    |
//|    oder proprietäre Projekte sind.                                           |
//| 2. NATIVE PERFORMANCE: Er nutzt direkt die "esp_http_server" C-API des       |
//|    ESP-IDF SDKs. Das ist speicherschonender und stabiler als externe Libs.   |
//| 3. ARDUINO-STYLE: Bietet die gewohnte Syntax (.on, .send, .arg), damit der   |
//|    Umstieg von Standard-Arduino-Bibliotheken leicht fällt.                   |
//================================================================================

#pragma once

#include <Arduino.h>
#include <esp_http_server.h>
#include <FS.h>
#include <functional>

// Mapping der HTTP Methoden für Kompatibilität zur Arduino-Welt
#define HTTP_ANY    -1

class AsyncWebServerRequest;

/**
 * @brief Kontext-Struktur für die Callbacks.
 * Da die native C-API (esp_http_server) keine C++ Lambda-Funktionen mit Capture
 * speichern kann, nutzen wir diese Struktur als "User Context".
 * Der statische Dispatcher holt sich diese Struktur und ruft die C++ Funktionen auf.
 */
struct RouteContext {
    std::function<void(AsyncWebServerRequest*)> handler;
    std::function<void(AsyncWebServerRequest*, uint8_t*, size_t, size_t, size_t)> bodyHandler;
    std::function<void(AsyncWebServerRequest*, String, size_t, uint8_t*, size_t, bool)> uploadHandler;
};

/**
 * @brief Wrapper um einen einzelnen HTTP-Request (httpd_req_t).
 * Diese Klasse kapselt den Zugriff auf Anfrage-Parameter, Header und
 * stellt Methoden zum Senden der Antwort bereit.
 */
class AsyncWebServerRequest {
public:
    /**
     * @brief Konstruktor.
     * @param req Zeiger auf das native Request-Handle des ESP-IDF.
     */
    AsyncWebServerRequest(httpd_req_t* req) : _req(req) {}

    /**
     * @brief Sendet eine Text- oder JSON-Antwort.
     * @param code HTTP Status Code (z.B. 200, 404).
     * @param contentType MIME-Type (z.B. "text/html", "application/json").
     * @param content Der eigentliche Inhalt als String.
     */
    void send(int code, const String& contentType, const String& content);
    
    /**
     * @brief Sendet eine Datei aus dem Dateisystem (SPIFFS/LittleFS).
     * Nutzt Chunked-Transfer, um RAM zu sparen.
     * @param fs Referenz auf das Dateisystem (z.B. SPIFFS).
     * @param path Pfad zur Datei.
     * @param contentType MIME-Type der Datei.
     * @param download Wenn true, wird der Browser zum Download gezwungen (Content-Disposition).
     */
    void send(fs::FS &fs, const String& path, const String& contentType, bool download = false);

    /**
     * @brief Ruft einen URL-Parameter ab (z.B. ?id=123).
     * @param name Name des Parameters.
     * @return Wert des Parameters oder leerer String.
     */
    String arg(const String& name);

    /**
     * @brief Prüft, ob ein Parameter existiert.
     */
    bool hasParam(const String& name, bool post = false);

    /**
     * @brief Fügt einen HTTP-Header zur Antwort hinzu.
     */
    void addHeader(const String& name, const String& value);

    /**
     * @brief Gibt die angeforderte URL zurück.
     */
    String url();

    /**
     * @brief Zugriff auf das native Handle (für fortgeschrittene C-API Nutzung).
     */
    httpd_req_t* getNativeRequest() { return _req; }

private:
    httpd_req_t* _req;
};

/**
 * @brief Hauptklasse für den Webserver.
 * Verwaltet die Konfiguration, das Starten des Servers und die Registrierung der Routen.
 */
class AsyncWebServer {
public:
    /**
     * @brief Erstellt eine neue Server-Instanz.
     * @param port Der TCP-Port (Standard: 80).
     */
    AsyncWebServer(int port);
    ~AsyncWebServer();

    /**
     * @brief Startet den Server und konfiguriert die ESP-IDF Parameter.
     */
    void begin();
    
    /**
     * @brief Registriert einen einfachen Request-Handler (GET, POST, etc.).
     * @param uri Der Pfad (z.B. "/api/status").
     * @param method HTTP Methode (HTTP_GET, HTTP_POST, HTTP_ANY).
     * @param handler Die Funktion, die aufgerufen wird.
     */
    void on(const char* uri, int method, std::function<void(AsyncWebServerRequest*)> handler);

    /**
     * @brief Registriert einen erweiterten Handler für Datei-Uploads.
     * @param uri Der Pfad.
     * @param method Meist HTTP_POST.
     * @param onRequest Wird aufgerufen, wenn der Upload fertig ist (zum Senden der Antwort).
     * @param onUpload Wird wiederholt aufgerufen, während Datenpakete ankommen.
     * @param onBody Optionaler Body-Handler (hier meist nullptr).
     */
    void on(const char* uri, int method, 
            std::function<void(AsyncWebServerRequest*)> onRequest,
            std::function<void(AsyncWebServerRequest*, String, size_t, uint8_t*, size_t, bool)> onUpload,
            std::function<void(AsyncWebServerRequest*, uint8_t*, size_t, size_t, size_t)> onBody = nullptr);

    /**
     * @brief Bedient statische Dateien aus dem Dateisystem.
     * Registriert Handler für "/" und "/*".
     * @param uri Basis-URI (meist "/").
     * @param fs Dateisystem (SPIFFS).
     * @param path Pfad im Dateisystem (meist "/").
     */
    void serveStatic(const char* uri, fs::FS& fs, const char* path);

    /**
     * @brief Handler für nicht gefundene Seiten (404).
     * Hinweis: In der nativen API oft durch Wildcard-Routen gelöst.
     */
    void onNotFound(std::function<void(AsyncWebServerRequest*)> handler);

private:
    int _port;
    httpd_handle_t _server = nullptr;
    
    /**
     * @brief Statische Dispatcher-Funktion.
     * Dient als Brücke zwischen der C-API (die Funktionszeiger erwartet)
     * und den C++ Member-Funktionen/Lambdas.
     */
    static esp_err_t _dispatcher(httpd_req_t *req);
};