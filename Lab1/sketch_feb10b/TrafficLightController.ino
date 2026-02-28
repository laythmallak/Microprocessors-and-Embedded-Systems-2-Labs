extern "C" {
  void red_on();
  void red_off();
  void green_on();
  void green_off();
  void yellow_on();
  void yellow_off();
  void buzzer_on();
  void buzzer_off();
  void gpio_init();
}

void setup() {
  gpio_init();
  timer1_init();
}

void loop() {
  if(oneSecondFlag){
    oneSecondFlag = false;
    trafficLightStateMachine();
  }

  checkKeypad();
}