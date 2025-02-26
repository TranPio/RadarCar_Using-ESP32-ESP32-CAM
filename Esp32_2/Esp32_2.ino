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

// Kích thước mảng cố định
#define MAX_ANGLES 161 // Từ góc 10 đến 170 (bao gồm 161 giá trị)
int radarData[MAX_ANGLES][2]; // Mảng 2 chiều: [Góc, Khoảng cách]
int currentIndex = 0; // Chỉ số hiện tại trong mảng

void setup()
{
    Serial.begin(115200);

    // Kết nối WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Đang kết nối WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nKết nối WiFi thành công!");
    Serial.print("Địa chỉ IP: ");
    Serial.println(WiFi.localIP());

    // Cấu hình Firebase
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    Firebase.begin(&config, &auth);
    Firebase.reconnectNetwork(true);
    Serial.println("Kết nối Firebase thành công!");

    // Cập nhật trạng thái kết nối lên Firebase
    if (Firebase.RTDB.setInt(&fbdo, "/Connect_ESP32/Esp32_2", 1))
    {
        Serial.println("Cập nhật trạng thái kết nối ESP32_2 thành công!");
    }
    else
    {
        Serial.println("Lỗi khi cập nhật trạng thái kết nối ESP32_2: " + fbdo.errorReason());
    }

    // Khởi tạo mảng
    for (int i = 0; i < MAX_ANGLES; i++)
    {
        radarData[i][0] = -1; // Giá trị góc
        radarData[i][1] = -1; // Giá trị khoảng cách
    }

    // Tạo task FreeRTOS
    xTaskCreatePinnedToCore(
        taskReadSerialAndUpdateArray,
        "Task_Read_Serial",
        8192,
        NULL,
        1,
        NULL,
        0);

    xTaskCreatePinnedToCore(
        taskUpdateFirebase,
        "Task_Update_Firebase",
        8192,
        NULL,
        1,
        NULL,
        1);
}

void loop()
{
    // Loop không làm gì
}

// Task 1: Đọc dữ liệu từ Serial và lưu vào mảng
void taskReadSerialAndUpdateArray(void *parameter)
{
    static bool updateRadar = false;

    while (true)
    {
        if (Serial.available())
        {
            String data = Serial.readStringUntil('\n'); // Đọc dữ liệu từ Arduino
            data.trim();

            if (data == "Update_Radar")
            {
                updateRadar = true;
                currentIndex = 0; // Reset chỉ số khi bắt đầu cập nhật mới
                Serial.println("Nhận lệnh Update_Radar từ Arduino. Sẵn sàng cập nhật dữ liệu.");
            }
            else if (updateRadar && data.indexOf(',') > 0)
            {
                int commaIndex = data.indexOf(','); // Tìm vị trí dấu phẩy
                String gocStr = data.substring(0, commaIndex); // Giá trị góc
                String khoangCachStr = data.substring(commaIndex + 1); // Giá trị khoảng cách

                if (!gocStr.isEmpty() && khoangCachStr != NULL) // Kiểm tra giá trị hợp lệ
                {
                    int goc = gocStr.toInt();
                    int khoangCach = khoangCachStr.toInt();

                    if (goc >= 10 && goc <= 170 && currentIndex < MAX_ANGLES) // Chỉ lưu góc hợp lệ
                    {
                        radarData[currentIndex][0] = goc;
                        radarData[currentIndex][1] = khoangCach;
                        Serial.println("Lưu dữ liệu: Góc = " + String(goc) + ", Khoảng cách = " + String(khoangCach));
                        currentIndex++;

                        if (goc >= 170)
                        {
                            updateRadar = false;
                            Serial.println("Hoàn thành cập nhật dữ liệu cho tất cả các góc. Dừng cập nhật.");
                        }
                    }
                }
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // Dừng task trong 10ms để giảm tải CPU
    }
}

// Task 2: Đẩy từng nhánh độc lập lên Firebase
void taskUpdateFirebase(void *parameter)
{
    while (true)
    {
        for (int i = 0; i < currentIndex; i++)
        {
            if (radarData[i][0] != -1 && radarData[i][1] != -1) // Kiểm tra dữ liệu hợp lệ
            {
                String path = "/Car_Support/Radar/Data/" + String(radarData[i][0]);

                // Gửi dữ liệu khoảng cách lên Firebase trực tiếp
                if (Firebase.RTDB.setInt(&fbdo, path, radarData[i][1]))
                {
                    Serial.println("Cập nhật thành công: " + path + " = " + String(radarData[i][1]));
                }
                else
                {
                    Serial.println("Lỗi khi cập nhật: " + path + " - " + fbdo.errorReason());
                }

                // Reset giá trị sau khi đẩy
                radarData[i][0] = -1;
                radarData[i][1] = -1;
            }
        }

        // Reset chỉ số sau khi xử lý tất cả dữ liệu
        currentIndex = 0;

        yield(); // Cho phép CPU xử lý các tác vụ khác
    }
}
