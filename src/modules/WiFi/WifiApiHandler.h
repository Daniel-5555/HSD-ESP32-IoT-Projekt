//================================================================================
//| DATEI: WifiApiHandler.h                                                      |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| Definiert die Handler-Klasse für alle WLAN-bezogenen API-Endpunkte. In       |
//| diesem Fall ist sie primär für das Entgegennehmen und Speichern von neuen    |
//| WLAN-Zugangsdaten zuständig, die über das Web-Interface übermittelt werden.  |
//================================================================================

#pragma once

#include "modules/Server/AsyncWebServer.h"
#include "WifiManager.h"

/**
 * @class WifiApiHandler
 * @brief Vermittler zwischen dem Webserver und der WLAN-Logik.
 *
 * @purpose Diese Klasse bündelt alle HTTP-Endpunkte, die sich auf die
 * WLAN-Konfiguration beziehen (in diesem Fall nur das Speichern der Zugangsdaten).
 * Sie folgt demselben Entkopplungsprinzip wie der `SystemApiHandler`.
 */
class WifiApiHandler {
public:
    /**
     * @brief Konstruktor.
     * @param wifiManager Eine Referenz auf das `WifiManager`-Objekt, das die
     *                    eigentliche Logik zum Verbinden und Konfigurieren enthält.
     */
    WifiApiHandler(WifiManager& wifiManager);

    /**
     * @brief Registriert die WLAN-spezifischen Routen am Webserver.
     * @param server Eine Referenz auf die globale Webserver-Instanz.
     */
    void registerRoutes(AsyncWebServer& server);

private:
    WifiManager& _wifiManager;

    // Private Handler-Methode für das Speichern der WLAN-Zugangsdaten.
    void handleSaveCredentials(AsyncWebServerRequest *request);
};