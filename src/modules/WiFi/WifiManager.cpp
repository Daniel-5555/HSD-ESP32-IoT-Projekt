//================================================================================
//| DATEI: WifiManager.cpp                                                       |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| Implementiert die Logik zur Verwaltung der WLAN-Verbindungen. Dies umfasst   |
//| das Auslesen gespeicherter Zugangsdaten aus dem NVS, den Verbindungsaufbau,  |
//| das Starten des Konfigurations-Access-Points und das zeitgesteuerte          |
//| Deaktivieren des APs nach erfolgreicher Verbindung.                          |
//================================================================================

#include "WifiManager.h"
#include "../../config.h" // Für globale Konfigurationen wie AP_SSID
#include <Preferences.h>
#include <esp_wifi.h>  // Für erweiterte WLAN-Funktionen

/**
 * @brief Konstruktor-Implementierung. Initialisiert die Member-Variablen.
 */
WifiManager::WifiManager() : _apStopTime(0), _apShouldBeDisabled(false) {}

/**
 * @brief Initialisiert das WLAN.
 */
void WifiManager::setup() {
    Preferences preferences;
    // Öffne den NVS-Namespace "wifi_creds". 'false' bedeutet Lese-/Schreibzugriff.
    preferences.begin("wifi_creds", false);
    // Lese SSID und Passwort aus NVS. Wenn die Schlüssel nicht existieren, wird ein leerer String zurückgegeben.
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");
    preferences.end(); // Schließe den Namespace.

    // Setze den Hostnamen des Geräts im Netzwerk.
    WiFi.setHostname(HOSTNAME);
    // Starte im kombinierten Modus (Station + Access Point).
    WiFi.mode(WIFI_AP_STA);
    
    // Starte immer den Access Point, damit das Gerät erreichbar ist.
    startAP();

    // Wenn eine SSID gespeichert ist, versuche, dich damit zu verbinden.
    if (ssid.length() > 0) {
        connectToWifi(ssid, password);
    } else {
        Serial.println("Keine WLAN-Daten gefunden. Nur AP-Modus aktiv.");
    }
}

/**
 * @brief Hauptschleifen-Funktion für den WifiManager.
 */
void WifiManager::loop() {
    // Prüfe, ob der AP-Abschalt-Timer abgelaufen ist.
    if (_apShouldBeDisabled && millis() > _apStopTime) {
        stopAP();
        _apShouldBeDisabled = false; // Verhindert mehrfaches Ausführen
    }
}

/**
 * @brief Startet den Access Point mit den Daten aus config.h.
 */
void WifiManager::startAP() {
    Serial.print("Starte Access Point: ");
    Serial.println(AP_SSID);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Serial.print("AP IP-Adresse: ");
    Serial.println(WiFi.softAPIP());
}

/**
 * @brief Verbindet sich mit einem gegebenen WLAN.
 */
void WifiManager::connectToWifi(const String& ssid, const String& password) {
    Serial.print("Verbinde mit WLAN: ");
    Serial.println(ssid);
    WiFi.begin(ssid.c_str(), password.c_str());

    // Warte maximal 10 Sekunden auf eine Verbindung.
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nErfolgreich mit WLAN verbunden.");
        Serial.print("IP-Adresse: ");
        Serial.println(WiFi.localIP());
        // Setze den Timer, um den AP in 5 Minuten abzuschalten.
        _apStopTime = millis() + AP_TIMEOUT_MS;
        _apShouldBeDisabled = true;
    } else {
        Serial.println("\nVerbindung zum WLAN fehlgeschlagen.");
        WiFi.disconnect(true); // Alte Konfiguration löschen
    }
}

/**
 * @brief Deaktiviert den Access Point, um Strom zu sparen.
 */
void WifiManager::stopAP() {
    if (WiFi.getMode() == WIFI_AP_STA) {
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_STA); // Wechsle in den reinen Station-Modus
        Serial.println("Access Point wurde deaktiviert.");
    }
}

// --- Implementierung der Getter-Funktionen ---

bool WifiManager::isStationConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

String WifiManager::getIpAddress() {
    return isStationConnected() ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
}

String WifiManager::getSsid() {
    return isStationConnected() ? WiFi.SSID() : "Nicht verbunden";
}

String WifiManager::getMode() {
    bool apActive = (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA);
    if (isStationConnected() && apActive) return "Station + AP";
    if (isStationConnected()) return "Station";
    if (apActive) return "Access Point";
    return "Inaktiv";
}