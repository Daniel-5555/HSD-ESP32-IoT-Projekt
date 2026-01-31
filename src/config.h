//================================================================================
//| DATEI: config.h                                                              |
//| AUTOR: M.Sc. Christian Kitzel, Hochschule Düsseldorf (HSD)                   |
//| LIZENZ: Proprietär - Siehe LICENSE.md für Details                            |
//|------------------------------------------------------------------------------|
//| ZWECK:                                                                       |
//| Enthält globale Konfigurationsparameter und Makros für das gesamte Projekt.  |
//| Hier werden zentrale Werte wie WLAN-AP-Daten, der Hostname des Geräts, die   |
//| Firmware-Version und Adressen für den Speicher an einem einzigen Ort         |
//| definiert, um die Konfiguration und Wartung zu vereinfachen.                 |
//================================================================================

#pragma once
#define AP_SSID "BalanceBot"
#define AP_PASSWORD "roboter123"
#define AP_TIMEOUT_MS 300000
#define firmware_version "1.0.0-BalanceBot-FINAL"
#define HOSTNAME "BalanceBot"
#define EEPROM_SIZE 128
#define SSID_ADDR 0
#define PASS_ADDR 64