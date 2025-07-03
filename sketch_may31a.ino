#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
#include <BluetoothSerial.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

Adafruit_BMP280 bmp;

#define LDR_PIN 34
#define MQ2_PIN 35  // <<< Novo: pino analógico para o MQ-2

float tempHistory[SCREEN_WIDTH];

BluetoothSerial SerialBT;

void drawIconTemp(int x, int y) {
  display.drawCircle(x + 6, y + 6, 6, WHITE);
  display.drawLine(x + 6, y + 12, x + 6, y + 20, WHITE);
  display.fillRect(x + 4, y + 20, 4, 4, WHITE);
}

void drawIconHum(int x, int y) {
  display.fillTriangle(x + 6, y, x, y + 12, x + 12, y + 12, WHITE);
}

void drawIconLuz(int x, int y) {
  display.fillCircle(x + 6, y + 6, 4, WHITE);
  for (int i = 0; i < 360; i += 45) {
    float angle = i * PI / 180;
    display.drawLine(x + 6, y + 6, x + 6 + 8 * cos(angle), y + 6 + 8 * sin(angle), WHITE);
  }
}

void drawIconPressao(int x, int y) {
  display.drawCircle(x + 6, y + 6, 6, WHITE);
  display.drawLine(x, y + 6, x + 12, y + 6, WHITE);
  display.drawLine(x + 6, y, x + 6, y + 12, WHITE);
}

void drawIconGas(int x, int y) {  // <<< Novo: ícone de gás
  display.drawRect(x, y + 4, 12, 8, WHITE);
  display.drawLine(x + 3, y + 4, x + 3, y, WHITE);
  display.drawLine(x + 9, y + 4, x + 9, y, WHITE);
}

void drawGraph() {
  for (int x = 0; x < SCREEN_WIDTH - 1; x++) {
    int y1 = SCREEN_HEIGHT - (int)tempHistory[x];
    int y2 = SCREEN_HEIGHT - (int)tempHistory[x + 1];
    display.drawLine(x, y1, x + 1, y2, WHITE);
  }
}

void shiftTempHistory(float newTemp) {
  for (int i = 0; i < SCREEN_WIDTH - 1; i++) {
    tempHistory[i] = tempHistory[i + 1];
  }
  tempHistory[SCREEN_WIDTH - 1] = map(newTemp, 0, 50, 0, SCREEN_HEIGHT - 10);
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32-Monitor");
  dht.begin();

  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 não encontrado!");
    while (1);
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED não encontrado!");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  for (int i = 0; i <= SCREEN_WIDTH; i += 8) {
    display.clearDisplay();
    display.fillRect(0, 0, i, SCREEN_HEIGHT, WHITE);
    display.display();
    delay(50);
  }

  display.clearDisplay();
  display.setCursor(10, 20);
  display.println("Sistema Iniciado");
  display.display();
  delay(1000);

  for (int i = 0; i < SCREEN_WIDTH; i++) {
    tempHistory[i] = 0;
  }

  SerialBT.println("🔵 Bluetooth iniciado!");
}

void loop() {
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F;
  float humidity = dht.readHumidity();
  int ldrValue = analogRead(LDR_PIN);
  float lightPercent = map(ldrValue, 0, 4095, 0, 100);

  int mq2Value = analogRead(MQ2_PIN);  // <<< Novo: leitura do MQ-2
  float mq2Percent = map(mq2Value, 0, 4095, 0, 100);  // percentual

  shiftTempHistory(temperature);

  for (int i = 0; i < 2; i++) {
    display.invertDisplay(true);
    delay(100);
    display.invertDisplay(false);
    delay(100);
  }

  display.clearDisplay();

  drawIconTemp(0, 0);
  drawIconHum(26, 0);
  drawIconLuz(52, 0);
  drawIconPressao(78, 0);
  drawIconGas(104, 0);  // <<< Novo: ícone de gás

  display.setCursor(0, 20);
  display.print("T: ");
  display.print(temperature, 1);
  display.println("C");

  display.setCursor(0, 30);
  display.print("H: ");
  display.print(humidity, 1);
  display.println("%");

  display.setCursor(0, 40);
  display.print("L: ");
  display.print(lightPercent, 0);
  display.println("%");

  display.setCursor(0, 50);
  display.print("P: ");
  display.print(pressure, 1);
  display.println("hPa");

  display.setCursor(70, 50);
  display.print("G: ");
  display.print(mq2Percent, 0);
  display.println("%");

  drawGraph();
  display.display();

  SerialBT.printf("🌡️ T: %.1f C | 💧 H: %.1f %% | 💡 L: %.0f %% | 📟 P: %.1f hPa | 🟡 G: %.0f %%\n",
                  temperature, humidity, lightPercent, pressure, mq2Percent);

  delay(2000);
}
