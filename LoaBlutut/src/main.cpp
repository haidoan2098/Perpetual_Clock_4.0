#include <WiFi.h>
#include <time.h>
#include "BluetoothA2DPSink.h"

// Wi-Fi và NTP
const char *ssid = "admin";
const char *password = "123456789";
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600; // GMT+7
const int daylightOffset_sec = 0;

// Thêm LED cho thời gian và Bluetooth
// const int ledTimePin = 16;      // LED báo lấy thời gian (GPIO2)
const int ledBluetoothPin = 21; // LED báo Bluetooth đang hoạt động (GPIO4) và lấy thời gian

// Nút nhấn
const int wifiButtonPin = 32; // Nút lấy giờ
const int btButtonPin = 33;   // Nút bật Bluetooth // màu xanh
const int maxGainPin = 25;

int lastWifiButtonState = HIGH;
int lastBtButtonState = HIGH;
bool isbtStarted = false;

// Các chân I2S
#define I2S_BCLK 27
#define I2S_LRC 26
#define I2S_DOUT 14

BluetoothA2DPSink a2dp_sink;

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) //gọi hàm getlocaltime lấy thời gian về gán vô timeinfo
  {
    Serial.println("Không thể lấy thời gian thực");
    return;
  }
  char weekDay[4];
  strftime(weekDay, sizeof(weekDay), "%a", &timeinfo);   //%a là lấy 3 kí tự đauà của timeinfo
  for (int i = 0; i < 3; i++)  //duyệt qua 3 kí tự để viết hoa lên
  {
    weekDay[i] = toupper(weekDay[i]);
  }
  char timeString[32];
  snprintf(timeString, sizeof(timeString), //ghi dữ liệu định dạng vào timestring theo định dạng dưới
           "%02d:%02d:%02d-%s-%02d/%02d/%02d\r\n", //%02d là số nguyên có 2 số
           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
           weekDay, timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year % 100);
  // Serial.print(timeString);

  // //   Gửi thời gian qua UART1 cho STM32
  Serial1.println(timeString);
 

  for (int i = 0; i < 3; i++)
  {
    digitalWrite(ledBluetoothPin, HIGH);
    delay(300);
    digitalWrite(ledBluetoothPin, LOW);
    delay(200);
  }

}

void setup()
{
  // Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 3, 1); //cấu hình tx rx

  // pinMode(ledTimePin, OUTPUT);
  pinMode(ledBluetoothPin, OUTPUT);
  // digitalWrite(ledTimePin, LOW);
  digitalWrite(ledBluetoothPin, LOW);

  pinMode(wifiButtonPin, INPUT_PULLUP);
  pinMode(btButtonPin, INPUT_PULLUP);

  pinMode(maxGainPin, OUTPUT);
  digitalWrite(maxGainPin, HIGH); // GAIN mặc định HIGH

  // Cấu hình I2S cho Bluetooth
  i2s_pin_config_t my_pins = {
      .bck_io_num = I2S_BCLK,
      .ws_io_num = I2S_LRC,
      .data_out_num = I2S_DOUT,
      .data_in_num = I2S_PIN_NO_CHANGE};
  a2dp_sink.set_pin_config(my_pins);
  a2dp_sink.set_auto_reconnect(true);
  delay(500); // chờ I2S ổn định

  Serial.println("ESP32 đã sẵn sàng!");
}

void loop()
{
  // Đọc trạng thái nút
  int wifiButtonState = digitalRead(wifiButtonPin);
  int btButtonState = digitalRead(btButtonPin);

  // Nút lấy giờ (Wi-Fi)
  if (wifiButtonState == LOW && lastWifiButtonState == HIGH)
  {
    delay(50);
    Serial.println("Bắt đầu kết nối Wi-Fi để lấy thời gian...");

    WiFi.begin(ssid, password);
    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20)
    {
      delay(500);
      Serial.print(".");
      retry++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\nĐã kết nối Wi-Fi!");

      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      delay(2000); // Chờ đồng bộ

      printLocalTime();

      Serial.println("Ngắt Wi-Fi để tiết kiệm năng lượng.");
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
    }
    else
    {
      Serial.println("\nKhông thể kết nối Wi-Fi!");
    }

    delay(300); // Chống rung nút
  }

  // Nút bật Bluetooth A2DP
  if (btButtonState == LOW && lastBtButtonState == HIGH)
  {
    delay(50);
    if (!isbtStarted)
    {

      Serial.println("Đang reset MAX98357A...");
      digitalWrite(maxGainPin, LOW);
      delay(100); // giữ LOW 100ms
      digitalWrite(maxGainPin, HIGH);
      delay(100); // chờ ổn định

      a2dp_sink.start("ESP32 loa");
      delay(1000); // chờ I2S ổn định
      isbtStarted = true;

      digitalWrite(ledBluetoothPin, HIGH); // Bật LED khi Bluetooth hoạt động
      Serial.println("Bluetooth A2DP đã bật!");
    }
    else
    {
      a2dp_sink.end(); // Tắt Bluetooth A2DP
      isbtStarted = false;

      digitalWrite(ledBluetoothPin, LOW); // Tắt LED khi Bluetooth ngắt
      Serial.println("Bluetooth A2DP đã tắt!");
    }

    delay(300); // Chống rung nút
  }

  lastWifiButtonState = wifiButtonState;
  lastBtButtonState = btButtonState;
}
