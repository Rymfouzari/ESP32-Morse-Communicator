#include "thingProperties.h"

const int LED_STATUS_PIN = 2;

const int LED_MORSE_PIN = 15;
const int LDR_PIN = 34;
const int BUZZER_PIN = 16;

const int LED_BRIGHTNESS = 255;
const int LED_CHANNEL = 0;
const int LED_FREQUENCY = 5000;
const int LED_RESOLUTION = 8;

const int BUZZER_CHANNEL = 1;
const int BUZZER_RESOLUTION = 8;
const int BUZZER_VOLUME = 128;
const int BUZZER_MORSE_FREQ = 1000;  

int LIGHT_THRESHOLD = 700;

const int DOT_TIME = 150;
const int DASH_TIME = DOT_TIME * 3;
const int SYMBOL_GAP = DOT_TIME;
const int LETTER_GAP = DOT_TIME * 3;
const int WORD_GAP = DOT_TIME * 7;

const unsigned long END_OF_MESSAGE_SILENCE = 3000;


bool isSending = false;
bool receivingLight = false;
bool messageSent = false;

unsigned long lightStart = 0;
unsigned long lastLightEnd = 0;

String currentMorseLetter = "";
String decodedMessage = "";

struct MorseMap {
  char letter;
  const char* morse;
};

MorseMap morseTable[] = {
  {'A', ".-"}, {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},
  {'E', "."}, {'F', "..-."}, {'G', "--."}, {'H', "...."},
  {'I', ".."}, {'J', ".---"}, {'K', "-.-"}, {'L', ".-.."},
  {'M', "--"}, {'N', "-."}, {'O', "---"}, {'P', ".--."},
  {'Q', "--.-"}, {'R', ".-."}, {'S', "..."}, {'T', "-"},
  {'U', "..-"}, {'V', "...-"}, {'W', ".--"}, {'X', "-..-"},
  {'Y', "-.--"}, {'Z', "--.."},
  {'0', "-----"}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"},
  {'4', "....-"}, {'5', "....."}, {'6', "-...."}, {'7', "--..."},
  {'8', "---.."}, {'9', "----."}
};

#define NOTE_C6  1047
#define NOTE_D6  1175
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_G6  1568
#define NOTE_A6  1760
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_D7  2349
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_G7  3136
#define NOTE_A7  3520
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_REST 0

struct Note {
  int frequency;
  int duration;
};

Note melodyAnniversaire[] = {
  {NOTE_C6, 300}, {NOTE_C6, 150}, {NOTE_D6, 450}, {NOTE_C6, 450},
  {NOTE_F6, 450}, {NOTE_E6, 900},
  {NOTE_C6, 300}, {NOTE_C6, 150}, {NOTE_D6, 450}, {NOTE_C6, 450},
  {NOTE_G6, 450}, {NOTE_F6, 900}
};

Note melodyMariage[] = {
  {NOTE_C6, 400}, {NOTE_F6, 200}, {NOTE_F6, 100}, {NOTE_F6, 600},
  {NOTE_C6, 400}, {NOTE_G6, 200}, {NOTE_F6, 200}, {NOTE_E6, 200},
  {NOTE_F6, 800}
};

Note melodyFelicitations[] = {
  {NOTE_C7, 200}, {NOTE_E7, 200}, {NOTE_G7, 200}, {NOTE_C7, 200},
  {NOTE_E7, 200}, {NOTE_G7, 200}, {NOTE_C7, 600}
};

Note melodyNoel[] = {
  {NOTE_E6, 250}, {NOTE_E6, 250}, {NOTE_E6, 500},
  {NOTE_E6, 250}, {NOTE_E6, 250}, {NOTE_E6, 500},
  {NOTE_E6, 250}, {NOTE_G6, 250}, {NOTE_C6, 375}, {NOTE_D6, 125},
  {NOTE_E6, 1000}
};

void setup() {
  Serial.begin(9600);
  delay(1500);

  pinMode(LED_STATUS_PIN, OUTPUT);

  ledcSetup(LED_CHANNEL, LED_FREQUENCY, LED_RESOLUTION);
  ledcAttachPin(LED_MORSE_PIN, LED_CHANNEL);
  ledcWrite(LED_CHANNEL, 0);

  ledcSetup(BUZZER_CHANNEL, BUZZER_MORSE_FREQ, BUZZER_RESOLUTION);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWrite(BUZZER_CHANNEL, 0);

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();

  if (!isSending) {
    listenMorse();
  }
}


void buzzerOn(int frequency) {
  ledcWriteTone(BUZZER_CHANNEL, frequency);
  ledcWrite(BUZZER_CHANNEL, BUZZER_VOLUME);
}

void buzzerOff() {
  ledcWrite(BUZZER_CHANNEL, 0);
}

void playNote(int frequency, int duration) {
  if (frequency == NOTE_REST) {
    delay(duration);
  } else {
    buzzerOn(frequency);
    delay(duration);
    buzzerOff();
    delay(30);  
  }
}

void playMelody(Note* melody, int length) {
  Serial.println(">>> Lecture melodie");
  for (int i = 0; i < length; i++) {
    ArduinoCloud.update();
    playNote(melody[i].frequency, melody[i].duration);
  }
  buzzerOff();
}


void lightAndBeep(int duration) {
  ledcWrite(LED_CHANNEL, LED_BRIGHTNESS);
  buzzerOn(BUZZER_MORSE_FREQ);
  delay(duration);
  ledcWrite(LED_CHANNEL, 0);
  buzzerOff();
}

