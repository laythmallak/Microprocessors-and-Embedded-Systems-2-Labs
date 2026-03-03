#include <avr/interrupt.h>
#include <Arduino.h>

/*
  functions are implemented in traffic.S using AVR assembly
  extern "C" prevents C++ name mangling
*/
extern "C" {
  void gpio_init();
  void red_on();    void red_off();
  void green_on();  void green_off();
  void yellow_on(); void yellow_off();
  void buzzer_on(); void buzzer_off();
}

/*
  Hardware pin mapping:
  - LEDs: D2 red, D3 green, D4 yellow
  - Keypad: D5–D12 (4 rows + 4 cols)
  - Buzzer: D13
*/

/*
  Keypad wiring:
  rowPins[] are the 4 row pins, colPins[] are the 4 column pins
  The keypad is scanned by driving one column LOW at a time and reading rows
*/
static const uint8_t rowPins[4] = {12, 11, 10, 9};
static const uint8_t colPins[4] = {8, 7, 6, 5};

/*
  Layout of 4x4 keypad
  keymap[r][c] is the char returned when row r and col c are connected.
*/
static const char keymap[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

/*
  Timer flags:
  main loop checks them.
*/
volatile bool tick1s = false;    // set by Timer1 ISR (1 sec)
volatile bool tick500 = false;   // set by Timer2 ISR (0.5 sec)

/*
  State machine states:
  - IDLE_FLASH: flashes red 1 second on/off until user starts
  - RED_RUN / GREEN_RUN / YELLOW_RUN: normal cycles
  - FAIL_FLASH: flashing red failure mode
*/
enum State { IDLE_FLASH, RED_RUN, GREEN_RUN, YELLOW_RUN, FAIL_FLASH };
static State state = IDLE_FLASH;

/*
  User controlled durations:
  redSeconds set by A(number))#
  greenSeconds set by B(number)#
*/
static int redSeconds = -1;
static int greenSeconds = -1;

/*
  secondsLeft is countdown timer for current state
*/
static int secondsLeft = 0;

static bool running = false;       // becomes TRUE after '*' starts the cycle
static bool failureMode = false;   // TRUE when "##" triggers failure mode
static bool flashLedOn = false;    // toggles ON/OFF during flashing intervals

/*
  Key vars:
  After pressing A or B collect inputs (keypad digits) until # is pressed
*/
static bool programming = false;
static char programTarget = 0;     // 'A' = red, 'B' = green
static int  programValue = 0;      // number being typed in

// Truewhen both timing vals are valid positive numbers
static inline bool durationsSet() {
  return (redSeconds > 0 && greenSeconds > 0);
}

// turn everything off 
static void all_off() {
  red_off();
  green_off();
  yellow_off();
  buzzer_off();
}

// back to startup behavior, flash red at 1 sec
static void enter_idle() {
  failureMode = false;
  running = false;
  state = IDLE_FLASH;
  all_off();
  flashLedOn = false;
}

// enter failure mode, flashing red for 0.5s until durations are set
static void enter_fail() {
  failureMode = true;
  running = false;
  state = FAIL_FLASH;
  all_off();
  flashLedOn = false;
}

/*
  Timer1 setup: 1 Hz interrupt.
  16 MHz / 1024 prescaler = 15625 counts/sec
  OCR1A=15624 gives one compare match per sec in CTC
*/
static void timer1_init_1hz() {
  cli();          // disable global interrupts during setup
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 15624;
  TCCR1B |= (1 << WGM12);               // CTC
  TCCR1B |= (1 << CS12) | (1 << CS10);  // prescaler 1024
  TIMSK1 |= (1 << OCIE1A);              // enable compare match interrupt
  sei();          //enable global interrupts
}

//Timer1 ISR: sets a flag
ISR(TIMER1_COMPA_vect) {
  tick1s = true;
}

/*
  Timer2 setup:

  Timer2 is 8-bit, can'tcount up to 0.5 sec
  generate a faster interrupt of 62.5 Hz and divide it

  OCR2A=249 and prescaler 1024:
  Timer2 interrupt rate ≈ 16ms
  count 31 interrupts
  31 * 16ms ≈ 496ms
*/
static void timer2_init_2hz() {
  cli();
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2  = 0;

  OCR2A = 249;
  TCCR2A |= (1 << WGM21);                             // CTC mode
  TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);  // prescaler 1024
  TIMSK2 |= (1 << OCIE2A);                            // enable compare match
  sei();
}

// Timer2 ISR: divide to 0.5 sec, set flag
ISR(TIMER2_COMPA_vect) {
  static uint8_t div = 0;
  div++;
  if (div >= 31) {
    div = 0;
    tick500 = true;
  }
}

/*
  scanKey():
  - set all col to INPUT_PULLUP, all cols are HIGH
  - drive one column LOW
  - check rows: if row LOW,pressed key connects row+col
  Return 0 if nothing pressed.
*/
static char scanKey() {
  for (int c = 0; c < 4; c++) {
    for (int k = 0; k < 4; k++) {
      pinMode(colPins[k], INPUT_PULLUP);
    }

    pinMode(colPins[c], OUTPUT);
    digitalWrite(colPins[c], LOW);

    for (int r = 0; r < 4; r++) {
      if (digitalRead(rowPins[r]) == LOW) {
        pinMode(colPins[c], INPUT_PULLUP); // put column back before returning
        return keymap[r][c];
      }
    }
  }
  return 0;
}

