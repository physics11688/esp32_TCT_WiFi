/*
 * WiFiライブラリのTCT専用ラッパーライブラリ
 *
 *  connect_TCTwifi() → そのまんま
 *  check_auth()      → 認証の状態をチェックします
 *  get_auth_page()   → 認証用ページにアクセスします
 *  uthenticate(void) → 実際にPOSTにして認証を行います
 *
 *  Author: physics11688 - 9.12.2022
 *
 */


#include <Arduino.h>  // PlatformIO用
#include <TCT_WiFi.h>
#include <WiFi.h>      // Wifi library
#include "esp_wpa2.h"  // wpa2 library for connections to Enterprise networks
#include <HTTPClient.h>
#define EAP_IDENTITY "login"
String USER        = "";                             // ユーザー名保持用
String PASS        = "";                             // パスワード保持用
String authpageURL = "";                             // 認証用ページのURL
HTTPClient reuse_http;                               // 使いまわし用インスタンス
const char* ipechoURL = "https://ipecho.net/plain";  // 接続先URL


IPAddress connect_TCTwifi(const char* SSID, const char* USER_NAME, const char* PASSWORD) {
    Serial.println();
    Serial.print("Connecting to network: ");
    Serial.println(SSID);

    WiFi.disconnect(true);  // 初期化のためにwifi切断
    WiFi.mode(WIFI_STA);    // 初期化

    // 接続開始
    WiFi.begin(SSID, WPA2_AUTH_PEAP, EAP_IDENTITY, USER_NAME, PASSWORD);

    int counter = 0;
    while (WiFi.status() != WL_CONNECTED) {  // 接続されていなければループ
        delay(500);
        Serial.print(".");
        counter++;
        if (counter >= 60) {  // 30秒経過したら
            ESP.restart();    // ESP32ボードをリセット
        }
    }
    Serial.print("\n");
    USER = String(USER_NAME);
    PASS = String(PASSWORD);
    return WiFi.localIP();
}

Auth_Status check_auth(void) {
    Auth_Status status = IN_DISABLE;
    reuse_http.begin(ipechoURL);
    int httpCode = reuse_http.GET();

    if (httpCode > 0) {                  // httpCode will be negative on error
        if (httpCode == HTTP_CODE_OK) {  // file found at server
            // bodyを取得してパース
            /* ex.
             * <html><body><scriptlanguage=\"JavaScript\">window.location="リダイレクト先URL";</script></body></html> */
            String body = reuse_http.getString();
            body.replace("<html><body><script language=\"JavaScript\">window.location=\"", "");
            body.replace("\";</script></body></html>", "");

            // パース後にURLから始まるなら
            if (body.charAt(0) == 'h') {
                /* ex. https://captive4.tokuyama.ac.jp:1003/fgtauth?0300ffcc95d657f9 */
                authpageURL = body;  // 未認証なのでURL格納
                // Serial.println(String("Redirect URL: " + *p_authpageURL));
                status = IN_DISABLE;
            } else {
                // Serial.println("already authenticated");
                // Serial.println(String("URL: ") + ipechoURL);
                // Serial.println(String("status code: ") + httpCode + "\n");
                // Serial.println(body);
                status = IN_ENABLE;
            }
        }
    } else {
        Serial.println(reuse_http.errorToString(httpCode));
        status = CHECK_ERROR;
    }

    reuse_http.end();
    return status;
}

void get_auth_page() {
    reuse_http.begin(authpageURL);
    int httpCode = reuse_http.GET();

    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            String body = reuse_http.getString();
            Serial.println(httpCode);
            Serial.println("");
        }
    } else {
        Serial.println(reuse_http.errorToString(httpCode));
    }
    reuse_http.end();
}

Auth_Status authenticate(void) {
    get_auth_page();
    Auth_Status status = CHECK_ERROR;
    reuse_http.begin(authpageURL);

    // パラメータの取り出し
    String magic = authpageURL;
    magic.replace("https://captive4.tokuyama.ac.jp:1003/fgtauth?", "");  // パース ex. 0300ffcc95d657f9
    // POST用のpayload
    String payload = String("4Tredir=") + ipechoURL + "&magic=" + magic + "&username=" + USER + "&password=" + PASS;

    int httpCode = reuse_http.POST(payload);  // POST
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            String body = reuse_http.getString();
            Serial.println("Authentication Successful");
            Serial.print("global IP: ");
            Serial.println(body);
            status = IN_ENABLE;
        } else {
        }
    } else {
        Serial.println(reuse_http.errorToString(httpCode));
    }
    reuse_http.end();

    return status;
}
