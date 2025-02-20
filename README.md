# RadarCar_Using-ESP32-ESP32-CAM
![image](https://github.com/user-attachments/assets/f51e6817-d3e2-475e-a9df-31247331361f)
Diagram Explanation:
Initial Setup and Sensor Connections

First, install and connect the sensors to the corresponding ESP devices as shown in the diagram.
ESP32_1: Receives radar control commands from Firebase and collects data from ultrasonic sensors, then sends the data to ESP32_2 via Serial communication.
ESP32_2: Receives angle and distance data from ESP32_1 via Serial and uploads this data to Firebase.
ESP32_3: Receives commands from Firebase and controls the vehicle's motors to perform actions such as moving forward, backward, turning left, and turning right.
ESP CAM: Continuously captures images and uploads them to a server hosted on AWS. It also receives commands for vehicle lights, speed, and image capture requests from the server.
ESP Programming and Data Collection

Next, write code for the ESP modules using appropriate software libraries to collect data from the sensors.
The collected data is then processed into a readable format and transmitted to the server.
Cloud Storage and Data Transmission

The data is sent to a cloud storage service such as Firebase or AWS. These are widely used cloud storage services that provide flexible storage options and various supporting features.
Through the HTTPS protocol, server data can be accessed from web and mobile applications. HTTPS is a secure protocol that ensures safe data transmission between the server and applications, preventing data theft.
Front-End Development and Real-Time Monitoring

For the Front-End, we use Android Studio to develop the vehicle control application.
WebSocket is used to display real-time images from the ESP CAM, making the system more comprehensive and flexible.
This setup not only enables remote vehicle control and monitoring but also enhances the user experience by providing real-time video streaming with an intuitive control interface.
