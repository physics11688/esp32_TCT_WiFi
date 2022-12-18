/*
 * WiFiライブラリのTCT専用ラッパーライブラリ
 *
 *  connect_TCTwifi() → そのまんま
 *  check_auth()      → 認証の状態をチェックします
 *  get_auth_page()   → 認証用ページにアクセスします
 *  authenticate(void) → 実際にPOSTにして認証を行います
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
String USER           = "";                          // ユーザー名保持用
String PASS           = "";                          // パスワード保持用
const char* ipechoURL = "https://ipecho.net/plain";  // 接続先URL
HTTPClient reuse_http;                               // 使いまわし用インスタンス
String authpageURL = "";                             // 認証用ページのURL

const IPAddress subnet(255, 255, 248, 0);    // 　サブネットマスク
const IPAddress gateway(192, 168, 40, 1);    // デフォルトゲートフェイ
const IPAddress primaryDNS(192, 168, 6, 1);  // DNSサーバ


/* WiFiに実際に接続する関数 */
IPAddress connect_TCTwifi(const char* SSID, const IPAddress user_ip, const char* USER_NAME, const char* PASSWORD) {
    Serial.println();
    Serial.print("Connecting to network: ");
    Serial.println(SSID);
    WiFi.config(user_ip, gateway, subnet, primaryDNS);
    if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect(true);  // 初期化のためにwifi切断
    }
    WiFi.mode(WIFI_STA);  // 初期化

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


/* 認証済みかどうかチェックする関数. 未認証ならURLを取得する. */
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
                Serial.println("already authenticated");
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


/* 認証用ページにアクセスする関数 */
void get_auth_page() {
    reuse_http.begin(authpageURL);
    int httpCode = reuse_http.GET();

    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            String body = reuse_http.getString();
            // Serial.println(httpCode);
            // Serial.println("");
        }
    } else {
        Serial.println(reuse_http.errorToString(httpCode));
    }
    reuse_http.end();
}


/* 認証用ページにPOSTする関数 */
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


/* セキュア版で使用: HTMLのbodyを返す */
String set_html(const char* ESP32_ssid) {
    String body = "";
    body += "<html lang=\"ja\">";
    body += "<head>";
    body += "<meta charset=\"UTF-8\">";
    body += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    body = body + "<title>" + ESP32_ssid + "</title>";
    body += "</head>";
    body += "<body>";
    body += "<style>.tmp{width: fit-content;margin: auto;}</style>";
    body += "<h2 align=\"center\">Enter Your UserName and PassWord</h2>";
    body += "<form action=\"\" method=\"post\" class=\"tmp\">";
    body += "<div><label>ユーザー名<input type=\"text\" name=\"username\" required=\"required\"></label></div>";
    body += "<div><label>パスワード<input type=\"password\" name=\"password\" required=\"required\"></label></div>";
    body += "<button type=\"submit\">ESP32へ送信</button></form>";
    body += "</body>";
    body += "</html>";

    return body;
}

/* セキュア版で使用: アクセスポイントを立ててパラーメタ取得 */
void get_param(const char* ESP32_ssid, const char* AP_password) {
    const IPAddress ESP32_ip(192, 168, 21, 1);          // ESP32のIPアドレス. 固定しておく.
    const IPAddress ESP32_AP_subnet(255, 255, 255, 0);  // サブネットマスク
    WiFiServer server(80);                              // 80番ポートでリッスン
    Serial.println("Configuring access point...");

    WiFi.softAP(ESP32_ssid, AP_password);
    WiFi.softAPConfig(ESP32_ip, ESP32_ip, ESP32_AP_subnet);  // AP設定

    server.begin();  // 開始
    Serial.println("Server started\n");

    while (true) {
        WiFiClient client = server.available();  // リッスン

        if (client) {  // クライアントからconnectされたら
            Serial.println("<New Client.>");
            while (client.connected()) {  // 接続中はループ
                // 応答
                client.println("HTTP/1.1 200 OK\r\nContent-type:text/html\r\n");
                client.print(set_html(ESP32_ssid));
                while (client.available()) {  // クライアントから読めるならループ
                    String line = client.readStringUntil('\r');
                    if (line.startsWith("\nusername", 0)) {  // パース
                                                             // ex. line == \nusername=mpmdg&password=qqqqqq\r\n
                        line.replace("\r", "");
                        line.replace("\n", "");
                        line.replace("username=", "");
                        line.replace("password=", "");
                        int index = line.indexOf('&');  // ex. mpmdg&qqqqqq
                        USER      = line.substring(0, index);
                        PASS      = line.substring(index + 1, line.length());
                        break;
                    } else {
                        Serial.print(line);  // 要求ヘッダを表示
                    }
                }
                break;
            }
            // close the connection:
            Serial.println("");
            client.stop();
            Serial.println("<Client Disconnected.>\n");
        }

        if (USER != String("")) {
            break;
        }
    }
    Serial.println("<Closing access point... >\n");
    server.end();
}


/* セキュア版で使用*/
IPAddress connect_TCTwifi_Secure(const char* SSID, const IPAddress user_ip) {
    char USER_NAME[50];
    char PASSWORD[50];
    USER.toCharArray(USER_NAME, 50);
    PASS.toCharArray(PASSWORD, 50);

    return connect_TCTwifi(SSID, user_ip, USER_NAME, PASSWORD);
}
