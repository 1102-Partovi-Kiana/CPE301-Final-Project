//ISR to move from disabled to idle and back 
//will use the 1 and 0 to indicate whether to start/stop the currents system

//Port PD3 for pin18
volatile unsigned char* port_d = (unsigned char*) 0x2B; 
volatile unsigned char* ddr_d  = (unsigned char*) 0x2A; 
volatile unsigned char* pin_d  = (unsigned char*) 0x29; 

//Port PJ1 and PJ0 for pin14 & 15, for yellow & green lights 
volatile unsigned char* port_j = (unsigned char*) 0x105; 
volatile unsigned char* ddr_j  = (unsigned char*) 0x104; 
volatile unsigned char* pin_j  = (unsigned char*) 0x103; 

volatile int masterButton = 0;

void setup() {
  Serial.begin(9600);
  //set PD3 as input with pullup 
  *ddr_d &= 0xF7;
  *port_d |= 0x08; 
  //set PJ1 as output 
  *ddr_j |= 0x02;
  //set PJ0 as output
  *ddr_j |= 0x01;
  attachInterrupt(digitalPinToInterrupt(18), buttonISR, FALLING);
}

void loop() {
  if(masterButton == 0){
    *port_j |= 0x02; //turning on yellow LED
    *port_j &= 0xFE; //turning off green LED  
  }
  else{
  *port_j &= 0xFD; //turning off yellow LED, moving out of disabled state
  *port_j |= 0x01; //turn on green LED, in idle state

  //everything else for system goes here when NOT disabled 
  
  }
  Serial.println(masterButton);
  delay(1000); 
}

void buttonISR() {
  masterButton++; 
  masterButton %= 2; //Keep as 1's and 0's aka on/off
}
