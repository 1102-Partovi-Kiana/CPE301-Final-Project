//Currently just reads temp and humidity and outputs on lcd with a series of delays even though it should be done once per minute later
//can we use lcd.print()? is that acceptable library function?

#include <dht.h> //install the DHTLib library
dht DHT;
#define DHT11_PIN 12

#include <LiquidCrystal.h>

// LCD pins <--> Arduino pins
const int RS = 6, EN = 7, D4 = 2, D5 = 3, D6 = 4, D7 = 5;

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

void lcdDisplay();
void tempSensor();

void setup(){
  Serial.begin(9600);
  lcd.begin(16,2); //setup cols and rows 
}

void loop(){
  lcdDisplay();
  delay(1000);
}

void lcdDisplay(){
  tempSensor();
  delay(1000);
}

void tempSensor(){
    int chk = DHT.read11(DHT11_PIN);
    lcd.setCursor(0,0); 
    lcd.print("Temp: ");
    lcd.print(DHT.temperature);
    lcd.print((char)223);
    lcd.print("C");
    lcd.setCursor(0,1);
    lcd.print("Humidity: ");
    lcd.print(DHT.humidity);
    lcd.print("%");
    delay(1000);
}