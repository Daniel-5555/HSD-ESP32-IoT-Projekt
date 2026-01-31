//================================================================================
//| DATEI: WifiApiHandler.cpp                                                    |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| Implementiert die Logik zum Verarbeiten der HTTP-Anfrage zum Speichern von   |
//| WLAN-Zugangsdaten. Die Klasse extrahiert SSID und Passwort aus der Anfrage,  |
//| speichert sie persistent im NVS (Non-Volatile Storage) und leitet einen      |
//| Neustart des Geräts ein.                                                     |
//================================================================================

#include "WifiApiHandler.h"
#include <Preferences.h> // Notwendig für das Speichern der Daten im NVS.

/**
 * @brief Konstruktor-Implementierung.
 */
WifiApiHandler::WifiApiHandler(WifiManager& wifiManager) : _wifiManager(wifiManager) {}

/**
 * @brief Implementierung der Routen-Registrierung.
 */
void WifiApiHandler::registerRoutes(AsyncWebServer& server) {
    // ÄNDERUNG: HTTP_GET statt HTTP_POST
    server.on("/save", HTTP_GET, std::bind(&WifiApiHandler::handleSaveCredentials, this, std::placeholders::_1));
}

/**
 * @brief Bearbeitet das Speichern der WLAN-Zugangsdaten.
 * Diese Funktion enthält die Logik, die zuvor direkt im `WebServer.cpp` stand.
 */
void WifiApiHandler::handleSaveCredentials(AsyncWebServerRequest *request) {
    // Prüft, ob die erwarteten Parameter "ssid" und "password" in der Anfrage enthalten sind.
    if (request->hasParam("ssid") && request->hasParam("password")) {
        String ssid = request->arg("ssid");
        String password = request->arg("password");

        // Die Logik zum Speichern der Daten im nicht-flüchtigen Speicher (NVS).
        // Ein noch sauberer Ansatz wäre, eine Methode `_wifiManager.saveCredentials(ssid, password)`
        // aufzurufen, um die Logik vollständig im WifiManager zu kapseln.
        Preferences preferences;
        preferences.begin("wifi_creds", false); // Namespace "wifi_creds" im Schreibmodus öffnen.
        preferences.putString("ssid", ssid);
        preferences.putString("password", password);
        preferences.end(); // Namespace schließen und Änderungen speichern.
        Serial.println("WLAN-Daten im NVS gespeichert.");

        // Bestätigung an den Client senden und Neustart einleiten, damit die neuen Daten verwendet werden.
        request->send(200, "text/plain", "Daten gespeichert. ESP32 startet neu.");
        delay(1000);
        ESP.restart();
    } else {
        // Wenn Parameter fehlen, wird ein "Bad Request"-Fehler (400) gesendet.
        request->send(400, "text/plain", "Fehlende Daten.");
    }
}