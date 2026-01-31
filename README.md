

# ESP32 High-Performance WebServer Framework

**Hochschule Düsseldorf (HSD)**  
**Autor:** M.Sc. Christian Kitzel  
**Version:** 1.0.0

Dieses Projekt beinhaltet ein leistungsstarkes Webserver-Framework für den ESP32. Es basiert auf einem eigens entwickelten C++ Wrapper um die native Espressif API (`esp_http_server`), um Abhängigkeiten zu LGPL-Bibliotheken zu vermeiden und maximale Performance zu gewährleisten.

---

## 📋 Inhaltsverzeichnis

- [ESP32 High-Performance WebServer Framework](#esp32-high-performance-webserver-framework)
  - [📋 Inhaltsverzeichnis](#-inhaltsverzeichnis)
  - [🛠 Voraussetzungen \& Installation](#-voraussetzungen--installation)
  - [⚠️ WICHTIG: Das Flashing-Konzept (Zwei Schritte)](#️-wichtig-das-flashing-konzept-zwei-schritte)
  - [⚡ Anleitung: Flashen des Geräts](#-anleitung-flashen-des-geräts)
    - [Schritt A: Firmware (C++ Code) hochladen](#schritt-a-firmware-c-code-hochladen)
    - [Schritt B: Dateisystem (SPIFFS) hochladen](#schritt-b-dateisystem-spiffs-hochladen)
      - [Methode 1: Über `upload.bat` (Skript)](#methode-1-über-uploadbat-skript)
      - [Methode 2: Über PlatformIO IDE](#methode-2-über-platformio-ide)
  - [🌐 Verbindung \& Erste Schritte](#-verbindung--erste-schritte)
    - [1. Mit dem Access Point verbinden](#1-mit-dem-access-point-verbinden)
    - [2. Web-Oberfläche aufrufen](#2-web-oberfläche-aufrufen)
    - [3. WLAN Konfigurieren](#3-wlan-konfigurieren)
    - [4. OTA Updates (Over-The-Air)](#4-ota-updates-over-the-air)
  - [📂 Projektstruktur](#-projektstruktur)
  - [⚖️ Lizenz](#️-lizenz)

---

## 🛠 Voraussetzungen & Installation

Damit Sie das Projekt kompilieren und auf den ESP32 übertragen können, muss folgende Software installiert sein:

1.  **Visual Studio Code (VS Code):** [Download hier](https://code.visualstudio.com/)
2.  **PlatformIO Extension:**
    *   Öffnen Sie VS Code.
    *   Gehen Sie links auf das Erweiterungs-Symbol (Vierecke).
    *   Suchen Sie nach "PlatformIO IDE" und installieren Sie es.
3.  **USB-Treiber:**
    *   Je nach ESP32-Board benötigen Sie Treiber für den **CP210x** oder **CH340** Chip. Meistens installiert Windows diese automatisch.

---

## ⚠️ WICHTIG: Das Flashing-Konzept (Zwei Schritte)

Ein häufiger Fehler ist, dass nur der Programmcode hochgeladen wird. Der ESP32 benötigt jedoch **zwei** verschiedene Uploads, damit der Webserver funktioniert:

1.  **Die Firmware:** Das ist der kompilierte C++ Code (Logik, WLAN-Steuerung, Server-Wrapper).
2.  **Das Dateisystem (SPIFFS):** Das sind die Dateien aus dem Ordner `data/` (HTML, CSS, JavaScript).

**Wenn Sie Schritt 2 vergessen, wird der Webserver "File not found" anzeigen!**

---

## ⚡ Anleitung: Flashen des Geräts

Verbinden Sie Ihren ESP32 per USB-Kabel mit dem Computer.

### Schritt A: Firmware (C++ Code) hochladen

Dies überträgt die eigentliche Programmlogik.

1.  Öffnen Sie das Projekt in VS Code.
2.  Klicken Sie in der blauen Leiste ganz unten auf den **Pfeil nach rechts (→)** ("PlatformIO: Upload").
3.  Warten Sie, bis im Terminal `[SUCCESS]` steht.

### Schritt B: Dateisystem (SPIFFS) hochladen

Hierfür gibt es zwei Methoden. Wählen Sie die, die Ihnen lieber ist.

#### Methode 1: Über `upload.bat` (Skript)

Im Hauptverzeichnis liegt eine Datei namens `upload.bat`. Diese führt folgende Schritte aus:
Öffnen Sie in VSCode das Terminalm geben Sie `./upload.bat` ein und drücken Sie Enter.

#### Methode 2: Über PlatformIO IDE

Dies ist der Weg direkt über die Benutzeroberfläche von VS Code:

1.  Klicken Sie in der linken Seitenleiste auf das **Alien-Kopf-Symbol** (PlatformIO).
2.  Im Menü "Project Tasks" öffnen Sie den Ordner Ihres Boards (z.B. `esp32dev` oder `Default`).
3.  Öffnen Sie den Unterordner **Platform**.
4.  Klicken Sie auf **Upload Filesystem Image**.
5.  Warten Sie auf die Meldung `[SUCCESS]` im Terminal.

---

## 🌐 Verbindung & Erste Schritte

Nachdem **beide** Uploads (Firmware + Filesystem) erfolgreich waren, startet der ESP32 neu.

### 1. Mit dem Access Point verbinden
Da der ESP32 noch nicht mit Ihrem WLAN verbunden ist, eröffnet er ein eigenes Netzwerk (Access Point Modus).

*   **WLAN-Name (SSID):** `ESP32-Konfiguration`
*   **Passwort:** `password123`

Verbinden Sie Ihr Smartphone oder Ihren Laptop mit diesem WLAN.

### 2. Web-Oberfläche aufrufen
Öffnen Sie einen Webbrowser (Chrome, Firefox, Safari, Edge) und geben Sie folgende Adresse ein:

👉 **http://192.168.4.1**

### 3. WLAN Konfigurieren
1.  Sie sehen nun das Dashboard.
2.  Klicken Sie auf den Link zur **WLAN Konfiguration** (oder navigieren Sie zu `/wifi.html`).
3.  Geben Sie den Namen (SSID) und das Passwort Ihres Heim-WLANs ein.
4.  Klicken Sie auf "Speichern".
5.  Der ESP32 startet neu und verbindet sich nun automatisch mit Ihrem Router.

### 4. OTA Updates (Over-The-Air)
Für zukünftige Updates müssen Sie das Gerät nicht mehr per USB anschließen.
1.  Rufen Sie die neue IP-Adresse des ESP32 in Ihrem Heimnetzwerk auf.
2.  Gehen Sie auf die Seite **System Update**.
3.  Dort können Sie neue `firmware.bin` (Code) oder `spiffs.bin` (Dateisystem) Dateien hochladen.

---

## 📂 Projektstruktur

Wo finde ich was?

*   `src/`: Hier liegt der C++ Quellcode (`main.cpp`, `WebServer.cpp`, etc.).
*   `data/`: **Hier liegt die Webseite!** Wenn Sie HTML oder CSS ändern wollen, müssen Sie die Dateien hier bearbeiten und danach das **Dateisystem neu flashen** (siehe Schritt B).
*   `include/`: Header-Dateien und `config.h` (Einstellungen).
*   `upload.bat`: Skript zum automatischen Hochladen des Dateisystems.

---

## ⚖️ Lizenz

**Copyright (c) 2025 M.Sc. Christian Kitzel**

Dieses Projekt ist für **private und akademische Zwecke** frei nutzbar.
Für eine **kommerzielle Nutzung** ist eine schriftliche Genehmigung des Autors erforderlich.

Details entnehmen Sie bitte der Datei `LICENSE.md`.

***
