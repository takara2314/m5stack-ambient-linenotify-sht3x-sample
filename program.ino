#include "M5_ENV.h"
#include <Ambient.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// SHT30 (温度、湿度センサ)
SHT3X sht30;

// Wi-Fi
const char *ssid = "XXXXX";
const char *password = "XXXXX";
// Ambient
unsigned int ambientChannelId = 99999;
const char *ambientWriteKey = "XXXXX";
// LINE Notify
const char *lineNotifyHost = "notify-api.line.me";
const char *lineNotifyToken = "XXXXX";

// Wi-Fiクライアント
WiFiClient client;
// Ambientクライアント
Ambient ambient;

// 温度
float temperature = 0.0;
// 湿度
float humidity = 0.0;

// LINE Notify にメッセージを送信する
boolean line_notify(String msg)
{
    WiFiClientSecure client;

    // SSL接続できるか確認
    client.setInsecure();
    if (!client.connect(lineNotifyHost, 443))
    {
        Serial.println("connect error!");
        return false;
    }

    // クエリを定義
    String query = String("message=") + msg;
    // リクエスト内容を定義
    String request = String("") + "POST /api/notify HTTP/1.1\r\n" + "host: " + lineNotifyHost + "\r\n" +
                     "Authorization: Bearer " + lineNotifyToken + "\r\n" + "Content-Length: " + String(query.length()) +
                     "\r\n" + "Content-Type: application/x-www-form-urlencoded\r\n\r\n" + query + "\r\n";
    // リクエストを送信
    client.print(request);

    return true;
}

void setup()
{
    // M5Stack
    M5.begin();
    M5.Power.begin();
    // I2C
    Wire.begin();
    // 液晶
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextDatum(4);

    // シリアル通信
    Serial.begin(115200);

    // Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        //  Wi-Fiアクセスポイントへの接続待ち
        delay(500);
        Serial.print(".");
    }
    Serial.println("Wi-Fi connected");

    // Ambient
    ambient.begin(ambientChannelId, ambientWriteKey, &client);
}

// 温度と湿度を取得
void getEnvirons()
{
    if (sht30.get() == 0)
    {
        temperature = sht30.cTemp;
        humidity = sht30.humidity;
    }
    else
    {
        temperature = 0;
        humidity = 0;
    }
}

void loop()
{
    // 温度と湿度を取得
    getEnvirons();

    // 液晶表示
    M5.lcd.setCursor(5, 50);
    M5.Lcd.printf("Temperature: %2.2f'C", temperature);
    M5.lcd.setCursor(5, 80);
    M5.Lcd.printf("Humidity: %2.0f%%", humidity);
    M5.lcd.setCursor(5, 130);

    // Ambientに気温、湿度を送信
    ambient.set(1, temperature);
    ambient.set(2, humidity);
    ambient.send();

    // LINEに温度と湿度を通知
    line_notify("\n温度: " + String(temperature, 2) + "℃" + "\n湿度: " + String(humidity, 0) + "％");

    // 30秒スリープ
    delay(30 * 1000);
    M5.Lcd.clear(BLACK);
}
