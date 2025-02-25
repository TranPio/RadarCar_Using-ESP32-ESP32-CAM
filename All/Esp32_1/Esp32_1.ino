#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

/* 1. Thông tin WiFi */
#define WIFI_SSID "Nhom11_NT131.P12"
#define WIFI_PASSWORD "12345678"

/* 2. Thông tin API Key và URL */
#define API_KEY "AIzaSyD3NsDrxPrOLWapTfT7COBwgWp-LntOrtE"
#define DATABASE_URL "https://esp32-3502a-default-rtdb.asia-southeast1.firebasedatabase.app/"

/* 3. Thông tin tài khoản Firebase */
#define USER_EMAIL "22521106@gm.uit.edu.vn"
#define USER_PASSWORD "Abc123"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseData stream;
FirebaseData streamAvoidObject;
FirebaseData streamRemoteControl;

int previousConnectionStatus = -1; // Trạng thái kết nối trước đó (-1 để báo chưa xác định)

void updateConnectionStatus(int status) {
    // Chỉ cập nhật nếu trạng thái thay đổi
    if (status != previousConnectionStatus) {
        if (Firebase.RTDB.setInt(&fbdo, "/Connect_ESP32/Esp32_1", status)) {
            Serial.printf("Cập nhật trạng thái kết nối: %d\n", status);
        } else {
            Serial.printf("Lỗi khi cập nhật trạng thái: %s\n", fbdo.errorReason().c_str());
        }
        previousConnectionStatus = status; // Lưu trạng thái hiện tại
    }
}

void setup()
{
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("\u0110ang kết nối WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nKết nối WiFi thành công!");

    // Cấu hình Firebase
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    Firebase.begin(&config, &auth);
    Firebase.reconnectNetwork(true);

    // Kiểm tra trạng thái kết nối ban đầu
    if (Firebase.ready()) {
        updateConnectionStatus(1); // Cập nhật trạng thái ban đầu là thành công
    } else {
        updateConnectionStatus(0); // Cập nhật trạng thái ban đầu là thất bại
    }

    if (!Firebase.RTDB.beginStream(&stream, "/Car_Support/Radar/7"))
    {
        Serial.printf("Lỗi bắt đầu stream: %s\n", stream.errorReason().c_str());
    }

    if (!Firebase.RTDB.beginStream(&streamAvoidObject, "/Interface/avoidObject"))
    {
        Serial.printf("Lỗi bắt đầu stream avoidObject: %s\n", streamAvoidObject.errorReason().c_str());
    }

    if (!Firebase.RTDB.beginStream(&streamRemoteControl, "/Interface/remoteControl"))
    {
        Serial.printf("Lỗi bắt đầu stream remoteControl: %s\n", streamRemoteControl.errorReason().c_str());
    }
}

void loop()
{
    // Kiểm tra trạng thái kết nối Firebase và chỉ cập nhật nếu thay đổi
    if (Firebase.ready()) {
        updateConnectionStatus(1); // Kết nối thành công
    } else {
        updateConnectionStatus(0); // Kết nối thất bại
    }

    if (Firebase.ready() && Firebase.RTDB.readStream(&stream))
    {
        if (stream.streamAvailable())
        {
            Serial.printf("\u0110ã nhận thay đổi:\nPath: %s\nData: %s\n", stream.dataPath().c_str(), stream.stringData().c_str());
            if (stream.stringData() == "true")
            {
                Serial.println("Thực hiện quét radar...");
                Serial.println("7"); // Gửi lệnh xuống Arduino
            }
        }
    }
    else if (stream.streamTimeout())
    {
        Serial.println("Stream timeout. \u0110ang thử lại...");
    }
    else if (!stream.httpConnected())
    {
        Serial.println("Stream đã bị ngắt. \u0110ang bắt đầu lại...");
        Firebase.RTDB.beginStream(&stream, "/Car_Support/Radar/7");
    }

    // Stream avoidObject
    if (Firebase.ready() && Firebase.RTDB.readStream(&streamAvoidObject))
    {
        if (streamAvoidObject.streamAvailable())
        {
            Serial.printf("avoidObject,%s\n", streamAvoidObject.stringData().c_str());
        }
    }

    // Stream remoteControl
    if (Firebase.ready() && Firebase.RTDB.readStream(&streamRemoteControl))
    {
        if (streamRemoteControl.streamAvailable())
        {
            Serial.printf("remoteControl,%s\n", streamRemoteControl.stringData().c_str());
        }
    }

    checkArduinoResponse();
}

void checkArduinoResponse()
{
    while (Serial.available())
    {
        String response = Serial.readStringUntil('\n');
        response.trim();
        if (response == "7_done")
        {
            Serial.println("Nhận tín hiệu hoàn thành từ Arduino. Cập nhật Firebase...");
            if (Firebase.RTDB.setString(&fbdo, "/Car_Support/Radar/7", "false"))
            {
                Serial.println("Cập nhật thành công: /Car_Support/Radar/7 = false");
            }
            else
            {
                Serial.println("Lỗi khi cập nhật Firebase: " + fbdo.errorReason());
            }
        }
    }
}
