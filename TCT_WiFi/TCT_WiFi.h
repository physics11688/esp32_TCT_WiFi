#ifndef TCTWIFI_H
#define TCTWIFI_H
#include <Arduino.h>  // PlatformIO用
#include <HTTPClient.h>

typedef enum { CHECK_ERROR = -1, IN_ENABLE, IN_DISABLE } Auth_Status;


IPAddress connect_TCTwifi(const char* SSID, const IPAddress user_ip, const char* USER_NAME, const char* PASSWORD);
Auth_Status check_auth(void);
void get_auth_page(void);
Auth_Status authenticate(void);

String set_html(const char* ESP32_ssid);
void get_param(const char* ESP32_ssid, const char* AP_password);
IPAddress connect_TCTwifi_Secure(const char* SSID, const IPAddress user_ip);
#endif