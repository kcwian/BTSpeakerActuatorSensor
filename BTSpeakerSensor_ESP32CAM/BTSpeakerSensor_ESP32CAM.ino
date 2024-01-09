// turn on debug messages
//#define VERBOSE
//#include "EloquentSurveillance.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "index_config.h"

/**
   Replace with your WiFi credentials
*/
#define WIFI_SSID "Sprzedam Opla"
#define WIFI_PASS "schnappi"
#define WIFI_SSID2 "AndroidAP5"
#define WIFI_PASS2 "yaog8837"
#define SERVER_NAME_SENSOR "btspeakersensor2"
#define SERVER_NAME_ACTUATOR1 "btspeakeractuator1"
#define SERVER_NAME_ACTUATOR2 "btspeakeractuator2"
#define PIR_SENSOR_PIN 12

bool actuatorFound[2] = {false, false};
bool actuatorState[2] = {false, false};
/**
   80 is the port to listen to
   You can change it to whatever you want, 80 is the default for HTTP
*/
//EloquentSurveillance::StreamServer streamServer(80);
//EloquentSurveillance::Motion motion;
WiFiClient wifiClient;
IPAddress actuatorIp[2];

WebServer server(80);
int delayIntervalMin = 1;
long delaySetTime;

void setup() {

  delay(1000);
  Serial.begin(115200);
  //  debug("INFO", "Init");
  pinMode(PIR_SENSOR_PIN, INPUT_PULLDOWN);
  /**
     See CameraCaptureExample for more details
  */
  //  camera.aithinker();
  //  camera.highQuality();
  //  camera.hd();
  //
  //  motion.setMinChanges(0.05);
  //  motion.setMinPixelDiff(10);
  //  motion.setMinSizeDiff(0.02);
  /**
     Initialize the camera
     If something goes wrong, print the error message
  */
  //  while (!camera.begin())
  //  {
  //    debug("ERROR", camera.getErrorMessage());
  //    ESP.restart();
  //  }

  /**
     Connect to WiFi
     If something goes wrong, print the error message
  */
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int connectCnt = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    if (connectCnt++ > 30)
    {
      connectCnt = 0;
      WiFi.reconnect();
    }
  }

  if (MDNS.begin(SERVER_NAME_SENSOR)) {
    Serial.println("MDNS responder started");
  }
  actuatorIp[0] = checkActuatorIP(SERVER_NAME_ACTUATOR1, actuatorFound[0]);
  actuatorIp[1] = checkActuatorIP(SERVER_NAME_ACTUATOR2, actuatorFound[1]);
  // Enable if found
  if (actuatorFound[0])
    actuatorState[0] = true;

  if (actuatorFound[1])
    actuatorState[1] = true;

  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  server.on("/getState", HTTP_GET, handleGetState);
  server.on("/setActuator1State", HTTP_GET, handleSetActuator1State);
  server.on("/setActuator2State", HTTP_GET, handleSetActuator2State);
  server.on("/setDelayInterval", HTTP_GET, handleSetDelayInterval);
  server.onNotFound(handleNotFound);
  server.begin();

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
  /**
     Initialize stream web server
     If something goes wrong, print the error message
  */
  //  while (!streamServer.begin())
  //    debug("ERROR", streamServer.getErrorMessage());

  /**
     Display address of stream server
  */
  //  debug("SUCCESS", streamServer.getWelcomeMessage());
  delaySetTime = millis();
}


void loop() {
  server.handleClient(); // For ping from actuator

  static unsigned long cnt = 0;
  cnt++;
  static bool cameraDetected = false;
  static bool pirDetected = false;
  static unsigned long lastTimeSpeakerOff = millis();
  static int wifiCheckCnt =0;

  if(wifiCheckCnt++ > 200)
  {
    wifiCheckCnt = 0;
    checkWifiStatusAndReconnect();
  }
  //  if (!camera.capture()) {
  //    debug("ERROR", camera.getErrorMessage());
  //    static int errors = 0;
  //    if (errors++ > 4)
  //      ESP.restart();
  //  }


  if (millis() - delaySetTime < delayIntervalMin * 60 * 1000)
    return;

  Serial.println("Checking started");

  // Check the PIR sensor
  if (digitalRead(PIR_SENSOR_PIN))
    pirDetected = true;

  // Handle camera / PIR sensor
  if (pirDetected) {
    //    if (cameraDetected)
    //          Serial.println("Camera Detected");
    //    if (pirDetected)
    //      Serial.println("PIR Detected");

    cameraDetected = pirDetected = false;
    unsigned long actualTime = millis();
    if (actualTime - lastTimeSpeakerOff > 5000) {
      //      while (!turnSpeakerOff(actuatorIp[1]));
      bool actuatorIsOff[2] = {false, false};
      bool actuatorShouldBeTrunedOff[2] = {false, false};
      while (true)
      {
        for (int i = 0; i < 2; i++)
        {
          if (actuatorState[i] && actuatorFound[i] && !actuatorIsOff[i]) {
            actuatorShouldBeTrunedOff[i] = true;
            actuatorIsOff[i] = turnSpeakerOff(actuatorIp[i]);
            Serial.println("Turning off actuator " + String(i));
          }
        }
        // If actuator1 should be off butits not, or if the actuator2 should be off but its not
        if ( (actuatorShouldBeTrunedOff[0] && !actuatorIsOff[0]) || (actuatorShouldBeTrunedOff[1] && !actuatorIsOff[1]))
          continue;
        else
        {
          Serial.println("Request finished");
          lastTimeSpeakerOff = actualTime;
          break;
        }
      }
    }
  }

  /**
     Look for motion.
     In the `true` branch, you can handle a motion event.
     For the moment, just print the processing time for motion detection.
    //     */
  //  if (!motion.update())
  //    return;
  //
  //  // Code below is called only when motion detected
  //
  //  if (motion.detect()) {
  //    debug("INFO", String("Motion detected in ") + motion.getExecutionTimeInMicros() + " us");
  //    cameraDetected = true;
  //  }
  //  else if (!motion.isOk()) {
  //    /**
  //       Something went wrong.
  //       This part requires proper handling if you want to integrate it in your project
  //       because you can reach this point for a number of reason.
  //       For the moment, just print the error message
  //    */
  //    debug("ERROR", motion.getErrorMessage());
  //  }
}

