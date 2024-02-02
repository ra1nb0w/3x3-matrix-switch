
#ifndef _CONFIG_H_
#define _CONFIG_H_

// uncomment if you want to see debug logs
//#define DEBUG_PRINTS

// web server port
#define WEBSERVER_HTTP_PORT     80

#ifdef DEBUG_PRINTS
  #define DEBUG(TEXT)         Serial.print(TEXT);
  #define DEBUGLN(TEXT)       Serial.println(TEXT);
#else
  #define DEBUG(TEXT)
  #define DEBUGLN(TEXT)
#endif

#endif /* _CONFIG_H_ */