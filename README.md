# esp32_TCT_WiFi
某高専内でESP32からインターネットに出るための処理をまとめたライブラリです.

早い話がesp32の [WiFiライブラリ](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi) のラッパーライブラリです.

コンセプトは「できるだけ分かりやすく・使いやすく」です.

<br>

創造演習やら卒研やら単に構内でESP32で遊ぶやら, 自由に使ってください.


<br>

## Requirement
- [PlatformIO](https://docs.platformio.org/en/stable/integration/ide/vscode.html)  を使っている場合は特に何もなし

<br>

- ArduinoIDE を使っている場合は[ボードマネージャURL](https://arduinobook.stradty.com/accounts/Appendix3-1/#arduino%E7%92%B0%E5%A2%83%E3%81%A7%E9%96%8B%E7%99%BA%E3%81%A7%E3%81%8D%E3%82%8B%E3%82%88%E3%81%86%E3%81%AB%E3%81%97%E3%81%A6%E3%81%BF%E3%82%88%E3%81%86)を設定すること
ちなみに ↑ のページのIDとパスワードが分からない場合は,[ここにアクセスする](https://kosenjp.sharepoint.com/sites/tokuyama_math/Shared%20Documents/Forms/AllItems.aspx?csf=1&=&e=KqWJAU&cid=c6b07108%2D9ebd%2D4086%2D9b64%2Db93a75877c42&FolderCTID=0x012000BB4AF7FACC37C8499A7FDC4EDFA32C79&id=%2Fsites%2Ftokuyama%5Fmath%2FShared%20Documents%2F%E3%82%B3%E3%83%B3%E3%83%94%E3%83%A5%E3%83%BC%E3%82%BF%E9%96%A2%E9%80%A3%2F4%5FC%E3%81%A8Arduino&viewid=2457f412%2D06ce%2D47d6%2Da1a7%2D51703d5e915a).

<br>

## Install
```bash
# くろーん
> git clone git@github.com:physics11688/esp32_TCT_WiFi.git

# ↑ が出来ない人は ssh を調べるとして, とりあえず ↓ で出来る
> git clone https://github.com/physics11688/esp32_TCT_WiFi.git

# 移動
> cd esp32_TCT_WiFi.git

> ls
合計 236
drwxrwxrwx 1 takuo   4096 12月  9 15:23 ./
drwxrwxrwx 1 takuo   4096 12月  9 15:17 ../
drwxrwxrwx 1 takuo   4096 12月  9 13:40 .git/
-rwxrwxrwx 1 takuo   3302 12月  9 15:56 README.md*
drwxrwxrwx 1 takuo   4096 12月  9 14:34 TCT_WiFi/     # こいつをまるごとコピーして使う
drwxrwxrwx 1 takuo   4096 12月  9 15:23 examples/
-rwxrwxrwx 1 takuo 233650 12月  9 15:17 pic.svg*

# ArduinoIDEの場合
## Windows
> Copy-Item .\TCT_WiFi\ "${HOME}\Documents\Arduino\libraries\" -Force -Recurse

## Linux or Mac
> cp -r ./TCT_WiFi/ "${HOME}\Documents\Arduino\libraries\"

```
<br>

#### TCT_WiFi/の設置場所
クローンして出来た `TCT_WiFi/` を ↓ のように設置すればいい.

<br>

![pic](./pic.svg)

<br>




<br>

## Usage

メインファイルで ↓ のように使ってください.

1. WiFiに接続して

2. ブラウザを使わずに認証

します.

<br>

```cpp
#include <Arduino.h>   // platformio 用
#include "TCT_WiFi.h"  // インクルードします
const char* SSID      = "TCT802.1X";       // アクセスポイントの SSID
const char* USER_NAME = "m99kadomatu";     // アカウント名: m99kadomatu とか
const char* PASSWORD  = "trumpet23234";   // パスワード: trumpet23234 とか


void setup() {
    Serial.begin(115200);
    delay(10);

    // 1/2. wifi接続. この時点ではLANには参加してるけど外には出れない
    // connect_TCTwifi() は 30秒待ってもLANに参加できないときは再起動するようにしてある
    IPAddress localIP = connect_TCTwifi(SSID, USER_NAME, PASSWORD);
    Serial.print("local IP: ");
    Serial.println(localIP);

    // 2/2. 認証チェック
    int counter = 0;                     // カウンタ
    while (check_auth() != IN_ENABLE) {  // 認証されていなければループ
        authenticate();                  // 認証開始
        delay(500);
        Serial.print("-|");
        counter++;
        if (counter >= 20) {  // 10秒経過したら
            ESP.restart();    // ESP32ボードをリセット
        }
    }
    Serial.println("");


    /********** ここから自分のやりたい処理を書く **************/
    // TCP/IP通信でも, http通信でもなんでもいい
}

void loop() { 
    delay(1000);
}

```