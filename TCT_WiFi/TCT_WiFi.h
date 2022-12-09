#ifndef TCTWIFI_H
#define TCTWIFI_H
#include <Arduino.h>  // PlatformIO用

typedef enum { CHECK_ERROR = -1, IN_ENABLE, IN_DISABLE } Auth_Status;

IPAddress connect_TCTwifi(const char* SSID, const char* USER_NAME, const char* PASSWORD);
Auth_Status check_auth(void);
void get_auth_page(void);
Auth_Status authenticate(void);

#endif