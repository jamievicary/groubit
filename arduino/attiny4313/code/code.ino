#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>

// buttons
const uint16_t button_CZl = 1<<0; //D0
const uint16_t button_H   = 1<<2; //D2
const uint16_t button_Z   = 1<<3; //D3
const uint16_t button_P   = 1<<4; //D4
const uint16_t button_CZr = 1<<5; //D5
const uint16_t button_M   = 1<<6; //D6

// all buttons - useful variable to have
const uint16_t all_buttons = button_CZl | button_H | button_Z | button_P | button_CZr | button_M;

// LEDs
// 1 at bit 8 to indicate register B.
const uint16_t LED_zero = 1<<4 | 1<<8; //B4
const uint16_t LED_one  = 1<<3 | 1<<8; //B3
const uint16_t LED_two  = 1<<2 | 1<<8; //B2
const uint16_t LED_act  = 1<<1 | 1<<8; //B1


// jacks
// 1 at bit 9 to indicate register A.
const uint16_t TX_l = 1<<1 | 1<<9; // A1
const uint16_t RX_l = 1<<1;        // D1
const uint16_t TX_r = 1<<0 | 1<<9; // A0
const uint16_t RX_r = 1<<0 | 1<<8; // B0



// Variables:
uint8_t internalState = 0;         
uint8_t logicalState = 0; 



void sleep()
 {  
  cli();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  
  power_all_disable ();
  sleep_enable();                //enable sleep mode, a safety pin
  sei();
  sleep_cpu();                  // go into sleep mode.
  // interrupt 
  sleep_disable();              //disable sleep mode
  power_all_enable();
  sei();
 }  


    
/* Function: Write
 * ---------------------
 * 'pin':     pin number 
 * 'is_high': HIGH or LOW
 *  Writes either HIGH or LOW on pin 'pin'.
 */
void Write(uint16_t pin, uint8_t is_high) {
  if (pin & (1<<8)){ //1 at bit 8 -> register B
    if(is_high) PORTB |= (pin & ~(1<<8));
    else PORTB &= ~(pin & ~(1<<8));
  }
  else if (pin & (1<<9)){ //1 at bit 9 -> register A
    if(is_high) PORTA |= (pin & ~(1<<9));
    else PORTA &= ~(pin & ~(1<<9));
  }
  else {
    if(is_high) PORTD |= pin;
    else PORTD &= ~pin;
  }
}



/* Function: Read
 * ---------------------
 * 'pin':     pin number 
 *  reads pin state HIGH or LOW 
 */
uint8_t Read(uint16_t pin){
  if(pin & (1<<8)){ //read register B
    if(PINB & (pin & ~(1<<8))) return HIGH;
    return LOW;
  }
  else{  // read register D (no input on register A)
    if(PIND & pin) return HIGH;
    return LOW;
  }
}


/* Function: M_down
 * ---------------------
 * Measure button pressed
 */
void M_down() {
    // randomize internal state
    internalState=random(0,2);
    // turn LED on:
    if (logicalState==1){
      Write(LED_one,HIGH);
    } 
    else if (logicalState==2) {
      Write(LED_two,HIGH);
    }
    else {
      Write(LED_zero,HIGH);
    }  
}

/* Function: M_up
 * ---------------------
 * Measure button not pressed
 */
void M_up() {
  Write(LED_zero | LED_one | LED_two | LED_act , LOW); 
  // only works because all LEDs are on the same register.
}

/* Function: H_down
 * ---------------------
 * Hadamard button pressed
 */
void H_down() {
    int8_t dummy = internalState;
    internalState = logicalState;
    logicalState = dummy;
}

/* Function: H_up
 * ---------------------
 * Hadamard button not pressed
 */
void H_up() {
  // Do nothing
}

/* Function: Z_down
 * ---------------------
 * Z button pressed
 */
void Z_down() {
  internalState = (internalState + 1) % 2;
}

/* Function: Z_up
 * ---------------------
 * Z button not pressed
 */
void Z_up() {
  // Do nothing
}

/* Function: P_down
 * ---------------------
 * Phase button pressed
 */
