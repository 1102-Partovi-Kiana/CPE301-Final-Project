//Authors: Brandon Rowell, Kiana Partovi, Biniam Gashaw
//Purpose: Final Compilation of group project
//Date: 05/10/24



//STATE LIGHTS & ISR

//Port PD3 for pin18 and PD2 for reset button 
volatile unsigned char* port_d = (unsigned char*) 0x2B; 
volatile unsigned char* ddr_d  = (unsigned char*) 0x2A; 
volatile unsigned char* pin_d  = (unsigned char*) 0x29; 

//Port PJ1 and PJ0 for pin14 & 15, for yellow & green lights 
volatile unsigned char* port_j = (unsigned char*) 0x105; 
volatile unsigned char* ddr_j  = (unsigned char*) 0x104; 
volatile unsigned char* pin_j  = (unsigned char*) 0x103; 

//Port PH1 & PH0, pin16 & pin17 for red and blue light 
volatile unsigned char* port_h = (unsigned char*) 0x102; 
volatile unsigned char* ddr_h  = (unsigned char*) 0x101; 
volatile unsigned char* pin_h  = (unsigned char*) 0x100; 

volatile int masterButton = 0;

void masterButtonCheck();



//STEPPER MOTOR 

#include <Stepper.h>
const int stepsPerRevolution = 2038; //number of steps per rotation
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper myStepper = Stepper(stepsPerRevolution, 8, 10, 9, 11);

void stepperDirection();
void controlVent();

//Port PK2 for pin A10
volatile unsigned char* port_k = (unsigned char*) 0x108; 
volatile unsigned char* ddr_k  = (unsigned char*) 0x107; 
volatile unsigned char* pin_k  = (unsigned char*) 0x106;



//WATER SENSOR 

#define POWER_PIN 13
#define SIGNAL_PIN A0

//Port PB7 for pin13
volatile unsigned char* port_b = (unsigned char*) 0x25; 
volatile unsigned char* ddr_b  = (unsigned char*) 0x24; 
volatile unsigned char* pin_b  = (unsigned char*) 0x23;

unsigned int waterSensorVal;
void errorState();
void checkWaterLv();



// UART and RTC stuff 

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



// LCD and Fan for Temp & Humidity 
#include <dht.h> //install the DHTLib library
dht DHT;
#define DHT11_PIN 12

#include <LiquidCrystal.h>

// LCD pins <--> Arduino pins
const int RS = 6, EN = 7, D4 = 2, D5 = 3, D6 = 4, D7 = 5;

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

void lcdDisplay();
void getReadings();

unsigned int temp = 0;
unsigned int humidity = 0;

//Port PA0 for pin22 for fan
volatile unsigned char* port_a = (unsigned char*) 0x22; 
volatile unsigned char* ddr_a  = (unsigned char*) 0x21; 
volatile unsigned char* pin_a  = (unsigned char*) 0x20;

void updateLCD();
unsigned long currentTime = 0;
unsigned long pastTime = 0;
const long interval = 60000;

//handle the different states 
void disabledState();
void idleState();
void runningState();
void errorState();



//Timer details for delay 

volatile unsigned char *myTCCR1A = (unsigned char *) 0x80;
volatile unsigned char *myTCCR1B = (unsigned char *) 0x81;
volatile unsigned char *myTCCR1C = (unsigned char *) 0x82;
volatile unsigned char *myTIMSK1 = (unsigned char *) 0x6F;
volatile unsigned int  *myTCNT1  = (unsigned  int *) 0x84;
volatile unsigned char *myTIFR1 =  (unsigned char *) 0x36;



