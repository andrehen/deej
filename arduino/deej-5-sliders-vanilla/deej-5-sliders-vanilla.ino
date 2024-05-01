#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "images.h"

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C

#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// OLED

const int NUM_SLIDERS = 2;
const int analogInputs[NUM_SLIDERS] = { A1, A0 };
const String names[] = { "Master", "Chrome" };

int analogSliderValues[NUM_SLIDERS];

// Definição do tamanho da janela para a média móvel
const int windowSize = 15;

// Arrays para armazenar as últimas leituras de cada potenciômetro
int readings[NUM_SLIDERS][windowSize];
int index[NUM_SLIDERS] = { 0 };
int total[NUM_SLIDERS] = { 0 };

// Controls

uint8_t DELAY = 10;
const unsigned long milisecsToTriggerScreenSaver = 20ul * 1000ul;
unsigned long lastInteractionTimestamp = millis();

bool screenSaveActive = false;

unsigned char control_screen_save_pos_X = 0;
unsigned char control_screen_save_pos_Y = 0;
unsigned char control_screen_save_image = 1;

void setup() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    pinMode(analogInputs[i], INPUT);
  }

  Serial.begin(9600);

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

  // Prints the Adafruit logo
  display.display();
  display.setRotation(3);
  display.setTextColor(WHITE);
  delay(250);
  // display.setRotation(2);
}

void loop() {
  updateSliderValues();
  sendSliderValues();  // Actually send data (all the time)
  // printSliderValues(); // For debug

  if (screenSaveActive) {
    printScreenSave();
  } else {
    printValuesToDisplay();
    if (millis() - lastInteractionTimestamp >= milisecsToTriggerScreenSaver) {
      screenSaveActive = true;
    }
  }

  delay(DELAY);
}

void updateSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    int readValue = calculateMovingAverage(i, analogRead(analogInputs[i]));
    int diff = abs(analogSliderValues[i] - readValue);

    if (diff >= 2) {  // The lower the number the mor sensible the turn will be. If value is high, slowing changing the slider my never "wakes" the screen
      lastInteractionTimestamp = millis();
      if (screenSaveActive == true) screenSaveActive = false;
    }

    analogSliderValues[i] = readValue;
  }
}

void sendSliderValues() {
  String builtString = String("");

  for (int i = 0; i < NUM_SLIDERS; i++) {
    builtString += String((int)analogSliderValues[i]);

    if (i < NUM_SLIDERS - 1) {
      builtString += String("|");
    }
  }

  Serial.println(builtString);
}

/**
Text Size 1 = 8 height
Text Size 2 = 16 height
*/
void printValuesToDisplay() {

  uint8_t lineX = 3;
  uint8_t lineY[] = { 3, 16, 28, 44, 54 };

  display.clearDisplay();
  display.drawRect(0, 0, 64, 128, WHITE);
  display.drawLine(0, 12, 64, 12, WHITE);
  display.setCursor(13, lineY[0]);

  display.setTextSize(1);
  display.println("Values");
  display.println("");

  display.setCursor(lineX, lineY[1]);
  display.setTextSize(1);
  display.println(names[0]);
  display.setCursor(lineX, lineY[2]);
  display.setTextSize(2);
  display.println(String((int)calculateMovingAverage(0, analogSliderValues[0]) / 10));

  display.setCursor(lineX, lineY[3]);
  display.setTextSize(1);
  display.println(names[1]);
  display.setCursor(lineX, lineY[4]);
  display.setTextSize(2);
  display.println(String((int)calculateMovingAverage(1, analogSliderValues[1]) / 10));

  // display.setTextSize(1);
  // display.println("Steam");
  // display.setTextSize(2);
  // display.println(String((int)calculateMovingAverage(2, analogSliderValues[2])/10));

  // display.setTextSize(1);
  // display.println("Input");
  // display.setTextSize(2);
  // display.println(String((int)calculateMovingAverage(3, analogSliderValues[3])/10));

  // for (int i = 0; i < NUM_SLIDERS; i++) {
  //   // display.print(i);
  //   // display.print(": ");
  //   display.println(String((int)analogSliderValues[i]));
  // }

  display.display();
}

void printScreenSave() {
  display.clearDisplay();

  if (millis() % 5000UL < 50) {
    control_screen_save_pos_X = random(64 - 32);
    control_screen_save_pos_Y = random(128 - 32);
    control_screen_save_image = random(5);
  }

  int x = control_screen_save_pos_X;
  int y = control_screen_save_pos_Y;

  switch (control_screen_save_image) {
    case 0:
      display.drawBitmap(x, y, volume_up_32_32, 32, 32, WHITE);
      break;
    case 1:
      display.drawBitmap(x, y, headphones_32, 32, 32, WHITE);
      break;
    case 2:
      display.drawBitmap(x, y, queue_music_32_24, 32, 24, WHITE);
      break;
    case 3:
      display.drawBitmap(x, y, microphone_filled_32, 32, 32, WHITE);
      break;
    case 4:
      display.drawBitmap(x, y, music_note_32, 32, 32, WHITE);
      break;
  }

  display.display();
}

int generateRandomCoords() {
  return random(128 - 32);
}

int calculateMovingAverage(int potIndex, int sensorValue) {
  // Subtrair a leitura mais antiga e adicionar a nova
  total[potIndex] = total[potIndex] - readings[potIndex][index[potIndex]] + sensorValue;

  // Armazenar a nova leitura no array
  readings[potIndex][index[potIndex]] = sensorValue;

  // Incrementar o índice e voltar ao início se atingir o tamanho máximo
  index[potIndex] = (index[potIndex] + 1) % windowSize;

  // Calcular a média das últimas leituras
  int smoothedValue = total[potIndex] / windowSize;

  return smoothedValue;
}


void printSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    String printedString = String("Slider #") + String(i + 1) + String(": ") + String(analogSliderValues[i]) + String(" mV");
    Serial.write(printedString.c_str());

    if (i < NUM_SLIDERS - 1) {
      Serial.write(" | ");
    } else {
      Serial.write("\n");
    }
  }
}
