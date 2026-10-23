#define BLYNK_TEMPLATE_ID "TMPL3cbM5tvh5" 
#define BLYNK_TEMPLATE_NAME "DRUNK DRIVE DETECTION SYSTEM" 
#define BLYNK_AUTH_TOKEN "_xNvvkmP_vF0hTDR7fZzc3WX41xnzULU" 
 
#include <Servo.h> 
#include <Wire.h> 
#include <LiquidCrystal_I2C.h> 
#include <ESP8266WiFi.h> 
#include <BlynkSimpleEsp8266.h> 
 
// WiFi Credentials 
char ssid[] = "PARTH"; 
char pass[] = "parth2005"; 
 
// ================== PIN DEFINITIONS ================== 
#define IR_SENSOR   D5      
#define MQ3_PIN     A0      
#define FAN         D6      
#define BUZZER      D7      
#define SERVO_PIN   D4      
 
Servo gateServo; 
LiquidCrystal_I2C lcd(0x27, 16, 2);    
 
// ================== CONFIGURATION ================== 
int alcoholThreshold = 1800;      
int gateClosedPos = 180; 
int gateOpenPos   = 100; 
 
int openSpeed  = 20;   // Gate khulte waqt slow 
int closeSpeed = 8;    // Gate band hote waqt fast 
 
// Counters 
int totalVehicles = 0; 
int drunkCount    = 0; 
int safeCount     = 0; 
 
BlynkTimer timer; 
 
void setup() { 
  Serial.begin(115200); 
   
  pinMode(IR_SENSOR, INPUT); 

  pinMode(FAN, OUTPUT); 
  pinMode(BUZZER, OUTPUT); 
  digitalWrite(FAN, LOW); 
  digitalWrite(BUZZER, LOW); 
 
  Wire.begin(D2, D1); 
  Wire.setClock(100000); 
 
  gateServo.attach(SERVO_PIN, 500, 2400); 
  gateServo.write(gateClosedPos); 
 
  lcd.init(); 
  lcd.backlight(); 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("DRUNK DRIVE"); 
  lcd.setCursor(0, 1); 
  lcd.print("SYSTEM + BLYNK"); 
 
  // Connect to Blynk 
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); 
 
  Serial.println("=== System + Blynk Started ==="); 
  delay(2000); 
 
  timer.setInterval(5000L, sendDataToBlynk); 
} 
 
// Send counters to Blynk every 5 seconds 
void sendDataToBlynk() { 
  Blynk.virtualWrite(V1, totalVehicles); 
  Blynk.virtualWrite(V2, drunkCount); 
  Blynk.virtualWrite(V3, safeCount); 
} 
 
// Servo movement function 
void moveServo(int startPos, int endPos, int speedDelay) { 
  if (startPos < endPos) { 
    for (int pos = startPos; pos <= endPos; pos++) { 
      gateServo.write(pos); 
      delay(speedDelay); 
    } 
  } else { 
    for (int pos = startPos; pos >= endPos; pos--) { 
      gateServo.write(pos); 
      delay(speedDelay); 
    } 
  } 

} 
 
void loop() { 
  Blynk.run(); 
  timer.run(); 
 
  if (digitalRead(IR_SENSOR) == LOW) {   // Vehicle Detected 
     
    totalVehicles++; 
    Blynk.virtualWrite(V1, totalVehicles); 
 
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.print("VEHICLE DETECTED"); 
    lcd.setCursor(0, 1); 
    lcd.print("TESTING..."); 
 
    delay(2000);                        
 
    digitalWrite(FAN, HIGH); 
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.print("FAN ON - EXHALE"); 
     
    unsigned long startTime = millis(); 
    while (millis() - startTime < 6000) { 
      int mq3Value = analogRead(MQ3_PIN); 
      lcd.setCursor(0, 1); 
      lcd.print("VALUE: "); 
      lcd.print(mq3Value); 
      lcd.print("   "); 
       
      Blynk.virtualWrite(V0, mq3Value);     // Live Alcohol Value 
      delay(400); 
    } 
 
    digitalWrite(FAN, LOW); 
 
    int finalValue = analogRead(MQ3_PIN); 
    Blynk.virtualWrite(V0, finalValue); 
 
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.print("FINAL VALUE:"); 
    lcd.setCursor(0, 1); 
    lcd.print(finalValue); 
    delay(2000); 
 

    // Decision 
    if (finalValue > alcoholThreshold) { 
      drunkCount++; 
      Blynk.virtualWrite(V2, drunkCount); 
 
      lcd.clear(); 
      lcd.setCursor(0, 0); 
      lcd.print("ALCOHOL DETECTED"); 
      lcd.setCursor(0, 1); 
      lcd.print("ACCESS DENIED"); 
 
      digitalWrite(BUZZER, HIGH); 
      delay(4000); 
      digitalWrite(BUZZER, LOW); 
 
      moveServo(gateServo.read(), gateClosedPos, closeSpeed); 
    }  
    else { 
      safeCount++; 
      Blynk.virtualWrite(V3, safeCount); 
 
      lcd.clear(); 
      lcd.setCursor(0, 0); 
      lcd.print("ACCESS GRANTED"); 
      lcd.setCursor(0, 1); 
      lcd.print("GATE OPENING"); 
 
      moveServo(gateClosedPos, gateOpenPos, openSpeed); 
 
      lcd.setCursor(0, 1); 
      lcd.print("GATE OPEN    ");  
 
      delay(4000);                    // 4 second open 
 
      lcd.setCursor(0, 1); 
      lcd.print("GATE CLOSING "); 
       
      moveServo(gateOpenPos, gateClosedPos, closeSpeed); 
    } 
 
    delay(1000); 
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.print("SYSTEM READY"); 
    lcd.setCursor(0, 1); 
    lcd.print("NEXT VEHICLE"); 
    delay(2000); 
  } 

 
  delay(100); 
}