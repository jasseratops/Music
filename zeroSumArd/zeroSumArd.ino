#include "SPI.h"
#include "math.h"

short unsigned int readPin = 15;

short unsigned int dPotA_CS = 5;
short unsigned int dPotB_CS = 6;
short unsigned int dPotA_Val = 0;
short unsigned int dPotB_Val = 0;
byte digiPotAddress = 0x00;
short unsigned int digiPotBits = 127;

short unsigned int dPotA_Mode = 0;
short unsigned int dPotB_Mode = 0;

unsigned int UniStep = 0;
int voltage = 0;

short int expnntl[128] = { };
short int lgrthmc[128] = { };
short int linear[128] = { };

short unsigned int expDepth = 4;

int digiPotBits = 127;

void curveCreator{
  for (int i = 0, i<128,i++){
    expnntl[i] = round((pow(i,expDepth))/(pow(127,expDepth-1)));
    lgrthmc[i] = round((log(i+1)/log(digiPotBits))*digiPotBits);
    linear[i] = i;
  }
}

void setup()
{
  curveCreator();
  pinMode(dPotA_CS, OUTPUT);
  pinMode(dPotB_CS, OUTPUT);
  Serial.begin(9600);
  SPI.begin();
  Serial.write("Program Start");
}

// The loop function is called in an endless loop
void loop()
{
  voltage = analogRead(readPin);
  // calc
  UniStep = ((voltage + 1)/8);

  Serial.print("\n");
  Serial.print(voltage);

  if(UniStep != 0){
    UniStep--;
  }

  
  dPotA_Val = UniStep;
  dPotB_Val = 127-UniStep;

  digiPotWrite(dPotA_CS,dPotA_Val);
  digiPotWrite(dPotB_CS,dPotB_Val);
}

void digiPotWrite(int CS, int value){
  Serial.print("\nHello from digiPotWrite. Value: ");
  Serial.print(value);
  digitalWrite(CS, LOW);
  SPI.transfer(digiPotAddress);
  SPI.transfer(value);
  digitalWrite(CS, HIGH);
}

