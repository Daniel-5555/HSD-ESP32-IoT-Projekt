//================================================================================
//| DATEI: OtaApiHandler.h                                                       |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| Definiert die Schnittstelle für die OTA-Update-Funktionalität.               |
//| Diese Klasse kapselt die gesamte Logik für Firmware- und Dateisystem-Updates |
//| über das HTTP-Protokoll.                                                     |
//================================================================================

#pragma once

#include "AsyncWebServer.h"
#include <Update.h>

class OtaApiHandler {
public:
    /**
     * @brief Konstruktor.
     */
    OtaApiHandler();

    /**
     * @brief Registriert die notwendigen URL-Routen (/update, /update_spiffs) am Webserver.
     * @param server Referenz auf das Webserver-Objekt.
     */
    void registerRoutes(AsyncWebServer& server);

private:
    /**
     * @brief Interne Hilfsfunktion, die den eigentlichen Schreibvorgang in den Flash übernimmt.
     * 
     * @param request Der aktuelle HTTP-Request.
     * @param filename Name der Datei (wird bei Raw-Upload oft ignoriert, dient nur Logzwecken).
     * @param index Aktuelle Byte-Position im Upload-Stream.
     * @param data Zeiger auf die empfangenen Datenbytes.
     * @param len Anzahl der Bytes im aktuellen Paket.
     * @param final True, wenn dies das letzte Paket ist.
     * @param isSpiffs True für Dateisystem-Update, False für Firmware-Update.
     */
    void handleUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final, bool isSpiffs);
};