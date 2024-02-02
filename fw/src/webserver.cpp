#include "webserver.h"

ESP8266WebServer http_socket(WEBSERVER_HTTP_PORT);

WebServer::WebServer()
{
}

WebServer::~WebServer()
{
}

void WebServer::init()
{
    //Initialize File System
    SPIFFS.begin();
    Serial.println("File System Initialized");

    // initialize web server
    http_socket.on("/", WebServer::http_handle_root);
    http_socket.on("/api/getPorts", WebServer::http_handle_getPorts);
    http_socket.on("/api/setRoutes", WebServer::http_handle_setRoutes);
    http_socket.on("/api/getName", WebServer::http_handle_getName);
    http_socket.on("/api/setName", WebServer::http_handle_setName);
    http_socket.onNotFound(WebServer::http_handle_web_requests); //Set setver all paths are not found so we can handle as per URI
    http_socket.begin();
    Serial.println();
    Serial.println("HTTP Server listening...");
}

void WebServer::handleClient()
{
    // check http requests
    http_socket.handleClient();
}

void WebServer::http_handle_root()
{
  DEBUG(millis());
  DEBUG(" > ");
  DEBUGLN(__func__);

  http_socket.sendHeader("Location", "/index.html", true); //Redirect to our html web page
  http_socket.send(302, "text/plain", "");
}

bool WebServer::loadFromSpiffs(String path)
{
  DEBUG(millis());
  DEBUG(" > ");
  DEBUG(__func__);
  DEBUGLN(path);

  String dataType = "text/plain";
  if (path.endsWith("/"))
    path += "index.html";

  if (path.endsWith(".src"))
    path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html"))
    dataType = "text/html";
  else if (path.endsWith(".htm"))
    dataType = "text/html";
  else if (path.endsWith(".css"))
    dataType = "text/css";
  else if (path.endsWith(".js"))
    dataType = "application/javascript";
  else if (path.endsWith(".png"))
    dataType = "image/png";
  else if (path.endsWith(".gif"))
    dataType = "image/gif";
  else if (path.endsWith(".jpg"))
    dataType = "image/jpeg";
  else if (path.endsWith(".ico"))
    dataType = "image/x-icon";
  else if (path.endsWith(".xml"))
    dataType = "text/xml";
  else if (path.endsWith(".pdf"))
    dataType = "application/pdf";
  else if (path.endsWith(".zip"))
    dataType = "application/zip";
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (http_socket.hasArg("download"))
    dataType = "application/octet-stream";
  if (http_socket.streamFile(dataFile, dataType) != dataFile.size())
  {
  }

  dataFile.close();
  return true;
}

void WebServer::http_handle_web_requests()
{
  DEBUG(millis());
  DEBUG(" > ");
  DEBUGLN(__func__);

  if (loadFromSpiffs(http_socket.uri()))
    return;
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += http_socket.uri();
  message += "\nMethod: ";
  message += (http_socket.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += http_socket.args();
  message += "\n";
  for (uint8_t i = 0; i < http_socket.args(); i++)
  {
    message += " NAME:" + http_socket.argName(i) + "\n VALUE:" + http_socket.arg(i) + "\n";
  }
  http_socket.send(404, "text/plain", message);
}

void WebServer::http_handle_getPorts()
{
  DEBUG(millis());
  DEBUG(" > ");
  DEBUGLN(__func__);

  // we can use array with index and so on but we keep simple
  String s;
  s  = "{ \"in1\" : \"";
  s += ports.switch_connection[1];
  s += "\", ";
  s += "\"in2\" : \"";
  s += ports.switch_connection[2];
  s += "\", ";
  s += "\"in3\" : \"";
  s += ports.switch_connection[3];
  s += "\"";
  s += "}";

  http_socket.send(200, "application/json", s);
}

void WebServer::http_handle_setRoutes()
{
  DEBUG(millis());
  DEBUG(" > ");
  DEBUGLN(__func__);

  uint8_t in1 = BETWEEN(http_socket.arg("in1").toInt(), 0, OUTPUT_PORTS+1) ? http_socket.arg("in1").toInt() : 0;
  uint8_t in2 = BETWEEN(http_socket.arg("in2").toInt(), 0, OUTPUT_PORTS+1) ? http_socket.arg("in2").toInt() : 0;
  uint8_t in3 = BETWEEN(http_socket.arg("in3").toInt(), 0, OUTPUT_PORTS+1) ? http_socket.arg("in3").toInt() : 0;
  uint8_t save = http_socket.arg("save").toInt() == 1 ? 1 : 0;

  if (in1 != in2 && in2 != in3 && in3 != in1) {
    ports.set_inout(in1, in2, in3, save);
    http_socket.send(200, "text/plain", "");
  }
  else
    http_socket.send(400, "text/plain", "parameters out of bound or equal to each other. check.");
}

void WebServer::http_handle_getName()
{
  DEBUG(millis());
  DEBUG(" > ");
  DEBUGLN(__func__);

  // should be dynamic based on INPUT_PORTS/OUTPUT_PORTS
  // but to simplify
  String s;
  s  = "{ ";
  s += "\"in1\" : \"";
  s += String(ports.in_port_name[1]);
  s += "\", ";
  s += "\"in2\" : \"";
  s += String(ports.in_port_name[2]);
  s += "\", ";
  s += "\"in3\" : \"";
  s += String(ports.in_port_name[3]);
  s += "\", ";
  s += "\"out1\" : \"";
  s += String(ports.out_port_name[1]);
  s += "\", ";
  s += "\"out2\" : \"";
  s += String(ports.out_port_name[2]);
  s += "\", ";
  s += "\"out3\" : \"";
  s += String(ports.out_port_name[3]);
  s += "\" ";
  s += "}";

  http_socket.send(200, "application/json", s);
}

void WebServer::http_handle_setName()
{
  DEBUG(millis());
  DEBUG(" > ");
  DEBUGLN(__func__);

  String in1 = BETWEEN(http_socket.arg("in1").length(), 0, 16) ? http_socket.arg("in1") : "0";
  String in2 = BETWEEN(http_socket.arg("in2").length(), 0, 16) ? http_socket.arg("in2") : "0";;
  String in3 = BETWEEN(http_socket.arg("in3").length(), 0, 16) ? http_socket.arg("in3") : "0";;
  String out1 = BETWEEN(http_socket.arg("out1").length(), 0, 16) ? http_socket.arg("out1") : "0";;
  String out2 = BETWEEN(http_socket.arg("out2").length(), 0, 16) ? http_socket.arg("out2") : "0";;
  String out3 = BETWEEN(http_socket.arg("out3").length(), 0, 16) ? http_socket.arg("out3") : "0";;


  if (in1.compareTo("0") == 0 || in2.compareTo("0") == 0 || in3.compareTo("0") == 0 ||
      out1.compareTo("0") == 0 || out2.compareTo("0") == 0 || out3.compareTo("0") == 0)
      http_socket.send(400, "text/plain", "The name is too short or too long. The maximum length is 15 charactes");
  else {
    ports.set_port_name(in1, in2, in3, out1, out2, out3);
    http_socket.send(200, "text/plain", "");
  }
}

// declare the object
WebServer webserver;