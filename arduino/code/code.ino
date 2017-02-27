const int zeroLED = 2;
const int oneLED  = 14;
const int activityLED = 13;
//const int TickOut = 9;  //Output Pin for tick
//const int TickIn = 8;  //Input Pin for tick
const int buttonSwap = 0;
const int buttonRead = 1;
//const int buttonTick = 5;  //Button Pin for tick
//const int buttonError= 10;
//const bool debug= false;

// variables will change:
int internalState = 0;         // variable for reading the pushbutton status
int logicalState = 0; 

int counter=0;
void read_down() {
    // turn LED on:
    //internalState=1;
    internalState=random(0,2);
    if (logicalState==1){
      digitalWrite(oneLED, HIGH);
    } else {
      digitalWrite(zeroLED, HIGH);
    }  
}

void read_up() {
  digitalWrite(oneLED, LOW);
  digitalWrite(zeroLED, LOW);
  digitalWrite(activityLED, LOW);
}

/*
void error_down() {
  internalState = (internalState + 1) % 2;
}
void error_up() {
  // Do nothing
}
*/

void swap_down() {
    int dummy = internalState;
    internalState = logicalState;
    logicalState = dummy;
}
void swap_up() {
  // Do nothing
}

/*
void tick_up(){
  digitalWrite(TickOut,LOW);
}

void tick_down(){
  if(debug==true)Serial.println("tick_down");
  digitalWrite(TickOut,HIGH);
  int TickIn_state = LOW;
  while (digitalRead(buttonTick)==LOW){
    TickIn_state=digitalRead(TickIn);
    if (TickIn_state==LOW) delay(10);
    else {
      delay(100);
      digitalWrite(TickOut,logicalState==0 ? LOW : HIGH);
      if(debug==true){
        Serial.print("Sending ");
        Serial.println(logicalState==0 ? "LOW" : "HIGH");}
      delay(50);
      TickIn_state=digitalRead(TickIn);
      if(debug==true){
        Serial.print("Other Arduino:");
        Serial.println(TickIn_state == HIGH ? 1 : 0);}
      if (TickIn_state==HIGH) internalState=(internalState+1)%2;
      delay(50);
      break;
    }
  }
  if(debug==true)Serial.println("tick_down exit");
  digitalWrite(TickOut,LOW);
}
*/

void setup() {
  // initialize the LED pin as an output:
  pinMode(zeroLED, OUTPUT);
  pinMode(oneLED, OUTPUT);
  pinMode(activityLED, OUTPUT);
  //pinMode(TickOut,OUTPUT);
  //pinMode(TickIn, INPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonSwap, INPUT_PULLUP);
  pinMode(buttonRead, INPUT_PULLUP);
  //pinMode(buttonError, INPUT_PULLUP);
  //pinMode(buttonTick, INPUT_PULLUP);
  //randomSeed(analogRead(0));
  //internalState = 0;
  logicalState = 1;
  internalState = random(0,2);
  //if (debug==true) Serial.begin(9600);
}

int old_read_state=LOW;
int old_swap_state=LOW;
//int old_error_state=LOW;
int old_tick_state=LOW;
void loop() {

  int read_state = digitalRead(buttonRead);
  int swap_state = digitalRead(buttonSwap);
  //int error_state = digitalRead(buttonError);
  //int tick_state = digitalRead(buttonTick);

  // Check if there is any change
  if (read_state == old_read_state && swap_state == old_swap_state /* && error_state == old_error_state */ /*&& tick_state ==old_tick_state*/) {
    delay(50);
    return;
  }

  // Set activity LED correctly
  if (read_state == HIGH && swap_state == HIGH /*  && error_state == HIGH */  /*&& tick_state==HIGH*/) digitalWrite(activityLED, LOW);
  else digitalWrite(activityLED, HIGH);

  if (read_state != old_read_state) {
    if (read_state == LOW) read_down();
    else read_up();
  }
  if (swap_state != old_swap_state) {
    if (swap_state == LOW) swap_down();
    else swap_up();
  }
  /*
  if (error_state != old_error_state) {
    if (error_state == LOW) error_down();
    else error_up();
  }
  */
/*
  if (tick_state != old_tick_state) {
    if (tick_state == LOW) tick_down();
    else tick_up();
  }
  */
  /*
  if(debug==true) {
    Serial.print(counter);
    Serial.print(". ");
    Serial.print("("); 
    Serial.print(logicalState);
    Serial.print(","); 
    Serial.print(internalState);
    Serial.println(")");
    counter++;
  }
  */
  // Delay a little bit to avoid bouncing
  delay(50);

  old_read_state = read_state;
  old_swap_state = swap_state;
  //old_error_state = error_state;
  //old_tick_state = tick_state;


}
