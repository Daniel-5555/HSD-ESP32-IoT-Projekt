//================================================================================
//| DATEI: OtaApiHandler.cpp                                                     |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| Diese Klasse implementiert die serverseitige Logik für Over-the-Air (OTA)    |
//| Updates. Sie ist verantwortlich für:                                         |
//| 1. Das Bereitstellen einer HTML-Seite für den Upload.                        |
//| 2. Das Entgegennehmen von Firmware- und Dateisystem-Dateien.                 |
//| 3. Das Schreiben dieser Dateien in den Flash-Speicher des ESP32.             |
//================================================================================

#include "OtaApiHandler.h"
#include <SPIFFS.h> // Wird benötigt, um die HTML-Seite aus dem Dateisystem zu laden.

/**
 * @brief Standard-Konstruktor.
 * In diesem Fall sind keine speziellen Initialisierungen notwendig.
 */
OtaApiHandler::OtaApiHandler() {}

/**
 * @brief Registriert alle OTA-bezogenen Routen am Webserver.
 * Diese Funktion wird einmal in `AppWebServer::setup()` aufgerufen, um dem Server
 * mitzuteilen, wie er auf Anfragen an die OTA-URLs reagieren soll.
 */
void OtaApiHandler::registerRoutes(AsyncWebServer& server) {

    // --- ROUTE 1: Die HTML-Seite für den Upload bereitstellen ---
    // Reagiert auf GET-Anfragen an "/update.html".
    server.on("/update.html", HTTP_GET, [](AsyncWebServerRequest *request){
        // Da wir keine Platzhalter mehr ersetzen müssen, senden wir die Datei direkt.
        // Das ist effizienter und weniger fehleranfällig.
        if(SPIFFS.exists("/update.html")) {
            request->send(SPIFFS, "/update.html", "text/html");
        } else {
            request->send(404, "text/plain", "update.html nicht gefunden");
        }
    });


    // --- ROUTE 2: Den Firmware-Upload verarbeiten ---
    // Reagiert auf POST-Anfragen an "/update".
    // WICHTIG: Durch die Änderung im HTML (Raw Upload) kommen hier reine Binärdaten an.
    server.on("/update", HTTP_POST, 
        // "onRequest"-Handler: Wird ausgeführt, wenn der Upload fertig ist.
        [](AsyncWebServerRequest *request) {
            // Prüfen, ob während des Schreibens in den Flash ein Fehler aufgetreten ist.
            bool success = !Update.hasError();
            
            // Eine Antwort an den Client senden.
            request->send(200, "text/plain", success ? "Update ERFOLGREICH! Neustart..." : "Update FEHLGESCHLAGEN!");
            request->addHeader("Connection", "close"); 
            
            // Wenn das Update erfolgreich war, starte den ESP32 neu.
            if (success) {
                delay(1000);
                ESP.restart();
            }
        },
        // "onUpload"-Handler: Wird während des Uploads für jeden Daten-Chunk aufgerufen.
        [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            // Wir delegieren die eigentliche Arbeit an unsere zentrale `handleUpdate`-Funktion.
            // `false` signalisiert, dass es sich um ein Firmware-Update handelt.
            this->handleUpdate(request, filename, index, data, len, final, false);
        }
    );


    // --- ROUTE 3: Den SPIFFS-Upload verarbeiten ---
    // Reagiert auf POST-Anfragen an "/update_spiffs".
    server.on("/update_spiffs", HTTP_POST, 
        // "onRequest"-Handler
        [](AsyncWebServerRequest *request) {
            bool success = !Update.hasError();
            request->send(200, "text/plain", success ? "SPIFFS Update ERFOLGREICH! Neustart..." : "SPIFFS Update FEHLGESCHLAGEN!");
            request->addHeader("Connection", "close");
            
            if (success) {
                delay(1000);
                ESP.restart();
            }
        },
        // "onUpload"-Handler
        [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            // `true` signalisiert, dass es sich um ein SPIFFS-Update handelt.
            this->handleUpdate(request, filename, index, data, len, final, true);
        }
    );
}

/**
 * @brief Verarbeitet die ankommenden Binärdaten-Chunks während des Uploads.
 * Dies ist die Kernfunktion der OTA-Logik. Sie wird für jeden Teil der hochgeladenen
 * Datei aufgerufen und schreibt ihn direkt in den Flash-Speicher.
 */
void OtaApiHandler::handleUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final, bool isSpiffs) {
    
    // --- Schritt 1: Das Update starten (wird nur einmal beim allerersten Chunk ausgeführt) ---
    if (index == 0) {
        Serial.printf("Update Start: %s (%s)\n", filename.c_str(), isSpiffs ? "SPIFFS" : "Firmware");
        
        // Wähle das Ziel-Kommando für die `Update`-Bibliothek.
        // U_FLASH -> Nächste freie App-Partition
        // U_SPIFFS -> Die SPIFFS-Partition
        int cmd = isSpiffs ? U_SPIFFS : U_FLASH;
        
        // Starte den Update-Prozess. `UPDATE_SIZE_UNKNOWN` ist wichtig, da wir bei
        // einem Raw-Stream die Gesamtgröße oft nicht im Header haben.
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) {
            Update.printError(Serial);
        }
    }

    // --- Schritt 2: Die Daten schreiben (wird für jeden Chunk ausgeführt) ---
    // Schreibe den aktuellen Daten-Chunk in den Flash, solange kein Fehler vorliegt.
    if (len > 0 && !Update.hasError()) {
        // `Update.write` gibt die Anzahl der tatsächlich geschriebenen Bytes zurück.
        if (Update.write(data, len) != len) {
            Update.printError(Serial);
        }
    }

    // --- Schritt 3: Das Update abschließen (wird nur einmal beim letzten Chunk ausgeführt) ---
    if (final) {
        // `Update.end(true)` beendet den Schreibvorgang und verifiziert das Update.
        if (Update.end(true)) {
            Serial.printf("Update erfolgreich abgeschlossen: %u Bytes\n", index + len);
        } else {
            Update.printError(Serial);
        }
    }
}