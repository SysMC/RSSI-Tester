typedef struct  //Тип для идентификатора модема.
{
  byte msb;
  byte lsb;
}moduleAddress_t;

typedef struct  //Заголовок API-фрейма.
{
  byte sof;
  byte lenMsb;
  byte lenLsb;
}frameHeader_t;

typedef struct  //Формат ответного API-фрейма.
{
  frameHeader_t frameHeader;
  byte frameType;
  moduleAddress_t destination;
  byte receivedRSSI;
  byte options;
  byte text[10];
  byte checkSum;//Контрольная сумма
}testPacket_t;

typedef enum //Состояния конечного автомата.
{
  WAIT_FOR_ANSWER,
  PAUSE
}fsm_t;

