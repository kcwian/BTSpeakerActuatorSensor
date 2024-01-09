//#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <mDNSResolver.h>
#include <ESP8266mDNS.h>
#include <time.h>
#include "index.h"

// ----------------- Server
const char* ssid = "Sprzedam Opla";
const char* password = "schnappi";
#define SERVER_NAME_ACTUATOR "btspeakeractuator1"
#define SERVER_NAME_SENSOR "btspeakersensor.local"  // To periodically ping it
IPAddress sensorIp;
WiFiUDP udp;
mDNSResolver::Resolver resolver(udp);

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

ESP8266WebServer server(80);


// ----------------- PINS
#define SPEAKER_STOP_PIN D3
#define SPEAKER_LED_PIN A0


bool userState = 1; // ON - OFF
bool musicPlaying = false;
bool checkSensor = false;

//void ICACHE_RAM_ATTR onTimerISR() {
//  musicPlaying = isMusicPlaying();
//  timer1_write(3125);// 10ms * 2
//}

void setup(void) {

  // Set LOW at the transistor's base
  // Otherwise, the speaker will not turn on
  pinMode(SPEAKER_STOP_PIN, OUTPUT);
  digitalWrite(SPEAKER_STOP_PIN, LOW);

  // LED to read when the music is played
  pinMode(SPEAKER_LED_PIN, INPUT);

  // Turn off onboard LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Set Sensors pins as input
  //  pinMode(VIBRATION_SENSOR_PIN, INPUT); // ADC Input
  //  pinMode(NOISE_SENSOR_PIN, INPUT); // ADC Input

  // Start WiFi station
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  int connectCnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    //    digitalWrite(LED_BUILTIN, 0);
    delay(100);
    //    digitalWrite(LED_BUILTIN, 1);
    delay(100);
    if (connectCnt++ > 30)
    {
      connectCnt = 0;
      WiFi.reconnect();
    }
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  if (MDNS.begin(SERVER_NAME_ACTUATOR)) {
    Serial.println("MDNS responder started");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/getState", HTTP_GET, handleGetState);
  server.on("/setState", HTTP_GET, handleSetState);
  server.on("/setCheckSensor", HTTP_GET, handleSetCheckSensor);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  MDNS.addService("http", "tcp", 80);

  delay(200);

  // Commented as it causes server to hang sometimes
  //  //Initialize Ticker every 0.5s
  //  timer1_attachInterrupt(onTimerISR);
  //  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
  //  timer1_write(3125*2); //10 *2 ms
}

void loop(void) {

  static bool isMusicPlaying8s = false; 
  
  MDNS.update();
  server.handleClient();
  // If user set music to OFF
  // But the music plays
  // Press button
  if (userState == 0 && isMusicPlaying8s == 1) {
    static unsigned long lastPress = 0;
    // Limit the time between buttons press, as the LED blinking needs time to change
    if (millis() - lastPress > 16000) {
      simulateButtonPress();
      lastPress = millis();
      Serial.println("Pressing key");
    }
  }
  static int musicCheckCnt = 0;
  if (musicCheckCnt++ > 20)
  {
    isMusicPlaying8s = isMusicPlaying();
    musicCheckCnt = 0;
    Serial.println("Music playing? : " + String(isMusicPlaying8s));
  }
  static int sensorCheckCnt = 0;
  if (checkSensor && sensorCheckCnt++ > 30000)
  {
    sensorCheckCnt = 0;
    if (isSensorResponding())
    {
      Serial.println("Sensor OK");
    }
    else
    {
      Serial.println("Sensor not found!");
      userState = 0;
    }
  }
  delay(1); // Do not change as the sensorCheckCnt will also change
}

bool isSensorResponding()
{
  static bool initalize = true;
  int initCnt = 0;
  while (initalize) {
    if (WiFi.status() == WL_CONNECTED) {
      sensorIp = resolver.search(SERVER_NAME_SENSOR);
      Serial.print("IP address of server: ");
      Serial.println(sensorIp.toString());
    }
    if (sensorIp[0] == 192)
    {
      Serial.print("Got server IP:  ");
      Serial.println(sensorIp.toString());
      initalize = false;
    }
    else
    {
      if (initCnt++ >= 4)
        return false;
    }
    delay(50);
  }

  HTTPClient http;
  WiFiClient wifiClient;
  bool success = false;
  int tryCnt = 0;
  while (true)
  {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("[HTTP] begin...\n");
      String request;
      request = "http://" + sensorIp.toString();
      if (http.begin(wifiClient, request)) { // HTTP
        // start connection and send HTTP header
        int httpCode = http.GET();
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          success = true;
        }
        http.end();
      }
    }
    if (success || tryCnt >= 9)
      break;

    tryCnt++;
    delay(500);
  }
  return success;
}

