//This code just turns the DC motor on and off to make sure hardware setup is good
//It will actually turn on and off based on water sensor

//Port PA0 for pin22
volatile unsigned char* port_a = (unsigned char*) 0x22; 
volatile unsigned char* ddr_a  = (unsigned char*) 0x21; 
volatile unsigned char* pin_a  = (unsigned char*) 0x20;

void setup() {
 *ddr_a |= 0x01;   // Set PA0 set to output
 *port_a &= ~0x01; // turn off fan 
}

void loop() {
  delay(1000);
  *port_a |= 0x01;  // turn on fan
  delay(1000);
  *port_a &= ~0x01; // turn off fan 
}

