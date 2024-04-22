//Compiling all pieces together 



//STATE LIGHTS & ISR

//Port PD3 for pin18 and PD2 for reset button 
volatile unsigned char* port_d = (unsigned char*) 0x2B; 
volatile unsigned char* ddr_d  = (unsigned char*) 0x2A; 
volatile unsigned char* pin_d  = (unsigned char*) 0x29; 

//Port PJ1 and PJ0 for pin14 & 15, for yellow & green lights 
volatile unsigned char* port_j = (unsigned char*) 0x105; 
volatile unsigned char* ddr_j  = (unsigned char*) 0x104; 
volatile unsigned char* pin_j  = (unsigned char*) 0x103; 

//Port PH1, pin16 for red light
volatile unsigned char* port_h = (unsigned char*) 0x102; 
volatile unsigned char* ddr_h  = (unsigned char*) 0x101; 
volatile unsigned char* pin_h  = (unsigned char*) 0x100; 

volatile int masterButton = 0;



//STEPPER MOTOR 

#include <Stepper.h>
const int stepsPerRevolution = 2038; //number of steps per rotation
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper myStepper = Stepper(stepsPerRevolution, 8, 10, 9, 11);

void stepperDirection();

//Port PK2 for pin A10
volatile unsigned char* port_k = (unsigned char*) 0x108; 
volatile unsigned char* ddr_k  = (unsigned char*) 0x107; 
volatile unsigned char* pin_k  = (unsigned char*) 0x106;



//WATER SENSOR 

#define POWER_PIN 13
#define SIGNAL_PIN A0

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

//Port PB7 for pin13
volatile unsigned char* port_b = (unsigned char*) 0x25; 
volatile unsigned char* ddr_b  = (unsigned char*) 0x24; 
volatile unsigned char* pin_b  = (unsigned char*) 0x23;

unsigned int waterSensorVal;
void waterLevel();
void checkWaterLv();



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
  //set PD2 as input with pullup 
  *ddr_d &= 0xFB;
  *port_d |= 0x04;  
  attachInterrupt(digitalPinToInterrupt(18), buttonISR, FALLING);

  //set PK2 as input with pullup
  *ddr_k &= 0xFB;
  *port_k |= 0x04;
  myStepper.setSpeed(10);


  adc_init(); //setting up ADC
  //set PB7 as output
  *ddr_b |= 0x80;
  *port_b &= 0x7F; //starting with sensor off
}

void loop() {
  if(masterButton == 0){
    *port_j |= 0x02; //turning on yellow LED
    *port_j &= 0xFE; //turning off green LED  
  }
  else{
  *port_j &= 0xFD; //turning off yellow LED, moving out of disabled state
  *port_j |= 0x01; //turn on green LED, in idle state

  //control vent here 
  if(*pin_k & 0x04){
    stepperDirection();  
  }

  //checking water sensor level
  waterLevel();

  //everything else for system goes here when NOT disabled 
  
  }
  delay(100); 
}

void buttonISR() {
  masterButton++; 
  masterButton %= 2; //Keep as 1's and 0's aka on/off
}

//function for switching vent direction
int currentDir = stepsPerRevolution; //global variable to switch direction by changing sign
void stepperDirection(){
    currentDir = (-1) * currentDir;
    myStepper.step(currentDir);
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

void waterLevel(){
  checkWaterLv();
  while((waterSensorVal == 0)){
      while(((*pin_d & 0x04) == 0)){ //stay in ERROR state until reset button pressed AND water level is no longer zero
      *port_j &= 0xFE; //turning off green LED
      *port_h |= 0x02; //turn red light on
      Serial.println("Error water level low!");
      checkWaterLv();
    }
  }
  *port_h &= 0xFD; //turn red light off 
}

void checkWaterLv(){
  *port_b |= 0x80; // turn the sensor on
  delay(10); // wait 10 milliseconds
  waterSensorVal = adc_read(0); // read the analog value from water sensor
  *port_b &= 0x7F; // turn the water sensor off
}
