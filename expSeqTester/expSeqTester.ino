#include <SPI.h>

byte address = 0x00;
int CS = 6;

volatile int mode = 6;


bool ascend;
volatile int stepVal = 0;
int nOhmSteps = 127;

void setup() {
  pinMode (CS, OUTPUT);

  SPI.begin();
  Serial.begin(9600);

  Serial.print("Start");
  
  randomSeed(analogRead(0));
  

}

void loop() {
  Serial.print("\nloop\n");
  
  expSeq();
  Serial.print(stepVal);
  digitalPotWrite(stepVal);
  
  delay(10);
}


int expSeq(){
  Serial.write("\nexpSeq\n");

  if (stepVal == 0){ascend = 1;}
  else if (stepVal == nOhmSteps){ascend = 0;}
  switch(mode){
    case 1:                 // sine Wave    \_/
      if (ascend){stepVal++;}   // place holder from tri
      else{stepVal--;}
      break;
    case 2:                 // saw wave
      if(ascend){stepVal = nOhmSteps;}
      else{stepVal--;}
      break;
    case 3:                 // Reverse Saw Wave /|
      if(ascend){stepVal++;}
      else{stepVal = 0;}
      break;
    case 4:                 // triangle wave  \/\/
      if (ascend){stepVal++;}
      else{stepVal--;}
      break;
    case 5:                 // Square wave    |_|
      if (ascend){stepVal = nOhmSteps;}
      else{stepVal = 0;}
      break;
    case 6:                 // randomizer   |_-__--|
      stepVal = random(0,nOhmSteps);
      break;
  }
}

int digitalPotWrite(int value){
  Serial.write("\ndigipot write\n");
  digitalWrite(CS, LOW);
  SPI.transfer(address);
  SPI.transfer(value);
  digitalWrite(CS, HIGH);
}

