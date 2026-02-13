# Microprocessors-and-Embedded-Systems-2-Labs
Lab work for Micro 2. Group work. Labs are in separate folders. Make use of test branches.

***********************************************
***********************************************
GROUP WORK TODOS-

Build circuit

Define the required states

Identify triggers for state transitions

Define state transitions
*************************************************
*************************************************


LAB REQUIREMENTS:


This lab is to design a controller for traffic lights that face one traffic direction. There are three
lights: Red, Yellow, Green, which can be represented with LEDs of respective color.
The traffic lights operate in the following patterns:


(1) At the start of the system (power up), the Red light flashes (1-second on then 1-second
off) until the duration of both Red and Green lights are set (see (2)) AND “*” key is
pressed once (see (3)).


(2) Use the 16-button keypad to set the duration of Red and Green lights. For example,
pressing a sequence of “A”-“2”-“4”-“#” will set the Red light to stay on for 24 seconds.
Similarly, pressing a sequence of “B”-“2”-“0”-“#” will set the Green light to stay on for 20
seconds. Yellow light is on for 3 seconds and no need to change that


(3) After setting the light durations, press “*” key to start the operation of the traffic lights.


(4) The Red light stays on for 24 seconds (or X seconds based on the keypad inputs) before
the Green light is turned on. During the last three seconds, flash the Red light (0.5
second on then 0.5 second off repeatedly)


(5) The Green light stays on for 20 seconds (or X seconds based on the keypad inputs)
before switching the Yellow light on. During the last three seconds, flash the Green light
(0.5 second on then 0.5 second off repeatedly)


(6) The Yellow light stays on for 3 seconds before switching back to Red light. No need to
flash.


(7) The R-G-Y pattern continues until the system is powered off


(8) An active buzzer beeps for 3 seconds before a light is changed.


(9) Press “#” on the keypad twice to switch from the normal operation mode to “failure”
mode, where Red light flashes (0.5 second on then 0.5 second off) repeatedly, until
power resets or keypad inputs set the Green/Red duration (see (2)) again.

***************************************************************************************************************************
***************************************************************************************************************************

STEPS FOR LAB:

Step 1:
You can first check the resource files under the following folders
2.1 LED
2.3 Digital Inputs
2.5 Active Buzzer

These folders contain useful instructions on hardware wiring and sample code to drive these
components. You are strongly suggested to run these examples on your Arduino kit, and build
on them to implement Lab 1.


Step 2:
Using what you learn from Step 1, wire up the Arduino with other components on your
breadboard. Double check the wiring and basic sample code to make sure you can (a) turn
on/off LED (b) detect input from a switch (c) make beep sound with the active buzzer (d) make a
“clock” variable that counts down on every second


Step 3:
You will then need to design a state machine to implement the control logic on the rotating
pattern of traffic lights. The example 3.9 on page 62 of the textbook shows a traffic light control
at a pedestrian crosswalk. You can refer to that as a starting point to design yours for Lab 1.


Step 4:
Based on step 3, Implement the necessary code to represent the states and drive the state
transitions


Step 5:
Test and debug your design.


Step 6:
Push your source code to a private github repository
Optionally record demo video and post it on Youtube.