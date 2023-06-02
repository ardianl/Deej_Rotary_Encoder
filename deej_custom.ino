#include <Keyboard.h>

const int NUM_SLIDERS = 5;

// Slider Pins
const int slider1Pin = A0;
const int slider2Pin = A1;
const int slider3Pin = A2;
const int slider4Pin = A3;

// Rotary Encoder Pins
const int clkPin = 3;  // CLK pin
const int dtPin = 15;  // DT pin
const int swPin = 2;   // SW pin

// Encoder Sensitivity (pips for max volume)
const int sens1 = 30;   // One full turn
const int sens2 = 60;   // Two full turns
const int sens3 = 120;  // Four full turns

// RGB LED Pins
const int redPin = A7;    // Red pin
const int greenPin = A8;  // Green pin
const int bluePin = A9;   // Blue pin

// Button Pin
const int buttonPin = 7;  // Button pin

// Button States
const int numStates = 3;
const int LOW_STATE = 0;
const int MED_STATE = 1;
const int HIGH_STATE = 2;


// RGB LED Colors
const int colors[numStates][3] = {
  { 255, 0, 0 },  // Low state (Red color)
  { 0, 255, 0 },  // Med state (Green color)
  { 0, 0, 255 }   // High state (Blue color)
};

// Blink Rates (in milliseconds)
const int blinkRates[numStates] = {
  1000,  // Low state (1 second)
  500,   // Med state (0.5 seconds)
  250    // High state (0.25 seconds)
};

// Variables
unsigned long previousTime = 0;
volatile int counter = 0;                 // Rotary encoder counter
volatile boolean clkState = 0;            // CLK pin state
volatile boolean encoderPressed = false;  // Button press flag
unsigned long debounceTime = 0;           // Debounce time
const unsigned long debounceDelay = 200;  // Debounce delay in milliseconds
int currentState = LOW_STATE;
bool shutdownActive = false;
bool shutdownPressed = false;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
int range = sens1;  // Encoder range for max volume (how many pips to turn, for me 30 pips = 1 full rotation)

int analogSliderValues[NUM_SLIDERS];

void setup() {
  // Setup sliders and inputs
  pinMode(slider1Pin, INPUT);
  pinMode(slider2Pin, INPUT);
  pinMode(slider3Pin, INPUT);
  pinMode(slider4Pin, INPUT);

  // Rotary encoder pins as input with pull-up resistors
  pinMode(clkPin, INPUT_PULLUP);
  pinMode(dtPin, INPUT_PULLUP);
  pinMode(swPin, INPUT_PULLUP);

  // Power button pin
  pinMode(buttonPin, INPUT_PULLUP);

  // RGB LED pins
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Start with all LED off
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);

  // Attach interrupts to the CLK and SW pins
  attachInterrupt(digitalPinToInterrupt(clkPin), handleEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(swPin), handleSwitch, FALLING);
  Keyboard.begin();
  Serial.begin(9600);
}

void loop() {
  // If encoder was pressed
  if (encoderPressed) {//&& (millis() - debounceTime) >= debounceDelay) {
    Serial.println("Encoder button pressed!");
    changeState();
    encoderPressed = false;
    // debounceTime = 0;
  }

  // Read shutdown button
  readButton();
  // If shutdown button was pressed
  if (shutdownPressed) {
    if (!shutdownActive) {
      Serial.println("Shutdown!");
      executeShutdown();  // Shutdown PC
      shutdownActive = true;
    } else if (shutdownActive) {
      Serial.println("Aborting!");
      abortShutdown();
      shutdownActive = false;
    }
    shutdownPressed = false;
  }

  updateSliderValues();
  sendSliderValues();   // Actually send data (all the time)
  printSliderValues();  // For debug
  delay(500);
}

void updateSliderValues() {
  // Map counter from rotary encoder to same range as other sliders
  analogSliderValues[0] = map(counter, 0, range, 0, 1024);
  analogSliderValues[1] = analogRead(slider1Pin);
  analogSliderValues[2] = analogRead(slider2Pin);
  analogSliderValues[3] = analogRead(slider3Pin);
  analogSliderValues[4] = analogRead(slider4Pin);
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

// Interrupt handler for SW pin
void handleSwitch() {
  // Check if the debounce time has not elapsed
  if ((millis() - debounceTime) < debounceDelay) {
    return;  // Ignore the button press
  }

  // Set the button pressed flag
  encoderPressed = true;

  // Update the debounce time
  debounceTime = millis();
}

void executeShutdown() {
  Keyboard.press(KEY_LEFT_GUI);  // Press the Windows key
  Keyboard.press('r');           // Press 'r' to open the Run dialog
  Keyboard.releaseAll();         // Release all keys
  delay(500);                    // Delay to allow time for the Run dialog to open
  // Enter 'cmd' and press Enter to launch the Command Prompt
  Keyboard.print("cmd");
  Keyboard.press(KEY_RETURN);
  Keyboard.releaseAll();
  delay(500);
  Keyboard.print("pee pee poo poo");
}

void abortShutdown() {
  Keyboard.press(KEY_LEFT_GUI);  // Press the Windows key
  Keyboard.press('r');           // Press 'r' to open the Run dialog
  Keyboard.releaseAll();         // Release all keys
  delay(500);                    // Delay to allow time for the Run dialog to open
  // Enter 'cmd' and press Enter to launch the Command Prompt
  Keyboard.print("cmd");
  Keyboard.press(KEY_RETURN);
  Keyboard.releaseAll();
  delay(500);
  Keyboard.print("poo poo hahaha");
}

void readButton() {
  int buttonState = digitalRead(buttonPin);

  if (buttonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  if (millis() - lastDebounceTime > debounceDelay) {
    if (buttonState != HIGH) {
      shutdownPressed = true;
    }
  }

  lastButtonState = buttonState;
}

void changeState() {
  currentState++;
  if (currentState >= numStates) {
    currentState = LOW_STATE;
  }
  switch (currentState) {
    case LOW_STATE:
      Serial.println("LOW STATE");
      counter = counter * sens1 / sens3;
      range = 30;
      blinkRGB(3, 500, 255, 0, 0);
      break;
    case MED_STATE:
      Serial.println("MED STATE");
      counter = counter * sens2 / sens1;
      range = 60;
      blinkRGB(6, 250, 0, 255, 0);
      break;
    case HIGH_STATE:
      Serial.println("HIGH STATE");
      counter = counter * sens3 / sens2;
      range = 120;
      blinkRGB(12, 125, 0, 0, 255);
      break;
  }
}

void blinkRGB(int count, int timing, int red, int green, int blue) {
  for (int i = 0; i < count; i++) {
    analogWrite(redPin, red);
    analogWrite(greenPin, green);
    analogWrite(bluePin, blue);
    delay(timing);
    analogWrite(redPin, LOW);
    analogWrite(greenPin, LOW);
    analogWrite(bluePin, LOW);
    delay(timing);
  }
}
