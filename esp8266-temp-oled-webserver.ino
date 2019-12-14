#include "ESP8266WiFi.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WebServer.h>
#include <Fonts/FreeSans24pt7b.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 5 //PIN ZA TERMOMETAR DATA
#define OLED_SDA 13 //MOSI SDA PIN ZA OLED
#define OLED_SCL 14 //CLK SCL PIN ZA OLED
#define OLED_DC 2 //DC HARDWARE SPI, NE SPAJA SE
#define OLED_CS 12 //CS HARDWARE SPI, NE SPAJA SE
#define OLED_RESET 0 //RES
#define RELAY 3 //PIN ZA RELEJ

const char* ssid = ""; //NAZIV WIFI MREŽE
const char* password = ""; //LOZINKA ZA WIFI
const long pollTempInterval = 1000; //INTERVAL CITANJA SENZORA TEMPERATURE
String displayedTempC = "n/a"; //PLACEHOLDER ZA PODATAK SA TERMOMETRA
float temp; //PLACEHOLDER ZA DECIMALNI OBLIK PODATKA SA TERMOMETRA
unsigned long previousMillis;
const char* PARAM_INPUT_1 = "setpoint1"; //PLACEHOLDER ZA URL QUERY PARAMETAR 
const char* PARAM_INPUT_2 = "setpoint2"; //PLACEHOLDER ZA URL QUERY PARAMETAR 

// KONSTRUKTOR ZA OLED EKRAN
Adafruit_SSD1306 display(OLED_SDA,OLED_SCL, OLED_DC, OLED_RESET, OLED_CS); 
// KONSTRUKTOR ZA TERMOMETAR
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
// KONSTRUKTOR ZA HTTP SERVER
ESP8266WebServer server(80);

// SPAJANJE NA WIFI I PRIKAZ IP ADRESE
void setupWiFi(){
  Serial.println("WIFI SPAJANJE ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WIFI SPOJEN ");
  Serial.println(WiFi.localIP());
}
// SOFTWARE STOPERICA
bool updateInterval(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= pollTempInterval) {
    previousMillis = currentMillis;
    return true;
  }else{
    return false;
  }
}
// ODGOVOR NA HTTP / ZAHTJEV
void handleRoot() {
  display.fillRect(122, 0, 5, 5, 1);
  display.display();
  server.send(200, "text/plain", "Temperatura: " + displayedTempC + (char)176 + "C");
}
// ODGOVOR NA HTTP /temp ZAHTJEV
void handleNumberOnly() {
  display.fillRect(122, 0, 5, 5, 1);
  display.display();
  server.send(200, "text/plain", displayedTempC);
}
// 404 ODGOVOR
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
// PARAMETRIRANJE ISPISA NA EKRANU
void setupDisplay(){
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setFont(&FreeSans24pt7b);
  display.display();  
}
// PARAMETRIRANJE ARDUINA ( POKRETANJE )
void setup() {
  Serial.begin(115200);
  setupWiFi();
  setupDisplay();
  sensors.begin();
  Serial.println("SENZOR POKRENUT");
  server.on("/", handleRoot);
  server.on("/temp", handleNumberOnly);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP SERVER POKRENUT");
  pinMode(RELAY, OUTPUT); //definiranje relej pina kao izlaz
  Serial.println("RELEJ DEFINIRAN");
}
// PRIKAZ IP ADRESE NA EKRANU
void displayIP(){
  display.setFont();
  display.setCursor(0,0);
  display.print(WiFi.localIP());
}
// PRIKAZ TEMPERATURE NA EKRANU
void displayTemp(String celsius){
  display.clearDisplay();
  displayIP();
  display.setFont(&FreeSans24pt7b);
  display.setCursor(6,60);    
  display.print(celsius);
  display.display();  
}
// KONSTANTNA PETLJA ( NORMALNI RAD )
void loop() {
  server.handleClient();
  if(updateInterval()){
    sensors.requestTemperatures();
    displayedTempC = sensors.getTempCByIndex(0);
    temp = displayedTempC.toFloat() ;
    displayTemp(displayedTempC);
    if ( temp > 30 ){           
    // napraviti dinamičku izmjenu pomoću url query parametara
    digitalWrite( RELAY, HIGH);
    }
    if ( temp < 25 ){
    // napraviti dinamičku izmjenu pomoću url query parametara
    digitalWrite( RELAY, LOW);
    }
    // dodati još 2 setpointa
  }  
}
