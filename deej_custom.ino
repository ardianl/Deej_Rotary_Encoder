const int NUM_SLIDERS = 1;

// Rotary Encoder Pins
const int clkPin = 3;  // CLK pin
const int dtPin = 15;  // DT pin
const int swPin = A1;  // SW pin

// Encoder Sensitivity (pips for max volume)
const int sens1 = 30;   // One full turn
const int sens2 = 60;   // Two full turns
const int sens3 = 120;  // Four full turns

// Button States
const int numStates = 3;
const int LOW_STATE = 0;
const int MED_STATE = 1;
const int HIGH_STATE = 2;

// Variables
unsigned long previousTime = 0;

int range = 30;                           // Encoder range, 30 = 3 volume per pip, 100 = 1 volume per pip
volatile int counter = range / 2;         // Rotary encoder counter
int unMute = 0;                           // Unmute volume
volatile boolean clkState = 0;            // CLK pin state
volatile boolean encoderPressed = false;  // Button press flag
unsigned long debounceTime = 0;           // Debounce time
const unsigned long debounceDelay = 200;  // Debounce delay in milliseconds

bool Mute = false;
int masterState = 0;

int analogSliderValues[NUM_SLIDERS];

void setup() {

  // Rotary encoder pins as input with pull-up resistors
  pinMode(clkPin, INPUT_PULLUP);
  pinMode(dtPin, INPUT_PULLUP);
  pinMode(swPin, INPUT_PULLUP);

  // Attach interrupts to the CLK and SW pins
  attachInterrupt(digitalPinToInterrupt(clkPin), handleEncoder, CHANGE);
  Serial.begin(9600);
}

void loop() {

  checkButtons();
  // If encoder was pressed
  if (encoderPressed) {  //&& (millis() - debounceTime) >= debounceDelay) {
    Serial.println("Encoder button pressed!");
    if (Mute == true) {
      Mute = false;
      counter = unMute;
    } else {
      Mute = true;
      unMute = counter;
    }
    encoderPressed = false;
    // debounceTime = 0;
  }

  if ((millis() - previousTime) >= 500) {  // Only send value every 500ms
    updateSliderValues();
    sendSliderValues();   // Actually send data (all the time)
    printSliderValues();  // For debug
    previousTime = millis();
  }
  delay(10);
}

void updateSliderValues() {
  // Map counter from rotary encoder to same range as other sliders
  if (Mute == true) {
    analogSliderValues[0] = 0;
  } else {
    analogSliderValues[0] = map(counter, 0, range, 0, 1023);
  }
}

void sendSliderValues() {
  // Create String
  String builtString = String("");

  // For each slider add the value to the string
  for (int i = 0; i < NUM_SLIDERS; i++) {
    builtString += String((int)analogSliderValues[i]);


    if (i < NUM_SLIDERS - 1) {
      builtString += String("|");
    }
  }
  // Send the built string
  Serial.println(builtString);
}

void printSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    String printedString = String("Slider #") + String(i + 1) + String(": ") + String(analogSliderValues[i]) + String(" mV");
    Serial.write(printedString.c_str());

    if (i < NUM_SLIDERS - 1) {
      Serial.write(" | ");
    } else {
      Serial.write("\n");
    }
  }
}


// Interrupt handler for CLK pin
void handleEncoder() {
  // Read the current state of CLK and DT pins
  boolean clkStateNew = digitalRead(clkPin);
  boolean dtState = digitalRead(dtPin);

  // Check if CLK state has changed
  if (clkState != clkStateNew) {
    // If CLK and DT pins are different, increment the counter
    if (clkStateNew != dtState) {
      counter++;
    }
    // If CLK and DT pins are the same, decrement the counter
    else {
      counter--;
    }
    // Limit how many pips to count for max volume
    counter = constrain(counter, 0, range);
  }

  // Update the CLK pin state
  clkState = clkStateNew;
}

void checkButtons() {
  if (digitalRead(swPin) == 0 && masterState == 0) {
    Serial.println("Mute pressed");
    Mute = !Mute;
    masterState = 1;
    delay(25);
  }
  if (digitalRead(swPin) == 1) {
    masterState = 0;
  }
}
