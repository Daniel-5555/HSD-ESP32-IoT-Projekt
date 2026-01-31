# Lizenz für ESP32 Web Server Projekt

**Copyright (c) 2025 M.Sc. Christian Kitzel, Hochschule Düsseldorf**

Alle Rechte vorbehalten.

Diese Software und die zugehörigen Dokumentationsdateien (die "Software") werden unter den folgenden Bedingungen zur Verfügung gestellt:

### 1. Nicht-kommerzielle Nutzung

Hiermit wird jeder Person, die eine Kopie dieser Software erhält, die unentgeltliche Erlaubnis erteilt, die Software für **private, bildungsbezogene, akademische oder forschungsbezogene Zwecke** ohne Einschränkung zu nutzen, zu kopieren, zu ändern und weiterzugeben.

### 2. Bedingungen für die kommerzielle Nutzung

Eine **kommerzielle Nutzung** der Software ist grundsätzlich möglich, erfordert jedoch die **vorherige, ausdrückliche und schriftliche Genehmigung** des Urheberrechtsinhabers (M.Sc. Christian Kitzel).

**Zweck der Genehmigungspflicht:**
Der Zweck dieser Genehmigungspflicht ist nicht finanzieller Natur. Vielmehr dient sie dazu, eine **verpflichtende Rücksprache** sicherzustellen. Dies gibt dem Urheber die Möglichkeit, **Studierende und Nutzer beratend zu unterstützen**. Im Fokus stehen dabei die Aufklärung über **rechtliche Rahmenbedingungen** sowie die Vermeidung von **softwaretechnischen Fehlentscheidungen**, die bei einer Kommerzialisierung auftreten können. Dies dient dem Schutz und dem Lerneffekt des Nutzers.

Unter "kommerzieller Nutzung" fallen unter anderem, aber nicht ausschließlich:
*   Der Verkauf der Software oder eines Teils davon.
*   Die Integration der Software in ein Produkt oder eine Dienstleistung, die verkauft oder gegen Entgelt angeboten wird.
*   Die Nutzung der Software zur Erbringung von bezahlten Dienstleistungen (z.B. als Teil eines Beratungsauftrags).
*   Der Vertrieb der Software als Teil eines kommerziellen Angebots.

### 3. Bedingung für alle Nutzungen

Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.

### 4. Gewährleistungs- und Haftungsausschluss

Die Software wird unentgeltlich und im vorliegenden Zustand ("wie besehen") zur Verfügung gestellt.

Der Urheber übernimmt keinerlei Gewährleistung für die Funktionsfähigkeit, Fehlerfreiheit, Marktgängigkeit oder Eignung der Software für einen bestimmten Zweck.

Die Haftung des Urhebers für Schäden gleich welcher Art ist ausgeschlossen. Dieser Haftungsausschluss gilt nicht:
1.  für Schäden aus der Verletzung des Lebens, des Körpers oder der Gesundheit, die auf einer fahrlässigen Pflichtverletzung des Urhebers oder einer vorsätzlichen oder fahrlässigen Pflichtverletzung eines gesetzlichen Vertreters oder Erfüllungsgehilfen beruhen;
2.  für sonstige Schäden, die auf einer grob fahrlässigen Pflichtverletzung des Urhebers oder auf einer vorsätzlichen oder grob fahrlässigen Pflichtverletzung eines gesetzlichen Vertreters oder Erfüllungsgehilfen beruhen.


### 5. Nutzung von Drittanbieter-Bibliotheken und Frameworks

Dieses Projekt wurde speziell entwickelt, um Abhängigkeiten von restriktiven Lizenzen (wie der LGPL) im Bereich des Webservers zu vermeiden.

*   **Verzicht auf LGPL-Webserver:** Im Gegensatz zu vielen anderen ESP32-Projekten nutzt diese Software **nicht** die Bibliotheken `ESPAsyncWebServer` oder `AsyncTCP`.
*   **Nutzung der nativen ESP-IDF API:** Stattdessen implementiert dieses Projekt einen eigenen, hochperformanten Wrapper (`AsyncWebServer`-Klasse) um die native `esp_http_server` API des Herstellers Espressif.
*   **Apache License 2.0:** Das zugrundeliegende Espressif IoT Development Framework (ESP-IDF) ist unter der **Apache License 2.0** lizenziert.

**Bedeutung für die kommerzielle Nutzung:**
Durch den Verzicht auf LGPL-Komponenten entfällt für den Webserver-Teil die komplexe Verpflichtung, dem Endkunden das Austauschen von Bibliotheken ("Relinking") zu ermöglichen. Die Kombination aus dem proprietären Code dieses Projekts (vorbehaltlich Genehmigung gemäß Abschnitt 2) und der permissiven Apache 2.0 Lizenz des Frameworks erleichtert die Integration in geschlossene, kommerzielle Produkte erheblich.

Sollten Sie das Projekt um *andere*, hier nicht enthaltene Bibliotheken erweitern, prüfen Sie bitte deren jeweilige Lizenzbedingungen eigenständig.

---

**Um eine Genehmigung für die kommerzielle Nutzung des von M.Sc. Christian Kitzel erstellten Codes anzufordern, kontaktieren Sie bitte den Urheberrechtsinhaber unter:** [Christian.Kitzel@hs-duesseldorf.de]