void P_down() {
    internalState = (internalState + logicalState) % 2;
}

/* Function: P_up
 * ---------------------
 * Phase button not pressed
 */
void P_up() {
  // Do nothing
}

/* Function: CZ_down
 * ---------------------
 * 'left': 1 or 0 for left or right CZ button
 *  CZ button pressed
 */
void CZ_down(uint8_t left){
  //pin ID for left or right CZ
  uint16_t button_CZ =  (left ? button_CZl : button_CZr);
  uint16_t TX = (left ? TX_l : TX_r);
  uint16_t RX =  (left ? RX_l : RX_r);
  
  // set TX high
  Write(TX,HIGH);

  // RX state low
  uint8_t RX_state = LOW;
  // while CZ is pressed
  while (Read(button_CZ)==LOW){
    RX_state=Read(RX);
    if (RX_state==LOW) delay(10);
    else {
      Write(LED_act,HIGH);
      delay(100);
      Write(TX,logicalState==0 ? LOW : HIGH);
      delay(50);
      RX_state=Read(RX);
      if (RX_state==HIGH) internalState=(internalState+1) % 2;
      delay(50);
      break;
    }
  }
  
  Write(TX,LOW);
}

/* Function: CZ_up
 * ---------------------
 * 'left': 1 or 0 for left or right CZ button
 *  CZ not pressed
 */
void CZ_up(uint8_t left){
  Write(LED_act,LOW);
  if(left) Write(TX_l,LOW);
  else Write(TX_r,LOW);
}




void setup() {
  // initialize output on register A:
  DDRA |= (TX_l | TX_r) & ~(1<<9);
  // initialize input on register A:
  // --
  // initialize output on register B:
  DDRB |= (LED_zero | LED_one | LED_two | LED_act) & ~(1<<8);
  // initialize input on register B:
  DDRB &= ~(RX_r & ~(1<<8));
  // initialize output on register D:
  DDRD |= all_buttons;
  // initialize input on register D:
  DDRD &= ~RX_l;

  // internal pullup resistors for buttons on register D:
  PORTD |= all_buttons;

  // initialize logical and internal state  
  logicalState = 1;
  internalState = random(0,2);

  // TO DO: Set all *unused* pins as input + pull_up. This saves energy.

  
  /*// Pin change interrupt
  cli();                      // turn interrupts off while changing them.
  GIMSK |= (1<<4);            // enable Pin interrupt on register D (see datasheet)
  PCMSK0 |= all_buttons;      // use any button for interrupt.
  sei();                      // turn interrupts on.
  */
}



uint8_t old_button_state = all_buttons; // 0 on bit x iff button x pressed
uint8_t button_state = all_buttons;  

void loop() {
  
  button_state = PIND & all_buttons; //read state of buttons from register D.

  
  // Check if there is any change
  if (button_state == old_button_state) {
    delay(50);
    return;
  }

  // Set activity LED
  if (button_state==all_buttons) Write(LED_act,LOW); //no button pressed
  else Write(LED_act,HIGH);

  if ((button_state & button_M) != (old_button_state & button_M)) {
    if ((button_state & button_M) == LOW) M_down();
    else M_up();
  }
  if ((button_state & button_H) != (old_button_state & button_H)) {
    if ((button_state & button_H) == LOW) H_down();
    else H_up();
  }
  
  if ((button_state & button_Z) != (old_button_state & button_Z)) {
    if ((button_state & button_Z) == LOW) Z_down();
    else Z_up();
  }

  if ((button_state & button_P) != (old_button_state & button_P)) {
    if ((button_state & button_P) == LOW) P_down();
    else P_up();
  }

  if ((button_state & button_CZl) != (old_button_state & button_CZl)) {
    if ((button_state & button_CZl) == LOW) CZ_down(1);
    else CZ_up(1);
  }

  if ((button_state & button_CZr) != (old_button_state & button_CZr)) {
    if ((button_state & button_CZr) == LOW) CZ_down(0);
    else CZ_up(0);
  }


  // Delay a little bit to avoid bouncing
  delay(50);

  old_button_state = button_state;

  //sleep();
}
