#include <LiquidCrystal.h>  //Подключаем нужные библиотеки.
#include <SimpleTimer.h>

#include "types.h"  //Подключаем файл с типами.

#define quant 25  //Устанавливаем величину одного кванта времени в миллисекундах.
#define START_OF_FRAME_DELIMITER 0x7E

testPacket_t testPacket;  //Буфер принятого пакета.
SimpleTimer timer;        //Объявляем объект типа SimpleTimer
fsm_t fsm;                //Переменная с состояниями конечного автомата.

void introduction();           //Приветствие.
void notFound();               //Сообщение об ошибке.
void packetRead();             //Чтение пакета из модуля.
void displayUpdate();          //Обновление экрана.
void fakeDisplayUpdate();      //Обновления экрана при ошибке.
void mainSwitch();             //Конечный автомат.
byte getCheckSum(byte*, byte); //Подсчет контрольной суммы.

LiquidCrystal lcd(4, 5, 9, 10, 11, 12);  //Объявляем объект: LCD экран с соответтсвующими выводами.

unsigned char quantCounter;  //Счетчика квантов.

void setup()
{
  Serial.begin(9600);                     //Инициализация Serial.
  lcd.begin(16, 2);                       //Начинаем работу LCD.
  lcd.command(0x101010);                  //Переключаем страницу символов LCD экрана на вторую.
  introduction();                         //Экран приветствия.
  timer.setInterval(quant, mainSwitch);   //Устанавливаем таймер (задержка, callback-функция)
  fsm = PAUSE;                            //Инициализация конечного автомата.
}

void loop()
{
  timer.run(); //Обслуживание таймера.
}

void mainSwitch()
{
  switch(fsm)
  {
    case PAUSE: //В случае если конечный автомат в состоянии ПАУЗЫ.
      if(quantCounter >= 36) //Если счетчик квантов равняется 36, то есть прошло 0.9 секунды.
      {
        Serial.write(0x12);     //Посылаем в эфир любое число.
        fsm = WAIT_FOR_ANSWER;  //Переходим в состояние ожидания пакета.
        quantCounter = 0;       //Обнуляем счетчик квантов.
      }
      break;
    case WAIT_FOR_ANSWER: //В случае если конечный автомат в состоянии ОЖИДАНИЯ ПАКЕТА.
      if(quantCounter >= 4) //Если счетчик квантов равняется 4, то есть 0.1 секунды.
      {
        if(Serial.available()) //Если доступно сообщение по буферу.
        {
          packetRead(); //Вызываем функцию чтения пакета.
          if(testPacket.frameHeader.sof == START_OF_FRAME_DELIMITER) //Проверяем, не является первый байт в принятом пакете стартовым байтом.
          {
            if(getCheckSum(&testPacket.frameType, testPacket.frameHeader.lenLsb) == testPacket.checkSum)  //Если принятая контрольная сумма совпала с подсчитаной локально.
            {
              displayUpdate(); //Вызываем функцию обновления экрана.
            }
            else //Если принятая контрольная сумма с подсчитаной на `Ардуино` не совпала.
            {
              notFound();  //Вызываем функцию оповещения об ошибке.
            }
          }
          else //Первый байт не является стартовым байтом.
          {
            notFound(); //Вызываем функцию оповещения об ошибке.
          }
        }
        else //Если ответное сообщение не принято.
        {
          fakeDisplayUpdate(); //Вызываем функцию фиктивного обновления экрана.
        }
        fsm = PAUSE; //Переходим в состояние ПАУЗЫ.
        quantCounter = 0; //Обнуляем счетчик квантов.
      }
      break;
    default:
      break;
  }
  quantCounter++;  //Инкремент счетчика квантов.
}

void displayUpdate()
{
  lcd.clear();                                     
  lcd.setCursor(4, 0);                             
  lcd.print("MBee-868");                           
  lcd.setCursor(0, 1);                             
  lcd.print((char)0xD9);                           
  lcd.print("-");                                  
  lcd.print(0x100 - testPacket.text[6], DEC);     
  lcd.print("db ");                                
  lcd.print((char)0xDA);                           
  lcd.print("-");                                  
  lcd.print(0x100 - testPacket.receivedRSSI, DEC); 
  lcd.print("db");                                 
}

void fakeDisplayUpdate()
{
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("MBee-868");
  lcd.setCursor(0, 1);
  lcd.print((char)0xD9);
  lcd.print("---");
  lcd.print("db ");
  lcd.print((char)0xDA);
  lcd.print("---");
  lcd.print("db");
}

void introduction()
{
  lcd.setCursor(4, 0);
  lcd.print("SerialStar");
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
  byte i; 
  byte *buffer_p; //Объявляем указатель на буфер.
  buffer_p = (byte*)&testPacket; //Инициализируем укзатель.
  for(i = 0; i < sizeof(testPacket_t); i++)
  {
    *buffer_p++ = Serial.read();  //Копирование сообщения из приемного буфера UART в буфер пакетов.
  }
}

byte getCheckSum(byte* buffer_p, byte len)
{
  unsigned int checkSum = 0;
  byte count;
  for (count = 0; count < len; count++)
  {
    checkSum += *buffer_p++;
  }
  return 0xFF - checkSum & 0x00FF;
}

void notFound()
{
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("PACKET");
  lcd.setCursor(5, 1);
  lcd.print("ERROR");
}
