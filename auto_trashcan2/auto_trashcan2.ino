/*
  Created 2017
  by AlexGyver
  AlexGyver Home Labs Inc.
*/
/*
   Просыпаться каждую секунду, опрашивать расстояние
   Если заметили руку, включить серво, повернуть, запомнить время, снова спать каждую секунду
   Если руки нет и прошло > времени удержания, подключить серво и закрыть крышку
*/
//--------------------НАСТРОЙКИ----------------------
#define maxH 50             // высота срабатывания
#define cap_time 3          // время открытой крышки в секундах
#define open_angle 10       // угол открытия
#define close_angle 155     // угол закрытия
#define number_of_measures 5 // количество измерений для определения среднего

#define debug 0         // вывод информации в порт для отладки (0 выкл, 1 вкл)
//--------------------НАСТРОЙКИ----------------------

// ---ПОДКЛЮЧЕНИЕ---
#define trigPin 3
#define echoPin 2
#define sensorVCC 4
#define servoPowerPin 6
#define servoSignalPin 5
#define buttonPin 11
// ---ПОДКЛЮЧЕНИЕ---

#include "LowPower.h" // библиотека сна
#include <Servo.h>    // используем библиотеку для работы с сервоприводом
Servo servo;          // объявляем переменную servo типа Servo

long duration;
int measures = 0;
byte distance;

unsigned long open_timer;
boolean open_flag;
boolean serviceMode = false;

void setup() {
  pinMode(trigPin, OUTPUT);                       // пин триггера в режиме выхода
  pinMode(echoPin, INPUT);                        // пин эхо в режиме входа
  pinMode(sensorVCC, OUTPUT);                     // пин питания как выход
  digitalWrite(sensorVCC, 0);                     // выключить датчик
  digitalWrite(trigPin, 0);

  servo.attach(servoSignalPin);                   // серво на 5 порту
  pinMode(servoPowerPin, OUTPUT);                 // пин мосфета в режиме выхода
  digitalWrite(servoPowerPin, 0);

  if (debug) {
    Serial.begin(9600);
    Serial.println("system start");
  }
}

void loop() {
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);   // спать 1 секунду
  readButtonValue();                                // считываем показания кнопки
  makeTripleMeasure();                             // делаем три измерения
  processButtonAndSensorValues();                   // принимаем решение на основе полученных данных
}

void processButtonAndSensorValues() {
  if (distance > 5 && distance < maxH) {            // если расстояние попадает в диапазон
    open_timer = 0;                                 // сбросить таймер
    if (!open_flag) {                               // если крышка закрыта
      open_cap();                                   // открыть крышку
    }
  } else {
    if (serviceMode && !open_flag) {                // если включен сервисный режим и крышка закрыта
      open_cap();                                   // открываем крышку
    }
    if (open_flag && !serviceMode) {                // если крышка открыта и нет сервисного режима
      open_timer++;                                 // прибавить таймер
      if (open_timer > cap_time) {                  // если с момента открытия прошло больше 5сек
        close_cap();                                // закрыть крышку
      }
    }
  }
}

void open_cap() {
  if (debug)
    Serial.println("open");
  digitalWrite(servoPowerPin, 1);                   // подать питание на серво
  delay(5);                                         // ждем активации сервы
  servo.write(open_angle);                          // открыть крышку
  delay(1500);                                      // ждать пока крышка откроется
  digitalWrite(servoPowerPin, 0);                   // отключить серво
  open_flag = 1;                                    // флаг что крышка открыта
}

void close_cap() {
  if (debug)
    Serial.println("close");

  digitalWrite(servoPowerPin, 1);                   // подать питание на серво
  delay(5);                                         // ждем активации сервы
  servo.write(close_angle);                         // закрыть крышку
  delay(700);                                       // ждать серво
  digitalWrite(servoPowerPin, 0);                   // отключить серво
  open_flag = 0;                                    // флаг что крышка закрыта
}

void readButtonValue() {                            // проверяем, не нажата ли кнопка сервисного режима
  if (digitalRead(buttonPin) == HIGH) {
    if (serviceMode == false) {
      serviceMode = true;
    } else {
      open_timer = cap_time;                        // уравниваем таймер, чтобы крышка сразу закрылась
      serviceMode = false;
    }
  }
}

byte makeTripleMeasure() {
  digitalWrite(sensorVCC, 1);                       // включить дальномер
  measure();                                        // первый "холостой" проход, иначе датчик не алло
  measures = 0;
  for (int i = 0; i < number_of_measures; i++) {    // измеряем несколько раз
    measures = measures + measure();
  }

  digitalWrite(sensorVCC, 0);                       // выключить дальномер
  distance = measures / number_of_measures;          // делаем вывод о расстоянии на основе среднего арифиметического
  if (debug) {
    Serial.println(measures);
    Serial.println(distance);
  }
}

byte measure() {
  digitalWrite(trigPin, 0);                       // выключаем сигнал на всяки случай
  delayMicroseconds(5);                           // ждем 5мс
  digitalWrite(trigPin, 1);                       // начинаем посылать сигнал
  delayMicroseconds(15);                          // посылаем 15 мс
  digitalWrite(trigPin, 0);                       // прекращаем посылать сигнал
  duration = pulseIn(echoPin, HIGH);              // слушаем ответ
  return (duration / 2) / 29.1;                   // делим попалам и на скорость звука
}