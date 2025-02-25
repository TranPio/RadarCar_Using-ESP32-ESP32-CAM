#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ESP32Servo.h>
#include <NewPing.h>
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

// Các chân kết nối
#define TRIG_PIN  23  // ESP32 pin GPIO23 connected to Ultrasonic Sensor's TRIG pin
#define ECHO_PIN  22  // ESP32 pin GPIO22 connected to Ultrasonic Sensor's ECHO pin
#define SERVO_PIN 21  // ESP32 pin GPIO21 connected to Servo Motor's pin
#define BUZZER_PIN 18  // ESP32 pin GPIO8 connected to Buzzer

// Tạo đối tượng servo
Servo servo;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseData streamRadar;
FirebaseData streamControl;  // Lắng nghe lệnh điều khiển từ Firebase
FirebaseData streamBuzzer;  // Lắng nghe tín hiệu điều khiển còi từ Firebase

int previousConnectionStatus = -1; // Trạng thái kết nối trước đó (-1 để báo chưa xác định)
float duration_us, distance_cm;

char buffer[10];
int cm;
int pos;
int angsensor[171]; // 0 đến 170
int dist; // Khai báo biến dist để lưu khoảng cách

// Khởi tạo đối tượng NewPing để đo khoảng cách
#define MAX_DISTANCE 200 // Đặt giới hạn khoảng cách là 200 cm
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

// Hàm đo khoảng cách với NewPing
float measureDistance() {
  // Đo khoảng cách và trả về
  float cm = sonar.ping_cm();
  return cm;
}

// Hàm quét radar (giống như trong mã cũ của bạn)
void entdadossensor() {
  Serial.println("Update_Radar");

  for (pos = 10; pos <= 170; pos++) {
    servo.write(pos);  // Quay servo đến góc hiện tại
    delay(100);  // Thêm một chút độ trễ để servo kịp di chuyển

    // Đo khoảng cách
    cm = measureDistance();

    // Nếu khoảng cách lớn hơn 50 thì gán cm = 0
    if (cm > 50 || cm == 0) {
      cm = 0;
    }

    angsensor[pos] = cm;  // Lưu giá trị đo được vào mảng angsensor
    dist = angsensor[pos];  // Lấy giá trị khoảng cách

    // Chuyển giá trị đo được thành chuỗi và gửi qua Serial
    sprintf(buffer, "%d,%d", pos, dist);
    Serial.println(buffer);  // Gửi dữ liệu lên ESP32-CAM
    delay(50);  // Thêm độ trễ nhỏ giữa các lần quét
  }

  Serial.println("7_done");  // Gửi tín hiệu khi quét hoàn tất
}

// Hàm dừng servo (quay về vị trí 90 độ)
void pararsensor() {
  pos = 90;
  servo.write(pos);  // Quay servo về 90 độ
}

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

void setup() {
  Serial.begin(115200);  // Khởi tạo Serial Monitor
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Kết nối WiFi
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

  // Bắt đầu stream trên đường dẫn Firebase
  if (!Firebase.RTDB.beginStream(&streamRadar, "/Car_Support/Radar/7")) {
    Serial.printf("Lỗi bắt đầu stream Radar: %s\n", streamRadar.errorReason().c_str());
  }

  if (!Firebase.RTDB.beginStream(&streamControl, "/Car_Control/Control")) {
    Serial.printf("Lỗi bắt đầu stream Control: %s\n", streamControl.errorReason().c_str());
  }

  if (!Firebase.RTDB.beginStream(&streamBuzzer, "/Car_Control/Ken_Xe")) {
    Serial.printf("Lỗi bắt đầu stream Buzzer: %s\n", streamBuzzer.errorReason().c_str());
  }

  // Cấu hình các chân
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  servo.attach(SERVO_PIN);  // Gắn servo vào chân GPIO21
}

void loop() {
  // Stream radar từ Firebase
  if (Firebase.RTDB.readStream(&streamRadar)) {
    if (streamRadar.streamAvailable()) {
      String radarData = streamRadar.stringData();
      Serial.printf("Nhận dữ liệu radar: %s\n", radarData.c_str());

      // Nếu nhận được "true", thực hiện quét radar
      if (radarData == "true") {
        entdadossensor();  // Quét radar
        pararsensor();  // Quay servo về 90 độ
      }
    }
  }

  // Lắng nghe lệnh điều khiển từ Firebase (nhánh Car_Control)
  if (Firebase.RTDB.readStream(&streamControl)) {
    if (streamControl.streamAvailable()) {
      String controlData = streamControl.stringData();
      Serial.printf("Nhận dữ liệu điều khiển: %s\n", controlData.c_str());

      // Phân tích chuỗi điều khiển và gửi lệnh tương ứng xuống Serial
      if (controlData == "1,") {
        Serial.println("1");  // Gửi lệnh 1 xuống Serial
      } else if (controlData == "1,0") {
        Serial.println("0");  // Gửi lệnh 0 xuống Serial
      } else if (controlData == "2,") {
        Serial.println("2");  // Gửi lệnh 2 xuống Serial
      } else if (controlData == "2,0") {
        Serial.println("0");  // Gửi lệnh 0 xuống Serial
      } else if (controlData == "3,") {
        Serial.println("3");  // Gửi lệnh 3 xuống Serial
      } else if (controlData == "3,0") {
        Serial.println("0");  // Gửi lệnh 0 xuống Serial
      } else if (controlData == "4,") {
        Serial.println("4");  // Gửi lệnh 4 xuống Serial
      } else if (controlData == "4,0") {
        Serial.println("0");  // Gửi lệnh 0 xuống Serial
      } else if (controlData == "0") {
        Serial.println("0");  // Gửi lệnh 0 xuống Serial
      }
    }
  }

  // Lắng nghe tín hiệu điều khiển còi từ Firebase
  if (Firebase.RTDB.readStream(&streamBuzzer)) {
    if (streamBuzzer.streamAvailable()) {
      String buzzerData = streamBuzzer.stringData();
      Serial.printf("Nhận dữ liệu còi: %s\n", buzzerData.c_str());

      if (buzzerData == "1,") {
        digitalWrite(BUZZER_PIN, HIGH);  // Bật còi
      } else if (buzzerData == "1,0") {
        digitalWrite(BUZZER_PIN, LOW);  // Tắt còi
      }
    }
  }

  // Cập nhật trạng thái kết nối Firebase
  if (Firebase.ready()) {
    updateConnectionStatus(1);  // Kết nối Firebase thành công
  } else {
    updateConnectionStatus(0);  // Kết nối Firebase thất bại
  }

  delay(100);  // Chờ trước khi tiếp tục vòng lặp
}
