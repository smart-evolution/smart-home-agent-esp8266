#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

typedef struct WiFiCredentials {
    char *ssid;
    char *pass;
} WiFiCredentials;
 
ESP8266WebServer server(80);
const char* ssid = "ESPWebServer";
const char* password = "12345678";
int retries = 10;

String readString;
 
void setup() {
  EEPROM.begin(512);
  Serial.begin(9600);
  
  WiFi.begin("", "");
 
  /*while (WiFi.status() != WL_CONNECTED || retries > 0) {
    delay(500);
    retries--;
  }*/

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP);           //Only Access point
    WiFi.softAP(ssid, password);  //Start HOTspot removing password will disable security
    // IPAddress myIP = WiFi.softAPIP(); 
  }
 
  server.on("/", handleRootPath);   
  server.on("/api", handleApiPath);
  server.on("/config", handleConfigPath);
  
  server.begin();        
}
 
void loop() {
  if (Serial.available()) {
    char c = Serial.read();  
    readString += c;
  }
 
  server.handleClient();
}

void handleRootPath() {
  server.send(200, "text/plain", "Hardware: agent-type-1");
}

void handleApiPath() {
  server.send(200, "text/plain", readString);
  readString = "";
}

void handleConfigPath() {
  const String ssid = server.arg("ssid");
  const String pass = server.arg("pass");

  if (ssid != "" && pass != "") {
    for(int n = 0; n <= ssid.length() + pass.length() + 1; n++) {
      char sign = ' ';
  
      if (n < ssid.length()) {
        sign = ssid[n];
      } else if (n == ssid.length()) {
        sign = ':';
      } else if (n > ssid.length() && n <= ssid.length() + pass.length()) {
        sign = pass[n - (ssid.length() + 1)];
      } else if (n == ssid.length() + pass.length() + 1) {
        sign = ';';
      }
  
      EEPROM.write(n, sign);
    }
    EEPROM.commit();
    server.send(200, "text/plain", "Persisted [" + ssid + "|" + pass + "]");
  } else {
    char* ssid;
    char* pass;
    getWiFiCredentials(&ssid, &pass); 
    server.send(200, "text/plain", "[" + String(ssid) + "|" + String(pass) + "]");
  }
}

void getWiFiCredentials(char** ssid, char** pass) {
  char currChar;
  char charBuff[100];
  unsigned int totalLen = 0;
  unsigned int ssidLen = 0;
  unsigned int passLen = 0;
  unsigned int separatorIndex = 0;
  
  while(totalLen < 100) {
    currChar = char(EEPROM.read(totalLen));

    if (currChar == ';') {
      break;
    }
    
    charBuff[totalLen] = currChar;

    if (currChar == ':') {
      separatorIndex = totalLen;
    }
    
    totalLen++;
  }

  *ssid = (char*)malloc(sizeof(char) * (separatorIndex + 1));       // abc:123 (4)
  *pass = (char*)malloc(sizeof(char) * (totalLen - separatorIndex + 1));// 0123456 tL = 7 (4

  for(int i = 0; i < separatorIndex; i++) {
    (*ssid)[i] = charBuff[i];
  }
  (*ssid)[separatorIndex] = '\0';
  for(int i = separatorIndex + 1; i <= totalLen + 1; i++) {
    (*pass)[i - (separatorIndex + 1)] = charBuff[i];
  }
  (*pass)[totalLen + 1] = '\0';
}