const char* charToMorse(char c) {
  c = toupper(c);
  for (int i = 0; i < sizeof(morseTable) / sizeof(morseTable[0]); i++) {
    if (morseTable[i].letter == c) {
      return morseTable[i].morse;
    }
  }
  return "";
}

char morseToChar(String morse) {
  for (int i = 0; i < sizeof(morseTable) / sizeof(morseTable[0]); i++) {
    if (morse == morseTable[i].morse) {
      return morseTable[i].letter;
    }
  }
  return '?';
}

void sendMorseMessage(String text) {
  isSending = true;
  text.toUpperCase();

  Serial.print("Envoi morse : ");
  Serial.println(text);

  for (int i = 0; i < text.length(); i++) {
    ArduinoCloud.update();
    char c = text[i];

    if (c == ' ') {
      delay(WORD_GAP);
      continue;
    }

    const char* morse = charToMorse(c);

    for (int j = 0; morse[j] != '\0'; j++) {
      if (morse[j] == '.') {
        lightAndBeep(DOT_TIME);
      } else if (morse[j] == '-') {
        lightAndBeep(DASH_TIME);
      }
      delay(SYMBOL_GAP);
      ArduinoCloud.update();
    }
    delay(LETTER_GAP);
  }

  ledcWrite(LED_CHANNEL, 0);
  buzzerOff();
  isSending = false;
}


void finishCurrentLetter() {
  if (currentMorseLetter.length() > 0) {
    char decoded = morseToChar(currentMorseLetter);
    decodedMessage += decoded;

    Serial.print("Lettre recue : ");
    Serial.print(currentMorseLetter);
    Serial.print(" = ");
    Serial.println(decoded);

    currentMorseLetter = "";
  }
}

void checkKeywordsAndPlay(String message) {
  String upper = message;
  upper.toUpperCase();

  if (upper.indexOf("ANNIVERSAIRE") >= 0) {
    Serial.println(">>> Mot-cle detecte : ANNIVERSAIRE");
    playMelody(melodyAnniversaire, sizeof(melodyAnniversaire) / sizeof(Note));
  }
  else if (upper.indexOf("MARIAGE") >= 0) {
    Serial.println(">>> Mot-cle detecte : MARIAGE");
    playMelody(melodyMariage, sizeof(melodyMariage) / sizeof(Note));
  }
  else if (upper.indexOf("FELICITATIONS") >= 0) {
    Serial.println(">>> Mot-cle detecte : FELICITATIONS");
    playMelody(melodyFelicitations, sizeof(melodyFelicitations) / sizeof(Note));
  }
  else if (upper.indexOf("NOEL") >= 0) {
    Serial.println(">>> Mot-cle detecte : NOEL");
    playMelody(melodyNoel, sizeof(melodyNoel) / sizeof(Note));
  }
}

void listenMorse() {
  int lightValue = analogRead(LDR_PIN);
  bool lightDetected = lightValue > LIGHT_THRESHOLD;

  
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 200) {
      Serial.print("LDR: ");
      Serial.print(lightValue);
      Serial.print(" | ");
      Serial.println(lightDetected ? "LUMIERE" : "sombre");
      lastPrint = millis();
    }
  

  unsigned long now = millis();

  if (lightDetected && !receivingLight) {
    receivingLight = true;
    lightStart = now;
    messageSent = false;
  }

  if (!lightDetected && receivingLight) {
    receivingLight = false;
    unsigned long duration = now - lightStart;
    lastLightEnd = now;

    if (duration > 40) {
      if (duration < (DOT_TIME + DASH_TIME) / 2) {
        currentMorseLetter += ".";
        Serial.println("Point detecte");
      } else {
        currentMorseLetter += "-";
        Serial.println("Trait detecte");
      }
    }
  }

  if (!receivingLight && currentMorseLetter.length() > 0) {
    unsigned long silence = now - lastLightEnd;
    if (silence > (SYMBOL_GAP + LETTER_GAP) / 2) {
      finishCurrentLetter();
    }
  }

  if (!receivingLight && decodedMessage.length() > 0 && !messageSent) {
    unsigned long silence = now - lastLightEnd;
    if (silence > (LETTER_GAP + WORD_GAP) / 2 && silence < END_OF_MESSAGE_SILENCE) {
      if (!decodedMessage.endsWith(" ")) {
        decodedMessage += " ";
        Serial.println(">>> Espace ajoute (fin de mot)");
      }
    }
  }

  if (!receivingLight && decodedMessage.length() > 0 && !messageSent) {
    unsigned long silence = now - lastLightEnd;
    if (silence > END_OF_MESSAGE_SILENCE) {
      decodedMessage.trim();

      Serial.print(">>> MESSAGE COMPLET ENVOYE : '");
      Serial.print(decodedMessage);
      Serial.println("'");

      RecieveMessage = decodedMessage;
      ArduinoCloud.update();

      checkKeywordsAndPlay(decodedMessage);

      decodedMessage = "";
      currentMorseLetter = "";
      messageSent = true;

      Serial.println(">>> Variables nettoyees");
    }
  }
}

void onMessageChange() {
  if (Message.length() > 0) {
    decodedMessage = "";
    currentMorseLetter = "";
    messageSent = false;
    sendMorseMessage(Message);
  }
}

void onRecieveMessageChange() {}
void onTestChange() {}
void onLedTestChange() {}

void onIsOnSwitchChange() {
  digitalWrite(LED_STATUS_PIN, IsOnSwitch ? HIGH : LOW);
}
