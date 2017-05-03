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



// Variables:
uint8_t internalState = 0;
uint8_t logicalState = 0;





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
  DDRB &= ~(RX_r & ~(1 << 8));
  //DDRB = 11111111 & ~(RX_r & ~(1<<8)); // All except input pin to output.
  // initialize input on register D:
  DDRD &= ~(all_buttons & RX_l);

  // internal pullup resistors for buttons and RX on register D:
  PORTD |= (all_buttons | RX_l);
  // internal pullup resistors for RX on register B:
  PORTB |= (RX_r & ~(1 << 8));

  // set TX HIGH
  PORTA |= ((TX_l | TX_r) & ~(1 << 9));

  // initialize logical and internal state
  logicalState = 1;
  internalState = random(0, 2);


  /*
  // Pin change interrupt
  cli();                      // turn interrupts off while changing them.
  GIMSK |= (1<<PCIE2);            // enable Pin interrupt on register D (see datasheet)
  PCMSK2 |= all_buttons;      // use any button for interrupt.
  sei();                      // turn interrupts on.
  */
}


void lightup(){
  Write(LED_zero, HIGH);
  delay(30);
  Write(LED_zero,LOW);
  delay(10);
  Write(LED_one, HIGH);
  delay(30);
  Write(LED_one,LOW);
  delay(10);  
  Write(LED_two, HIGH);
  delay(30);
  Write(LED_two,LOW);
  delay(10);
  Write(LED_act, HIGH);
  delay(30);
  Write(LED_act,LOW);
  delay(10);
}

uint8_t old_button_state = all_buttons; // 0 on bit x iff button x pressed
uint8_t button_state = all_buttons;
uint8_t counter=0;

void loop() {

  button_state = PIND & all_buttons; //read state of buttons from register D.


  // Check if there is any change
  if (button_state == old_button_state) {
    delay(50);
    return;
  }


  if ((button_state & button_CZl) != (old_button_state & button_CZl) ){ // counter 0 and change on CZl
    if ((button_state & button_CZl)!=button_CZl){
      if (counter != 0) counter=0;
      else{
        counter++; 
        Write(LED_zero,HIGH);
        delay(50);
        Write(LED_zero,LOW);
      }
    }
  }
  
  else if ((button_state & button_H) != (old_button_state & button_H) ){ // counter 0 and change on CZl
    if ((button_state & button_H)!=button_H){
      if (counter != 1) counter=0;
      else{
        counter++; 
        Write(LED_one,HIGH);
        delay(50);
        Write(LED_one,LOW);
      }
    }
  }
   
  if (counter==2 && ((button_state & button_Z) != (old_button_state & button_Z))){ // counter 2 and change on Z
    if ((button_state & button_Z)!=button_Z){//button pressed
      counter++; 
      delay(50);
      old_button_state=button_state; 
      return;
    }
    else{
      delay(50);
      old_button_state=button_state; 
      return;
    }
  } 
  if (counter==3 && ((button_state & button_P) != (old_button_state & button_P))&& ((button_state & button_P)!=button_P)){ // counter 3 and change on P
    counter++; 
    delay(50); 
    return;
  } 
  if (counter==4 && ((button_state & button_CZr) != (old_button_state & button_CZr))&& ((button_state & button_CZr)!=button_CZr)){ // counter 4 and change on CZr
    counter++; 
    delay(50); 
    return;
  } 

  if (counter==5 && ((button_state & button_M) != (old_button_state & button_M))&& ((button_state & button_M)!=button_M)){ // counter 5 and change on M
    lightup();
    counter=0;
    delay(50); 
    return;
  } 

  
  delay(50);

  old_button_state = button_state;

}
