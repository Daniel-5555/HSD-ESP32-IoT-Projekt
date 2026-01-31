//================================================================================
//| DATEI: TimeService.h                                                         |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| Definiert die Klasse zur Verwaltung der Systemzeit. Der TimeService ist      |
//| verantwortlich für die Synchronisation der internen Uhr des ESP32 mit einem  |
//| NTP-Server, sobald eine WLAN-Verbindung besteht, und stellt die aktuelle     |
//| Zeit formatiert zur Verfügung.                                               |
//================================================================================

#pragma once

#include <Arduino.h>
#include "../modules/WiFi/WifiManager.h"

/**
 * @class TimeService
 * @brief Verwaltet die Systemzeit durch Synchronisation mit einem NTP-Server.
 */
class TimeService {
public:
    /**
     * @brief Konstruktor. Benötigt den WifiManager, um den Verbindungsstatus zu prüfen.
     * @param wifiManager Referenz auf das WifiManager-Objekt.
     */
    TimeService(WifiManager& wifiManager);

    /**
     * @brief Initialisiert den TimeService.
     */
    void setup();

    /**
     * @brief Wird in der Hauptschleife aufgerufen, um die Zeit zu aktualisieren.
     */
    void loop();

    /**
     * @brief Gibt den Unix-Timestamp der letzten erfolgreichen Synchronisation zurück.
     * @return Unix-Timestamp (Sekunden seit 1970-01-01) oder 0, wenn nie synchronisiert.
     */
    time_t getLastSyncTimestamp();

    /**
     * @brief Gibt die aktuelle, formatierte Zeit als String zurück.
     * @return Formatierte Zeit (z.B. "dd.mm.YYYY HH:MM:SS") oder "N/A".
     */
    String getFormattedTime();

private:
    void syncTime(); // Private Hilfsfunktion zur Durchführung der Synchronisation.

    WifiManager& _wifiManager;
    time_t _lastSyncTimestamp; // Speichert den Zeitpunkt der letzten erfolgreichen Synchronisation.
    unsigned long _nextSyncAttempt; // Zeitstempel für den nächsten Synchronisationsversuch.
};