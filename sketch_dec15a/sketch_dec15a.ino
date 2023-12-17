#include <LiquidCrystal.h>
#include <DHT.h>

#define DHTPIN A1         
#define DHTTYPE DHT11  
DHT dht(DHTPIN, DHTTYPE);

#define MOISTURE_SENSOR_PIN A0

const int VSS = 22;
const int VDD = 24;
const int A = 26;
const int K = 28;
const int rs = 34, en = 36, d4 = 38, d5 = 40, d6 = 42, d7 = 44;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  lcd.begin(16, 2);

  dht.begin();

  pinMode(VSS, OUTPUT);
  pinMode(VDD, OUTPUT);
  pinMode(A, OUTPUT);
  pinMode(K, OUTPUT);

  digitalWrite(VSS, HIGH);
  digitalWrite(VDD, LOW);
  digitalWrite(A, HIGH);
  digitalWrite(K, LOW);

  Serial.begin(9600);
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  int moisture_level = analogRead(MOISTURE_SENSOR_PIN);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hum:");
  lcd.print(humidity);
  lcd.print("% Tmp:");
  lcd.print(temperature);
  lcd.setCursor(0, 1);
  lcd.print("Moisture: ");
  lcd.print(moisture_level);

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%, Temp: ");
  Serial.print(temperature);
  Serial.print("C, Moisture: ");
  Serial.println(moisture_level);

  delay(2000);
}
