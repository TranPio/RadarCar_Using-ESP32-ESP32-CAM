#include <NewPing.h>
#include <Servo.h>
#include <Wire.h>
#include <AFMotor.h>
#include <IRremote.h> // Thư viện IRremote

#define TRIG_PIN A2
#define ECHO_PIN A3
#define MAX_DISTANCE 100

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);
Servo servosensor;

AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

const int receiverPin = 8; // Chân digital 8 dùng để đọc tín hiệu IR

int currentSpeed = 150;
char buffer[10];
int cm;
int pos;
int angsensor[171]; // 0 đến 170
int dist;

bool moveState[5] = {false, false, false, false, false}; // Trạng thái cho các lệnh 1, 2, 3, 4
bool remoteControlEnabled = false; // Trạng thái điều khiển từ xa
bool avoidObstacleEnabled = false; // Trạng thái tránh vật cản

void updateMotorSpeed(int spd) {
  motor1.setSpeed(spd);
  motor2.setSpeed(spd);
  motor3.setSpeed(spd);
  motor4.setSpeed(spd);
}

void moveForward() {
  updateMotorSpeed(currentSpeed);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
  Serial.println("Car moving forward!");
}

void moveBackward() {
  updateMotorSpeed(currentSpeed);
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
  Serial.println("Car moving backward!");
}

void turnLeft() {
  updateMotorSpeed(currentSpeed);
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
  Serial.println("Car turning left!");
}

void turnRight() {
  updateMotorSpeed(currentSpeed);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
  Serial.println("Car turning right!");
}

void stopMoving() {
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
  Serial.println("Car stopped!");
}

int readPing() {
  delay(70);
  int cm = sonar.ping_cm();
  if (cm == 0) {
    cm = 250;
  }
  return cm;
}

int lookRight() {
  servosensor.write(50);
  delay(500);
  int distance = readPing();
  delay(100);
  servosensor.write(90);
  return distance;
}

int lookLeft() {
  servosensor.write(170);
  delay(500);
  int distance = readPing();
  delay(100);
  servosensor.write(90);
  return distance;
}

void avoidObstacle() {
  int distance = readPing();
  if (distance <= 15) {
    stopMoving();
    delay(100);
    moveBackward();
    delay(300);
    stopMoving();
    delay(200);

    int distanceR = lookRight();
    delay(200);
    int distanceL = lookLeft();
    delay(200);

    if (distanceR >= distanceL) {
      turnRight();
    } else {
      turnLeft();
    }
    stopMoving();
  } else {
    moveForward();
  }
}

void processStringCommand(String command) {
  if (command == "remoteControl,true") {
    remoteControlEnabled = true;
    Serial.println("Remote control enabled");
  } else if (command == "remoteControl,false") {
    remoteControlEnabled = false;
    Serial.println("Remote control disabled");
  } else if (command == "avoidObject,true") {
    avoidObstacleEnabled = true;
    Serial.println("Obstacle avoidance enabled");
  } else if (command == "avoidObject,false") {
    avoidObstacleEnabled = false;
    stopMoving();
    Serial.println("Obstacle avoidance disabled");
  } else if (command == "1,true") {
    while (true) {
      moveForward();
      if (Serial.available() > 0) {
        String stopCommand = Serial.readStringUntil('\n');
        stopCommand.trim();
        if (stopCommand == "1,true,false") {
          stopMoving();
          break;
        }
      }
    }
  } else if (command == "2,true") {
    while (true) {
      moveBackward();
      if (Serial.available() > 0) {
        String stopCommand = Serial.readStringUntil('\n');
        stopCommand.trim();
        if (stopCommand == "2,true,false") {
          stopMoving();
          break;
        }
      }
    }
  } else if (command == "3,true") {
    while (true) {
      turnLeft();
      if (Serial.available() > 0) {
        String stopCommand = Serial.readStringUntil('\n');
        stopCommand.trim();
        if (stopCommand == "3,true,false") {
          stopMoving();
          break;
        }
      }
    }
  } else if (command == "4,true") {
    while (true) {
      turnRight();
      if (Serial.available() > 0) {
        String stopCommand = Serial.readStringUntil('\n');
        stopCommand.trim();
        if (stopCommand == "4,true,false") {
          stopMoving();
          break;
        }
      }
    }
  } else {
    Serial.println("Invalid command format or unknown command");
  }
}

void processIRCommand(unsigned long irData) {
  if (!remoteControlEnabled) {
    Serial.println("Remote control is disabled. Ignoring IR command.");
    return;
  }

  switch (irData) {
    case 0xE718FF00: moveForward(); break;
    case 0xAD52FF00: moveBackward(); break;
    case 0xF708FF00: turnLeft(); break;
    case 0xA55AFF00: turnRight(); break;
    case 0xE31CFF00: stopMoving(); break;
    default: Serial.println("Unknown IR command"); break;
  }
}

void setup() {
  Serial.begin(115200); // Giao tiếp với ESP32-CAM
  servosensor.attach(10);
  updateMotorSpeed(currentSpeed);
  stopMoving();
  Serial.println("Car is ready to receive commands!");
  IrReceiver.begin(receiverPin, ENABLE_LED_FEEDBACK); // Khởi động IR receiver
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n'); // Đọc lệnh qua Serial
    command.trim();
    processStringCommand(command);
  }

  if (avoidObstacleEnabled) {
    avoidObstacle();
  }

  if (IrReceiver.decode()) { // Nếu nhận được tín hiệu IR
    unsigned long irData = IrReceiver.decodedIRData.decodedRawData;
    Serial.print("Received IR value: ");
    Serial.println(irData, HEX);
    processIRCommand(irData);
    IrReceiver.resume(); // Chuẩn bị nhận tín hiệu tiếp theo
  }
}
