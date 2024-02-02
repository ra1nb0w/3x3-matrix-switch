/**
 * NOTE:
 *  in many parts is strictly implemented for 3x3 in/out ports
 *  therefore you should probably rewrite the webserver part
 *  and the index.html to use cycles to scan all values if you
 *  need more ports
 */



#include "Wire.h"
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include "basicOTA.h"
#include "webserver.h"
#include "ports.h"
#include "config.h"
#include "wifi.h"


void setup() {

  Serial.begin(9600);
  while (!Serial);

  ports.init();
  ports.initialize_leds();
  ports.leds_enable();
  ports.initialize_buttons();
  ports.initialize_relays();

  //Initialize File System
  SPIFFS.begin();
  Serial.println("File System Initialized");

  //Begin WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // set hostname
  WiFi.hostname("3x3-matrix");

  // Connecting to WiFi...
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  // Loop continuously while WiFi is not connected
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
  
  // Connected to WiFi
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // enable light sleep mode
  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);

  // Setup Firmware update over the air (OTA)
  setup_OTA();

  // initialize web server
  webserver.init();

  // disable all leds when everything is up
  ports.leds_disable();

  // configure the routing
  ports.make_routing();
}

void loop() {

  // Check for OTA updates
  ArduinoOTA.handle();

  // handle request from client
  webserver.handleClient();

  // can we use interrupt?
  ports.check_buttons();

}