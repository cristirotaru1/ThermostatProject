#include <LiquidCrystal.h>

// LCD Pins: RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// Define pins
const int tempPin = A0;
const int relayPin = 22;

// Variables
float temperature = 0.0;
float tempThreshold = 22.0; // Default temperature threshold
bool heatingOn = false;

float resolutionADC = 0.0049;
float resolutionSensor = 0.01;

byte flame[8] = {
  0b00100,
  0b01010,
  0b01010,
  0b01110,
  0b01110,
  0b00100,
  0b01110,
  0b00000
};


void setup() {
  lcd.begin(16, 2);

  pinMode(tempPin, INPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  Serial.begin(9600);
  
  lcd.print("Thermostat Init");
  lcd.createChar(0, flame);
  delay(2000);
}

void loop() {
  // Read sensors
  temperature = readTemperature(10, tempPin);

  // Control logic
  if (temperature < tempThreshold && !heatingOn) {
    digitalWrite(relayPin, HIGH);
    heatingOn = true;
  } else if (temperature >= tempThreshold && heatingOn) {
    digitalWrite(relayPin, LOW);
    heatingOn = false;
  }


  lcd.clear();
  // Display
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature, 1);
  lcd.print("C ");
  Serial.print("TEMP:");  //Serial1 pt comunicare cu ESP
  Serial.print(temperature, 1);
  Serial.print(",");

  if (heatingOn) {
    lcd.setCursor(15,0);
    lcd.write(byte(0));
    Serial.print("HEATING:ON,");
  }
  else {
    Serial.print("HEATING:OFF,");
  }

  lcd.setCursor(0, 1);
  lcd.print("Desired: ");
  lcd.print(tempThreshold, 1);
  lcd.print("C ");
  Serial.print("THRESH:");
  Serial.print(tempThreshold, 1);
  Serial.print("\n");

  delay(1000);
}

void serialEvent()
{
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    if (data.startsWith("SET_THRESHOLD:")) {
      tempThreshold = data.substring(14).toFloat();
    }
  }
}

float readTemperature(int count, int pin)
{
  float sumTemp = 0;
  for (int i = 0; i < count; i++) {
    int reading = analogRead(pin);
    float voltage = reading * resolutionADC;
    float tempCelsius = (voltage - 0.5) / resolutionSensor ;
    sumTemp = sumTemp + tempCelsius;
  }

  return sumTemp / (float)count;
}
