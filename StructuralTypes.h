typedef struct  //Объявляем структурный тип nodeAddress
{
  byte msb;
  byte lsb;
} moduleAddress_t;

typedef struct  //Объявляем структурный тип frameHeader
{
  byte sof;
  byte lenMsb;
  byte lenLsb;
} frameHeader_t;

typedef struct  //Объявляем структурный тип keyPacket
{
  frameHeader_t frameHeader;
  byte frameType;
  moduleAddress_t destination;
  byte receivedSSI;
  byte options;
  byte text[10];
  byte checkSum;             //Контрольная сумма
} testPacket_t;

typedef enum
{
  WAIT_FOR_ANSWER,
  PAUSE
} fsm_t;

