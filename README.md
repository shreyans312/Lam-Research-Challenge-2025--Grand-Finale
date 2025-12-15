![main-game-1](https://github.com/user-attachments/assets/122c18a6-f4da-4e7e-b536-80b21c094d79)# LAM Research Challenge 2025 – Grand Finale  
### Team **Mahagatbandhan** | Top **24** Nationwide | **6th Rank - Hardware Hustle** | **2nd Rank - Logical League

This repository contains the **complete firmware, control logic, and system-level integration** developed by **Team Mahagatbandhan** for the **LAM Research Challenge 2025 – Grand Finale**.

Our team was:
- **Selected among the Top 24 teams** across India from over 7000+ participants 
- 🏅 **Ranked 6th** in the **Hardware Hustle round**


![Uploading main-game-1.jpeg…](![main-game-2](https://github.com/user-attachments/assets/06012379-e962-40c6-9ed3-dc57447c0da7)
)

The project demonstrates a **full-stack embedded robotics system**, integrating autonomous navigation, closed-loop control, sensing, and arena-level automation under real-world constraints.

---

## System Overview

The final system is composed of **three tightly integrated subsystems**:

Subsystem | Description  
--- | ---  
**ALFR** | Autonomous Line Following Robot with PID control  
**SARM** | Smart Automated Robotic Manipulator + square base wheel drive  
**Arena** | Central automation system (gates, pump, weighing, validation)

Each subsystem was developed independently and later integrated into a **single deterministic workflow**.

---

## 1. ALFR — Autonomous Line Following Robot

### Objective
ALFR autonomously navigates the arena using **PID-based line following**, while ensuring safety via **ultrasonic obstacle detection**.

### Hardware Architecture
- **Controller**: Arduino Nano  
- **Drive**: Differential drive (DC motors + L298N)  
- **Sensors**:
  - 8-channel IR reflectance sensor array  
  - HC-SR04 ultrasonic sensor  

### Sensor Calibration
Before operation, the robot performs **dynamic calibration** by rotating in place and recording minimum and maximum reflectance values for each sensor.  
This makes the system robust to lighting variations and surface changes.

### Line Position Estimation
A weighted average method is used to compute line deviation, enabling sub-sensor resolution steering and smooth corrections.

### PID Control
The control law is:
```
output = KP × error + KD × (error − lastError)
```
- **KP = 15.0**
- **KD = 12.0**
- **KI = 0.0** (disabled to prevent wind-up)

### Motor Control
Anti-stall logic enforces a minimum PWM threshold to ensure reliable motion at low speeds.

### Obstacle Avoidance
Ultrasonic sensing overrides navigation logic to stop the robot when an obstacle is detected within the safety threshold.

---

## 2. SARM — Smart Automated Robotic Manipulator

SARM consists of:
1. **Square base wheel drive**
2. **Bluetooth-controlled 5-DOF robotic arm**

### Square Base Wheel Drive
Holonomic motion is achieved using mecanum-style kinematic mixing, allowing translation and rotation simultaneously.

### Square Mapping & Ramping
Non-linear square mapping improves low-speed precision, while ramping limits mechanical stress.

### Robotic Arm Control
- Incremental servo motion
- Software-enforced joint limits
- Smooth Bluetooth-based teleoperation

---

## 3. Arena System — Central Automation Backbone

The Arena system coordinates **IR gates, pump control, load-cell feedback, TFT display output, and final validation**.

### State Machine Design
Each gate triggers exactly once, ensuring deterministic execution.

### Closed-Loop Filling
Pump operation stops automatically once the target weight is reached or a safety timeout occurs.

### User Feedback
A TFT display and LEDs provide real-time system status and final success/failure indication.

---

## Hardware Hustle Highlights
- **6th Rank nationally**
- Stable PID navigation
- Accurate closed-loop filling
- Zero system crashes under competition pressure

---

## Repository Structure
```
Lam-Research-Challenge-2025--Grand-Finale/
├── ALFR/
├── SARM/
├── Arena/
└── README.md
```

---

## Team Mahagatbandhan
- Shreyans Jain (Mechanical Engineering, Team Leader)
- Mohammad Dilshan Alam (Computer Science Engineering)
- Sambhav Singh Aditya (Computer Science Engineering)
- Verchasv Garg (Electrical Engineering)

![full-bothteams](https://github.com/user-attachments/assets/9513ad6f-4f49-4d05-bbce-6f29a768346a)


