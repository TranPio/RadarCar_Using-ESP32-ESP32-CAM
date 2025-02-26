#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DIYables_IRcontroller.h> // Thư viện điều khiển hồng ngoại
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// 1. Thông tin WiFi
#define WIFI_SSID "Nhom11_NT131.P12"
#define WIFI_PASSWORD "12345678"

// 2. Thông tin API Key và URL
#define API_KEY "AIzaSyD3NsDrxPrOLWapTfT7COBwgWp-LntOrtE"
#define DATABASE_URL "https://esp32-3502a-default-rtdb.asia-southeast1.firebasedatabase.app/"

// 3. Thông tin tài khoản Firebase
#define USER_EMAIL "22521106@gm.uit.edu.vn"
#define USER_PASSWORD "Abc123"

// Các chân điều khiển động cơ
const int MR1 = 14; // Động cơ phải tiến
const int MR2 = 12; // Động cơ phải lùi
const int ML1 = 26; // Động cơ trái tiến
const int ML2 = 27; // Động cơ trái lùi

// Kênh PWM cho động cơ
#define MR1_ch 0
#define MR2_ch 1
#define ML1_ch 2
#define ML2_ch 3

// IR Receiver
#define IR_RECEIVER_PIN 25
DIYables_IRcontroller_17 irController(IR_RECEIVER_PIN, 200); // debounce time là 200ms

FirebaseData fbdo;
FirebaseConfig config;
FirebaseAuth auth;
FirebaseData streamControl;
FirebaseData streamRemoteControl;
FirebaseData streamDuongDi;

bool remoteControlEnabled = false; // Trạng thái kiểm soát từ remote IR

// Các hàm điều khiển động cơ
void Forward() {
    ledcWrite(MR1_ch, 255);
    ledcWrite(MR2_ch, 0);
    ledcWrite(ML1_ch, 255);
    ledcWrite(ML2_ch, 0);
    Serial.println("Xe tiến");
}

void Backward() {
    ledcWrite(MR1_ch, 0);
    ledcWrite(MR2_ch, 255);
    ledcWrite(ML1_ch, 0);
    ledcWrite(ML2_ch, 255);
    Serial.println("Xe lùi");
}

void Left() {
    ledcWrite(MR1_ch, 255);
    ledcWrite(MR2_ch, 0);
    ledcWrite(ML1_ch, 0);
    ledcWrite(ML2_ch, 255);
    Serial.println("Xe rẽ trái");
}

void Right() {
    ledcWrite(MR1_ch, 0);
    ledcWrite(MR2_ch, 255);
    ledcWrite(ML1_ch, 255);
    ledcWrite(ML2_ch, 0);
    Serial.println("Xe rẽ phải");
}

void Stop() {
    ledcWrite(MR1_ch, 0);
    ledcWrite(MR2_ch, 0);
    ledcWrite(ML1_ch, 0);
    ledcWrite(ML2_ch, 0);
    Serial.println("Xe dừng");
}

// Hàm xử lý stream từ /Car_Control/Duong_Di
void handleDuongDiStream() {
    if (Firebase.RTDB.readStream(&streamDuongDi)) {
        if (streamDuongDi.streamAvailable()) {
            String data = streamDuongDi.stringData();
            Serial.printf("Dữ liệu đường đi: %s\n", data.c_str());
            
            // Phân tích chuỗi đường đi
            int startIndex = 0;
            int endIndex = 0;
            while (endIndex != -1) {
                endIndex = data.indexOf(',', startIndex);
                String number = (endIndex == -1) 
                                ? data.substring(startIndex) 
                                : data.substring(startIndex, endIndex);
                startIndex = endIndex + 1;

                int command = number.toInt();
                switch (command) {
                    case 1:
                        Forward();
                        break;
                    case 2:
                        Backward();
                        break;
                    case 3:
                        Left();
                        //Forward();
                        break;
                    case 4:
                        Right();
                        //Forward();
                        break;
                    case 0:
                        Stop();
                        break;
                    default:
                        Serial.println("Lệnh không hợp lệ!");
                        break;
                }
                delay(400); // Delay giữa các lệnh
            }
        }
    }
}

// Hàm xử lý stream từ /Car_Control/Lenh
void handleControlStream() {
    if (Firebase.RTDB.readStream(&streamControl)) {
        if (streamControl.streamAvailable()) {
            String controlData = streamControl.stringData(); // Lấy dữ liệu từ Firebase
            Serial.printf("Control data: %s\n", controlData.c_str());

            // Kiểm tra và xử lý lệnh điều khiển
            if (controlData == "1,") { // Tiến liên tục
                Forward();
            } else if (controlData == "1,0") { // Tiến một khoảng rồi dừng
                Forward();
                delay(500);
                Stop();
            } else if (controlData == "2,") { // Lùi liên tục
                Backward();
            } else if (controlData == "2,0") { // Lùi một khoảng rồi dừng
                Backward();
                delay(500);
                Stop();
            } else if (controlData == "3,") { // Rẽ trái liên tục
                Left();
            } else if (controlData == "3,0") { // Rẽ trái một khoảng rồi dừng
                Left();
                delay(500);
                Stop();
            } else if (controlData == "4,") { // Rẽ phải liên tục
                Right();
            } else if (controlData == "4,0") { // Rẽ phải một khoảng rồi dừng
                Right();
                delay(500);
                Stop();
            } else { // Dừng trong trường hợp lệnh không xác định
                Stop();
            }
        }
    }
}

