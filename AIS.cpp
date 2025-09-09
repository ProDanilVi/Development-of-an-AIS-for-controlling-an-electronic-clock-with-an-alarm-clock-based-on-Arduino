#include <ThreeWire.h>       // Библиотека для работы с тремя проводами
#include <RtcDS1302.h>       // Библиотека для RTC DS1302

// Пины подключения модуля RTC
#define RST_PIN 36
#define DAT_PIN 35
#define CLK_PIN 34

// Инициализация интерфейса ThreeWire и модуля RTC
ThreeWire rtcWire(DAT_PIN, CLK_PIN, RST_PIN);
RtcDS1302<ThreeWire> rtc(rtcWire);

// Пины для сегментов A, B, C, D, E, F, G
const int segmentPins[] = {22, 23, 24, 25, 26, 27, 28};

// Пин для десятичной точки
const int dpPin = 29;

// Пины для разрядов D1, D2, D3, D4
const int digitPins[] = {30, 31, 32, 33}; // Разряды: D1, D2, D3, D4

// Пин для пьезоизлучателя
const int buzzerPin = 8;

unsigned long previousMillis = 0;  // Переменная для хранения времени последнего мигания

// Таблица сегментов для цифр 0-9 (7-сегментный дисплей)
const int digits[10][7] = {
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}  // 9
};

// Ноты для мелодии "Звездные войны"
const int c = 261;    // Нота До
const int d = 294;    // Нота Ре
const int e = 329;    // Нота Ми
const int f = 349;    // Нота Фа
const int g = 391;    // Нота Соль
const int gS = 415;   // Нота Соль-диез
const int a = 440;    // Нота Ля
const int aS = 455;   // Нота Ля-диез
const int b = 466;    // Нота Си
const int cH = 523;   // Нота До высокой октавы
const int cSH = 554;  // Нота До-диез высокой октавы
const int dH = 587;   // Нота Ре высокой октавы
const int dSH = 622;  // Нота Ре-диез высокой октавы
const int eH = 659;   // Нота Ми высокой октавы
const int fH = 698;   // Нота Фа высокой октавы
const int fSH = 740;  // Нота Фа-диез высокой октавы
const int gH = 784;   // Нота Соль высокой октавы
const int gSH = 830;  // Нота Соль-диез высокой октавы
const int aH = 880;   // Нота Ля высокой октавы

// Переменные состояний
enum State {NORMAL, SET_TIME, SET_ALARM};
State currentState = NORMAL;

int setHour = 0, setMinute = 0;      // Устанавливаемые часы и минуты
int alarmHour = 0, alarmMinute = 0; // Устанавливаемое время будильника
int currentDigit = 0;                // 0 - часы, 1 - минуты
bool resetPressed = false;           // Флаг нажатия кнопки сброса
bool exitPressed = false;           // Флаг нажатия кнопки выхода (сохранение + выход)
bool alarmActive = false;           // Флаг нажатия кнопки сброса

const int buttonSetTimePin = 40;  // Кнопка установки времени часов
const int buttonSetAlarmPin = 41; // Кнопка установки времени будильника
const int buttonResetPin = 42;    // Кнопка сброса установки
const int buttonExitPin = 43;    // Кнопка выхода из режима
const int buttonSelectDigitPin = 44; // Кнопка для выбора разряда

bool buttonSelectDigitState = HIGH;   // Текущее состояние кнопки
bool lastButtonSelectDigitState = HIGH; // Предыдущее состояние кнопки

void setup() {
  // Инициализация пинов сегментов
  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
    digitalWrite(segmentPins[i], HIGH); // Выключаем сегменты
  }

  // Инициализация пина для точки
  pinMode(dpPin, OUTPUT);
  digitalWrite(dpPin, HIGH); // Точка выключена

  // Инициализация пинов разрядов
  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], LOW); // Все разряды выключены
  }

  // Инициализация RTC
  rtc.Begin();

  // Проверка корректности времени
  if (!rtc.IsDateTimeValid()) {
    rtc.SetDateTime(RtcDateTime(__DATE__, __TIME__)); // Устанавливаем время компиляции
  }

  if (rtc.GetIsWriteProtected()) {
    rtc.SetIsWriteProtected(false); // Снимаем защиту записи
  }

  if (!rtc.GetIsRunning()) {
    rtc.SetIsRunning(true); // Запускаем RTC
  }

  // Инициализация пина для пьезоизлучателя
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonSetTimePin, INPUT_PULLUP);
  pinMode(buttonSetAlarmPin, INPUT_PULLUP);
  pinMode(buttonResetPin, INPUT_PULLUP);
  pinMode(buttonExitPin, INPUT_PULLUP);
  pinMode(buttonSelectDigitPin, INPUT_PULLUP);
}