void checkWifiStatusAndReconnect()
{
  if (WiFi.status() != WL_CONNECTED)
  
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int connectCnt = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    if (connectCnt++ > 30)
    {
      connectCnt = 0;
      WiFi.reconnect();
    }
  }
}
IPAddress checkActuatorIP(String actIP, bool & found)
{
  found = false;
  IPAddress resultIp;
  if (WiFi.status() == WL_CONNECTED) {
    resultIp = MDNS.queryHost(actIP);
    Serial.print("IP address of server: ");
    Serial.println(resultIp.toString());
  }
  if (resultIp[0] == 192)
  {
    Serial.print("Got server IP:  ");
    Serial.println(resultIp.toString());
    found = true;
  }

  return resultIp;
}

bool turnSpeakerOff(IPAddress actuatorIp)
{
  HTTPClient http;
  bool success = false;
  static bool state = 0;
  //  state ^= 1;
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[HTTP] begin...\n");
    String request;
    //    if (state)
    //      request = "http://" + actuatorIp.toString() + "/setState?state=1";
    //    else
    request = "http://" + actuatorIp.toString() + "/setState?state=0";

    Serial.printf("Request ");
    Serial.print(request);
    if (http.begin(request)) { // HTTP
      // start connection and send HTTP header
      int httpCode = http.GET();
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        if (payload == String(state))
          success = true;
      }
      http.end();
    }
  }
  return success;
}

void handleGetState()
{
  String response = String("actuator1Found=") + String(actuatorFound[0]) + String("<br>");
  response += String("actuator1State=")  + String(actuatorState[0]) + String("<br>");
  response += String("actuator2Found=") + String(actuatorFound[1]) + String("<br>");
  response += String("actuator2State=")  + String(actuatorState[1]) + String("<br>");
  response += String("delayInterval=")  + String(delayIntervalMin) + String("<br>");

  server.send(200, "text/plane", response);
}

// Web server for ping
void handleRoot() {
  server.send(200, "text/plain", "OK");
  Serial.println("Got pinged");
}

void handleConfig() {
  String s = CONFIG_page;
  server.send(200, "text/html", s);
}

void handleNotFound() {
  server.send(404, "text/plain", "");
}


void handleSetDelayInterval()
{
  String delay_s = server.arg("delay");
  delayIntervalMin = delay_s.toInt();
  if (delayIntervalMin < 0)
    delayIntervalMin = 0;
  delaySetTime = millis();

  Serial.println("Delay interval set to: " + String(delayIntervalMin));
  server.send(200, "text/plane", delay_s);
}

void handleSetActuator1State()
{
  int actuatorNum = 0;
  String state_s = server.arg("state");
  if (state_s == "1") {
    // Check its IP;
    actuatorIp[actuatorNum] = checkActuatorIP(SERVER_NAME_ACTUATOR1, actuatorFound[actuatorNum]);
    if (actuatorFound[actuatorNum] == true)
      //Enable the actuator
      actuatorState[actuatorNum] = 1;
    else
      actuatorState[actuatorNum] = false;
  }
  else {
    actuatorState[actuatorNum] = 0;
  }
  server.send(200, "text/plane", String(actuatorState[actuatorNum] * 1 ));
}

void handleSetActuator2State()
{
  int actuatorNum = 1;
  String state_s = server.arg("state");
  if (state_s == "1") {
    // Check its IP;
    actuatorIp[actuatorNum] = checkActuatorIP(SERVER_NAME_ACTUATOR2, actuatorFound[actuatorNum]);
    if (actuatorFound[actuatorNum] == true)
      //Enable the actuator
      actuatorState[actuatorNum] = 1;
    else
      actuatorState[actuatorNum] = false;
  }
  else {
    actuatorState[actuatorNum] = 0;
  }
  server.send(200, "text/plane", String(actuatorState[actuatorNum] * 1 ));
}
