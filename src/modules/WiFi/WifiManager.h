//================================================================================
//| DATEI: WifiManager.h                                                         |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| Definiert die Klasse zur Verwaltung der WLAN-Konnektivität. Sie kümmert sich |
//| um den Start des Access Points (AP), die Verbindung zu einem gespeicherten   |
//| Netzwerk (Station-Modus) und stellt Methoden zur Abfrage des aktuellen       |
//| Netzwerkstatus bereit.                                                       |
//================================================================================

#pragma once

#include <WiFi.h>
#include <Preferences.h>

/**
 * @class WifiManager
 * @brief Verwaltet die WLAN-Konnektivität des ESP32.
 *
 * Diese Klasse kümmert sich um den Start des Access Points (AP), die Verbindung
 * zu einem gespeicherten Heimnetzwerk (Station-Modus, STA) und den zeitgesteuerten
 * Wechsel zwischen diesen Modi.
 */
class WifiManager {
public:
    /**
     * @brief Konstruktor für den WifiManager.
     */
    WifiManager();

    /**
     * @brief Initialisiert den WLAN-Modus. Liest gespeicherte Daten und startet AP oder STA.
     */
    void setup();

    /**
     * @brief Wird in der Hauptschleife aufgerufen, um zeitgesteuerte Aufgaben zu erledigen.
     * Insbesondere das Deaktivieren des AP nach einer gewissen Zeit.
     */
    void loop();

    /**
     * @brief Prüft, ob der ESP32 erfolgreich mit einem WLAN-Netzwerk verbunden ist.
     * @return true, wenn verbunden, sonst false.
     */
    bool isStationConnected();

    /**
     * @brief Gibt die aktuelle IP-Adresse zurück (entweder STA oder AP).
     * @return Die IP-Adresse als String.
     */
    String getIpAddress();

    /**
     * @brief Gibt die SSID des verbundenen Netzwerks zurück.
     * @return Die SSID als String oder "Nicht verbunden".
     */
    String getSsid();

    /**
     * @brief Gibt den aktuellen Betriebsmodus als Text zurück.
     * @return Der Modus als String (z.B. "Station + AP").
     */
    String getMode();

private:
    // Private Hilfsfunktionen, die nur innerhalb der Klasse verwendet werden.
    void startAP();
    void connectToWifi(const String& ssid, const String& password);
    void stopAP();

    // Member-Variablen zum Verwalten des AP-Timeouts.
    unsigned long _apStopTime;
    bool _apShouldBeDisabled;
};