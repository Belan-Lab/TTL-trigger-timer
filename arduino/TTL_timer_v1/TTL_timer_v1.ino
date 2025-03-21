/*
 * Arduino Timer Code with 0.1-Second Precision Display in SET Mode
 * Using Arduino Uno and Arduino LCD Keypad Shield.
 *
 * The timer value is stored in tenths of seconds. In SET mode, the user can choose between:
 * - Seconds Mode: UP/DOWN adjust the time by 1.0 sec (10 tenths)
 * - 0.1 s Mode: UP/DOWN adjust the time by 0.1 sec (1 tenth)
 * The mode can be toggled using the LEFT button.
 *
 * Pressing RIGHT starts the countdown (RUN mode). During RUN mode, the external trigger pin
 * is set HIGH immediately if the timer value is non-zero and remains HIGH until the countdown
 * reaches 0. The countdown updates every 0.1 second and is displayed with one decimal place.
 * Pressing SELECT during the countdown cancels the timer and immediately sets the trigger pin LOW.
 */

#include <LiquidCrystal.h>

// Define button codes based on analog input values
#define btnRIGHT   1
#define btnUP      2
#define btnDOWN    3
#define btnLEFT    4
#define btnSELECT  5
#define btnNONE    0

// Initialize the LCD (RS, E, D4, D5, D6, D7)
// Default for Arduino LCD Keypad Shield: RS = 8, E = 9, D4 = 4, D5 = 5, D6 = 6, D7 = 7.
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Analog pin for reading keypad buttons.
const int analogPin = A0;

// Digital output pin used to trigger an external device.
const int triggerPin = 13;

// Time variables are stored in tenths of a second.
// Default value: 100 tenths = 10.0 seconds.
int timerSet = 50;      // Timer value set by user (in tenths of sec).
int timeRemaining = 0;   // Countdown value (in tenths of sec).

// Timer and mode state variables.
bool timerRunning = false;     // false = SET mode, true = RUN mode.
bool isTenthsMode = false;     // false: seconds mode (increments in 10s), true: tenths mode (increments in 1).

// For non-blocking timing.
unsigned long previousMillis = 0;
const unsigned long interval = 100;   // 100 ms = 0.1 s step.

// For simple button debouncing.
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200; // 200 ms debounce period

// Function to read the pressed button based on the analog value.
// Adjust thresholds if necessary.
int read_LCD_buttons() {
  int adc_key_in = analogRead(analogPin);
  if (adc_key_in > 1000) return btnNONE;
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 250)  return btnUP;
  if (adc_key_in < 450)  return btnDOWN;
  if (adc_key_in < 650)  return btnLEFT;
  if (adc_key_in < 850)  return btnSELECT;
  return btnNONE;
}

void setup() {
  pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW); // Ensure trigger is LOW initially.

  lcd.begin(16, 2); // 16 columns, 2 rows.
  lcd.clear();

  // Initial welcome message in SET mode.
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TTL Triger Timer");
  lcd.setCursor(3, 1);
  lcd.print("DoMB 2025");
  delay(2000);
}

void loop() {
  unsigned long currentMillis = millis();
  int button = read_LCD_buttons();
  
  // SET Mode: timer configuration
  if (!timerRunning) {
    // Process button actions if debounce period has passed.
    if (button != btnNONE && (currentMillis - lastButtonPress > debounceDelay)) {
      if (button == btnUP) {
        // Increase time:
        // In tenths mode, adjust by 1 tenth; in seconds mode, by 10 tenths (1.0 sec).
        if (isTenthsMode) {
          timerSet++;
        } else {
          timerSet += 10;
        }
      } else if (button == btnDOWN) {
        // Decrease time ensuring timer does not go below 0.
        if (isTenthsMode && timerSet > 0) {
          timerSet--;
        } else if (!isTenthsMode && timerSet >= 10) {
          timerSet -= 10;
        }
      } else if (button == btnLEFT) {
        // Toggle mode between seconds and tenths.
        isTenthsMode = !isTenthsMode;
      } else if (button == btnRIGHT) {
        // Start the countdown
        timerRunning = true;
        timeRemaining = timerSet; // Set countdown from configured timer value.
        previousMillis = currentMillis;
        // Activate external trigger if time is greater than 0.
        if (timeRemaining > 0)
          digitalWrite(triggerPin, HIGH);
        else
          digitalWrite(triggerPin, LOW);
      }
      lastButtonPress = currentMillis;  // Reset debounce timer.
    }
    
    // Update LCD display in SET mode with fixed 0.1 s resolution:
    lcd.clear();
    lcd.setCursor(0, 0);
    if (isTenthsMode)
      lcd.print("SET Mode: 0.1s");
    else
      lcd.print("SET Mode: 1.0s");
      
    lcd.setCursor(0, 1);
    lcd.print("Time: ");
    int sec = timerSet / 10;
    int tenth = timerSet % 10;
    lcd.print(sec);
    lcd.print(".");
    lcd.print(tenth);
    delay(100); // Short delay for display update in SET mode.
  } 
  // RUN Mode: countdown timer
  else {
    // Cancel countdown if SELECT button pressed.
    if (button == btnSELECT && (currentMillis - lastButtonPress > debounceDelay)) {
      timerRunning = false;
      digitalWrite(triggerPin, LOW);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("STOP!");
      delay(1000);
      lastButtonPress = currentMillis;
    }
    
    // Non-blocking countdown every 0.1 second using millis()
    if (currentMillis - previousMillis >= interval) {
      if (timeRemaining > 0) {
        timeRemaining--;  // Decrease timer by 0.1 seconds
      }
      previousMillis += interval;
      
      // If timer reaches 0, stop the countdown and reset trigger.
      if (timeRemaining <= 0) {
        timerRunning = false;
        timeRemaining = 0;
        digitalWrite(triggerPin, LOW);
      }
    }
    
    // Update LCD display in RUN mode with 0.1 second precision.
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RUN Mode");
    lcd.setCursor(0, 1);
    lcd.print("Rem: ");
    int sec = timeRemaining / 10;
    int tenth = timeRemaining % 10;
    lcd.print(sec);
    lcd.print(".");
    lcd.print(tenth);
  }
}
