#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <time.h>
#include "index.h"

// ----------------- Server
const char* ssid = "Sprzedam Opla";
const char* password = "schnappi";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

WiFiClient server(80);

// Data for logging
#define LOG_LENGHT 200
int logData[LOG_LENGHT][2];

// ----------------- PINS
#define VIBRATION_SENSOR_PIN  39
#define NOISE_SENSOR_PIN 34

bool state = 0; // ON - OFF
int audioLevel = 123;

void setup(void) {

  // Turn off onboard LED
  pinMode(LED_BUILTIN, OUTPUT);
  state = 0 ;

  // Set Sensors pins as input
  pinMode(VIBRATION_SENSOR_PIN, INPUT); // ADC Input
  pinMode(NOISE_SENSOR_PIN, INPUT); // ADC Input

  // ADC
  // analogReadResolution(9);
  // analogSetCycles(8);
  // analogSetSamples(1);

  // Start WiFi station
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  int connectCnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, 0);
    delay(100);
    digitalWrite(LED_BUILTIN, 1);
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

  if (MDNS.begin("BTSpeakerActuator")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/getState", HTTP_GET, handleGetState);
  server.on("/setState", HTTP_GET, handleSetState);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  delay(2000);
}

void loop(void) {

  server.handleClient();
  delay(1);
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
    state = 1;
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else {
    state = 0;
    digitalWrite(LED_BUILTIN, LOW);

  }
  server.send(200, "text/plane", state_s);
}

void handleGetState()
{
  String response = String("state=") + String(state) + String("<br>");
  response += String("audioLevel=")  + String(audioLevel) + String("<br>");

  server.send(200, "text/plane", response);
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

int getTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return 0;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  int t_hour = timeinfo.tm_hour; // Blue
  int t_min  = timeinfo.tm_min;  // Green
  int t_sec  = timeinfo.tm_sec;  // Red
  return t_hour * 10000 + t_min * 100 + t_sec;
}
