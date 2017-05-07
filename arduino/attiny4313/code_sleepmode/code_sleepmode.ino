#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>


// buttons
const uint16_t button_CZl = 1 << 0; //D0
const uint16_t button_H   = 1 << 2; //D2
const uint16_t button_Z   = 1 << 3; //D3
const uint16_t button_P   = 1 << 4; //D4
const uint16_t button_CZr = 1 << 5; //D5
const uint16_t button_M   = 1 << 6; //D6

// all buttons - useful variable to have
const uint16_t all_buttons = button_CZl | button_H | button_Z | button_P | button_CZr | button_M;

// LEDs
// 1 at bit 8 to indicate register B.
const uint16_t LED_zero = 1 << 4 | 1 << 8; //B4
const uint16_t LED_one  = 1 << 3 | 1 << 8; //B3
const uint16_t LED_two  = 1 << 2 | 1 << 8; //B2
const uint16_t LED_act  = 1 << 1 | 1 << 8; //B1


// jacks
// 1 at bit 9 to indicate register A.
const uint16_t TX_l = 1 << 1 | 1 << 9; // A1
const uint16_t RX_l = 1 << 1;      // D1
const uint16_t TX_r = 1 << 0 | 1 << 9; // A0
const uint16_t RX_r = 1 << 0 | 1 << 8; // B0



// Variables:
uint8_t internalState = 0;
uint8_t logicalState = 0;
uint8_t randloop=0;

void delay_ms(int t) {delay(4*t);}

void sleepnow()
{
  //ACSR = 1<<ACD;
  //PRR= 0xff;  
  cli();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  power_all_disable ();
  sleep_enable();                //enable sleep mode, a safety pin
  BODCR= 1<<1|1<<0;
  BODCR=1<<1;
  GIFR |= 1<<PCIF2;
  sei();
  sleep_cpu();                  // go into sleep mode.
  // interrupt
  cli();
  sleep_disable();              //disable sleep mode
  power_all_enable();
}

ISR(PCINT2_Vect){
}


/* Function: Write
   ---------------------
   'pin':     pin number
   'is_high': HIGH or LOW
    Writes either HIGH or LOW on pin 'pin'.
*/
void Write(uint16_t pin, uint8_t is_high) {
  if (pin & (1 << 8)) { //1 at bit 8 -> register B
    if (is_high) PORTB |= (pin & ~(1 << 8));
    else PORTB &= ~(pin & ~(1 << 8));
  }
  else if (pin & (1 << 9)) { //1 at bit 9 -> register A
    if (is_high) PORTA |= (pin & ~(1 << 9));
    else PORTA &= ~(pin & ~(1 << 9));
  }
  else {
    if (is_high) PORTD |= pin;
    else PORTD &= ~pin;
  }
}



/* Function: Read
   ---------------------
   'pin':     pin number
    reads pin state HIGH or LOW
*/
uint8_t Read(uint16_t pin) {
  if (pin & (1 << 8)) { //read register B
    if (PINB & (pin & ~(1 << 8))) return HIGH;
    return LOW;
  }
  else { // read register D (no input on register A)
    if (PIND & pin) return HIGH;
    return LOW;
  }
}


/* Function: M_down
   ---------------------
   Measure button pressed
*/
void M_down() {
  // randomize internal state
  internalState = randloop;
  // turn LED on:
  if (logicalState == 1) {
    Write(LED_one, HIGH);
  }
  else if (logicalState == 2) {
    Write(LED_two, HIGH);
  }
  else {
    Write(LED_zero, HIGH);
  }
}

/* Function: M_up
   ---------------------
   Measure button not pressed
*/
void M_up() {
  Write(LED_zero | LED_one | LED_two | LED_act , LOW);
  // only works because all LEDs are on the same register.
}

/* Function: H_down
   ---------------------
   Hadamard button pressed
*/
void H_down() {
  int8_t dummy = internalState;
  internalState = logicalState;
  logicalState = dummy;
}

/* Function: H_up
   ---------------------
   Hadamard button not pressed
*/
void H_up() {
  // Do nothing
}

/* Function: Z_down
   ---------------------
   Z button pressed
*/
void Z_down() {
  internalState = (internalState + 1) % 2;
}

