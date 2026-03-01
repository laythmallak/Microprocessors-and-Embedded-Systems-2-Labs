#include <avr/interrupt.h>
#include <Arduino.h>

extern "C" {
  void gpio_init();
  void red_on();    void red_off();
  void green_on();  void green_off();
  void yellow_on(); void yellow_off();
  void buzzer_on(); void buzzer_off();
}

// LEDs: D2 red, D3 green, D4 yellow
// Keypad: D5–D12
// Buzzer: D13

static const uint8_t rowPins[4] = {12, 11, 10, 9};
static const uint8_t colPins[4] = {8, 7, 6, 5};

static const char keymap[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

volatile bool tick1s = false;
volatile bool tick500 = false;

enum State { IDLE_FLASH, RED_RUN, GREEN_RUN, YELLOW_RUN, FAIL_FLASH };
static State state = IDLE_FLASH;

static int redSeconds = -1;
static int greenSeconds = -1;
static int secondsLeft = 0;

static bool running = false;
static bool failureMode = false;
static bool flashLedOn = false;

static bool programming = false;
static char programTarget = 0;
static int  programValue = 0;

static inline bool durationsSet() {
  return (redSeconds > 0 && greenSeconds > 0);
}

static void all_off() {
  red_off(); green_off(); yellow_off(); buzzer_off();
}

static void enter_idle() {
  failureMode = false;
  running = false;
  state = IDLE_FLASH;
  all_off();
  flashLedOn = false;
}

static void enter_fail() {
  failureMode = true;
  running = false;
  state = FAIL_FLASH;
  all_off();
  flashLedOn = false;
}

// Timer1 1Hz
static void timer1_init_1hz() {
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 15624;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

ISR(TIMER1_COMPA_vect) {
  tick1s = true;
}

// Timer2 500ms 
static void timer2_init_2hz() {
  cli();
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2  = 0;

  OCR2A = 249;
  TCCR2A |= (1 << WGM21);
  TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
  TIMSK2 |= (1 << OCIE2A);
  sei();
}

ISR(TIMER2_COMPA_vect) {
  static uint8_t div = 0;
  div++;
  if (div >= 31) {
    div = 0;
    tick500 = true;
  }
}

// Keypad  
static char scanKey() {
  for (int c = 0; c < 4; c++) {
    for (int k = 0; k < 4; k++)
      pinMode(colPins[k], INPUT_PULLUP);

    pinMode(colPins[c], OUTPUT);
    digitalWrite(colPins[c], LOW);

    for (int r = 0; r < 4; r++) {
      if (digitalRead(rowPins[r]) == LOW) {
        pinMode(colPins[c], INPUT_PULLUP);
        return keymap[r][c];
      }
    }
  }
  return 0;
}

static void handleKey(char k) {
  static bool sawHash = false;

  if (k == '#') {
    if (sawHash) {
      sawHash = false;
      enter_fail();
      programming = false;
      return;
    }
    sawHash = true;
  } else sawHash = false;

  if (k == 'A' || k == 'B') {
    programming = true;
    programTarget = k;
    programValue = 0;
    return;
  }

  if (programming && k >= '0' && k <= '9') {
    programValue = programValue * 10 + (k - '0');
    return;
  }

  if (programming && k == '#') {
    if (programTarget == 'A') redSeconds = programValue;
    if (programTarget == 'B') greenSeconds = programValue;
    programming = false;

    if (failureMode && durationsSet()) enter_idle();
    return;
  }

  if (k == '*') {
    if (!failureMode && durationsSet()) {
      running = true;
      state = RED_RUN;
      secondsLeft = redSeconds;
      red_on(); green_off(); yellow_off(); buzzer_off();
    }
  }
}

static void checkKeypad() {
  static char last = 0;
  static unsigned long lastMs = 0;

  char k = scanKey();
  if (!k) { last = 0; return; }
  if (k == last) return;
  if (millis() - lastMs < 120) return;

  lastMs = millis();
  last = k;
  handleKey(k);
}

// flashing output 
static void applyFlash() {
  if (state == FAIL_FLASH) {
    if (flashLedOn) red_on(); else red_off();
    green_off(); yellow_off(); buzzer_off();
    return;
  }

  if (state == RED_RUN && secondsLeft <= 3 && secondsLeft > 0) {
    if (flashLedOn) red_on(); else red_off();
    buzzer_on();
  }

  if (state == GREEN_RUN && secondsLeft <= 3 && secondsLeft > 0) {
    if (flashLedOn) green_on(); else green_off();
    buzzer_on();
  }
}

// 1sec state machine 
static void tickSM() {
  static bool idle = false;

  if (state == IDLE_FLASH) {
    idle = !idle;
    if (idle) red_on(); else red_off();
    return;
  }

  if (state == FAIL_FLASH || !running) return;

  switch (state) {
    case RED_RUN:
      if (secondsLeft > 3) red_on();
      secondsLeft--;
      if (secondsLeft <= 0) {
        red_off();
        state = GREEN_RUN;
        secondsLeft = greenSeconds;
        green_on();
      }
      break;

    case GREEN_RUN:
      if (secondsLeft > 3) green_on();
      secondsLeft--;
      if (secondsLeft <= 0) {
        green_off();
        state = YELLOW_RUN;
        secondsLeft = 3;
        yellow_on();
      }
      break;

    case YELLOW_RUN:
      secondsLeft--;
      if (secondsLeft <= 0) {
        yellow_off();
        state = RED_RUN;
        secondsLeft = redSeconds;
        red_on();
      }
      break;

    default: break;
  }
}

void setup() {
  gpio_init();

  for (int r = 0; r < 4; r++) pinMode(rowPins[r], INPUT_PULLUP);
  for (int c = 0; c < 4; c++) pinMode(colPins[c], INPUT_PULLUP);

  timer1_init_1hz();
  timer2_init_2hz();
  enter_idle();
}

void loop() {
  checkKeypad();

  if (tick500) {
    tick500 = false;
    flashLedOn = !flashLedOn;
    applyFlash();
  }

  if (tick1s) {
    tick1s = false;
    tickSM();
    applyFlash();
  }
}