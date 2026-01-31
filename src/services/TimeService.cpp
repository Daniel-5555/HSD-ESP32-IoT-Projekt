//================================================================================
//| DATEI: TimeService.cpp                                                       |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| Implementiert die Logik für die NTP-Zeitsynchronisation. Die Klasse prüft    |
//| periodisch den WLAN-Status und versucht, die Zeit zu synchronisieren. Sie    |
//| verwaltet den Zeitpunkt der letzten erfolgreichen Synchronisation und plant  |
//| erneute Versuche bei Fehlschlägen oder nach Ablauf einer bestimmten Zeit.    |
//================================================================================

#include "TimeService.h"
#include "time.h" // C-Standardbibliothek für Zeitfunktionen

// --- NTP-Konfiguration ---
const char* NTP_SERVER = "pool.ntp.org";
// Zeitzone: GMT+1 (Berlin). Im Sommer GMT+2.
// Die komplexe Regel für die Sommerzeitumstellung ist hier als String definiert.
const char* TIME_ZONE = "CET-1CEST,M3.5.0,M10.5.0/3";

TimeService::TimeService(WifiManager& wifiManager)
    : _wifiManager(wifiManager), _lastSyncTimestamp(0), _nextSyncAttempt(0) {}

void TimeService::setup() {
    // Die setup-Funktion ist hier bewusst leer. Die Initialisierung der Zeit
    // (configTime) wird erst aufgerufen, wenn eine WLAN-Verbindung besteht.
    Serial.println("TimeService initialisiert.");
}

void TimeService::loop() {
    // Führe die Logik nur aus, wenn eine WLAN-Verbindung besteht und der nächste
    // Versuch fällig ist (verhindert ständige Versuche bei fehlgeschlagener Verbindung).
    if (_wifiManager.isStationConnected() && millis() > _nextSyncAttempt) {
        // Wenn die Zeit noch nie synchronisiert wurde, versuche es sofort.
        if (_lastSyncTimestamp == 0) {
            syncTime();
        }
        // Ansonsten, synchronisiere alle 24 Stunden erneut, um die Zeit präzise zu halten.
        else if (time(nullptr) > _lastSyncTimestamp + (24 * 3600)) {
            syncTime();
        }
    }
}

void TimeService::syncTime() {
    Serial.println("Versuche, Zeit via NTP zu synchronisieren...");
    // Konfiguriere den ESP32, um die Zeit vom NTP-Server mit der korrekten Zeitzone zu holen.
    configTime(0, 0, NTP_SERVER);
    setenv("TZ", TIME_ZONE, 1);
    tzset();

    // Warte auf die Synchronisation.
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10000)) { // Warte maximal 10 Sekunden
        _lastSyncTimestamp = time(nullptr);
        Serial.print("Zeitsynchronisation erfolgreich. Aktuelle Zeit: ");
        Serial.println(getFormattedTime());
        // Nächster Versuch erst wieder in 24 Stunden.
        _nextSyncAttempt = millis() + (24 * 3600 * 1000);
    } else {
        Serial.println("Zeitsynchronisation fehlgeschlagen.");
        // Versuche es in 5 Minuten erneut.
        _nextSyncAttempt = millis() + (5 * 60 * 1000);
    }
}

time_t TimeService::getLastSyncTimestamp() {
    return _lastSyncTimestamp;
}

String TimeService::getFormattedTime() {
    if (_lastSyncTimestamp == 0) {
        return "N/A";
    }
    struct tm timeinfo;
    localtime_r(&_lastSyncTimestamp, &timeinfo);
    char buffer[32];
    // Formatiert die Zeit in ein deutsches Datumsformat.
    strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", &timeinfo);
    return String(buffer);
}