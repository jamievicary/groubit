const int zeroLED = 12;
const int oneLED  = 11;
const int activityLED =13;
const int swapPin = 2;
const int readPin = 4;
const int phasePin= 3;
const int tickPin = 5;

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

void setup() {
  // initialize the LED pin as an output:
  pinMode(zeroLED, OUTPUT);
  pinMode(oneLED, OUTPUT);
  pinMode(activityLED, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(swapPin, INPUT);
  pinMode(readPin, INPUT);
  pinMode(phasePin, INPUT);
  pinMode(tickPin, INPUT);
  randomSeed(analogRead(0));
  internalState = 0;
  logicalState = random(0,2);
}

int old_read_state=LOW;
int old_swap_state=LOW;
int old_phase_state=LOW;
void loop() {

  int read_state = digitalRead(readPin);
  int swap_state = digitalRead(swapPin);
  int phase_state = digitalRead(phasePin);

  // Check if there is any change
  if (read_state == old_read_state && swap_state == old_swap_state && phase_state == old_phase_state) {
    delay(50);
    return;
  }

  // Set activity LED correctly
  if (read_state == LOW && swap_state == LOW && phase_state == LOW) digitalWrite(activityLED, LOW);
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

  // Delay a little bit to avoid bouncing
  delay(50);

  old_read_state = read_state;
  old_swap_state = swap_state;
  old_phase_state = phase_state;


}