bool isMusicPlaying()
{
  bool ledState = LOW;
  int ledAnalog = analogRead(SPEAKER_LED_PIN);
  if (ledAnalog > 300)
    ledState = HIGH;
  static unsigned long lastStateChangedTime = 0;
  static bool lastState = LOW;
  static int phase = 0;
  static unsigned long lastTime = millis();
  unsigned long actualTime = millis();

  // Check if server response caused the delay
  //  Serial.println("Time diff: " + String(actualTime - lastTime));
  if ((actualTime - lastTime > 80)) {
    phase = 0;
  }
  else {
    switch (phase)
    {
      // Detect long (~1s) HIGH state
      case 0:
        if (actualTime - lastStateChangedTime > 800 && ledState == LOW)
          phase = 1;
        else if (actualTime - lastStateChangedTime > 1200 && ledState == HIGH) {
          // Happens when the request takes too long time
          phase = 0;
          musicPlaying = false;
          //          Serial.println("Music stopped, phase 0");
        }
        break;

      // Detect (500ms) LOW state
      case 1:
        if (actualTime - lastStateChangedTime > 300 && ledState == HIGH)
          phase = 2;
        else if (actualTime - lastStateChangedTime > 800) {
          phase = 0;
          musicPlaying = false;
          //          Serial.println("Music stopped, phase 1");
        }
        break;

      // Detect (100ms) HIGH state
      case 2:
        if (actualTime - lastStateChangedTime < 300 && ledState == LOW)
          phase = 3;
        else if (actualTime - lastStateChangedTime > 300) {
          phase = 0;
          musicPlaying = false;
          //          Serial.println("Music stopped, phase 2");
        }
        break;

      // Detect (500ms) LOW state
      case 3:
        if (actualTime - lastStateChangedTime > 300 && ledState == HIGH)
          phase  = 4 ;
        else if (actualTime - lastStateChangedTime > 800) {
          phase = 0;
          musicPlaying = false;
          //          Serial.println("Music stopped, phase 3");
        }
        break;

      case 4:
        musicPlaying  = true;
        phase = 0;
        break;
    }
  }

  if (ledState != lastState) {
    lastStateChangedTime = actualTime;
    lastState = ledState;
  }



  // To prevent detecting Musing when unlocking phone
  // Music must be playing for more than several seconds
  static long musicPlayingTime = 0;
  bool returnValue = false;
  if (musicPlaying)
  {
    musicPlayingTime += (actualTime - lastTime);
    if (musicPlayingTime > 8000)
      returnValue = true;
    Serial.println(musicPlayingTime);
  }
  else
    musicPlayingTime = 0;

  lastTime = actualTime;
  return returnValue;
}


void handleRoot()
{
  String s = MAIN_page;
  server.send(200, "text/html", s);
}


void handleSetState()
{
  String state_s = server.arg("state");
  if (state_s == "1") {
    userState = 1;
    //    digitalWrite(LED_BUILTIN, LOW);
  }
  else {
    userState = 0;
    //    digitalWrite(LED_BUILTIN, HIGH);
  }
  server.send(200, "text/plane", state_s);
}

void handleGetState()
{
  String response = String("state=") + String(userState) + String("<br>");
  response += String("audioState=")  + String(musicPlaying) + String("<br>");
  response += String("checkSensor=")  + String(checkSensor) + String("<br>");

  server.send(200, "text/plane", response);
}

void handleSetCheckSensor()
{
  String checkSensor_s = server.arg("checkSensor");
  if (checkSensor_s == "1") {
    checkSensor = 1;
  }
  else {
    checkSensor = 0;
  }
  server.send(200, "text/plane", checkSensor_s);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void simulateButtonPress()
{
  pinMode(SPEAKER_STOP_PIN, OUTPUT);
  digitalWrite(SPEAKER_STOP_PIN, HIGH);
  delay(100);
  digitalWrite(SPEAKER_STOP_PIN, LOW);
}

int getTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return 0;
  }
  //  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  int t_hour = timeinfo.tm_hour; // Blue
  int t_min  = timeinfo.tm_min;  // Green
  int t_sec  = timeinfo.tm_sec;  // Red
  return t_hour * 10000 + t_min * 100 + t_sec;
}