// Hàm xử lý stream từ /Interface/remoteControl
void handleRemoteControlStream() {
    if (Firebase.RTDB.readStream(&streamRemoteControl)) {
        if (streamRemoteControl.streamAvailable()) {
            String data = streamRemoteControl.stringData();
            Serial.printf("Trạng thái remoteControl: %s\n", data.c_str());
            remoteControlEnabled = (data == "true"); // Bật hoặc tắt điều khiển IR
        }
    }
}

// Hàm xử lý điều khiển từ IR Remote
void handleRemoteControl() {
    if (remoteControlEnabled) { // Chỉ xử lý nếu được phép
        Key17 key = irController.getKey();
        if (key != Key17::NONE) {
            switch (key) {
                case Key17::KEY_1:
                case Key17::KEY_UP:
                    Forward();
                    delay(500);
                    Stop();
                    break;
                case Key17::KEY_2:
                case Key17::KEY_DOWN:
                    Backward();
                    delay(500);
                    Stop();
                    break;
                case Key17::KEY_3:
                case Key17::KEY_LEFT:
                    Left();
                    delay(500);
                    Stop();
                    break;
                case Key17::KEY_4:
                case Key17::KEY_RIGHT:
                    Right();
                    delay(500);
                    Stop();
                    break;
                case Key17::KEY_0:
                case Key17::KEY_OK:
                    Stop();
                    break;
                default:
                    Serial.println("Lệnh IR không hợp lệ!");
                    break;
            }
        }
    }
}

// Hàm cập nhật trạng thái kết nối vào nhánh Firebase "/Connect_ESP32/Esp32_3"
void updateConnectionStatus(int status) {
    static int previousConnectionStatus = -1;
    if (status != previousConnectionStatus) {
        if (Firebase.RTDB.setInt(&fbdo, "/Connect_ESP32/Esp32_3", status)) {
            Serial.printf("Cập nhật trạng thái kết nối: %d\n", status);
        } else {
            Serial.printf("Lỗi khi cập nhật trạng thái: %s\n", fbdo.errorReason().c_str());
        }
        previousConnectionStatus = status;
    }
}

void setup() {
    Serial.begin(115200);

    // Khởi tạo IR Remote
    irController.begin();

    // Kết nối WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Đang kết nối WiFi");
    while (WiFi.status() != WL_CONNECTED) {
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

    // Bắt đầu stream từ Firebase
    if (!Firebase.RTDB.beginStream(&streamControl, "/Car_Control/Lenh")) {
        Serial.printf("Lỗi bắt đầu stream Control: %s\n", streamControl.errorReason().c_str());
    }
    if (!Firebase.RTDB.beginStream(&streamRemoteControl, "/Interface/remoteControl")) {
        Serial.printf("Lỗi bắt đầu stream remoteControl: %s\n", streamRemoteControl.errorReason().c_str());
    }
    if (!Firebase.RTDB.beginStream(&streamDuongDi, "/Car_Control/Duong_Di")) {
        Serial.printf("Lỗi bắt đầu stream Duong_Di: %s\n", streamDuongDi.errorReason().c_str());
    }

    // Cấu hình chân điều khiển động cơ
    pinMode(MR1, OUTPUT);
    pinMode(MR2, OUTPUT);
    pinMode(ML1, OUTPUT);
    pinMode(ML2, OUTPUT);

    // Gán chân với kênh PWM
    ledcAttachPin(MR1, MR1_ch);
    ledcAttachPin(MR2, MR2_ch);
    ledcAttachPin(ML1, ML1_ch);
    ledcAttachPin(ML2, ML2_ch);

    // Cấu hình tần số và độ phân giải PWM
    ledcSetup(MR1_ch, 2000, 8); // 2kHz, 8-bit
    ledcSetup(MR2_ch, 2000, 8);
    ledcSetup(ML1_ch, 2000, 8);
    ledcSetup(ML2_ch, 2000, 8);

    Stop(); // Đảm bảo động cơ dừng khi khởi động
}

// Vòng lặp chính
void loop() {
    if (Firebase.ready()) {
        updateConnectionStatus(1); // Trạng thái "1" khi kết nối sẵn sàng
        handleControlStream();     // Xử lý lệnh từ Firebase tại /Car_Control/Lenh
        handleRemoteControlStream(); // Xử lý stream từ /Interface/remoteControl
        handleDuongDiStream();       // Xử lý lệnh từ /Car_Control/Duong_Di
    } else {
        updateConnectionStatus(0); // Trạng thái "0" khi kết nối không sẵn sàng
    }

    handleRemoteControl(); // Xử lý lệnh từ IR Remote
    delay(100);
}