/* Function: Z_up
   ---------------------
   Z button not pressed
*/
void Z_up() {
  // Do nothing
}

/* Function: P_down
   ---------------------
   Phase button pressed
*/
void P_down() {
  internalState = (internalState + logicalState) % 2;
}

/* Function: P_up
   ---------------------
   Phase button not pressed
*/
void P_up() {
  // Do nothing
}

/* Function: CZ_down
   ---------------------
   'left': 1 or 0 for left or right CZ button
    CZ button pressed
*/
void CZ_down(uint8_t left) {
  //pin ID for left or right CZ
  uint16_t button_CZ =  (left ? button_CZl : button_CZr);
  uint16_t TX = (left ? TX_l : TX_r);
  uint16_t RX =  (left ? RX_l : RX_r);

  // set TX low
  Write(TX, LOW);

  // RX state high
  uint8_t RX_state = HIGH;
  // while CZ is pressed
  while (Read(button_CZ) == LOW) {
    RX_state = Read(RX);
    if (RX_state == HIGH) delay_ms(10);
    else {
      Write(LED_act, HIGH);
      delay_ms(100);
      Write(TX, logicalState == 0 ? HIGH : LOW);
      delay_ms(50);
      RX_state = Read(RX);
      if (RX_state == LOW) internalState = (internalState + 1) % 2;
      delay_ms(50);
      break;
    }
  }

  Write(TX, HIGH);
  Write(LED_act, LOW);
}


void Wait(uint8_t button_pin){
  Write(LED_act,HIGH);
  while(Read(button_pin)==LOW) delay_ms(50);
  Write(LED_act,LOW);
}





void setup() {
  // initialize output on register A:
  DDRA |= (TX_l | TX_r) & ~(1 << 9);
  // initialize input on register A:
  // --
  // initialize output on register B:
  DDRB |= (LED_zero | LED_one | LED_two | LED_act) & ~(1 << 8);
  // initialize input on register B:
  DDRB &= ~(RX_r & ~(1 << 8));
  // initialize input on register D:
  DDRD &= ~(all_buttons | RX_l);
  // internal pullup resistors for buttons and RX on register D:
  PORTD |= (all_buttons | RX_l);
  // internal pullup resistors for RX on register B:
  PORTB|= (RX_r & ~(1 << 8)); //set output to 0

  // set TX HIGH
  PORTA |= ((TX_l | TX_r) & ~(1 << 9));

  // initialize logical and internal state
  logicalState = 1;
  internalState = 0;


  
  // Pin change interrupt
  cli();                      // turn interrupts off while changing them.
  GIMSK |= (1<<PCIE2);            // enable Pin interrupt on register D (see datasheet)
  PCMSK2 |= all_buttons;      // use any button for interrupt.

}


uint8_t button_state = all_buttons;         // 0 on bit x iff button x pressed

void loop() {
  delay_ms(20);
  button_state = PIND & all_buttons; //read state of buttons from register D.

  /*
  Write(LED_act,HIGH);
  delay_ms(1000);
  Write(LED_act,LOW);
  delay_ms(1000);
  Write(LED_act,HIGH);
  delay_ms(1000);
  Write(LED_act,LOW);
  delay_ms(1000);
  Write(LED_act,HIGH);
  delay_ms(1000);
  Write(LED_act,LOW);
  delay_ms(1000);
    Write(LED_act,HIGH);
  delay_ms(1000);
  Write(LED_act,LOW);
  delay_ms(1000);
  */


  
  if ((button_state & button_M) == LOW){
    M_down();
    Write(LED_act,HIGH);
    while(Read(button_M)==LOW) { delay_ms(50); randloop ^=1;}
    Write(LED_act,LOW);
    M_up();
  }

  else if ((button_state & button_H) == LOW){
    H_down();
    Wait(button_H);
    H_up();
  }

  else if ((button_state & button_Z) == LOW){
    Z_down();
    Wait(button_Z);
    Z_up();
  }

  else if ((button_state & button_P) == LOW){
    P_down();
    Wait(button_P);
    P_up();
  }

  else if ((button_state & button_CZl) == LOW){
    CZ_down(1);
  }

  else if ((button_state & button_CZr) == LOW){
    CZ_down(0);
  }

  delay_ms(50);

  sleepnow();
}

