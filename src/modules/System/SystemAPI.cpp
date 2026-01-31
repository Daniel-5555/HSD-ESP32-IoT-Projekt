//================================================================================
//| DATEI: SystemAPI.cpp                                                         |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| Implementiert die Logik zum Sammeln von System-Diagnosedaten. Hier werden    |
//| ESP32-spezifische Funktionen aufgerufen, um Informationen über den freien    |
//| Speicher (Heap), die Systemlaufzeit, den letzten Neustartgrund und weitere   |
//| relevante Metriken zu ermitteln und diese in einem JSON-Format aufzubereiten.|
//================================================================================

#include "SystemAPI.h"
#include <esp_task_wdt.h> // Für den Watchdog Timer Reset Grund
#include <esp_system.h>   // Für esp_get_free_heap_size etc.
#include <WiFi.h>         // Für WiFi.macAddress() etc.
#include "../../config.h"
#include "../../services/TimeService.h"
    
// Definiert, wie oft der Neustart-Grund im RTC-Speicher gespeichert wird, um eine Historie zu haben.
// RTC-Speicher überlebt einen Neustart (aber keinen Stromausfall).
#define MAX_LOG_ENTRIES 5
// `RTC_NOINIT_ATTR` sorgt dafür, dass die Variable bei einem Neustart nicht auf 0 gesetzt wird.
RTC_NOINIT_ATTR char log_entries[MAX_LOG_ENTRIES][128];
RTC_NOINIT_ATTR int log_count = 0;


SystemAPI::SystemAPI(WifiManager& wifiManager, TimeService& timeService) : _wifiManager(wifiManager), _timeService(timeService) {
    // Beim allerersten Start (nach dem Flashen) initialisieren wir den Log-Zähler.
    if (esp_reset_reason() == ESP_RST_POWERON) {
        log_count = 0;
    }

    // Füge den aktuellen Neustart-Grund zu den Logs hinzu, wenn Platz ist.
    if (log_count < MAX_LOG_ENTRIES) {
        String reason = "Neustart-Grund: " + getResetReasonText(esp_reset_reason());
        snprintf(log_entries[log_count], sizeof(log_entries[log_count]), "%s", reason.c_str());
        log_count++;
    }
}

/**
 * @brief Sammelt alle System-Gesundheitsdaten und formatiert sie als JSON-String.
 * 
 * Diese Funktion ist der Kern der Diagnose-API. Sie liest Hardware- und Software-Zustände
 * aus und stellt sie strukturiert für das Frontend bereit.
 * 
 * @return Ein String, der das JSON-Objekt mit allen Gesundheitsdaten enthält.
 */
String SystemAPI::getSystemHealthJson() {
    // Wir verwenden ArduinoJson, um das JSON-Objekt sicher und effizient zu erstellen.
    // Die Größe (1024 Bytes) ist großzügig bemessen, um alle Daten aufzunehmen.
    StaticJsonDocument<1024> doc;

    // --- Stabilitäts-Daten ---
    int reasonCode = esp_reset_reason();
    doc["reset_reason_code"] = reasonCode;
    doc["reset_reason_text"] = getResetReasonText(reasonCode);
    doc["uptime_seconds"] = esp_timer_get_time() / 1000000;

    // --- Heap-Speicher (RAM) Daten ---
    doc["heap_total"] = ESP.getHeapSize();
    doc["heap_free"] = ESP.getFreeHeap();
    doc["heap_min_free"] = ESP.getMinFreeHeap(); // Wichtigster Wert zur Speicher-Analyse!

    // --- Netzwerk-Daten ---
    doc["wifi_ssid"] = _wifiManager.getSsid();
    doc["wifi_rssi"] = _wifiManager.isStationConnected() ? WiFi.RSSI() : 0;
    doc["ip_address"] = _wifiManager.getIpAddress();
    doc["last_time_sync"] = 0; // Platzhalter, für eine echte Implementierung mit NTP-Client

    // --- System-Identifikations-Daten ---
    doc["firmware_version"] = firmware_version;
    doc["mac_address"] = WiFi.macAddress();
    #ifdef CONFIG_IDF_TARGET_ESP32
        // Die CPU-Temperatur ist nur auf einigen ESP32-Chips verfügbar.
        doc["cpu_temp"] = temperatureRead();
    #else
        doc["cpu_temp"] = 0;
    #endif

    // --- System-Zeit ---
    doc["last_time_sync"] = _timeService.getLastSyncTimestamp();

    // Das erstellte JSON-Objekt in einen String umwandeln.
    String output;
    serializeJson(doc, output);
    return output;
}

/**
 * @brief Gibt die gespeicherten Neustart-Gründe als einfachen Text zurück.
 */
String SystemAPI::getLogs() {
    if (log_count == 0) {
        return "Keine Logs vorhanden.";
    }
    String log_string = "";
    for (int i = 0; i < log_count; i++) {
        log_string += String(i + 1) + ": " + log_entries[i] + "\n";
    }
    return log_string;
}

/**
 * @brief Löscht die im RTC-Speicher gehaltenen Logs.
 */
void SystemAPI::clearLogs() {
    log_count = 0;
}


/**
 * @brief Übersetzt den numerischen ESP32-Reset-Grund in einen für Menschen lesbaren Text.
 * @param reasonCode Der numerische Code von `esp_reset_reason()`.
 * @return Ein String mit der Beschreibung des Grundes.
 */
String SystemAPI::getResetReasonText(int reasonCode) {
    switch (reasonCode) {
        case 1  : return "Power on"; // Normaler Start nach Einschalten
        case 3  : return "Software reset via ESP.restart()"; // Kontrollierter Neustart
        case 4  : return "Legacy Watchdog reset"; // Software-Problem (Schleife)
        case 5  : return "Deep Sleep wakeup";
        case 6  : return "Reset by external pin (EN)";
        case 7  : return "Timer Group 0 Watchdog"; // Software-Problem (blockiert)
        case 8  : return "Timer Group 1 Watchdog"; // Software-Problem (blockiert)
        case 9  : return "RTC Watchdog";
        case 10 : return "Brownout reset (voltage dip)"; // Stromversorgungs-Problem
        case 11 : return "RTC Watchdog";
        case 12 : return "CPU0 Panic"; // Schwerer Software-Fehler
        case 13 : return "CPU1 Panic"; // Schwerer Software-Fehler
        default : return "Unknown";
    }
}