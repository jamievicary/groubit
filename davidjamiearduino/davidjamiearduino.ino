const int zeroLED = 12;
const int oneLED  = 11;
const int activityLED =13;
//const int POut = 0;  //Output Pin for tick
// const int PIn = 1;  //Input Pin for tick
const int swapPin = 2;
const int readPin = 4;
const int phasePin= 3;
const int tickPin = 5;  //Button Pin for tick
const bool debug= false;

// variables will change:
int internalState = 0;         // variable for reading the pushbutton status
int logicalState = 0; 


void read_down() {
    // turn LED on:
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
}

void phase_down() {
  internalState = (internalState + logicalState) % 2;
}
void phase_up() {
  // Do nothing
}
void swap_down() {
    int dummy = internalState;
    internalState = logicalState;
    logicalState = dummy;
}
void swap_up() {
  // Do nothing
}

void tick_up(){
  digitalWrite(POut,LOW);
}

void tick_down(){
  digitalWrite(POut,HIGH);
  int PIn_state = LOW;
  while (digitalRead(tickPin)==HIGH){
    PIn_state=digitalRead(PIn);
    if (PIn_state==LOW) delay(10);
    else {
      delay(100);
      digitalWrite(POut,logicalState==0 ? LOW : HIGH);
      delay(50);
      PIn_state==digitalRead(PIn);
      if (PIn_state==HIGH) internalState==(internalState+1)%2;
      delay(50);
      break;
    }
    digitalWrite(POut,LOW);
  }
}

void setup() {
  // initialize the LED pin as an output:
  pinMode(zeroLED, OUTPUT);
  pinMode(oneLED, OUTPUT);
  pinMode(activityLED, OUTPUT);
  // pinMode(POut,OUTPUT);
  // pinMODE(PIn, INPUT);
  // initialize the pushbutton pin as an input:
  pinMode(swapPin, INPUT);
  pinMode(readPin, INPUT);
  pinMode(phasePin, INPUT);
  pinMode(tickPin, INPUT);
  randomSeed(analogRead(0));
  internalState = 0;
  logicalState = random(0,2);
  if (debug==true) Serial.begin(9600);
}

int old_read_state=LOW;
int old_swap_state=LOW;
int old_phase_state=LOW;
int old_tick_state=LOW;
void loop() {

  int read_state = digitalRead(readPin);
  int swap_state = digitalRead(swapPin);
  int phase_state = digitalRead(phasePin);
  int tick_state = digitalRead(tickPin);

  // Check if there is any change
  if (read_state == old_read_state && swap_state == old_swap_state && phase_state == old_phase_state && tick_state ==old_tick_state) {
    delay(50);
    return;
  }

  // Set activity LED correctly
  if (read_state == LOW && swap_state == LOW && phase_state == LOW && tick_state==LOW) digitalWrite(activityLED, LOW);
  else digitalWrite(activityLED, HIGH);

  if (read_state != old_read_state) {
    if (read_state == HIGH) read_down();
    else read_up();
  }
  if (swap_state != old_swap_state) {
    if (swap_state == HIGH) swap_down();
    else swap_up();
  }
  if (phase_state != old_phase_state) {
    if (phase_state == HIGH) phase_down();
    else phase_up();
  }

  if (tick_state != old_tick_state) {
    //if (tick_state == HIGH) tick_down();
    //else tick_up();
  }

  if(debug==true) {
    Serial.print("("); 
    Serial.print(logicalState);
    Serial.print(","); 
    Serial.print(internalState);
    Serial.println(")");
  }
  // Delay a little bit to avoid bouncing
  delay(50);

  old_read_state = read_state;
  old_swap_state = swap_state;
  old_phase_state = phase_state;
  old_tick_state = tick_state;


}
