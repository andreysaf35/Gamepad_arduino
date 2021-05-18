#include <avr/wdt.h>

// NRF модуль
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
RF24 radio(9, 10); // "создать" модуль на пинах 9 и 10
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб

int transmit_data[2]; // массив, хранящий передаваемые данные
int latest_data[2]; // массив, хранящий последние переданные данные
boolean flag; // флажок отправки данных

void setup() {
  Serial.begin(9600);

  wdt_disable();                                // бесполезная строка до которой не доходит выполнение при bootloop
  wdt_enable (WDTO_8S);                         // Активируем Watch Dogs
  Serial.println("Watch Dog enable");
  
  // NRF модуль
  radio.begin(); //активировать модуль
  delay(2000);
  radio.setAutoAck(0);          //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);      //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload();     //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);             //размер пакета, в байтах
  radio.openWritingPipe(address[0]);    //мы - труба 0, открываем канал для передачи данных
  radio.setChannel(0x60);               //выбираем канал (в котором нет шумов!)
  radio.setPALevel (RF24_PA_MAX);       //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_250KBPS);     //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!
  radio.powerUp();        //начать работу
  radio.stopListening();  //не слушаем радиоэфир, мы передатчик
  
}

void loop() {

  if(millis() % 1000 == 0) {    // Выполняем по таймеру
    wdt_reset(); // Сбрасываем таймер Watch Dogs
  }
  // Левый джойстик джойстик на А0 и А1
  int signalXL = analogRead(0);
  int signalYL = analogRead(1);
  
  transmit_data[0] = signalXL;   // Правая гусеница
  transmit_data[1] = signalYL;   // Левая гусеница
  //Serial.println(transmit_data[1]);

  for (int i = 0; i < 2; i++) { // в цикле от 0 до числа каналов
    if (transmit_data[i] != latest_data[i]) { // если есть изменения в transmit_data
      flag = 1; // поднять флаг отправки по радио
      latest_data[i] = transmit_data[i]; // запомнить последнее изменение
      Serial.println(latest_data[i]);
    }
  }

  if (flag == 1) {
    radio.powerUp(); // включить передатчик
    //radio.flush_tx();
    radio.write(&transmit_data, sizeof(transmit_data)); // отправить по радио
    flag = 0; //опустить флаг
    radio.powerDown(); // выключить передатчик
    Serial.println("send");
  }
}
