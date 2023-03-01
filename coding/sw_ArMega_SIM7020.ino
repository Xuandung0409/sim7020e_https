
#include "config.h"
const String token="RilhWV5UCyT8TTkY7IH0LBhQaflJI32d";

float temp=0, humi=0;
int rainVal=0, gasVal=0, cuaVal=0;
int rainThres=600, gasThres=400;
bool gasBUZZ=false;
char str[20];
int tt1=0,tt2=0,tt3=0,tt4=0,tt5=0;
int ttget1=0,ttget2=0,ttget3=0,ttget5=0;
int ck1=0,ck2=0,ck3=0, ck5=0;
unsigned long tims=0;
int cntTims=0;

void timerFunc()
{
//-----------------------------------------
  gasVal  = analogRead(GAS);
  if (gasVal > gasThres)
  {
    tt4 = 1; // relay4 ON ~~ FAN
    gasBUZZ=true; // turnON BUZZ Warning 
  }
  if (gasVal < gasThres-50)
  {
    tt4 = 0; // relay4 OFF ~~ FAN
    gasBUZZ=false;
  }
//-----------------------------------------
  rainVal = analogRead(RAIN);
  if (rainVal > rainThres && ck5==0)    {tt5 = 1; ck5=1;} // relay5 ON
  if (rainVal < rainThres-50 && ck5==1) {tt5 = 0; ck5=0;}// relay5 OFF
//-----------------------------------------  
  if (gasBUZZ) digitalWrite(BUZZ,!digitalRead(BUZZ));
  else digitalWrite(BUZZ,0);
//-----------------------------------------
//-----------------------------------------
//----------------------------------------- 
  if (digitalRead(SW1)==1 && ck1==0) ck1=1;
  if (digitalRead(SW2)==1 && ck2==0) ck2=1;
  if (digitalRead(SW3)==1 && ck3==0) ck3=1;
  
  if (digitalRead(SW1)==0 && ck1==1) {tt1=1-tt1; ck1=0;}
  if (digitalRead(SW2)==0 && ck2==1) {tt2=1-tt2; ck2=0;}
  if (digitalRead(SW3)==0 && ck3==1) {tt3=1-tt3; ck3=0;}

  digitalWrite(RL1,tt1);
  digitalWrite(RL2,tt2);
  digitalWrite(RL3,tt3);
  digitalWrite(RL4,tt4);
  digitalWrite(RL5,tt5);
}

void setup()
{
  Serial.begin(9600);
  pinInit();
  simInit();
  TimerLib.setInterval_us(timerFunc, 100000);
  
}

void loop()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h)) humi=h;
  if (!isnan(t)) temp=t;
    lcd.setCursor(0,0); lcd.print("NhietDo: ");
    lcd.print(temp); lcd.write(1); lcd.print("C ");
    lcd.setCursor(0,1); lcd.print("Do am:   ");
    lcd.print(humi); lcd.print("% ");

  
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) 
  {
    String rfid = conv(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println(rfid);
    if (rfid == "CA6D5219") // thay đổi mã thẻ
    {
      tich(2,100);
      Servo_ON; cuaVal=1;
      guidulieu("AT+CHTTPSEND=0,0,\"/batch/update?token=" + token + "&v9=" + String(cuaVal) + "\"\r\n",2000);
      delay(5000);
      tich(2,100);
      Servo_OFF; cuaVal=0;
      guidulieu("AT+CHTTPSEND=0,0,\"/batch/update?token=" + token + "&v9=" + String(cuaVal) + "\"\r\n",2000);
      delay(1000);
    }
    else
    {
      tich(1,1000);
    }
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    tims = millis();
  }

  if (millis()-tims>3000)
  {
    cntTims++;
    if (cntTims==1)// update data
    {
      Serial1.end(); Serial1.begin(9600);
      String dat = "&v5=" + String(temp,1) + "&v6=" + String(humi,1);
      if (tt1 != ttget1) 
      {
        ttget1 = tt1;
        dat += "&v0=" + String(tt1);
      }
      if (tt2 != ttget2) 
      {
        ttget2 = tt2;
        dat += "&v1=" + String(tt2);
      }
      if (tt3 != ttget3) 
      {
        ttget3 = tt3;
        dat += "&v2=" + String(tt3);
      }
      if (tt5 != ttget5) 
      {
        ttget5 = tt5;
        dat += "&v4=" + String(tt5);
      }
      guidulieu("AT+CHTTPSEND=0,0,\"/batch/update?token=" + token + dat + "\"\r\n",2000);
    }
    else if (cntTims==2) // get Value
    {
      cntTims=0;
      Serial1.end(); Serial1.begin(9600);
      String rev = guidulieu("AT+CHTTPSEND=0,0,\"/get?token=" + token + "&v0&v1&v2&v4\"\r\n",2500, false);
      if (rev.indexOf("+CHTTPNMIC")!=-1)
      {
        int st = rev.indexOf("+CHTTPNMIC: 0,0,29,29");
        rev = rev.substring(st);
        if (rev.length()>=80)
        {
          int stt1 = rev.substring(34,36).toInt()-30;
          int stt2 = rev.substring(48,50).toInt()-30;
          int stt3 = rev.substring(62,64).toInt()-30;
          int stt5 = rev.substring(76,78).toInt()-30;
          Serial.println();
          Serial.print(stt1);
          Serial.print(stt2);
          Serial.print(stt3);
          Serial.print(stt5);
          Serial.println();
          if (stt1 != ttget1) {ttget1=stt1; tt1=stt1;}
          if (stt2 != ttget2) {ttget2=stt2; tt2=stt2;}
          if (stt3 != ttget3) {ttget3=stt3; tt3=stt3;}
          if (stt5 != ttget5) 
          {
            ttget5=stt5; tt5=stt5;
            if (analogRead(RAIN) > rainThres) ck5=1; else ck5=0;
          }
        }
      }
    }
    tims = millis();
  }
}
