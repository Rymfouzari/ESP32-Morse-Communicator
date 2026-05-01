# ESP32 Morse Code Project

ESP32 Arduino project for sending and receiving Morse code using LED, LDR sensor, and buzzer, with Arduino IoT Cloud integration.

---

## Features

* Send text messages as Morse code (LED + buzzer)
* Receive Morse code via LDR (light sensor)
* Real-time Morse decoding
* Keyword detection triggering melodies:

  * ANNIVERSAIRE → birthday melody
  * MARIAGE → wedding melody
  * FELICITATIONS → congratulations melody
  * NOEL → Christmas melody
* Remote control and monitoring via Arduino IoT Cloud

---

## Hardware Requirements

* ESP32 (x2 for full communication setup)
* LED
* LDR (photoresistor)
* Buzzer
* Resistors (for LED and LDR voltage divider)
* Breadboard and jumper wires

---

## Wiring

| Component  | ESP32 Pin | Description              |
| ---------- | --------- | ------------------------ |
| Status LED | GPIO 2    | Controlled by IoT switch |
| Morse LED  | GPIO 15   | Sends Morse signals      |
| LDR sensor | GPIO 34   | Receives light signals   |
| Buzzer     | GPIO 16   | Plays Morse + melodies   |
| Power      | 3.3V      | Power supply             |
| Ground     | GND       | Common ground            |

### LDR Circuit (Important)

The LDR must be wired as a **voltage divider**:

* One side of the LDR → 3.3V
* Other side → GPIO 34
* A resistor between GPIO 34 → GND

This setup allows analog light reading via:

```cpp
analogRead(LDR_PIN);
```

---

## Arduino IoT Cloud Setup

Create the following variables:

| Variable name    | Type   | Permission   | Description              |
| ---------------- | ------ | ------------ | ------------------------ |
| `message`        | String | Read & Write | Message to send          |
| `RecieveMessage` | String | Read         | Decoded received message |
| `is_on_switch`   | Bool   | Read & Write | Controls status LED      |

⚠️ Important:

* The name `RecieveMessage` must match the code exactly
* If you rename it → update the code accordingly

---

## How It Works

### Sending

* Text is converted into Morse code
* LED and buzzer emit dots and dashes
* Timing is based on standard Morse intervals

### Receiving

* LDR detects light pulses
* Pulse duration determines:

  * dot (short)
  * dash (long)
* Morse sequences are decoded into characters
* Words are reconstructed with timing gaps

### Smart Behavior

* When a full message is received:

  * It is sent to Arduino Cloud
  * Keywords trigger melodies automatically

---

## Calibration

Light sensitivity depends on environment.

Threshold in code:

```cpp
int LIGHT_THRESHOLD = 700;
```

### To calibrate:

1. Open Serial Monitor
2. Observe LDR values
3. Adjust threshold:

   * Too sensitive → increase value
   * Not detecting → decrease value

---

## Timing Configuration

Morse timing constants:

```cpp
const int DOT_TIME = 150;
const int DASH_TIME = DOT_TIME * 3;
const int SYMBOL_GAP = DOT_TIME;
const int LETTER_GAP = DOT_TIME * 3;
const int WORD_GAP = DOT_TIME * 7;
```

Changing these affects:

* Transmission speed
* Decoding accuracy

---

## Project Structure

```
/project
│── main.ino
│── thingProperties.h
│── /images
│    └── circuit.jpg
│── README.md
```

---

## Circuit Preview

Add your wiring image here:

```
/images/circuit.jpg
```

```md
![Circuit wiring](images/circuit.jpg)
```

---

## Security

Do NOT upload sensitive data:

* Wi-Fi credentials
* Arduino IoT Cloud keys

Add to `.gitignore` if needed:

```
thingProperties.h
```

---

## Notes

* Use two ESP32 boards for full communication testing
* Ambient light can affect reception accuracy
* Keep consistent distance between LED and LDR

---

## License

MIT License (or your choice)