/*
  handleKey():
  Handles keypad commands:
  - "##" triggers failure mode
  - 'A'(num)'#' sets redSeconds
  - 'B'(num)'#' sets greenSeconds
  - '*' starts cycles if inputs valid
*/
static void handleKey(char k) {
  static bool sawHash = false; // detects "##"

  // Detect failure command "##"
  if (k == '#') {
    if (sawHash) {
      sawHash = false;
      enter_fail();
      programming = false;   // partial num entry
      return;
    }
    sawHash = true;
  } else {
    sawHash = false;
  }

  // start entering a value for red or green
  if (k == 'A' || k == 'B') {
    programming = true;
    programTarget = k;
    programValue = 0;
    return;
  }

  // combine digits
  if (programming && k >= '0' && k <= '9') {
    programValue = programValue * 10 + (k - '0');
    return;
  }

  // Finish num put with '#'
  if (programming && k == '#') {
    if (programTarget == 'A') redSeconds = programValue;
    if (programTarget == 'B') greenSeconds = programValue;
    programming = false;

    // If in failure mode setting valid times returns state back to idle mode
    if (failureMode && durationsSet()) {
      enter_idle();
    }
    return;
  }

  // Start light cycle
  if (k == '*') {
    if (!failureMode && durationsSet()) {
      running = true;
      state = RED_RUN;
      secondsLeft = redSeconds;

      red_on();
      green_off();
      yellow_off();
      buzzer_off();
    }
  }
}

/*
  checkKeypad():
  - reads keys using scanKey()
  - debounces using new key press and short delay
  - sends key to handleKey()
*/
static void checkKeypad() {
  static char last = 0;
  static unsigned long lastMs = 0;

  char k = scanKey();

  // no key pressed,reset last, next press accepted
  if (!k) {
    last = 0;
    return;
  }

  // same key still held, ignore
  if (k == last) return;

  // debounce timing
  if (millis() - lastMs < 120) return;

  lastMs = millis();
  last = k;
  handleKey(k);
}

/*
  applyFlash():
  - fast blinking driven by tick500 
  - FAIL_FLASH: red flashes at 0.5s
  - RED_RUN/GREEN_RUN: last 3 secs flash + buzzer warning
*/
static void applyFlash() {
  // Failure mode: flash red
  if (state == FAIL_FLASH) {
    if (flashLedOn) red_on(); else red_off();
    green_off();
    yellow_off();
    buzzer_off();
    return;
  }

  // Warning phase for red
  if (state == RED_RUN && secondsLeft <= 3 && secondsLeft > 0) {
    if (flashLedOn) red_on(); else red_off();
    buzzer_on();
  }

  // Warning phase for green
  if (state == GREEN_RUN && secondsLeft <= 3 && secondsLeft > 0) {
    if (flashLedOn) green_on(); else green_off();
    buzzer_on();
  }
}

/*
  tickSM():
  Main 1-sec state machine,
  called when tick1s is set by Timer1
*/
static void tickSM() {
  static bool idle = false;

  // Startup mode: flash red 1 sec on/off until '*' pressed
  if (state == IDLE_FLASH) {
    idle = !idle;
    if (idle) red_on(); else red_off();
    return;
  }

  // Failure mode- applyFlash() on the 0.5s tick
  if (state == FAIL_FLASH || !running) return;

  switch (state) {
    case RED_RUN:
      // Normal red stays solid until last 3 sec 
      if (secondsLeft > 3) {
        red_on();
        buzzer_off();
      }

      secondsLeft--;

      // Transition to green when countdown hits 0
      if (secondsLeft <= 0) {
        red_off();
        buzzer_off();
        state = GREEN_RUN;
        secondsLeft = greenSeconds;
        green_on();
      }
      break;

    case GREEN_RUN:
      if (secondsLeft > 3) {
        green_on();
        buzzer_off();
      }

      secondsLeft--;

      // Transition to yellow, 3 sec
      if (secondsLeft <= 0) {
        green_off();
        buzzer_off();
        state = YELLOW_RUN;
        secondsLeft = 3;
        yellow_on();
      }
      break;

    case YELLOW_RUN:
      secondsLeft--;

      // Transition back to red
      if (secondsLeft <= 0) {
        yellow_off();
        state = RED_RUN;
        secondsLeft = redSeconds;
        red_on();
      }
      break;

    default:
      break;
  }
}

void setup() {
  // Config LED pins + buzzer pin in assembly
  gpio_init();

  // Rows/cols idle HIGH using internal pull-up resistors
  // Cols driven LOW during scanning
  for (int r = 0; r < 4; r++) pinMode(rowPins[r], INPUT_PULLUP);
  for (int c = 0; c < 4; c++) pinMode(colPins[c], INPUT_PULLUP);

  timer1_init_1hz();
  timer2_init_2hz();

  // Start in idle flash mode
  enter_idle();
}

void loop() {
  // Check keypad in the background
  checkKeypad();

  // 0.5s tick- toggle flash state, updates outputs requiring flashing
  if (tick500) {
    tick500 = false;
    flashLedOn = !flashLedOn;
    applyFlash();
  }

  // 1s tick- update the main state machine
  if (tick1s) {
    tick1s = false;
    tickSM();
    applyFlash(); // keeps warning flashing/buzzer consistent after state updates
  }
}