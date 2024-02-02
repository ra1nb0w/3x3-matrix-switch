#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h> //Include File System Headers
#include "ports.h"
#include "config.h"

#define BETWEEN(value, min, max) (value < max && value > min)

class WebServer
{
public:
    WebServer();
    ~WebServer();
    void init();
    static void handleClient();

private:
    const char *html_index_page = "/index.html";

    static void http_handle_root();
    static void http_handle_web_requests();
    static void http_handle_getPorts();
    static void http_handle_setRoutes();
    static void http_handle_getName();
    static void http_handle_setName();
    static bool loadFromSpiffs(String path);
};

extern WebServer webserver;

#endif /* _WEBSERVER_H_ */