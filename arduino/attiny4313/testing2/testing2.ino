//#include <avr/interrupt.h>
//#include <avr/sleep.h>
//#include <avr/power.h>

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





void setup() {
  // initialize output on register A:
  DDRA |= (TX_l | TX_r) & ~(1 << 9);
  //DDRA |=00000111;  // All possible registers on A to output, to save battery.
  // initialize input on register A:
  // --
  // initialize output on register B:
  DDRB |= (LED_zero | LED_one | LED_two | LED_act) & ~(1 << 8);
  // initialize input on register B:
  DDRB &= ~(RX_r & ~(1 << 8)) ;
  //DDRB = 11111111 & ~(RX_r & ~(1<<8)); // All except input pin to output.
  // initialize input on register D:
  DDRD &= ~(all_buttons | RX_l);

  // internal pullup resistors for buttons and RX on register D:
  PORTD |= (all_buttons | RX_l);
  // internal pullup resistors for RX on register B:
  PORTB |= (RX_r & ~(1 << 8));

  // set TX HIGH
  PORTA |= ((TX_l | TX_r) & ~(1 << 9));




  /*
  // Pin change interrupt
  cli();                      // turn interrupts off while changing them.
  GIMSK |= (1<<PCIE2);            // enable Pin interrupt on register D (see datasheet)
  PCMSK2 |= all_buttons;      // use any button for interrupt.
  sei();                      // turn interrupts on.
  */
}



uint8_t old_button_state = all_buttons; // 0 on bit x iff button x pressed
uint8_t button_state = all_buttons;
uint8_t RX_r_state = HIGH;
uint8_t RX_l_state = HIGH;
uint8_t old_RX_r_state = HIGH;
uint8_t old_RX_l_state = HIGH;

void loop() {

  button_state = PIND & all_buttons; //read state of buttons from register D.
  RX_r_state= Read(RX_r);  // Read state from RX_r
  RX_l_state=Read(RX_l);   // Read state from RX_l
  
  if (old_RX_l_state!= RX_l_state ){ // change on RX_l
    if(RX_l_state==LOW) Write(LED_zero, HIGH);
    else Write(LED_zero,LOW);
  }
  if (old_RX_r_state!= RX_r_state ){ // change on RX_r
    if(RX_r_state==LOW) Write(LED_act, HIGH);
    else Write(LED_act,LOW);
  }
  

  if((button_state & button_CZl) != (old_button_state & button_CZl)){ //button CZl changed
    if((button_state & button_CZl) == button_CZl){//button not pressed, i.e. HIGH
      Write(TX_l, HIGH); 
      Write(LED_one,LOW);
    }
    else { // button pressed
      Write(TX_l, LOW);
      Write(LED_one,HIGH);
    }
  } 
  
  if((button_state & button_CZr) != (old_button_state & button_CZr)){ //button CZl changed
    if((button_state & button_CZr) == button_CZr){//button not pressed, i.e. HIGH
      Write(TX_r, HIGH); 
      Write(LED_two,LOW);
    }
    else { // button pressed
      Write(TX_r, LOW);
      Write(LED_two,HIGH);
    }
  } 
  delay(50);

  old_button_state = button_state;
  old_RX_r_state = RX_r_state;
  old_RX_l_state = RX_l_state;

}
