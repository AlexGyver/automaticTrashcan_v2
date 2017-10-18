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
#define maxH 50         // высота срабатывания
#define cap_time 3      // время открытой крышки в секундах
#define open_angle 10   // угол открытия
#define close_angle 155 // угол закрытия

#define debug 0         // вывод информации в порт для отладки (0 выкл, 1 вкл)
//--------------------НАСТРОЙКИ----------------------

// ---ПОДКЛЮЧЕНИЕ---
#define trigPin 3
#define echoPin 2
#define sensorVCC 4
#define MOSFETpin 6
#define servoPin 5
// ---ПОДКЛЮЧЕНИЕ---

#include "LowPower.h" // библиотека сна
#include <Servo.h>    // используем библиотеку для работы с сервоприводом
Servo servo;          // объявляем переменную servo типа Servo

long duration;
byte distance;
unsigned long open_timer;
boolean open_flag;

void setup() {
  Serial.begin(9600);
  if (debug) Serial.println("system start");
  servo.attach(servoPin);         // серво на 5 порту
  pinMode(trigPin, OUTPUT);       // пин триггера в режиме выхода
  digitalWrite(trigPin, 0);
  pinMode(MOSFETpin, OUTPUT);     // пин мосфета в режиме выхода
  digitalWrite(MOSFETpin, 0);
  pinMode(echoPin, INPUT);        // пин эхо в режиме входа

  pinMode(sensorVCC, OUTPUT);     // пин питания как выход
  digitalWrite(sensorVCC, 0);     // выключить датчик

  open_cap();                     // открытие крышки при запуске
}

void open_cap() {
  if (debug) Serial.println("open");
  digitalWrite(MOSFETpin, 1);     // подать питание на серво
  delay(1);
  servo.write(open_angle);        // открыть крышку
  delay(1000);                    // ждать серво
  digitalWrite(MOSFETpin, 0);     // отключить серво
  open_flag = 1;                  // флаг что крышка открыта
}
void close_cap() {
  if (debug) Serial.println("close");
  digitalWrite(MOSFETpin, 1);     // подать питание на серво
  delay(1);
  servo.write(close_angle);       // закрыть крышку
  delay(1000);                    // ждать серво
  digitalWrite(MOSFETpin, 0);     // отключить серво
  open_flag = 0;                  // флаг что крышка закрыта
}

void loop() {
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);  // спать 1 секунду

  digitalWrite(sensorVCC, 1);           // включить дальномер
  measure();                            // первый "холостой" проход, иначе датчик не алло
  distance = measure();                 // получить расстояние
  digitalWrite(sensorVCC, 0);           // выключить дальномер
  if (debug) {
    Serial.println(distance);
    delay(500);
  }  

  if (distance > 5 && distance < maxH){ // если расстояние попадает в диапазон
    open_timer = 0;                     // сбросить таймер
    if (!open_flag) {                   // если крышка закрыта
      open_cap();                       // открыть крышку
    }
  } else {
    if (open_flag) {                    // если крышка открыта
      open_timer++;                     // прибавить таймер
      if (open_timer > cap_time) {      // если с момента открытия прошло больше 5сек
        close_cap();                    // закрыть крышку
      }
    }

  }
}

// блок, измеряющий расстояние
byte measure() {
  digitalWrite(trigPin, 0);
  delayMicroseconds(5);
  digitalWrite(trigPin, 1);
  delayMicroseconds(15);
  digitalWrite(trigPin, 0);
  duration = pulseIn(echoPin, HIGH);
  return (duration / 2) / 29.1;        // получаем расстояние с датчика
}
