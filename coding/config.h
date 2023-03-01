#include <Servo.h>
Servo myservo;

#define SW1     3
#define SW2     5
#define SW3     7
#define SW4     9
#define SW5    11
#define SW6    13
#define RAIN   A13
#define GAS    A15
#define SERVO   6
#define RL1    36
#define RL2    38
#define RL3    40
#define RL4    42 // FAN
#define RL5    44
#define BUZZ   34
#define PWR     2
#define Servo_ON    myservo.write(60);
#define Servo_OFF   myservo.write(115);

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#include "DHT.h"
DHT dht(12, DHT11);

#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN   8
#define SS_PIN    10
MFRC522 mfrc522(SS_PIN, RST_PIN);

#include "Arduino.h"
#include "uTimerLib.h"

byte degree[8] = {
  0B01110,
  0B01010,
  0B01110,
  0B00000,
  0B00000,
  0B00000,
  0B00000,
  0B00000
}; //trien khai lcd duoi dang I2C
  
void tich(int _x=1, int _dl=100)
{
  while(_x--)
  {
    digitalWrite(BUZZ,1); delay(_dl);
    digitalWrite(BUZZ,0); delay(_dl);
  }
}
void pinInit()
{
  pinMode(BUZZ,OUTPUT); tich();
  lcd.begin();
  lcd.backlight();
  lcd.createChar(1, degree);
  
  pinMode(PWR,OUTPUT); digitalWrite(PWR,1); delay(500); digitalWrite(PWR,0); delay(2000);
  pinMode(SW1,INPUT); pinMode(SW2,INPUT); pinMode(SW3,INPUT);
  pinMode(SW4,INPUT); pinMode(SW5,INPUT); pinMode(SW6,INPUT);
  pinMode(RL1,OUTPUT); pinMode(RL2,OUTPUT); pinMode(RL3,OUTPUT);
  pinMode(RL4,OUTPUT); pinMode(RL5,OUTPUT);
  
  dht.begin();
  
  Serial1.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  myservo.attach(SERVO);
  Servo_OFF;
}

String guidulieu(String lenh, const long thoigian, bool debug=true) 
{
  String chuoigiatri = "";
  Serial1.print(lenh);
  long int time = millis();
 
  while ((time + thoigian) > millis()) {
    while (Serial1.available()) {
      char c = Serial1.read(); // read the next character.
      chuoigiatri += c;
      if (debug==true && (chuoigiatri.indexOf("OK")!=-1 || chuoigiatri.indexOf("ERROR")!=-1))
      {
        Serial.println(chuoigiatri);
        return chuoigiatri;
      }
    }
  }
 
  Serial.println(chuoigiatri);
  return chuoigiatri;
}
void simInit()
{
 resetConfig:
  lcd.clear(); lcd.print("   Waiting...   ");
  tich();
  guidulieu("AT+CHTTPDISCON=0\r\n",1000); delay(1500);
  guidulieu("AT+CHTTPDESTROY=0\r\n",1000); delay(1500);
  String s1 = guidulieu("AT+CSQ\r\n",1000);
  unsigned int d=0;
  while(s1.indexOf("+CSQ: 0,0")!=-1 || s1.indexOf("+CSQ: 99,99")!=-1)
  {
    lcd.setCursor(6,1); lcd.print(d);
    delay(3000);
    s1 = guidulieu("AT+CSQ\r\n",1500);
    d++;
    tich();
  }
  /////////////////////////////////////////////////////////////////////
  lcd.clear(); lcd.print("Connect Server..");
  tich();
  delay(1000);
  String s3 = guidulieu("AT+CHTTPCREATE=\"https://blynk.cloud/external/api/\"\r\n",10000);
  if (s3.indexOf("OK")==-1) 
  {
    tich(1,1000);
    goto resetConfig;
  }
  delay(1000);
  Serial1.end(); Serial1.begin(9600); // reset buffer Serial1
  String ss2 = guidulieu("AT+CHTTPCON=0\r\n",60000);
  if (ss2.indexOf("OK")==-1) 
  {
    tich(1,1000);
    goto resetConfig;
  }
  else
  {
    tich(3,100);
  }
  lcd.clear(); lcd.print("DONE");
  delay(1000);
}

String conv (byte *buffer, byte bufferSize) 
{
  String ss="";
  for (byte i = 0; i < bufferSize; i++) {
    ss+=String(buffer[i] < 0x10 ? "0" : "");
    ss+=String(buffer[i], HEX);
  }
  ss.toUpperCase();
  return ss;
}
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
