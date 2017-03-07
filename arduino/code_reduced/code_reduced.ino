//

//Out on register A:
const uint16_t zeroLED = (1<<2);     // Pin A2
const uint16_t oneLED  = (1<<3);     // Pin A3
const uint16_t activityLED = (1<<7); // Pin A7
// const uint8_t RTickOut[2] = {0,1};

//In on register A:
const uint16_t buttonSwap = (1<<0);  // Pin A0
const uint16_t buttonRead = (1<<1);  // Pin A1
const uint16_t buttonError= (1<<4);  // Pin A4
// const uint8_t RTickIn[2] ={0,1} ;

// Out on register B:
const uint16_t LTickOut = (1<<3 | 1<<8);


// In on register B:
const uint16_t buttonLTick = (1<<5 | 1<<8);  //Button Pin for tick
// const uint8_t buttonRTick[2] = {1,1<<6};  //Button Pin for tick
const uint16_t LTickIn = (1<<4 | 1<<8);  //Button Pin for tick


// Variables:
int internalState = 0;         
int logicalState = 0; 


// Custom write function
void Write(uint16_t Pin, int is_high) {
  if (Pin & (1<<8)){
    if(is_high)PORTB |= (Pin & ~(1<<8));
    else PORTB &= ~(Pin & ~(1<<8));
  }
  else{
    if(is_high)PORTA |= Pin;
    else PORTA &= ~Pin;
  }
    
}

// Custom read function
int Read(uint16_t Pin){
  if(Pin & (1<<8)){
    if(PINB & (Pin & ~(1<<8))) return HIGH;
    return LOW;
  }
  else{
    if(PINA & Pin) return HIGH;
    return LOW;
  }

}

// Read button pressed
void read_down() {
    // randomize internal state
    internalState=random(0,2);
    // turn LED on:
    if (logicalState==1){
      Write(oneLED,HIGH);
    } else {
      Write(zeroLED,HIGH);
    }  
}

// Read button not pressed
void read_up() {
  // All LEDs on same register
  Write(oneLED, LOW);
  Write(zeroLED, LOW);
  Write(activityLED, LOW);
}

// Error button pressed
void error_down() {
  internalState = (internalState + 1) % 2;
}

// Error button not pressed
void error_up() {
  // Do nothing
}

// Swap button pressed
void swap_down() {
    int dummy = internalState;
    internalState = logicalState;
    logicalState = dummy;
}

// Swap button not pressed
void swap_up() {
  // Do nothing
}

// LTick button not pressed
void tick_up(bool left){
  if(left) Write(LTickOut,LOW);
  //else Write(RTickOut,LOW);
}

void tick_down(bool left){
  uint16_t button = buttonLTick;
  uint16_t TOut = LTickOut;
  uint16_t TIn =  LTickIn;
  
  /*
  if(~left){
    uint8_t button[2] = {buttonRTick[0],buttonRTick[1]};
    uint8_t TOut[2] = {RTickOut[0] , RTickOut[1]};
    uint8_t TIn[2] =  {RTickIn[0], RTickIn[1]};
  }
  */

  Write(TOut,HIGH);
  
  int TickIn_state = LOW;
  while (Read(button)==LOW){
    TickIn_state=Read(TIn);
    if (TickIn_state==LOW) delay(10);
    else {
      delay(100);
      Write(TOut,logicalState==0 ? LOW : HIGH);
      delay(50);
      TickIn_state=Read(TIn);
      if (TickIn_state==HIGH) internalState=(internalState+1)%2;
      delay(50);
      break;
    }
  }
  
  Write(TOut,LOW);
}


void setup() {
  // initialize output of register A:
  DDRA |= (oneLED | zeroLED | activityLED /*| RTickOut*/);

  // initialize output of register B:
  DDRB |= (LTickOut & ~(1<<8));

  // initialize input of register A:
  DDRA &= ~(buttonSwap | buttonRead | buttonError /*| RTickIn*/);
  // Set internal pullup resistors for buttons on register A:
  PORTA |= (buttonSwap | buttonRead | buttonError);

  // initialize input of register B:
  DDRB &= ~ ((buttonLTick /*| buttonRTick[1] */ | LTickIn) & ~ (1<<8));
  // internal pullup resistors for buttons on register B
  PORTB |= (/*buttonRTick[1] |*/ buttonLTick) & ~ (1<<8);
  
  logicalState = 1;
  internalState = random(0,2);
}

int old_read_state=LOW;
int old_swap_state=LOW;
int old_error_state=LOW;
int old_ltick_state=LOW;
//int old_rtick_state=LOW;

void loop() {

  int read_state = Read(buttonRead);
  int swap_state = Read(buttonSwap);
  int error_state = Read(buttonError);
  int ltick_state = Read(buttonLTick);
//  int rtick_state = Read(buttonRTick);

  // Check if there is any change
  if (read_state == old_read_state && swap_state == old_swap_state && error_state == old_error_state  && ltick_state ==old_ltick_state /*&& rtick_state ==old_rtick_state */) {
    delay(50);
    return;
  }

  // Set activity LED correctly
  if (read_state == HIGH && swap_state == HIGH && error_state == HIGH  && ltick_state==HIGH /* && rtick_state==HIGH */) Write(activityLED,LOW);
  else Write(activityLED,HIGH);

  if (read_state != old_read_state) {
    if (read_state == LOW) read_down();
    else read_up();
  }
  if (swap_state != old_swap_state) {
    if (swap_state == LOW) swap_down();
    else swap_up();
  }
  
  if (error_state != old_error_state) {
    if (error_state == LOW) error_down();
    else error_up();
  }

  if (ltick_state != old_ltick_state) {
    if (ltick_state == LOW) tick_down(1);
    else tick_up(1);
  }

  /*
  if (rtick_state != old_rtick_state) {
    if (rtick_state == LOW) tick_down(0);
    else tick_up(0);
  }
  */

  // Delay a little bit to avoid bouncing
  delay(50);

  old_read_state = read_state;
  old_swap_state = swap_state;
  old_error_state = error_state;
  old_ltick_state = ltick_state;
  //old_rtick_state = rtick_state;


}
