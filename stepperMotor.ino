#include <Stepper.h>
const int stepsPerRevolution = 2038; //number of steps per rotation
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper myStepper = Stepper(stepsPerRevolution, 8, 10, 9, 11);

//Port PK2 for pin A10
volatile unsigned char* port_k = (unsigned char*) 0x108; 
volatile unsigned char* ddr_k  = (unsigned char*) 0x107; 
volatile unsigned char* pin_k  = (unsigned char*) 0x106; 

void stepperDirection();

void setup() {
  Serial.begin(9600);
  //set PK2 to input & enable pullup resistor for button 
  *ddr_k &= 0xFB;
  *port_k |= 0x04;
  myStepper.setSpeed(10); 
}

void loop() {
  if(*pin_k & 0x04){
    Serial.println("PIN IS HIGH");
    stepperDirection();  
  }
  Serial.println("Loop running");
}

int currentDir = stepsPerRevolution; //global variable to switch direction by changing sign
void stepperDirection(){
    currentDir = (-1) * currentDir;
    myStepper.step(currentDir);
}