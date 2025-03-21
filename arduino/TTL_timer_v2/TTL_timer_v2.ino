#include <LiquidCrystal.h>

// Define button codes based on LCD Keypad Shield analog reading
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

// Digital output pin used for triggering an external device.
const int triggerPin = 13;

// Digital input pin for the additional RUN/STOP button.
const int runStopButtonPin = 12;

// Timer variables (stored in tenths of a second)
int timerSet = 100;    // Default value: 10.0 sec (i.e. 100 tenths)
int timeElapsed = 0;   // Elapsed time in tenths of sec

// State variables
bool timerRunning = false;  // TRUE: RUN mode; FALSE: SET mode
bool runStopped = false;    // TRUE: timer зупинено за допомогою кнопки на PIN 12
bool isTenthsMode = false;  // FALSE: Seconds Mode (increment by 10 tenths), TRUE: 0.1 s Mode (increment by 1 tenth)

// Timing control (non-blocking)
unsigned long previousMillis = 0;
const unsigned long interval = 100;  // 0.1 sec interval (100 ms)

// Debounce variables
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200;  // 200 ms debounce period

// Функція для зчитування кнопок з LCD Keypad Shield через аналоговий вхід
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

// Функція аварійної зупинки таймера (SEL), скидання лічильника та оновлення дисплея
void emergencyStop() {
  timerRunning = false;
  runStopped = false;
  timeElapsed = 0;
  digitalWrite(triggerPin, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("EMERG STOP");
  lcd.setCursor(0, 1);
  lcd.print("Time: 0.0");
  delay(1000);  // невелика затримка для демонстрації повідомлення
}

void setup() {
  lcd.begin(16, 2);
  lcd.clear();

  pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW);

  pinMode(runStopButtonPin, INPUT_PULLUP);  // використання внутрішнього pull-up резистора
  
  // Відображення початкового повідомлення в SET Mode
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TTL Triger Timer");
  lcd.setCursor(3, 1);
  lcd.print("DoMB 2025");
  delay(2000);
}

void loop() {
  unsigned long currentMillis = millis();
  int lcdButton = read_LCD_buttons();
  bool runStopButtonState = (digitalRead(runStopButtonPin) == HIGH);

  // SET Mode: Налаштування таймера (timer не працює та не знаходиться у режимі зупинки)
  if (!timerRunning && !runStopped) {
    // Обробка кнопок з LCD, якщо пройшло достатньо часу для дебаунсу
    if (lcdButton != btnNONE && (currentMillis - lastButtonPress > debounceDelay)) {
      if (lcdButton == btnUP) {
        // Збільшення часу: на 1.0 сек у Seconds Mode або на 0.1 сек у 0.1 s Mode
        if (isTenthsMode) {
          timerSet++;
        } else {
          timerSet += 10;
        }
      } else if (lcdButton == btnDOWN) {
        // Зменшення часу, не менше нуля
        if (isTenthsMode && timerSet > 0) {
          timerSet--;
        } else if (!isTenthsMode && timerSet >= 10) {
          timerSet -= 10;
        }
      } else if (lcdButton == btnLEFT) {
        // Перемикання режиму коригування: Seconds Mode <-> 0.1 s Mode
        isTenthsMode = !isTenthsMode;
      } else if (lcdButton == btnRIGHT) {
        // Початок таймера (RUN Mode) при натисканні кнопки RIGHT
        timerRunning = true;
        runStopped = false;
        timeElapsed = 0;
        previousMillis = currentMillis;
        digitalWrite(triggerPin, HIGH);
      }
      lastButtonPress = currentMillis;
    }
    // Також, якщо натиснуто кнопку на PIN 12, стартуємо RUN Mode
    if (runStopButtonState && (currentMillis - lastButtonPress > debounceDelay)) {
      timerRunning = true;
      runStopped = false;
      timeElapsed = 0;
      previousMillis = currentMillis;
      digitalWrite(triggerPin, HIGH);
      lastButtonPress = currentMillis;
    }
    
    // Оновлення дисплея в SET Mode (час відображається з точністю до 0.1 сек)
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(isTenthsMode ? "SET Mode: 0.1s" : "SET Mode: 1.0s");
    lcd.setCursor(0, 1);
    lcd.print("Time: ");
    int sec = timerSet / 10;
    int tenth = timerSet % 10;
    lcd.print(sec);
    lcd.print(".");
    lcd.print(tenth);
    delay(100);
  }
  // RUN Mode: Лічильник працює від 0.0 до заданого часу
  else if (timerRunning) {
    // Аварійна зупинка кнопкою SEL: скидання таймера та завершення роботи
    if (lcdButton == btnSELECT && (currentMillis - lastButtonPress > debounceDelay)) {
      emergencyStop();
      lastButtonPress = currentMillis;
      return;  // вихід з функції loop для негайного оновлення
    }
    // Звичайна зупинка кнопкою на PIN 12: зупинення таймера, але збереження пройденого часу
    if (runStopButtonState && (currentMillis - lastButtonPress > debounceDelay)) {
      timerRunning = false;
      runStopped = true;
      digitalWrite(triggerPin, LOW);
      lastButtonPress = currentMillis;
    }
    
    // Неперервний відлік часу з використанням millis() кожні 0.1 секунди
    if (currentMillis - previousMillis >= interval) {
      previousMillis += interval;
      timeElapsed++;
      if (timeElapsed >= timerSet) {  // досягнення встановленого значення часу
        timerRunning = false;
        digitalWrite(triggerPin, LOW);
      }
    }
    
    // Оновлення дисплея в RUN Mode (відображення пройденого часу з точністю 0.1 сек)
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RUN Mode");
    lcd.setCursor(0, 1);
    int secElapsed = timeElapsed / 10;
    int tenthElapsed = timeElapsed % 10;
    lcd.print("Time: ");
    lcd.print(secElapsed);
    lcd.print(".");
    lcd.print(tenthElapsed);
  }
  // Режим зупинки (STOPPED Mode), коли таймер зупинено кнопкою на PIN 12 (RUN/STOP)
  else if (runStopped) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RUN STOP");
    lcd.setCursor(0, 1);
    int secElapsed = timeElapsed / 10;
    int tenthElapsed = timeElapsed % 10;
    lcd.print("Time: ");
    lcd.print(secElapsed);
    lcd.print(".");
    lcd.print(tenthElapsed);
    delay(100);
    // Натискання будь-якої кнопки повертає нас до SET Mode
    if (lcdButton != btnNONE && (currentMillis - lastButtonPress > debounceDelay)) {
      runStopped = false;
      timerRunning = false;
      lastButtonPress = currentMillis;
    }
  }
}
