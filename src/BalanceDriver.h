#ifndef BALANCE_DRIVER_H_
#define BALANCE_DRIVER_H_

#include <Arduino.h>
#include <Wire.h>             
#include <Adafruit_SSD1306.h> 
#include <Adafruit_GFX.h>     
#include <cmath>              

// --- GLOBALE VARIABLEN DEFINITIONEN (WICHTIG: Hier definiert, nicht extern!) ---
byte MPU_ADDR = 0x68; 
float Kp = 3.0;    
float Ki = 0.005;  
float Kd = 0.3;    
int16_t accelXOffset = 0; 
int16_t gyroYOffset = 0;  
float targetAngle = -2150.0; // <-- DEIN LETZTER STABILER WERT!
float deadzone = 50.0; 

// --- TIMING & LIMITS ---
#define MIN_MOTOR_SPEED 80   
#define MAX_MOTOR_SPEED 255  
#define EMERGENCY_ANGLE 30.0 
#define BALANCE_LOOP_TIME_MS 10  
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// --- PID VARIABLEN ---
unsigned long lastBalanceTime = 0;
float lastError = 0;
float errorSum = 0;
float filteredAngle = 0;     
float gyroAngleRate = 0;     

// --- STATUS FLAGS ---
bool mpuInitialized = false;        
bool displayLinksInitialized = false; 
bool displayRechtsInitialized = false; 
bool motorsEnabled = true;          

// --- BEWEGUNGSBEFEHLE VON WEB ---
int webMoveX = 0; 
int webMoveY = 0; 

// --- MOTOR PINS (L298N Verkabelung) ---
#define ENA 14
#define IN1 27
#define IN2 26
#define IN3 25
#define IN4 13
#define ENB 12

