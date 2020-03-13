/* Copyright (c) 2018 LeRoy Miller
 *  
 *  This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses>
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson
#include "SSD1306.h" //https://github.com/squix78/esp8266-oled-ssd1306
#include <TimeLib.h> 
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS D0  //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
#define TFT_DC D8  //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
#define TFT_RST -1 //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
#define TS_CS D3   //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)

// #define TFT_CS 14  //for D32 Pro
// #define TFT_DC 27  //for D32 Pro
// #define TFT_RST 33 //for D32 Pro
// #define TS_CS  12 //for D32 Pro

Adafruit_ILI9341 display = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

//Find your Latitude and Longitude here
//https://www.latlong.net/
float mylat = 50.838807;
float mylon = -0.133425;
float isslat, isslon;
int distance, number, count;
String payload;
String name[10], craft[10],risetime[5];
float duration[5];
 
const String iss = "http://api.open-notify.org/iss-now.json"; 
const String ppl = "http://api.open-notify.org/astros.json";
String pas = "http://api.open-notify.org/iss-pass.json?";

void setup() {
  Serial.begin(9600);
  display.begin();
  display.setTextSize(2);
  display.setTextColor(ILI9341_WHITE);
  display.setRotation(3);
  display.fillScreen(ILI9341_BLACK);
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  pas = pas + "lat=" + (String)mylat+"&lon="+ (String)mylon;
  display.println(pas);
  Serial.println(pas);
  delay(3000);
}

void loop() {
 
 getJson(iss);
 //Serial.println(payload);   //Print the response payload
 decodeLocJson();
 getDistance();
 issLocOLEDDisplay();
 issLocSerialDisplay();
 delay(5000);
  
 getJson(pas);
 decodePassJson();
 displayPassSerial();
 displayPassOLED();
 delay(5000);
  
 getJson(ppl);
 decodePeopleJson();
 displayPeopleSerial();
 displayPeopleOLED();
 delay(5000);    //Send a request every 30 seconds
 
}
void issLocOLEDDisplay() {
 display.fillScreen(ILI9341_BLACK);
 display.setCursor(0,0);
 display.println("The ISS is currently at: ");
 char temp[15];
 sprintf(temp, "%d.%02d,%d.%02d",(int)isslat,abs((int)(isslat*100)%100),(int)isslon,abs((int)(isslon*100)%100));
 display.setCursor(25,20);
 display.println(temp);
 char temp1[30];
 sprintf(temp1, "ISS is about %d miles", distance);
 display.setCursor(0,45);
 display.println(temp1);
 display.setCursor(30,65);
 display.println("from you. ");
 display.setCursor(12,85);
 display.println("And moving fast!!");
}

void issLocSerialDisplay() {
  Serial.print("The ISS is currently at ");
  Serial.print(isslat, 4); Serial.print(","); Serial.println(isslon,4);
  Serial.print("The ISS is about "); Serial.print(distance); Serial.println(" miles from you now.\nAnd moving fast!");
    
}

void getJson(String url) {
  
   if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
     HTTPClient http;  //Declare an object of class HTTPClient
     http.begin(url);  //Specify request destination
    int httpCode = http.GET();                                                                  //Send the request
     if (httpCode > 0) { //Check the returning code
       payload = http.getString();   //Get the request response payload
     
    }
 
    http.end();   //Close connection
 
  }
}

void decodeLocJson() {
  StaticJsonBuffer<512> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  isslat=root["iss_position"]["latitude"];
  isslon=root["iss_position"]["longitude"];
}

void getDistance() {
  float theta, dist, miles;
  theta = mylon - isslon;
  dist = sin(deg2rad(mylat)) * sin(deg2rad(isslat)) + cos(deg2rad(mylat)) * cos(deg2rad(isslat)) * cos(deg2rad(theta));
  dist = acos(dist);
  dist = rad2deg(dist);
  miles = dist * 60 * 1.1515;
  distance = miles;
}

float deg2rad(float n) {
  float radian = (n * 71)/4068;
  return radian;
}

float rad2deg(float n) {
  float degree = (n*4068)/71;
  return degree;
}

void decodePeopleJson() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  number = root["number"];
  if (number > 10) {number = 10;}
  for (int i=0;i<number; i++){
    
    const char* temp1 = root["people"][i]["name"];
    const char* temp2 = root["people"][i]["craft"];
    name[i] = (String)temp1;
    craft[i] = (String)temp2;
  }
 }

 void displayPeopleSerial() {
  Serial.print("There are "); Serial.print(number); Serial.println(" people in space right now.");
  for (int i=0;i<number; i++) {
    Serial.print(name[i]);Serial.print(" on board ");Serial.println(craft[i]);
  }
 }

 void displayPeopleOLED() {
  //display.clear();
  display.fillScreen(ILI9341_BLACK);
  char temp2[50];
  sprintf(temp2, "%d people are in space.", number);
   display.setCursor(0,0);
 display.println(temp2);

 if (number > 5) {number = 5;} //Display the 1st 5 Astros on OLED 
 for (int i=0;i<number; i++) {
  display.setCursor(0,20*(i+1));
  display.println(name[i] + ", " + craft[i]);
 }
}

void decodePassJson() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  count = root["request"]["passes"];
  
  if (count > 5) {count = 5;}
  for (int i=0;i<count; i++){
    
    unsigned int tempEpoch = root["response"][i]["risetime"];
    risetime[i] = convertEpoch(tempEpoch);
    duration[i] = root["response"][i]["duration"];
    duration[i] = duration[i] / 60;
      }
 }

 String convertEpoch(unsigned int epoch) {
  int h = hour(epoch);
  int m = minute(epoch);
  int d = day(epoch);
  int mn = month(epoch);
  int y = year(epoch);
   char temp[100];
  sprintf(temp, "RT: %d/%d %d:%d UTC",mn,d,h,m);
  return (String)temp;
 }

void displayPassSerial() {
  Serial.println("Pass Prediction");
  for (int i=0;i<count; i++) {
    Serial.print(risetime[i]); Serial.print(" [");Serial.print(duration[i]);Serial.println(" mins.]");
  }
}

 void displayPassOLED() {
  display.fillScreen(ILI9341_BLACK);
  display.setCursor(0, 0);
  display.println("Pass Prediction");

 for (int i=0;i<count; i++) {
  display.setCursor(0,20*(i+1));
  display.println(risetime[i] + " [" + (String)duration[i]+" m]");
 }

}
