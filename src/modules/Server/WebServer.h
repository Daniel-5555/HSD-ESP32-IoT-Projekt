//================================================================================
//| DATEI: WebServer.h                                                           |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| Definiert die zentrale Webserver-Klasse der Anwendung. Diese Klasse ist      |
//| verantwortlich für die Initialisierung des asynchronen Webservers, das       |
//| Routing von Anfragen an die spezialisierten API-Handler und das Ausliefern   |
//| von statischen Dateien (HTML, CSS etc.) aus dem SPIFFS-Dateisystem.          |
//================================================================================

#pragma once

#include "modules/Server/AsyncWebServer.h"
#include "../../modules/WiFi/WifiManager.h"
#include "../../modules/System/SystemApiHandler.h" // Einbinden des neuen System-Handlers
#include "../../modules/WiFi/WifiApiHandler.h"     // Einbinden des neuen WLAN-Handlers
#include "OtaApiHandler.h"

/**
 * @class WebServer
 * @brief Verwaltet das Routing von HTTP-Anfragen und die Auslieferung statischer Dateien.
 *
 * @purpose Diese Klasse ist jetzt nur noch für das Management des Webservers
 * zuständig. Sie kennt die API-Logik nicht mehr selbst, sondern erhält
 * spezialisierte Handler-Objekte, die sich um die API-Endpunkte kümmern.
 * Ihre Hauptaufgaben sind:
 * 1. Statische HTML/CSS-Dateien ausliefern.
 * 2. Die Handler anweisen, ihre API-Routen zu registrieren.
 * 3. Den OTA-Dienst (ElegantOTA) initialisieren.
 */
class WebServer {
public:
    /**
     * @brief Konstruktor, der die Abhängigkeiten entgegennimmt.
     * @param wifiManager Wird für den Platzhalter-Prozessor benötigt (%IP%, %SSID%).
     * @param systemApiHandler Der Handler für alle System-API-Routen.
     * @param wifiApiHandler Der Handler für alle WLAN-API-Routen.
     */
    WebServer(WifiManager& wifiManager, SystemApiHandler& systemApiHandler, WifiApiHandler& wifiApiHandler, OtaApiHandler& otaApiHandler);

    void setup();

private:
    // Verarbeitet Platzhalter wie %HOSTNAME% in HTML-Dateien.
    String processor(const String& var);

    AsyncWebServer _server;
    WifiManager& _wifiManager;
    
    // Referenzen auf die Handler-Klassen, die die API-Logik enthalten.
    SystemApiHandler& _systemApiHandler;
    WifiApiHandler& _wifiApiHandler;
    OtaApiHandler& _otaApiHandler;
};