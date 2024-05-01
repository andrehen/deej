#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C

#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// OLED

const int NUM_SLIDERS = 2;
const int analogInputs[NUM_SLIDERS] = { A1, A0 };
const String names[] = {"Master", "Chrome"};

int analogSliderValues[NUM_SLIDERS];

// Definição do tamanho da janela para a média móvel
const int windowSize = 15;

// Arrays para armazenar as últimas leituras de cada potenciômetro
int readings[NUM_SLIDERS][windowSize];
int index[NUM_SLIDERS] = {0};
int total[NUM_SLIDERS] = {0};

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
  
  printValuesToDisplay();

  delay(10);
}

void updateSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    analogSliderValues[i] = analogRead(analogInputs[i]);
  }
}

void sendSliderValues() {
  String builtString = String("");

  for (int i = 0; i < NUM_SLIDERS; i++) {
    builtString += String((int)calculateMovingAverage(i, analogSliderValues[i]));

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
  uint8_t lineY[] = {3, 16, 28, 44, 54};

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
  display.println(String((int)calculateMovingAverage(0, analogSliderValues[0])/10));

  display.setCursor(lineX, lineY[3]);
  display.setTextSize(1);
  display.println(names[1]);
  display.setCursor(lineX, lineY[4]);
  display.setTextSize(2);
  display.println(String((int)calculateMovingAverage(1, analogSliderValues[1])/10));
  
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
