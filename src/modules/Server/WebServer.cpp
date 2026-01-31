//================================================================================
//| DATEI: WebServer.cpp                                                         |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| Implementiert die Logik der AppWebServer-Klasse. Die `setup()`-Methode       |
//| konfiguriert alle HTTP-Routen, indem sie die Registrierung der API-Endpunkte |
//| an die jeweiligen Handler-Klassen delegiert. Der `processor` kümmert sich um |
//| das dynamische Ersetzen von Platzhaltern in den HTML-Templates.              |
//================================================================================

#include "WebServer.h"
#include "../../config.h"
#include <SPIFFS.h>
// Wir brauchen den Balancer, um Befehle an ihn zu senden
#include "../../BalanceDriver.h" 

// Externe Funktionsdeklarationen aus BalanceDriver.h
extern void setRobotMovement(int moveX, int moveY);
extern void toggleMotors(bool enable);
extern void updatePidValues(float Kp, float Ki, float Kd);
extern void getCurrentRobotStatus(float& angle, float& error, float& gyroRate, int& motorSpeed, bool& motorsEnabled, float& currentKp, float& currentKi, float& currentKd);


// Konstruktor
WebServer::WebServer(WifiManager& wifiManager, SystemApiHandler& systemApiHandler, WifiApiHandler& wifiApiHandler, OtaApiHandler& otaApiHandler)
    : _server(80), 
      _wifiManager(wifiManager), 
      _systemApiHandler(systemApiHandler), 
      _wifiApiHandler(wifiApiHandler),
      _otaApiHandler(otaApiHandler) {}

// Setup
void WebServer::setup() {
    _server.begin(); 

    // 1. Standard API-Routen registrieren (vom Uni-Framework)
    _systemApiHandler.registerRoutes(_server);
    _wifiApiHandler.registerRoutes(_server);
    _otaApiHandler.registerRoutes(_server);

    if (SPIFFS.begin(true)) {
        
        // 2. SPEZIAL-ROUTE: wifi.html (Dein Fix)
        _server.on("/wifi.html", HTTP_GET, [this](AsyncWebServerRequest *request){
            if(SPIFFS.exists("/wifi.html")){
                File file = SPIFFS.open("/wifi.html", "r");
                String content = file.readString();
                file.close();

                content.replace("%HOSTNAME%", HOSTNAME); 
                content.replace("%MODE%", _wifiManager.getMode());
                content.replace("%IP%", _wifiManager.getIpAddress());
                content.replace("%SSID%", _wifiManager.getSsid());
                
                request->send(200, "text/html", content);
            } else {
                request->send(404, "text/plain", "wifi.html nicht gefunden");
            }
        });

        // =========================================================
        // NEU: API-ENDPUNKTE FÜR ROBOTER-STEUERUNG
        // =========================================================

        // API zum Senden von Bewegungsbefehlen
        _server.on("/api/robot/move", HTTP_POST, [](AsyncWebServerRequest *request){
            int moveX = 0; 
            int moveY = 0; 
            String command = "";

            // KORRIGIERT: Robuste Methode mit request->arg() und String-Check
            if(request->arg("x").length() > 0) moveX = request->arg("x").toInt(); 
            if(request->arg("y").length() > 0) moveY = request->arg("y").toInt();
            if(request->arg("cmd").length() > 0) command = request->arg("cmd");

            Serial.print("Web-Befehl: X="); Serial.print(moveX); Serial.print(", Y="); Serial.print(moveY); Serial.print(", CMD="); Serial.println(command);

            if (command == "enable") {
                toggleMotors(true);
            } else if (command == "disable") {
                toggleMotors(false);
            } else {
                setRobotMovement(moveX, moveY);
            }

            request->send(200, "text/plain", "OK");
        });

        // API zum Abrufen des Roboter-Status (für Dashboard-Anzeige)
        _server.on("/api/robot/status", HTTP_GET, [](AsyncWebServerRequest *request){
            float angle, error, gyroRate;
            int motorSpeed;
            bool enabled;
            float currentKp, currentKi, currentKd; 

            getCurrentRobotStatus(angle, error, gyroRate, motorSpeed, enabled, currentKp, currentKi, currentKd);

            String jsonResponse = "{";
            jsonResponse += "\"angle\": " + String(angle, 2) + ",";
            jsonResponse += "\"error\": " + String(error, 2) + ",";
            jsonResponse += "\"gyro\": " + String(gyroRate, 2) + ",";
            jsonResponse += "\"motor\": " + String(motorSpeed) + ",";
            jsonResponse += "\"enabled\": " + String(enabled ? "true" : "false") + ","; 
            jsonResponse += "\"kp\": " + String(currentKp, 2) + ","; 
            jsonResponse += "\"ki\": " + String(currentKi, 3) + ","; 
            jsonResponse += "\"kd\": " + String(currentKd, 2); 
            jsonResponse += "}";

            request->send(200, "application/json", jsonResponse);
        });

        // API zum Ändern der PID-Werte über das Web-Interface
        _server.on("/api/robot/pid", HTTP_POST, [](AsyncWebServerRequest *request){
            float Kp_new = Kp; 
            float Ki_new = Ki;
            float Kd_new = Kd;

            // KORRIGIERT: Robuste Methode mit request->arg() und String-Check
            if(request->arg("kp").length() > 0) Kp_new = request->arg("kp").toFloat();
            if(request->arg("ki").length() > 0) Ki_new = request->arg("ki").toFloat();
            if(request->arg("kd").length() > 0) Kd_new = request->arg("kd").toFloat();
            
            updatePidValues(Kp_new, Ki_new, Kd_new);

            request->send(200, "text/plain", "PID Updated");
        });

        // =========================================================

        // 3. STATISCHE ROUTEN (Dateien aus /data Ordner)
        _server.serveStatic("/", SPIFFS, "/");
    }

    _server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Seite nicht gefunden.");
    });

    Serial.println("HTTP-Server konfiguriert.");
}

String WebServer::processor(const String& var) {
    if (var == "HOSTNAME") return HOSTNAME;
    if (var == "MODE") return _wifiManager.getMode();
    if (var == "IP") return _wifiManager.getIpAddress();
    if (var == "SSID") return _wifiManager.getSsid();
    return String();
}