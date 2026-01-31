//================================================================================
//| DATEI: main.cpp                                                              |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| Dies ist der zentrale Einstiegspunkt der Anwendung. Die Datei ist            |
//| verantwortlich für die Initialisierung aller Kernmodule und API-Handler in   |
//| der korrekten Reihenfolge (`setup`-Funktion) und das Ausführen der           |
//| periodischen Aufgaben in der Hauptschleife (`loop`-Funktion).                |
//================================================================================

#include <Arduino.h>
#include "config.h"
#include "modules/WiFi/WifiManager.h"
#include "modules/System/SystemAPI.h"
#include "modules/Server/WebServer.h"
#include "services/TimeService.h"
#include "modules/System/SystemApiHandler.h"
#include "modules/WiFi/WifiApiHandler.h" 
#include "modules/Server/OtaApiHandler.h"

// UNSER ROBOTER TREIBER (Header einbinden)
#include "BalanceDriver.h"

// Uni-Framework Objekte erstellen
WifiManager wifiManager;
TimeService timeService(wifiManager);
SystemAPI systemApi(wifiManager, timeService);
SystemApiHandler systemApiHandler(systemApi);
WifiApiHandler wifiApiHandler(wifiManager);
OtaApiHandler otaApiHandler;
WebServer WebServer(wifiManager, systemApiHandler, wifiApiHandler, otaApiHandler);

void setup() {
    Serial.begin(115200);
    Serial.println("\n--- ROBOTER START ---");
    
    // 1. Roboter Hardware starten (Balancer, Displays, MPU, Kalibrierung!)
    setupBalancer(); 

    // 2. Uni-Framework starten (WLAN, Webserver, etc.)
    wifiManager.setup(); 
    WebServer.setup();   
    timeService.setup();

    Serial.println("System bereit. Beginne Balance Loop.");
}

void loop() {
    // 1. Die Balance-Schleife muss zuerst und sehr oft laufen!
    runBalanceLoop(); 

    // 2. Uni-Framework Hintergrund-Aufgaben
    wifiManager.loop();  
    timeService.loop();
}