//Getting the RTC working 

#include <RTClib.h>

#define RDA 0x80
#define TBE 0x20  

volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;
 
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78; 

RTC_DS1307 rtc;

void stateChangeUART();

void setup() {
  U0init(9600);
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {
  stateChangeUART();
}

void stateChangeUART(){
  delay(10000);

  DateTime now = rtc.now();

  //store values 
  int year = now.year();
  int month = now.month();
  int day = now.day();
  int hour = now.hour();
  int minute = now.minute();
  int second = now.second();

  //state change message
  char stateMsg[17] = "State change at: ";
  for(int i = 0; i < 17; i++){
    U0putchar(stateMsg[i]);
  }

  //converting hour 
  U0putchar(hour/10 + '0');
  hour %= 10;
  U0putchar(hour + '0');
  U0putchar(':');

  //converting minute
  U0putchar(minute/10 + '0');
  minute %= 10;
  U0putchar(minute + '0');
  U0putchar(':');

  //converting second 
  U0putchar(second/10 + '0');
  second %= 10;
  U0putchar(second + '0');

  U0putchar(' ');

  //converting day 
  U0putchar(day/10 + '0');
  day %= 10;
  U0putchar(day + '0');
  U0putchar('/');

  //converting month 
  U0putchar(month/10 + '0');
  month %= 10;
  U0putchar(month + '0');
  U0putchar('/');

  //converting year 
  U0putchar(year/1000 + '0');
  year %= 1000;
  U0putchar(year/100 + '0');
  year %= 100;
  U0putchar(year/10 + '0');
  year %= 10;
  U0putchar(year + '0');

  U0putchar('\n');
}

void U0init(int U0baud) //function to initialize USART0 to "int" Baud, 8 data bits 
{
 unsigned long FCPU = 16000000;
 unsigned int tbaud;
 tbaud = (FCPU / 16 / U0baud - 1);
 // Same as (FCPU / (16 * U0baud)) - 1;
 *myUCSR0A = 0x20;
 *myUCSR0B = 0x18;
 *myUCSR0C = 0x06;
 *myUBRR0  = tbaud;
}
unsigned char U0kbhit() //read USART0 status bit 
{
  return *myUCSR0A & RDA; //the return will be nonzero if data is available in USART buffer 
}
unsigned char U0getchar() //read input character from USART0 input buffer 
{
  return *myUDR0; //reading and returning data byte from USART location it is stored in
}
void U0putchar(unsigned char U0pdata) //transmit buffer, wait for USART0 TBE to be set then write character to it 
{
  while((*myUCSR0A & TBE)==0); //waiting for transmit buffer to be empty and flag to change to know when a new character can be written to it 
  *myUDR0 = U0pdata;
}

