# RadarCar_Using-ESP32-ESP32-CAM
![image](https://github.com/user-attachments/assets/f51e6817-d3e2-475e-a9df-31247331361f)
# Radar Car Using ESP32 & ESP32-CAM

## Diagram Explanation

### 1. Initial Setup and Sensor Connections
- First, install and connect the sensors to the corresponding ESP devices as shown in the diagram.
- **ESP32_1**: Receives radar control commands from Firebase and collects data from ultrasonic sensors, then sends the data to **ESP32_2** via Serial communication.
- **ESP32_2**: Receives angle and distance data from **ESP32_1** via Serial and uploads this data to Firebase.
- **ESP32_3**: Receives commands from Firebase and controls the vehicle's motors to perform actions such as moving forward, backward, turning left, and turning right.
- **ESP CAM**: Continuously captures images and uploads them to a server hosted on AWS. It also receives commands for vehicle lights, speed, and image capture requests from the server.

### 2. ESP Programming and Data Collection
- Write code for the ESP modules using appropriate software libraries to collect data from the sensors.
- The collected data is then processed into a readable format and transmitted to the server.

### 3. Cloud Storage and Data Transmission
- The data is sent to a cloud storage service such as **Firebase** or **AWS**.
- These are widely used cloud storage services that provide flexible storage options and various supporting features.
- Through the **HTTPS** protocol, server data can be accessed from web and mobile applications.
- **HTTPS** is a secure protocol that ensures safe data transmission between the server and applications, preventing data theft.

### 4. Front-End Development and Real-Time Monitoring
- For the **Front-End**, we use **Android Studio** to develop the vehicle control application.
- **WebSocket** is used to display real-time images from the **ESP CAM**, making the system more comprehensive and flexible.
- This setup not only enables remote vehicle control and monitoring but also enhances the user experience by providing **real-time video streaming** with an **intuitive control interface**.

## Technology Stack
- **Microcontrollers**: ESP32, ESP32-CAM
- **Cloud Services**: Firebase, AWS
- **Communication**: Serial, WebSocket, HTTPS
- **Development Tools**: Arduino IDE, Android Studio
- **Programming Languages**: C++, Java, JavaScript (for server)

## Installation and Setup
1. **Hardware Setup**:
   - Connect the ultrasonic sensors, motors, and ESP modules as per the wiring diagram.
   - Ensure that power is supplied correctly to all components.

2. **Software Setup**:
   - Install the required libraries in Arduino IDE.
   - Flash the ESP32 and ESP32-CAM with the corresponding firmware.

3. **Cloud Setup**:
   - Configure Firebase to store and retrieve data.
   - Deploy an AWS server for image processing.

4. **App Deployment**:
   - Build and run the Android application using Android Studio.
   - Connect the app to Firebase for real-time data exchange.

## Future Improvements
- Implement AI-based object recognition using **OpenCV**.
- Improve vehicle navigation with **LIDAR** and **SLAM**.
- Enhance security with **end-to-end encryption**.

---

🚀 *This project is an IoT-based radar car system designed for autonomous navigation and real-time monitoring. Stay tuned for updates!* 🚀
