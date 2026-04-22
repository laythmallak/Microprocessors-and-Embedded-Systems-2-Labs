#include <TimerOne.h>

//pin definitions
#define TRIG_PIN 9
#define ECHO_PIN 10

#define ENA 6
#define IN1 7
#define IN2 8

#define BUZZER_PIN 4

//global variables
volatile float distance_cm = 0;
volatile int pwmValue = 0;
volatile int alarmState = 0;

// setup
void setup() {
    Serial.begin(9600);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

    pinMode(BUZZER_PIN, OUTPUT);

    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    // Timer1 every 100ms
    Timer1.initialize(100000);
    Timer1.attachInterrupt(timerISR);
}

//main loop
void loop() {
    // everything handled in ISR
}

//distance measurement
float measureDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH);

    float dist = duration * 0.034 / 2.0;
    return dist;
}

// PWM mapping
int distance_to_pwm(float d) {
    if (d > 75) return 255;
    else if (d > 50) return 191;
    else if (d > 25) return 128;
    else return 0;
}

//classifying the alarm
int classify_alarm(float d) {
    if (d >= 60) return 0;
    else if (d >= 20) return 1;
    else return 2;
}

//motor control
void setMotorSpeed(int pwm) {
    analogWrite(ENA, pwm);

    if (pwm == 0) {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
    } else {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
    }
}

// buzzer control
void updateBuzzer(float d, int alarm) {
    if (alarm == 0) {
        noTone(BUZZER_PIN);
    }
    else if (alarm == 1) {
        int freq = map(d, 20, 60, 2000, 500);
        tone(BUZZER_PIN, freq);
    }
    else {
        tone(BUZZER_PIN, 2000);
    }
}

//timer ISR
void timerISR() {
    distance_cm = measureDistance();
    pwmValue = distance_to_pwm(distance_cm);
    alarmState = classify_alarm(distance_cm);

    setMotorSpeed(pwmValue);
    updateBuzzer(distance_cm, alarmState);

    Serial.print(distance_cm);
    Serial.print(",");
    Serial.print(pwmValue);
    Serial.print(",");
    Serial.println(alarmState);
}