void setup() {
  U0init(9600); //setup UART
  //set PD3 as input with pullup 
  *ddr_d &= 0xF7;
  *port_d |= 0x08; 
  //set PJ1 as output 
  *ddr_j |= 0x02;
  //set PJ0 as output
  *ddr_j |= 0x01;
  //set PH1 as output 
  *ddr_h |= 0x02;
  //set PH0 as output 
  *ddr_h |= 0x01;
  //set PD2 as input with pullup 
  *ddr_d &= 0xFB;
  *port_d |= 0x04; 

  //ISR 
  attachInterrupt(digitalPinToInterrupt(18), buttonISR, FALLING);
  *port_j |= 0x02; //turning on yellow LED

  //Stepper Motor
  *ddr_k &= 0xFB; //set PK2 as input with pullup
  *port_k |= 0x04;
  myStepper.setSpeed(10);

  //Temp & Humidity sensor + LCD 
  adc_init(); //setting up ADC
  *ddr_b |= 0x80; //set PB7 as output
  *port_b &= 0x7F; //starting with sensor off
  *ddr_a |= 0x01;   // Set PA0 set to output
  *port_a &= ~0x01; // make sure fan off to start
  lcd.begin(16,2); //setup cols and rows

  //RTC
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {
    //start in disabled state but check idle 
    //runs until the conditions for the other two states are met in which they are entered 
    idleState();
}


void disabledState(){
  stateChangeUART(); //entering new state
  *port_j |= 0x02; //turning on yellow LED
  *port_j &= 0xFE; //turning off green LED 
  *port_h &= 0xFE; //turn off blue LED 
  *port_h &= 0xFD; //turn off red LED 
  *port_a &= ~0x01; // turn off fan  
  while(masterButton == 0){ //in the DISABLED state
    //stay here 
  }
  stateChangeUART(); //leaving state
}

void buttonISR() { //ISR
  masterButton++; 
  masterButton %= 2; //Keep as 1's and 0's aka on/off
}

void idleState(){
    if(masterButton == 0){ //check if transition to Disabled 
      disabledState();
    }
    *port_j &= 0xFD; //turning off yellow LED, moving out of disabled state
    *port_j |= 0x01; //turn on green LED
    *port_h &= 0xFE; //turn blue light off
    *port_a &= ~0x01; // turn off fan
    *port_h &= 0xFD; //turn red light off
    
    updateLCD(); //should still be updating the LCD once per min
    controlVent(); //stepper should be controllable here 
    getReading(); //updating current temp and humidity 

    //runs as long as the temperature threshold is exceeded 
    if(temp > 40){
      runningState();
    }

    //runs until water level is satisfied AND reset button is pressed 
    checkWaterLv();
    if(waterSensorVal <= 10){
      errorState();  
    }
    my_delay(0.47662); //freq that maximizes delay given a 256 prescaler which is 1.048s
}

void runningState(){
  stateChangeUART();
  *port_h |= 0x01; //turn blue light on
  *port_a |= 0x01;  // turn on fan 
  *port_j &= 0xFE; //turning off green LED
  *port_h &= 0xFD; //turn off red LED  

  while(temp > 40){
    getReading(); //looking for when we drop below the threshold
    updateLCD(); //should still be updating the LCD once per min
    controlVent(); //should be able to adjust vent here still
    //water level error supercedes this state, so we go here if water level is too low 
    checkWaterLv();
    if(waterSensorVal <= 10){
      errorState();  
    }
    my_delay(0.47662);
  }
  stateChangeUART(); 
  
}

void errorState(){
  stateChangeUART();
  *port_h &= 0xFE; //turn blue light off
  *port_j &= 0xFE; //turning off green LED
  *port_a &= ~0x01; // make sure fan is off
  *port_h |= 0x02; //turn red light on
  //Error message on LCD displayed
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("Error water");
  lcd.setCursor(0,1);
  lcd.print("level low!");

  while((waterSensorVal <= 10)){
      while(((*pin_d & 0x04) == 0)){ //stay in ERROR state until reset button pressed AND water level is no longer zero
        checkWaterLv();
        controlVent(); //should be able to adjust vent here still
        getReading(); //still tracking temp and humidity 
        updateLCD(); //should still be updating the LCD once per min
    }
  }
  lcd.clear(); //get rid of error message
  stateChangeUART(); //leaving this state so indicate state change
}

void controlVent(){
    //control vent here 
    if(*pin_k & 0x04){
      stepperDirection();  
    }  
}

//for switching vent direction
int currentDir = stepsPerRevolution; //global variable to switch direction by changing sign
void stepperDirection(){
    currentDir = (-1) * currentDir;
    myStepper.step(currentDir);
    
    //indicating using UART vent position is changed 
    char motorChangeMsg[23] = "Vent Direction Changed!";
    for(int j = 0; j < 23; j++){
      U0putchar(motorChangeMsg[j]);
    }
    U0putchar('\n');
}

//continuously measures the temp and humidity
void getReading(){
  int statusCheck = DHT.read11(DHT11_PIN);
  temp = DHT.temperature;
  humidity = DHT.humidity;

  my_delay(0.47662);
}

//continuously reads the water level 
void checkWaterLv(){
  *port_b |= 0x80; // turn the sensor on
  waterSensorVal = adc_read(0); // read the analog value from water sensor
  *port_b &= 0x7F; // turn the water sensor off
  my_delay(0.47662);
}

void updateLCD(){
    //updating LCD with temp & humidity once per minute
    currentTime = millis();
    if((currentTime - pastTime) >= interval){
      lcdDisplay();
      pastTime = currentTime;
    }
}

//displaying to LCD once per minute 
void lcdDisplay(){ //referenced circuit basics website for DTH info 
    lcd.setCursor(0,0); 
    lcd.print("Temp: ");
    lcd.print(temp);
    lcd.print((char)223);
    lcd.print("C");
    lcd.setCursor(0,1);
    lcd.print("Humidity: ");
    lcd.print(humidity);
    lcd.print("%");
}

//fxn that outputs messages when a state change occurs 
void stateChangeUART(){
  DateTime now = rtc.now();

  //store values 
  int year = now.year();
  int month = now.month();
  int day = now.day();
  int hour = now.hour();
  int minute = now.minute();
  int second = now.second();

  my_delay(0.47662);

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

void adc_init()
{
  // setup the A register
  *my_ADCSRA |= 0b10000000; // set bit   7 to 1 to enable the ADC, aka writing 1 to ADEN
  *my_ADCSRA &= 0b11011111; // clear bit 6 to 0 to disable the ADC trigger mode, start conversion 
  *my_ADCSRA &= 0b11110111; // clear bit 5 to 0 to disable the ADC interrupt, making ADATE 0 to disble auto trigger 
  *my_ADCSRA &= 0b11111000; // clear bit 0-2 to 0 to set prescaler selection to slow reading
  // setup the B register, messing with sampling modes (3 options: free-running, single conversion, and auto-triggered)
  *my_ADCSRB &= 0b11110111; // clear bit 3 to 0 to reset the channel and gain bits
  *my_ADCSRB &= 0b11111000; // clear bit 2-0 to 0 to set free running mode, meaning a new sample is taken when a prior has completed conversion 
  // setup the MUX Register
  *my_ADMUX  &= 0b01111111; // clear bit 7 to 0 for AVCC analog reference
  *my_ADMUX  |= 0b01000000; // set bit   6 to 1 for AVCC analog reference
  *my_ADMUX  &= 0b11011111; // clear bit 5 to 0 for right adjust result, resetl ADLAR 
  *my_ADMUX  &= 0b11100000; // clear bit 4-0 to 0 to reset the channel and gain bits
}
unsigned int adc_read(unsigned char adc_channel_num) //channel taken as an argument 
{
  // clear the channel selection bits (MUX 4:0)sev
  *my_ADMUX  &= 0b11100000;
  // clear the channel selection bits (MUX 5)
  *my_ADCSRB &= 0b11110111;
  // set the channel number
  if(adc_channel_num > 7)
  {
    // set the channel selection bits, but remove the most significant bit (bit 3)
    adc_channel_num -= 8;
    // set MUX bit 5
    *my_ADCSRB |= 0b00001000;
  }
  // set the channel selection bits
  *my_ADMUX  += adc_channel_num;
  // set bit 6 of ADCSRA to 1 to start a conversion, START CONVERSION 
  *my_ADCSRA |= 0x40;
  // wait for the conversion to complete
  while((*my_ADCSRA & 0x40) != 0);
  // return the result in the ADC data register
  return *my_ADC_DATA;
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
void my_delay(unsigned int freq){ 
  //prescaler 
  const int prescaler = 256; //need to get a longer delay
  // calc period
  double period = 1.0/double(freq);
  // 50% duty cycle
  double half_period = period/ 2.0f;
  // clock period def
  double clk_period = 0.0000000625*prescaler;
  // calc ticks
  unsigned int ticks = half_period / clk_period;
  // stop the timer
  *myTCCR1B &= 0xF8;
  // set the counts
  *myTCNT1 = (unsigned int) (65536 - ticks);
  // start the timer
  * myTCCR1A = 0x0;
  * myTCCR1B |= 0b00000100;//set prescaler
  // wait for overflow
  while((*myTIFR1 & 0x01)==0); // 0b 0000 0000
  // stop the timer
  *myTCCR1B &= 0xF8;   // 0b 0000 0000
  // reset TOV           
  *myTIFR1 |= 0x01;
}