void loop() {
  handleButtons();
  // Считываем текущее время
  RtcDateTime now = rtc.GetDateTime();

  // Работа в зависимости от текущего состояния
  switch (currentState) {
    case NORMAL:
      displayTime(now.Hour(), now.Minute()); // Отображение текущего времени
      checkAlarm(now.Hour(), now.Minute());  // Проверка срабатывания будильника
      break;

    case SET_TIME:
      handleTimeSetting();
      break;

    case SET_ALARM:
      handleAlarmSetting();
      break;
  }
}

// Обработка кнопок
void handleButtons() {
  static unsigned long lastDebounceTime = 0; // Время последнего изменения кнопки
  const unsigned long debounceDelay = 50;   // Задержка для защиты от дребезга

  // Кнопка установки времени
  if (digitalRead(buttonSetTimePin) == LOW) {
    if (millis() - lastDebounceTime > debounceDelay) {
      lastDebounceTime = millis(); // Обновляем время
      if (currentState == NORMAL) {
        currentState = SET_TIME;
        setHour = 0;
        setMinute = 0;
      }
    }
  }

  // Кнопка установки будильника
  if (digitalRead(buttonSetAlarmPin) == LOW) {
    if (millis() - lastDebounceTime > debounceDelay) {
      lastDebounceTime = millis(); // Обновляем время
      if (currentState == NORMAL) {
        currentState = SET_ALARM;
        alarmHour = 0;
        alarmMinute = 0;
      }
    }
  }

  // Кнопка сброса
  if (digitalRead(buttonResetPin) == LOW) {
    if (millis() - lastDebounceTime > debounceDelay) {
      lastDebounceTime = millis(); // Обновляем время
      resetPressed = true;
    }
  }

  // Кнопка для выхода из режима установки
  if (digitalRead(buttonExitPin) == LOW) {
    if (millis() - lastDebounceTime > debounceDelay) {
      lastDebounceTime = millis(); // Обновляем время
      switch (currentState) {
        case NORMAL:
          break;

        case SET_TIME:
          rtc.SetDateTime(RtcDateTime(2023, 1, 1, setHour, setMinute, 0)); // Записываем новое время в RTC
          currentState = NORMAL; // Возвращаемся в нормальный режим
          break;

        case SET_ALARM:
          currentState = NORMAL; // Возвращаемся в нормальный режим
          break;
      }
    }
  }

  // Кнопка для выбора разряда (часы или минуты)
  bool currentButtonState = digitalRead(buttonSelectDigitPin);

  if (currentButtonState != lastButtonSelectDigitState) {
    lastDebounceTime = millis(); // Обновляем время последнего изменения
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (currentButtonState == LOW && buttonSelectDigitState == HIGH) {
      // Переключаем разряд
      currentDigit = (currentDigit == 0) ? 1 : 0;
    }
    // Обновляем текущее состояние кнопки
    buttonSelectDigitState = currentButtonState;
  }

  // Обновляем предыдущее состояние кнопки
  lastButtonSelectDigitState = currentButtonState;

}

void handleTimeSetting() {
  if (resetPressed) {
    setHour = 0;
    setMinute = 0;
    resetPressed = false;
  }

  // Отображаем устанавливаемое время
  displayTime(setHour, setMinute);

  // Эмуляция увеличения времени
  if (digitalRead(buttonSetTimePin) == LOW) {
    delay(200); // Защита от дребезга
    if (currentDigit == 0) {
      setHour++;
      if (setHour >= 24) setHour = 0;
    } else {
      setMinute++;
      if (setMinute >= 60) setMinute = 0;
    }
  }

  // Подтверждение времени и выход
  if (resetPressed) {
    rtc.SetDateTime(RtcDateTime(2024, 1, 1, setHour, setMinute, 0));
    currentState = NORMAL; // Возвращаемся в нормальный режим
  }
}

