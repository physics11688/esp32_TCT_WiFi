#include <Arduino.h>
#include "TCT_WiFi.h"
const char* SSID = "TCT802.1X";        // アクセスポイントの SSID
const IPAddress ip(192, 168, 43, 50);  // TCT内で使用したいIPアドレス. 重複していないもの使う.
// 利用可能IPアドレス: 192.168.40.2 ~ 192.168.47.254
const char* USER_NAME = "m23kadomatu";   // アカウント名: m23kadomatu とか
const char* PASSWORD  = "trumpet23234";  // パスワード: trumpet23234 とか

#include <WiFi.h>                        // 新規include
const char* host    = "133.125.37.142";  // 著者のサーバのIPアドレス
const uint16_t port = 19123;             // ポート番号


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
    Serial.print("connecting to ");
    Serial.println(host);

    WiFiClient client;  // (1) socket -> ソケットを作る (WiFiClientクラスのオブジェクトを作成)

    // (2) connect -> 接続を要求する
    if (!client.connect(host, port)) {         // 接続できなかったら
        Serial.println("Connection failed.");  // メーッセージを表示して
        Serial.println("Waiting 5 seconds before retrying...");
        delay(5000);
        return;  // リターン
    }

    // fromESP32: から始まる文字列だけ受信するようにサーバを設定しておきます
    client.print("fromESP32:AAAAAA");  // (3) write/send -> サーバにデータ送信

    uint32_t timeout = millis();  // プログラム開始からの経過時間
    /* available()の戻り値は 読み込み可能なバイト数(接続先のサーバーによってクライアントに書き込まれたデータの量) */
    while (client.available() == 0) {     // サーバからのデータが来ないならループ
        if (millis() - timeout > 5000) {  // もし 5秒以上 なら
            Serial.println(">>> Client Timeout !");
            client.stop();  // (4)(5) shutdown + close
            return;         // リターン
        }
    }

    Serial.print("From Server -> ");
    while (client.available()) {                     // サーバからのデータを受信
        String line = client.readStringUntil('\r');  // 1行読む
        Serial.print(line);
    }

    Serial.println();
    Serial.println("closing connection");
    client.stop();  // (4)(5) shutdown + close
}

void loop() { delay(1000); }