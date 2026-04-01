Lab 2 – MPU-6050 IMU Data Acquisition and Processing

Overview

In this lab you will build a complete pipeline for acquiring, processing, and visualizing data from the MPU-6050 6-axis IMU (accelerometer + gyroscope).

You will work through Steps 1–5, progressively implementing the core algorithms. Step 7 is a provided demonstration — run it and explore, no coding required.

Hardware
Arduino Mega 2560
MPU-6050 breakout board
USB cable + jumper wires

Wiring:

MPU-6050	Arduino Mega
VCC	5 V
GND	GND
SDA	Pin 20
SCL	Pin 21
AD0	GND
Setup
1. Flash the Arduino

Open arduino/imu_stream/imu_stream.ino in the Arduino IDE and flash it to the Arduino Mega.

2. Create a Python virtual environment

macOS / Linux

cd python
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt

Windows (PowerShell)

cd python
python -m venv venv
venv\Scripts\Activate.ps1
pip install -r requirements.txt

Re-activate the venv (source venv/bin/activate) each time you open a new terminal.

3. Test without hardware (simulator mode)
python step1_stream.py --simulate
Lab Steps
Step	Script	Who	Status
1 — Raw Streaming & Logging	step1_stream.py	All	Provided
2 — Real-Time 6-Axis Plot	step2_live_plot.py	All	Provided
3 — Accel Roll & Pitch	step3_accel_angles.py	All	TODO
4 — Gyro Integration	step4_gyro_integrate.py	All	TODO
5 — Complementary Filter	step5_comp_filter.py	All	TODO
7 — 3D Viz + Marble Madness	step7_3d_viz.py	All (demo)	Provided
What to Implement

Search each file for TODO markers:

File	What to implement
arduino/imu_stream/imu_stream.ino	writeRegister(), readRegister(), and the I2C burst-read + byte-combine + unit-convert sections of readAndSendIMU()
python/step3_accel_angles.py	accel_angles() — gravity-projection formulas for roll & pitch
python/step4_gyro_integrate.py	GyroIntegrator.update() — angle += gyro_rate × dt
python/step5_comp_filter.py	ComplementaryFilter.update() — fuse accel and gyro with α coefficient

Ignore Step 6 — you are not EECE 5520.

All other code (serial handshake, threading, plotting, rendering) is fully provided.

Running Steps 1–5
# Hardware (auto-detect port)
python step3_accel_angles.py

# Specify port explicitly
python step3_accel_angles.py --port /dev/tty.usbmodem1101   # macOS
python step3_accel_angles.py --port COM3                     # Windows

# Simulator (no Arduino needed)
python step3_accel_angles.py --simulate
Running Step 7 (provided demo)
python step7_3d_viz.py --simulate

Controls: drag the 3D panel to orbit the camera · V = reset view · R = restart game · ESC = quit

Provided Files (do not modify)
File	Description
python/imu_reader.py	Serial driver + simulator — fully provided
python/step1_stream.py	Raw data streaming + CSV logger
python/step2_live_plot.py	Real-time 6-axis plot
python/step7_3d_viz.py	3D orientation viewer + Marble Madness game
Files With TODOs
File	What to implement
arduino/imu_stream/imu_stream.ino	I2C register read/write + 14-byte burst read
python/step3_accel_angles.py	accel_angles()
python/step4_gyro_integrate.py	GyroIntegrator.update()
python/step5_comp_filter.py	ComplementaryFilter.update()
Deliverables

Submit on Blackboard:

GitHub repository link — your fork/clone with all steps implemented
PDF report — answers to all lab questions + required plots

See docs/lab-writeup.md for full assignment details.