// --- GLOBALE OBJEKTE ---
Adafruit_SSD1306 displayLinks(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
TwoWire I2C_Rechts = TwoWire(1); 
Adafruit_SSD1306 displayRechts(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_Rechts, -1);


// ====================================================================
// FUNKTIONEN IMPLEMENTIERUNG
// ====================================================================

// --- NEUE FUNKTION: I2C BUS & MPU initialisieren (robust) ---
bool initializeI2cAndMpu() {
    Serial.println("\n--- I2C & MPU ROBUSTER START ---");
    
    Wire.begin(21, 22);            
    Wire.setClock(400000);         
    I2C_Rechts.begin(32, 33);      

    Serial.print("Display Links... ");
    if(displayLinks.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        displayLinksInitialized = true;
        displayLinks.setRotation(2); 
        displayLinks.clearDisplay(); displayLinks.display();
        displayLinks.setTextSize(1); displayLinks.setTextColor(WHITE); displayLinks.setCursor(0,0);
        displayLinks.println("Booting..."); displayLinks.display();
        Serial.println("OK");
    } else { Serial.println("FEHLER!"); }
    
    Serial.print("Display Rechts... ");
    if(displayRechts.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        displayRechtsInitialized = true;
        displayRechts.setRotation(2); 
        displayRechts.clearDisplay(); displayRechts.display();
        displayRechts.setTextSize(1); displayRechts.setTextColor(WHITE); displayRechts.setCursor(0,0);
        displayRechts.println("Loading..."); displayRechts.display();
        Serial.println("OK");
    } else { Serial.println("FEHLER!"); }

    // MPU6050 initialisieren - mit Auto-Erkennung für 0x68/0x69
    Serial.println("\n--- MPU6050 Diagnose ---");
    delay(100); 
    
    byte activeAddr = 0x68; 
    Wire.beginTransmission(activeAddr);
    Wire.write(0x6B); Wire.write(0);
    byte error = Wire.endTransmission(true);
    
    if (error != 0) { 
        activeAddr = 0x69;
        Wire.beginTransmission(activeAddr);
        Wire.write(0x6B); Wire.write(0);
        error = Wire.endTransmission(true);
    }
    
    if (error == 0) {
        Serial.print("✓ MPU6050 gefunden auf Adresse 0x"); Serial.println(activeAddr, HEX);
        MPU_ADDR = activeAddr; 
        
        Wire.beginTransmission(MPU_ADDR); Wire.write(0x1C); Wire.write(0x00); Wire.endTransmission(true); 
        Wire.beginTransmission(MPU_ADDR); Wire.write(0x1B); Wire.write(0x00); Wire.endTransmission(true); 
        Wire.beginTransmission(MPU_ADDR); Wire.write(0x1A); Wire.write(0x03); Wire.endTransmission(true); 
        
        mpuInitialized = true;
        delay(100);
        calibrateMPU();  
        
        if(displayLinksInitialized) { displayLinks.clearDisplay(); displayLinks.setCursor(0,0); displayLinks.println("MPU OK!"); displayLinks.display(); }
        return true; 
    } else {
        Serial.print("✗ FEHLER: MPU6050 nicht gefunden. Code: "); Serial.println(error);
        mpuInitialized = false;
        if(displayLinksInitialized) { displayLinks.clearDisplay(); displayLinks.setCursor(0,0); displayLinks.println("MPU FAIL!"); displayLinks.display(); }
        return false; 
    }
}


// Kalibriert den MPU6050 Sensor beim Start
void calibrateMPU() {
    Serial.println("\n!!! KALIBRIERUNG STARTET !!!");
    Serial.println("Roboter JETZT GERADE hinstellen und NICHT bewegen!");
    if(displayLinksInitialized) { displayLinks.clearDisplay(); displayLinks.setCursor(0,0); displayLinks.println("Calibrating..."); displayLinks.display(); }
    if(displayRechtsInitialized) { displayRechts.clearDisplay(); displayRechts.setCursor(0,0); displayRechts.println("Hold Still!"); displayRechts.display(); } 
    
    // Countdown
    for(int i = 3; i > 0; i--) {
        Serial.print(i); Serial.print("...");
        if(displayLinksInitialized) { displayLinks.setCursor(0,10); displayLinks.println(String(i)+"..."); displayLinks.display(); }
        delay(1000);
    }
    Serial.println("GO!");
    
    long accelXSum = 0;
    long gyroYSum = 0; 
    const int samples = 200; 
    
    for(int i = 0; i < samples; i++) {
        Wire.beginTransmission(MPU_ADDR);
        Wire.write(0x3B); 
        Wire.endTransmission(false);
        Wire.requestFrom((uint8_t)MPU_ADDR, (size_t)14, (bool)true); 
        
        int16_t ax = (Wire.read() << 8 | Wire.read());
        Wire.read(); Wire.read(); // AY
        int16_t az = (Wire.read() << 8 | Wire.read()); // AZ lesen für atan2
        Wire.read(); Wire.read(); // Temp
        Wire.read(); Wire.read(); // GX
        int16_t gy = (Wire.read() << 8 | Wire.read()); // Gyro Y-Achse
        
        accelXSum += ax;
        gyroYSum += gy; 
        delay(5);
    }
    
    accelXOffset = accelXSum / samples; // Durchschnittlicher RAW X-Wert in gerader Position
    gyroYOffset = gyroYSum / samples;   // Durchschnittlicher RAW Gyro Y-Wert in Ruhe
    
    Serial.print("Accel Offset X: "); Serial.println(accelXOffset);
    Serial.print("Gyro Offset Y: "); Serial.println(gyroYOffset);
    
    targetAngle = 0.0; // Nach Kalibrierung ist 0 der Sollwert im gefilterten Winkel
    filteredAngle = 0; // Filter auch auf 0 setzen
    
    Serial.println("Kalibrierung abgeschlossen!\n");
    if(displayLinksInitialized) { displayLinks.clearDisplay(); displayLinks.setCursor(0,0); displayLinks.println("CALIBRATED!"); displayLinks.display(); }
    if(displayRechtsInitialized) { displayRechts.clearDisplay(); displayRechts.setCursor(0,0); displayRechts.println("READY!"); displayRechts.display(); }
    delay(1000);
}

// Initialisiert alle Balancer-Hardware
void setupBalancer() {
    Serial.println("\n=== BALANCE ROBOTER INITIALISIERUNG ===");
    
    // 1. Motor Pins konfigurieren
    pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
    pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
    setMotorSpeed(0, 0); // Motoren beim Start erstmal AUS!
    
    // 2. I2C und MPU initialisieren
    if(initializeI2cAndMpu()) { 
        // Wenn MPU gefunden, dann kalibrieren
        calibrateMPU();  // WICHTIG: Kalibrierung der Offsets!
    }
    
    delay(2000); // Lange Pause am Ende der Initialisierung
    if (displayLinksInitialized) displayLinks.clearDisplay(); displayLinks.display();
    if (displayRechtsInitialized) displayRechts.clearDisplay(); displayRechts.display();
}

// Zeichnet die Augen auf den Displays
void drawEyes(float currentFilteredAngle) { 
    if(!displayLinksInitialized && !displayRechtsInitialized) return;
    
    static unsigned long lastDraw = 0;
    if (millis() - lastDraw < 100) { return; } 
    lastDraw = millis();
    
    // Pupillenposition basierend auf gefiltertem Winkel
    int lookY = constrain((int)(currentFilteredAngle * 5.0), -15, 15); 
    
    if (displayLinksInitialized) {
        displayLinks.clearDisplay();
        displayLinks.fillCircle(64, 32, 28, WHITE); // Augapfel
        displayLinks.fillCircle(64, 32 + lookY, 12, BLACK); // Pupille
        displayLinks.display();
    }
    
    if (displayRechtsInitialized) {
        displayRechts.clearDisplay();
        displayRechts.fillCircle(64, 32, 28, WHITE); // Augapfel
        displayRechts.fillCircle(64, 32 + lookY, 12, BLACK);
        displayRechts.display();
    }
}

// Steuert die Motorgeschwindigkeit für beide Motoren
void setMotorSpeed(int speedLeft, int speedRight) {
    if (!motorsEnabled) { 
        speedLeft = 0; 
        speedRight = 0;
    }
    
    if (abs(speedLeft) < 10) { speedLeft = 0; } 
    if (abs(speedRight) < 10) { speedRight = 0; }

    if (speedLeft > 0 && speedLeft < MIN_MOTOR_SPEED) { speedLeft = MIN_MOTOR_SPEED; }
    if (speedLeft < 0 && speedLeft > -MIN_MOTOR_SPEED) { speedLeft = -MIN_MOTOR_SPEED; }
    if (speedRight > 0 && speedRight < MIN_MOTOR_SPEED) { speedRight = MIN_MOTOR_SPEED; }
    if (speedRight < 0 && speedRight > -MIN_MOTOR_SPEED) { speedRight = -MIN_MOTOR_SPEED; }
    
    speedLeft = constrain(speedLeft, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);
    speedRight = constrain(speedRight, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);
    
    // Motorrichtung linken Motor
    if (speedLeft > 0) { 
        digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    } else if (speedLeft < 0) { 
        digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    } else { 
        digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    }
    analogWrite(ENA, abs(speedLeft));

    // Motorrichtung rechten Motor
    if (speedRight > 0) { 
        digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    } else if (speedRight < 0) { 
        digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
    } else { 
        digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    }
    analogWrite(ENB, abs(speedRight));
}

// Steuert die Bewegung des Roboters über Web (X = Vor/Zurück, Y = Drehen)
void setRobotMovement(int moveX, int moveY) {
    webMoveX = constrain(moveX, -100, 100);
    webMoveY = constrain(moveY, -100, 100);
    Serial.print("Web Movement: X="); Serial.print(webMoveX); Serial.print(", Y="); Serial.print(webMoveY); Serial.println(" (Set)");
}

// Aktiviert/Deaktiviert die Motoren
void toggleMotors(bool enable) {
    motorsEnabled = enable;
    if (enable) Serial.println("Motoren AKTIVIERT!");
    else {
        Serial.println("Motoren DEAKTIVIERT!");
        setMotorSpeed(0, 0); // Sofort stoppen
    }
}

// Setzt neue PID-Werte von der Webseite
void updatePidValues(float Kp_new, float Ki_new, float Kd_new) {
    Kp = Kp_new;
    Ki = Ki_new;
    Kd = Kd_new;
    Serial.print("PID Updated: Kp="); Serial.print(Kp); Serial.print(", Ki="); Serial.print(Ki); Serial.print(", Kd="); Serial.println(Kd);
}

// Gibt den aktuellen Status des Roboters zurück
void getCurrentRobotStatus(float& angle, float& error, float& gyroRate, int& motorSpeed, bool& enabled, float& currentKp, float& currentKi, float& currentKd) {
    angle = filteredAngle;
    error = filteredAngle - targetAngle; 
    gyroRate = gyroAngleRate;
    
    int avgSpeed = (abs(webMoveX) + abs(webMoveY)) / 2;
    if (webMoveX > 0) motorSpeed = avgSpeed;
    else if (webMoveX < 0) motorSpeed = -avgSpeed;
    else if (webMoveY > 0) motorSpeed = avgSpeed; 
    else if (webMoveY < 0) motorSpeed = -avgSpeed;
    else motorSpeed = 0;


    enabled = motorsEnabled;
    currentKp = Kp;
    currentKi = Ki;
    currentKd = Kd;
}


// Haupt-Balancier-Logik
void runBalanceLoop() {
    if (!mpuInitialized) { 
        setMotorSpeed(0, 0);
        return;
    }

    unsigned long now = millis();
    if (now - lastBalanceTime < BALANCE_LOOP_TIME_MS) { return; } 
    float dt = (now - lastBalanceTime) / 1000.0; 
    lastBalanceTime = now;
    
    // === SENSOR DATEN LESEN ===
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);  
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)MPU_ADDR, (size_t)14, (bool)true);  
    
    if (Wire.available() < 14) {
        return;
    }
    
    // Rohdaten lesen und Offset korrigieren
    int16_t rawAccelX = (Wire.read() << 8 | Wire.read()) - accelXOffset;
    Wire.read(); Wire.read();  // AY (ignorieren)
    int16_t rawAccelZ = (Wire.read() << 8 | Wire.read()); // AZ lesen für atan2
    Wire.read(); Wire.read();  // Temp (ignorieren)
    Wire.read(); Wire.read();  // GX (ignorieren)
    
    int16_t rawGyroY = (Wire.read() << 8 | Wire.read()) - gyroYOffset; // Gyro Y-Achse
    
    // === COMPLEMENTARY FILTER === (Sensor Fusion!)
    // Winkel aus Accelerometer berechnen (In Grad)
    float accelAngle = atan2(rawAccelX, rawAccelZ) * 180.0 / PI;
    
    // Winkelgeschwindigkeit aus Gyro 
    gyroAngleRate = rawGyroY / 131.0; 
    
    // Gefilterten Winkel berechnen
    filteredAngle = FILTER_ALPHA * (filteredAngle + gyroAngleRate * dt) + 
                    (1.0 - FILTER_ALPHA) * accelAngle;
    
    // === PID REGELUNG ===
    float error = filteredAngle - targetAngle;  // Fehler = Aktueller Winkel - Sollwinkel
    
    // Debug Ausgabe (alle 200ms)
    static unsigned long printTimer = 0;
    if (now - printTimer > 200) {
        printTimer = now;
        Serial.print("Ang: "); Serial.print(filteredAngle, 1);
        Serial.print(" | Err: "); Serial.print(error, 1);
        Serial.print(" | Gyro: "); Serial.println(gyroAngleRate, 2);
    }
    
    // Not-Aus bei zu starker Neigung 
    if (abs(filteredAngle - targetAngle) > EMERGENCY_ANGLE) {
        setMotorSpeed(0, 0);
        motorsEnabled = false;
        Serial.println("!!! NOTAUS: Zu schraeg !!!");
        return; 
    }
    motorsEnabled = true;

    // Deadzone Check
    if (abs(error) < deadzone) { 
        setMotorSpeed(0, 0);
        errorSum = 0;  
        lastError = error;
        return; 
    }
    
    // Integral (mit Anti-Windup)
    errorSum += error * dt;
    errorSum = constrain(errorSum, -100.0, 100.0); 
    
    // Differential
    float dError = (error - lastError) / dt;
    lastError = error;
    
    // PID Output berechnen
    float output = (Kp * error) + (Ki * errorSum) + (Kd * dError);
    
    // Bewegung basierend auf Web-Befehlen
    float movementBias = webMoveX / 100.0 * MAX_MOTOR_SPEED; // Vor/Zurück
    float rotationBias = webMoveY / 100.0 * MAX_MOTOR_SPEED; // Drehen
    
    int motorSpeedLeft = (int)(output + movementBias + rotationBias);
    int motorSpeedRight = (int)(output + movementBias - rotationBias);
    
    setMotorSpeed(motorSpeedLeft, motorSpeedRight);
    
    // Augen aktualisieren (basierend auf dem gefilterten Winkel)
    drawEyes(filteredAngle); 
}

#endif