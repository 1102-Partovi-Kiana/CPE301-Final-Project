//Currently turns on sensor, reads using adc, and outputs to serial which is not required since it will be used as threshold to enter error state
//included UART stuff although used standard serial libraries 
//unsure how to replay delay with my_delay() since takes in a frequency 

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

//Port PB7 for pin D13
volatile unsigned char* port_b = (unsigned char*) 0x25; 
volatile unsigned char* ddr_b  = (unsigned char*) 0x24; 
volatile unsigned char* pin_b  = (unsigned char*) 0x23; 

//pointers for timer manipulation
// volatile unsigned char *myTCCR1A = (unsigned char *) 0x80;
// volatile unsigned char *myTCCR1B = (unsigned char *) 0x81;
// volatile unsigned char *myTCCR1C = (unsigned char *) 0x82;
// volatile unsigned char *myTIMSK1 = (unsigned char *) 0x6F;
// volatile unsigned int  *myTCNT1  = (unsigned  int *) 0x84;
// volatile unsigned char *myTIFR1 =  (unsigned char *) 0x36;

int value = 0; // variable to store the sensor value
void setup() {
 U0init(9600); //setup UART
 adc_init(); //setting up ADC
 *ddr_b |= 0x80;
 *port_b &= 0x7F; //starting with sensor off 
}

void loop() {
 *port_b |= 0x80; // turn the sensor on
 delay(10); // wait 10 milliseconds
 unsigned int val = adc_read(0); // read the analog value from sensor
 *port_b &= 0x7F; // turn the sensor off
 Serial.print("Sensor value: " );
 Serial.println(val);
 delay(1000);
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
// void my_delay(unsigned int freq){ //code provided in timer_example.ino
//   // calc period
//   double period = 1.0/double(freq);
//   // 50% duty cycle
//   double half_period = period/ 2.0f;
//   // clock period def
//   double clk_period = 0.0000000625;
//   // calc ticks
//   unsigned int ticks = half_period / clk_period;
//   // stop the timer
//   *myTCCR1B &= 0xF8;
//   // set the counts
//   *myTCNT1 = (unsigned int) (65536 - ticks);
//   // start the timer
//   * myTCCR1A = 0x0;
//   * myTCCR1B |= 0b00000001;
//   // wait for overflow
//   while((*myTIFR1 & 0x01)==0); // 0b 0000 0000
//   // stop the timer
//   *myTCCR1B &= 0xF8;   // 0b 0000 0000
//   // reset TOV           
//   *myTIFR1 |= 0x01;
// }