# 🚗 ESP32 Buggy - PS4 Controller Steuerung

Ein ESP32-basiertes RC-Buggy Steuerungssystem mit PS4 Controller über Bluetooth.

### **Pin-Belegung**

#### Motor

Pin 13  → Motor PWM Vorwärts
Pin 12  → Motor PWM Rückwärts
Pin 14  → Motor DauerHigh Vorwärts
Pin 27  → Motor DauerHigh Rückwärts

#### Servo

Pin 26  → Servo Signal

#### LEDs

Pin 16  → Blinker Links
Pin 5   → Blinker Rechts
Pin 19  → Frontlicht
Pin 18  → Rücklicht
Pin 4   → Bremslicht


## 📥 Installation

### 1. Arduino IDE vorbereiten

#### ESP32 Board Support installieren
1. Arduino IDE öffnen
2. Datei → Voreinstellungen
3. Bei "Zusätzliche Boardverwalter-URLs" einfügen:
   
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   
4. Werkzeuge → Board → Boardverwalter
5. "esp32" suchen und die bibliothek esp32 von espressif System installieren

#### Board auswählen
- Werkzeuge → Board → ESP32 Arduino → ESP32 Dev Module

### **2. Bibliotheken installieren**

Über Sketch → Bibliothek einbinden → Bibliotheken verwalten:

| Bibliothek | Version | Link |
|-----------|---------|------|
| PS4Controller | latest | [GitHub](https://github.com/aed3/PS4-esp32) |
| ESP32Servo | latest | Über Library Manager |

**Custom Libraries** (manuell installieren):
- BuggyControl: https://github.com/niklasschoening/BuggyControl
  über die grüne Schaltfläche "<>Code" und dann auf Download ZIP.

Diese in Arduino IDE > Sketch > Inlcude Libary > Add .ZIP Library... 
und dann Die heruntergeladene Datei BuggyControl.zip auswählen.

### **3. Code hochladen**

1. ESP32 per USB verbinden
2. Port auswählen: `Werkzeuge → Port → /dev/ttyUSB0` (oder COM-Port unter Windows)
3. Upload-Speed: `921600`
4. `Sketch → Hochladen`

---

## 🎮 Benutzung

### **PS4 Controller verbinden**

#### **Methode 1: Controller MAC-Adresse festlegen**
```cpp
// In der Konfiguration ändern:
const char* CONTROLLER_MAC = "AA:BB:CC:DD:EE:FF"; // Die Host-Mac-Adresse eingeben
```

**Host-MAC-Adresse herausfinden (Windows):**
Das Tool sixaxispairtool herunterladen und den Controller via USB-Leitung mit dem PC/Laptop verbinden.
Es Sollte nun eine Host-Mac-Adresse in dem Tool stehen.

<img width="209" height="112" alt="Screenshot 2025-11-26 113141 2" src="https://github.com/user-attachments/assets/cd6cd1cd-2b2e-4f93-aa6c-20d4299a9159" />

---

## 🕹️ Steuerung


### **Steuerungsbelegung**

| Input | Funktion |
|-------|----------|
| **R2** | Gas (Vorwärts) |
| **L2** | Bremse (Rückwärts) |
| **Linker Stick X** | Lenkung (Links/Rechts) |
| **D-Pad Links** | Blinker Links |
| **D-Pad Rechts** | Blinker Rechts |
| **△ (Triangle)** | LED Animation |

### **Steuerungsbereiche**

- **Motor**: -100% (Rückwärts) bis +100% (Vorwärts)
- **Lenkung**: -100% (Links) bis +100% (Rechts)
- **Deadzone**: Joystick-Werte < 10 werden als 0 behandelt

---

## ⚙️ Konfiguration

Alle wichtigen Parameter sind am Anfang des Codes definiert:

```cpp
// === KONFIGURATION ===

// Controller
const char* CONTROLLER_MAC = "60:5b:b4:b2:90:b6"; // Oder NULL für beliebig

// Motor
const uint8_t MOTOR_MAX_DUTY = 100;  // Maximale Geschwindigkeit (%)
const uint8_t MOTOR_MIN_DUTY = 30;   // Minimale Geschwindigkeit (%)

// Servo
const uint8_t SERVO_MAX_ANGLE = 90;  // Maximaler Lenkwinkel
const uint8_t SERVO_DEADZONE = 6;    // Deadzone in Grad

// Controller Input
const uint8_t JOYSTICK_DEADZONE = 10; // Joystick Deadzone
const uint8_t DATA_SKIP_RATE = 3;     // Verarbeite jeden 3. Datensatz
```

### **Anpassungen vornehmen**


1. **Lenkwinkel ändern:**
   ```cpp
   const uint8_t SERVO_MAX_ANGLE = 90; // → 120 für stärkeres Lenken
   const uint8_t REST_POSITION = 90 // -> Änderung der Startposition
   ```
---

## 🔍 Troubleshooting


### ❌ **"Compilation error: esp_base_mac_addr_set"**

**Problem:** Falsche ESP32 Core Version

**Lösung:**
```
Werkzeuge → Board → Boardverwalter
esp32 suchen → Version 2.0.14 installieren
```


---

## 📚 Bibliotheken

### **Verwendete Libraries**

```cpp
#include <BuggyControl.h>     // Buggy Controller steuerung für Motor, LED und Servo
#include <PS4Controller.h>    // PS4 Bluetooth Kommunikation
#include <ESP32Servo.h>       // Servo-Steuerung
#include <BLESecurity.h>      // Bluetooth Security
#include <esp_task_wdt.h>     // Watchdog Timer
```