void handleAlarmSetting() {
  if (resetPressed) {
    alarmHour = 0;
    alarmMinute = 0;
    resetPressed = false;
  }

  // Отображаем устанавливаемое время будильника
  displayTime(alarmHour, alarmMinute);

  // Эмуляция увеличения времени
  if (digitalRead(buttonSetAlarmPin) == LOW) {
    delay(200); // Защита от дребезга
    if (currentDigit == 0) {
      alarmHour++;
      if (alarmHour >= 24) alarmHour = 0;
    } else {
      alarmMinute++;
      if (alarmMinute >= 60) alarmMinute = 0;
    }
  }

  // Подтверждение времени будильника и выход
  if (resetPressed) {
    currentState = NORMAL; // Возвращаемся в нормальный режим
  }
}


void checkAlarm(int hour, int minute) {
  if (hour == alarmHour && minute == alarmMinute && !alarmActive) {
    alarmActive = true;
    playMelody();
    alarmActive = false;
  }
}

// Функция отображения времени в формате HH.MM
void displayTime(int hours, int minutes) {
  // Разбиваем часы и минуты на разряды
  int digitsToDisplay[4] = {hours / 10, hours % 10, minutes / 10, minutes % 10};

  // Поочерёдное отображение разрядов
  for (int i = 0; i < 4; i++) {
    digitalWrite(digitPins[i], HIGH); // Включаем текущий разряд

    // Отображаем текущую цифру
    displayDigit(digitsToDisplay[i]);

    // Включаем точку между часами и минутами (на втором разряде)
    if (i == 1) {
      digitalWrite(dpPin, LOW); // Включаем точку
    } else {
      digitalWrite(dpPin, HIGH); // Выключаем точку
    }

    delay(3); // Устанавливаем время отображения разряда
    digitalWrite(digitPins[i], LOW); // Выключаем разряд
  }
}

// Функция отображения одной цифры
void displayDigit(int num) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], digits[num][i] ? LOW : HIGH); // LOW = вкл., HIGH = выкл.
  }
}

// Функция для воспроизведения мелодии "Звездные войны"
void playMelody() {
  // Первая часть мелодии
  firstSection();

  // Вторая часть мелодии
  secondSection();

  // Вариант 1
  beep(f, 250);
  beep(gS, 500);
  beep(f, 350);
  beep(a, 125);
  beep(cH, 500);
  beep(a, 375);
  beep(cH, 125);
  beep(eH, 650);
  delay(500); // Задержка перед повторением

  // Повторяем вторую часть мелодии
  secondSection();

  // Вариант 2
  beep(f, 250);
  beep(gS, 500);
  beep(f, 375);
  beep(cH, 125);
  beep(a, 500);
  beep(f, 375);
  beep(cH, 125);
  beep(a, 650);
  delay(650); // Задержка перед завершением

  // Остановка музыки
  noTone(buzzerPin);
}

// Воспроизведение первой части мелодии
void firstSection() {
  beep(a, 500);
  beep(a, 500);
  beep(a, 500);
  beep(f, 350);
  beep(cH, 150);
  beep(a, 500);
  beep(f, 350);
  beep(cH, 150);
  beep(a, 650);

  delay(500); // Задержка перед продолжением
  beep(eH, 500);
  beep(eH, 500);
  beep(eH, 500);
  beep(fH, 350);
  beep(cH, 150);
  beep(gS, 500);
  beep(f, 350);
  beep(cH, 150);
  beep(a, 650);
  delay(500); // Задержка перед второй частью
}

// Воспроизведение второй части мелодии
void secondSection() {
  beep(aH, 500);
  beep(a, 300);
  beep(a, 150);
  beep(aH, 500);
  beep(gSH, 325);
  beep(gH, 175);
  beep(fSH, 125);
  beep(fH, 125);
  beep(fSH, 250);
  delay(325); // Задержка между нотами

  beep(aS, 250);
  beep(dSH, 500);
  beep(dH, 325);
  beep(cSH, 175);
  beep(cH, 125);
  beep(b, 125);
  beep(cH, 250);
  delay(350); // Задержка перед следующим циклом
}

// Функция для воспроизведения одной ноты
void beep(int note, int duration) {
  tone(buzzerPin, note, duration); // Проигрываем тон на пьезоизлучателе
  delay(duration);                 // Держим звук ноты
  noTone(buzzerPin);               // Останавливаем звук
  delay(50);                       // Задержка между нотами
}
