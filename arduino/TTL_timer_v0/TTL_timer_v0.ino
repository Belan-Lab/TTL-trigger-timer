#include <LiquidCrystal.h>

// Definitions for button values based on analogRead values.
// You may need to adjust these thresholds based on your LCD Keypad Shield.
#define btnRIGHT   1
#define btnUP      2
#define btnDOWN    3
#define btnLEFT    4
#define btnSELECT  5
#define btnNONE    0

// Initialize the LCD using pins (RS, E, D4, D5, D6, D7)
// For Arduino LCD Keypad Shield, typically these are: RS = 8, E = 9, D4 = 4, D5 = 5, D6 = 6, D7 = 7.
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Analog input pin for reading keypad buttons
const int analogPin = A0;

// Digital output pin for triggering external device.
// While the timer is counting down, this pin will be set to HIGH.
const int triggerPin = 13;

// Timer variables:
// timerSet stores the user-set time (in seconds)
// timeRemaining is used for the countdown
unsigned long timerSet = 5;    // initial timer value in seconds (можна змінити за потребою)
unsigned long timeRemaining = 0; 
unsigned long previousMillis = 0;  // для організації не блокуючого відліку часу

// Timer state: false = SET Mode, true = RUN Mode
bool timerRunning = false;

// Function to read the pressed button from the LCD Keypad Shield using the analog value.
int read_LCD_buttons() {
  int adc_key_in = analogRead(analogPin);
  
  // The thresholds below are typical; adjust if your shield returns different values.
  if (adc_key_in > 1000) return btnNONE;
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 250)  return btnUP;
  if (adc_key_in < 450)  return btnDOWN;
  if (adc_key_in < 650)  return btnLEFT;
  if (adc_key_in < 850)  return btnSELECT;
  return btnNONE;
}

void setup() {
  // Initialize the LCD's number of columns and rows
  lcd.begin(16, 2);
  lcd.clear();
  
  // Initialize digital output for external trigger
  pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW);  // Ensure trigger is LOW initially
  
  // Display initial message in SET mode
  lcd.setCursor(0, 0);
  lcd.print("Timer Setup");
  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(timerSet);
  delay(1000);
}

void loop() {
  // Зчитування натиснутої кнопки
  int button = read_LCD_buttons();
  
  if (!timerRunning) {
    // Режим SET: встановлення часу таймера через кнопки UP та DOWN
    
    // Якщо натиснуто кнопку UP, збільшуємо значення таймера на 1 секунду
    if (button == btnUP) {
      timerSet++;
      delay(100); // затримка для усунення "дребезгу" контактів
    }
    // Якщо натиснуто кнопку DOWN, зменшуємо значення таймера, не допускаючи значення нижче 0
    else if (button == btnDOWN) {
      if (timerSet > 0) {
        timerSet--;
      }
      delay(100);
    }
    // При натисканні кнопки RIGHT запускаємо таймер
    else if (button == btnRIGHT) {
      timerRunning = true;          // перемикаємо в режим RUN
      timeRemaining = timerSet;     // ініціалізуємо час відліку
      previousMillis = millis();    // скидаємо таймер для відліку
      digitalWrite(triggerPin, HIGH);  // встановлюємо triggerPin у HIGH (активація зовнішнього пристрою)
      delay(100);
    }
    
    // Оновлення дисплея у режимі SET
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SET Mode");
    lcd.setCursor(0, 1);
    lcd.print("Time: ");
    lcd.print(timerSet);
    lcd.print(" sec");
    delay(100);
  } 
  else {
    // Режим RUN: відліку таймера
    
    // Непряме (не блокуюче) вимірювання часу для зменшення лічильника кожну секунду
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 1000) {
      if (timeRemaining > 0) {
        timeRemaining--;
      }
      previousMillis = currentMillis;
    }
    
    // Якщо натиснуто кнопку SEL, перериваємо таймер
    if (button == btnSELECT) {
      timerRunning = false;         // повертаємося до режиму SET
      digitalWrite(triggerPin, LOW);  // вимикаємо triggerPin (повертаємо LOW)
      delay(100);
      return;  // вихід з loop(), щоб оновлення дисплея здійснилося в наступному циклі
    }
    
    // Оновлення дисплея для режиму RUN: показуємо зворотний відлік
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RUN Mode");
    lcd.setCursor(0, 1);
    lcd.print("Rem: ");
    lcd.print(timeRemaining);
    lcd.print(" sec");
    
    // По досягненню нуля відлік завершується
    if (timeRemaining == 0) {
      timerRunning = false;
      digitalWrite(triggerPin, LOW);  // вимикаємо тригер зовнішнього пристрою
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Time Up!");
      delay(500);  // затримка для демонстрації повідомлення
    }
  }
}
