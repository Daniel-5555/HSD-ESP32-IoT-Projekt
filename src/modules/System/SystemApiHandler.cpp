//================================================================================
//| DATEI: SystemApiHandler.cpp                                                  |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| Implementiert die Logik zum Verarbeiten von HTTP-Anfragen für die System-API.|
//| Jede Methode in dieser Klasse entspricht einem API-Endpunkt (z.B. für        |
//| Systemzustand, Logs oder Neustart) und ruft die entsprechende Funktion in    |
//| der `SystemAPI`-Klasse auf.                                                  |
//================================================================================

#include "SystemApiHandler.h"

/**
 * @brief Konstruktor-Implementierung.
 * Die Initialisierungsliste `: _systemApi(systemApi)` weist die übergebene
 * Referenz dem privaten Klassenmitglied `_systemApi` zu.
 */
SystemApiHandler::SystemApiHandler(SystemAPI& systemApi) : _systemApi(systemApi) {}

/**
 * @brief Implementierung der Routen-Registrierung.
 * Hier wird die Verbindung zwischen einer URL (z.B. "/api/system/health") und
 * einer Funktion, die diese Anfrage bearbeitet, hergestellt.
 */
void SystemApiHandler::registerRoutes(AsyncWebServer& server) {
    // server.on registriert einen neuen Endpunkt.
    // Der Lambda-Ausdruck `[this](...){...}` wurde durch `std::bind` ersetzt.
    // Grund: `std::bind` ist eine saubere Methode, um eine Member-Funktion (die ein 'this'-Objekt benötigt)
    // als Callback zu übergeben. `std::placeholders::_1` ist ein Platzhalter für das
    // `AsyncWebServerRequest*`-Objekt, das vom Server automatisch übergeben wird.
    server.on("/api/system/health", HTTP_GET, std::bind(&SystemApiHandler::handleGetHealth, this, std::placeholders::_1));
    server.on("/api/logs", HTTP_GET, std::bind(&SystemApiHandler::handleGetLogs, this, std::placeholders::_1));
    server.on("/api/logs/clear", HTTP_POST, std::bind(&SystemApiHandler::handleClearLogs, this, std::placeholders::_1));
    server.on("/api/system/reboot", HTTP_POST, std::bind(&SystemApiHandler::handleReboot, this, std::placeholders::_1));
}

/**
 * @brief Bearbeitet Anfragen für System-Gesundheitsdaten.
 * Ruft die entsprechende Methode der `_systemApi` auf und sendet deren
 * Rückgabewert (einen JSON-String) an den Client.
 */
void SystemApiHandler::handleGetHealth(AsyncWebServerRequest *request) {
    request->send(200, "application/json", _systemApi.getSystemHealthJson());
}

/**
 * @brief Bearbeitet Anfragen zum Abrufen der Logs.
 */
void SystemApiHandler::handleGetLogs(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", _systemApi.getLogs());
}

/**
 * @brief Bearbeitet Anfragen zum Löschen der Logs.
 */
void SystemApiHandler::handleClearLogs(AsyncWebServerRequest *request) {
    _systemApi.clearLogs(); // Zuerst die Aktion ausführen...
    request->send(200, "text/plain", "Logs gelöscht."); // ...dann den Erfolg bestätigen.
}

/**
 * @brief Bearbeitet Anfragen für einen Geräteneustart.
 */
void SystemApiHandler::handleReboot(AsyncWebServerRequest *request) {
    // Zuerst eine Antwort an den Client senden, damit die Anfrage abgeschlossen wird.
    request->send(200, "text/plain", "Neustart wird eingeleitet.");
    // Eine kurze Verzögerung gibt dem Browser Zeit, die Antwort zu empfangen,
    // bevor die Verbindung durch den Neustart abrupt getrennt wird.
    delay(1000);
    ESP.restart();
}