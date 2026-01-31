//================================================================================
//| DATEI: SystemAPI.h                                                           |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| Definiert die Schnittstelle der SystemAPI-Klasse. Diese Klasse ist die       |
//| zentrale Anlaufstelle für das Sammeln und Bereitstellen von                  |
//| System-Diagnosedaten wie Speicherverbrauch, Laufzeit, Neustartgründe und     |
//| Netzwerkstatus. Sie kapselt die Hardware-nahen Abfragen.                     |
//================================================================================

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h> // Wichtig: ArduinoJson muss in platformio.ini stehen
#include "../../modules/WiFi/WifiManager.h"
#include "../../services/TimeService.h"

class SystemAPI {
public:
    // Der Konstruktor benötigt eine Referenz zum WifiManager, um Netzwerkdaten abzurufen.
    SystemAPI(WifiManager& wifiManager, TimeService& timeService);

    // Generiert die JSON-Antwort für den /api/system/health Endpunkt.
    String getSystemHealthJson();

    // Gibt die gesammelten "Logs" zurück (in diesem Beispiel der Neustart-Grund).
    String getLogs();

    // Löscht die "Logs".
    void clearLogs();

private:
    WifiManager& _wifiManager; // Referenz zum WifiManager
    TimeService& _timeService; // Referenz zum TimeService
    // Hilfsfunktion, um den numerischen Neustart-Grund in einen lesbaren Text zu übersetzen.
    String getResetReasonText(int reasonCode);
};