#include <LiquidCrystal.h>  //Подключаем нужные библиотеки
#include <SimpleTimer.h>

#include "StructuralTypes.h"  //Подключаем файл со структурными типами

#define quant 25  //Устанавливаем величину одного кванта времени

testPacket_t testPacket;  //Объявляем объект типа testPacket_t
SimpleTimer timer;        //Объявляем объект типа SimpleTimer
fsm_t fsm;                //Объявляем объект типа fsm_t

void introduction();           //Объявляем функцию вступления
void notFound();               //Объявляем функцию неполадок с принятым пакетом
void packetRead();             //Объявляем функцию чтения и записывания полученого пакета
void displayUpdate();          //Объявляем функцию обновления экрана
void fakeDisplayUpdate();      //Объявляем функцию ложного обновления экрана
void mainSwitch();             //Объявляем главную функцию, которая будет управлять состояниями конечного автомата
byte getCheckSum(byte*, byte); //Объявляем функцию подсчета контрольной суммы

LiquidCrystal lcd(4, 5, 9, 10, 11, 12);  //Объявляем объект: LCD экран с настроенными выводами

unsigned char quantCounter;  //Объявляем переменную счетчика квантов

void setup()
{
  Serial.begin(9600);                     //Объявляем компонент Serial
  lcd.begin(16, 2);                       //Начинаем работу LCD экрана
  lcd.command(0x101010);                  //Переключаем страницу символов LCD экрана на вторую
  introduction();                         //Вызываем функцию вступления
  timer.setInterval(quant, mainSwitch);   //Устанавливаем таймер (задержка, функция)
  fsm = PAUSE;                            //Устанавливаем конечный автомат в состояние паузы
}

void loop()
{
  timer.run();  //Запускаем таймер
}

void mainSwitch()
{
  switch (fsm)
  {
    case PAUSE:  //В случае если конечный автомат в состоянии ПАУЗЫ
      if (quantCounter >= 36)  //Если счетчик квантов равняется 36, то есть 0.9 секунды
      {
        Serial.write(0x12);     //Пишем в эфир любое число
        fsm = WAIT_FOR_ANSWER;  //Переходим в состояние ожидания пакета
        quantCounter = 0;       //Обнуляем счетчик квантов
      }
      break;
    case WAIT_FOR_ANSWER:  //В случае если конечный автомат в состоянии ОЖИДАНИЯ ПАКЕТА
      if (quantCounter >= 4)  //Если счетчик квантов равняется 4, то есть 0.1 секунды
      {
        if (Serial.available())  //Если доступно сообщение по буферу
        {
          packetRead();  //Вызываем функцию чтения пакета
          if (testPacket.frameHeader.sof == 0x7E)  //Если тип принятого пакета 7E
          {
            if (getCheckSum(&testPacket.frameType, testPacket.frameHeader.lenLsb) == testPacket.checkSum)  //Если принятая контрольная сумма совпала с подсчитаной на `Ардуино`
            {
              displayUpdate();  //Вызываем функцию обновления экрана
            }
            else  //Если принятая контрольная сумма с подсчитаной на `Ардуино` не совпала
            {
              notFound();  //Вызываем функцию оповещения об ошибке
            }
          }
          else  //Если тип принятого пакета отличный от 7E
          {
            notFound();  //Вызываем функцию оповещения об ошибке
          }
        }
        else                     //Если сообщение по буферу не доступно
        {
          fakeDisplayUpdate();  //Вызываем функцию ложного обновления экрана
        }
        fsm = PAUSE;  //Переходим в состояние ПАУЗЫ
        quantCounter = 0;  //Обнуляем счетчик квантов
      }
      break;
    default:
      break;
  }
  quantCounter++;  //Прибавляем единичку к счетчику квантов
}

void displayUpdate()  //
{
  lcd.clear();                                     //
  lcd.setCursor(4, 0);                             //
  lcd.print("MBee-868");                           //
  lcd.setCursor(0, 1);                             //
  lcd.print((char)0xD9);                           //
  lcd.print("-");                                  //
  lcd.print(0x100 - testPacket.text[6], DEC);     //
  lcd.print("db ");                                //
  lcd.print((char)0xDA);                           //
  lcd.print("-");                                  //
  lcd.print(0x100 - testPacket.receivedSSI, DEC);  //
  lcd.print("db");                                 //
}

void fakeDisplayUpdate()
{
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("MBee-868");
  lcd.setCursor(0, 1);
  lcd.print((char)0xD9);
  lcd.print("-");
  lcd.print(" -");
  lcd.print("db ");
  lcd.print((char)0xDA);
  lcd.print("-");
  lcd.print(" -");
  lcd.print("db");
}

void introduction()
{
  lcd.setCursor(4, 0);
  lcd.print("SysMC`s");
  delay(500);
  lcd.clear();
  delay(250);
  lcd.setCursor(5, 0);
  lcd.print("RSSI");
  delay(500);
  lcd.clear();
  delay(250);
  lcd.setCursor(5, 0);
  lcd.print("Tester");
  delay(750);
  lcd.clear();
}

void packetRead()
{
  byte i;  //Объявляем переменную для цикла for
  byte *buffer_p;  //Объявляем переменную указателя на буфер
  buffer_p = (byte*)&testPacket;  //Объявляем переменную типа байт с указателем на буфер
  for (i = 0; i < sizeof(testPacket_t); i++)
  {
    *buffer_p++ = Serial.read();  //Пока i меньше количества байт в типе принимаемого пакета переменною buffer_p будет присваиваться значение считаное по буферу
  }
}

byte getCheckSum(byte* buffer_p, byte len)  //Функция подсчета контрольной суммы
{
  unsigned int checkSum = 0;  //Объявляем переменную контрольной суммы
  byte count;  //Объявляем переменную счетчика байт
  for (count = 0; count < len; count++) //Пока счет меньше длины пакета к нему прибавляется по единичке
  {
    checkSum += *buffer_p++; //К контрольной сумме будет прибавляться очередное число считанное по буферу
  }
  return 0xFF - checkSum & 0x00FF;  //Функция возвращает разницу 255 и младшего байта контрольной суммы
}

void notFound()
{
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("PACKET");
  lcd.setCursor(5, 1);
  lcd.print("ERROR");
}
