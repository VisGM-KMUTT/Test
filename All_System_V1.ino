#define relay1 7  //circulate pump
#define relay2 6  //Mixing
#define relay3 5  //Gas N2 valve
#define relay4 4  //Acid pump
#define relay5 3  //Base pump
#define relay6 2  //Heater

//---------------------Oxygen sensor--------------------------------
#include "DFRobot_OxygenSensor.h"
#define COLLECT_NUMBER 10  // collect number, the collection range is 1-100.
#define Oxygen_IICAddress ADDRESS_3
DFRobot_OxygenSensor Oxygen;

//-----------------------pH-----------------------------------------
const int analogPhPin = A0;  //pH module pin P0 connected to analog pin A0
long phTot, temTot;
float phAvg, temAvg;
int x;
float C = 4.8082;   //Constant of straight line (Y = mx + C)
float m = -0.2427;  // Slope of straight line (Y = mx + C)

//------------------------------------------------------------------
#include "max6675.h"
//-----------------------Temp Heater--------------------------------
int thermoDO = 8;
int thermoCS = 9;
int thermoCLK = 10;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

//-----------------------Temp Reactor--------------------------------
int thermoDO1 = 30;
int thermoCS1 = 31;
int thermoCLK1 = 32;
MAX6675 thermocouple1(thermoCLK1, thermoCS1, thermoDO1);

//-----------------------Temp Sample Collecting--------------------------------
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

int thermoDO2 = 33;
int thermoCS2 = 34;
int thermoCLK2 = 35;
MAX6675 thermocouple2(thermoCLK2, thermoCS2, thermoDO2);

void setup() {
  Serial.begin(9600);

  lcd.begin();  //if lcd.begin(); Shows an error message, we should check the LIBRARY first.
  lcd.backlight();

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(relay5, OUTPUT);
  pinMode(relay6, OUTPUT);
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);
  digitalWrite(relay5, LOW);
  digitalWrite(relay6, LOW);

  pinMode(22, INPUT);
  pinMode(23, OUTPUT);

//  while (!Oxygen.begin(Oxygen_IICAddress)) {
//    Serial.println("I2C Oxygen sensor error !");
//    delay(1000);
//  }
//  Serial.println("I2C connect success !");
}

void loop() {

  //--------------------Temp Read-----------------------------
  float TempHot = thermocouple.readCelsius();
//  Serial.print("Temp Hotwater Tank = ");
//  Serial.println(TempHot);

  float TempReactor = thermocouple1.readCelsius() - 4.5;
//  Serial.print("Temp Reactor = ");
//  Serial.println(TempReactor);

  float TempSampleCollecting = thermocouple2.readCelsius() - 3.00;
//  Serial.print("Temp Sample Collecting = ");
//  Serial.println(TempSampleCollecting);

  lcd.setCursor(1, 0);
  lcd.print("TEMPERATURE IS ");
  lcd.setCursor(6, 1);
  lcd.print(thermocouple2.readCelsius() - 3.00);

  //--------------------Temp Control-----------------------------
  if (TempHot <= 31) {
    digitalWrite(relay6, HIGH);
//    Serial.println("Heater ON");
  }
  if (TempHot > 31) {
    digitalWrite(relay6, LOW);
//    Serial.println("Heater OFF");
  }
  //-------------------------pH Read-----------------------------
  phTot = 0;
  temTot = 0;
  phAvg = 0;
  temAvg = 0;

  for (x = 0; x < 10; x++) {
    phTot += analogRead(A0);
    temTot += analogRead(A1);
    delay(10);
  }
  float temAvg = temTot / 10;
  float phAvg = temTot / 10;
  float temVoltage = temAvg * (5000.0 / 1023.0);
  float phVoltage = phAvg * (5.0 / 1023.0);
  float Etemp = temVoltage * 0.1;
  float pHValue = (phVoltage-C)/(m);
//  Serial.print("  pH = ");
//  Serial.println(pHValue);

  //---------------------------pH Control-----------------------------

  if (pHValue >= 8) {
    digitalWrite(relay4, HIGH);
//    Serial.println("ON Acidpump");
    digitalWrite(relay5, LOW);
//    Serial.println("OFF Basepump");
  }
  if (pHValue <= 6) {
    digitalWrite(relay4, LOW);
//    Serial.println("OFF Acidpump");
    digitalWrite(relay5, HIGH);
//    Serial.println("ON Basepump");
  }
  if (pHValue > 6 & pHValue < 8) {
    digitalWrite(relay4, LOW);
//    Serial.println("OFF Acidpump");
    digitalWrite(relay5, LOW);
//    Serial.println("OFF Basepump");
  }
  //---------------------Oxygen Read--------------------------------------
  float oxygenData = Oxygen.getOxygenData(COLLECT_NUMBER);
//  Serial.print("Oxygen concentration is");
//  Serial.print(oxygenData);
//  Serial.println("%vol");

  //---------------------Oxygen Control-----------------------------------
  if (oxygenData >= 1) {
    digitalWrite(relay3, HIGH);
//    Serial.println("N2 Valve is ON");
  }
  if (oxygenData < 1) {
    digitalWrite(relay3, LOW);
//    Serial.println("N2 Valve is OFF");
  }
  //-------------------------Send to ESP32--------------------------------

  String C = String(pHValue)+ "," + String(oxygenData)+ "," + String(TempReactor) + "," + String(TempSampleCollecting);
  Serial.println(C);

  //-----------------------------------------------------------------------
  delay(60000);
}
