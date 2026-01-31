//================================================================================
//| DATEI: SystemApiHandler.h                                                    |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| Definiert die Handler-Klasse für alle System-bezogenen API-Endpunkte. Diese  |
//| Klasse dient als Vermittler zwischen dem Webserver und der `SystemAPI`-Logik,|
//| um eine saubere Trennung der Verantwortlichkeiten zu gewährleisten.          |
//================================================================================

#pragma once

#include "modules/Server/AsyncWebServer.h" // Notwendig, da wir mit Web-Anfragen (Requests) arbeiten.
#include "SystemAPI.h"         // Wir benötigen Zugriff auf die SystemAPI, um die eigentlichen Daten abzurufen.

/**
 * @class SystemApiHandler
 * @brief Vermittler zwischen dem Webserver und der System-Logik.
 *
 * @purpose Diese Klasse bündelt alle HTTP-Endpunkte, die sich auf Systeminformationen
 * beziehen (z.B. /api/system/health, /api/system/reboot). Ihre einzige Aufgabe
 * ist es, Web-Anfragen entgegenzunehmen und die passenden Funktionen in der
 * `SystemAPI`-Klasse aufzurufen. Dies entkoppelt die Web-Schicht von der
 * eigentlichen Anwendungslogik.
 */
class SystemApiHandler {
public:
    /**
     * @brief Konstruktor, der die Abhängigkeiten injiziert.
     * @param systemApi Eine Referenz auf das `SystemAPI`-Objekt. Der Handler
     *                  besitzt die Logik nicht selbst, sondern delegiert die
     *                  Aufgaben an dieses Objekt. Dieses Muster nennt sich
     *                  "Dependency Injection".
     */
    SystemApiHandler(SystemAPI& systemApi);

    /**
     * @brief Registriert alle Routen, für die dieser Handler zuständig ist.
     * @param server Eine Referenz auf die globale Webserver-Instanz. Der Handler
     *               "meldet" seine Endpunkte beim Server an.
     */
    void registerRoutes(AsyncWebServer& server);

private:
    // Eine private Referenz auf das Logik-Modul.
    // 'const' ist hier nicht möglich, da z.B. `clearLogs` den Zustand ändert.
    SystemAPI& _systemApi;

    // Private Handler-Methoden für jeden einzelnen API-Endpunkt.
    // Diese Kapselung sorgt dafür, dass die Methoden nur innerhalb der Klasse
    // aufgerufen werden können und die `registerRoutes` Funktion übersichtlich bleibt.
    void handleGetHealth(AsyncWebServerRequest *request);
    void handleGetLogs(AsyncWebServerRequest *request);
    void handleClearLogs(AsyncWebServerRequest *request);
    void handleReboot(AsyncWebServerRequest *request);
};