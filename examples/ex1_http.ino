#include <Arduino.h>
#include "TCT_WiFi.h"

const char* SSID = "TCT802.1X";        // アクセスポイントの SSID
const IPAddress ip(192, 168, 43, 50);  // TCT内で使用したいIPアドレス. 重複していないもの使う.
// 利用可能IPアドレス: 192.168.40.2 ~ 192.168.47.254

const char* USER_NAME = "m23kadomatu";   // アカウント名: m23kadomatu とか
const char* PASSWORD  = "trumpet23234";  // パスワード: trumpet23234 とか

#include <HTTPClient.h>  // 新規include
HTTPClient http;
const char* TEST_URL = "https://arduinobook.stradty.com/";


void setup() {
    Serial.begin(115200);
    delay(10);

    // wifi接続. この時点ではLANには参加してるけど外には出れない
    // connect_TCTwifiは 30秒待ってもLANに参加できないときは再起動する
    IPAddress localIP = connect_TCTwifi(SSID, ip, USER_NAME, PASSWORD);
    Serial.print("local IP: ");
    Serial.println(localIP);

    // 認証チェック
    int counter = 0;                     // カウンタ
    while (check_auth() != IN_ENABLE) {  // 認証されていなければループ
        authenticate();                  // 認証開始
        delay(500);
        Serial.print(".");
        counter++;
        if (counter >= 20) {  // 10秒経過したら
            ESP.restart();    // ESP32ボードをリセット
        }
    }
    Serial.println("");


    /********** ここから自分のやりたい処理を書く **************/
    Serial.println(String("GET: ") + TEST_URL);
    http.begin(TEST_URL);
    int httpCode = http.GET();
    if (httpCode > 0) {                  // httpCode will be negative on error
        if (httpCode == HTTP_CODE_OK) {  // file found at server
            String body = http.getString();
            Serial.println(body);
        }
    } else {
        Serial.println(http.errorToString(httpCode));
    }
}

void loop() { delay(1000); }