#include "Arduino.h"
#include "SPI.h"

volatile int mode = 3;

// expSeq params
int expSeq_i = 0;
bool ascend = 0;


// Set interrupt pins
const byte modeInterruptPin = 2;
const byte tapInterruptPin = 3;
const byte encInterruptPin = 18;
const byte OnOffInterrupt = 20;


const byte encPinB = 19;
int blinkerLED = 9;


// tap tempo params for MM and expSeq
float tdiffMaxMM = 500.00;
float tdiffMinMM = 250.00;  // spec say 50ms, but that is impossible to tap correctly, and fucks up the range
float tdiffMaxExpSeq = 1000.00;
float tdiffMinExpSeq = 250.00;

// tap tempo parameters
volatile unsigned long int tTap1 = 1;
volatile unsigned long int tTap2 = 1;
volatile unsigned long int tdiff = 500;
volatile float tdiffMax = tdiffMaxMM; // set to 500ms for meet maude
volatile float tdiffMin = tdiffMinMM;
float tdiffRST = 2000.00; // set to 750ms for meet maude
unsigned int BPMmin = ((60.00*1000.00)/tdiffMax);
unsigned int BPMmax = ((60.00*1000.00)/tdiffMin);
volatile unsigned  int BPM = ((60.00*1000.00)/tdiff);
unsigned int BPMlast;


// Digital Potentiometer Parameters
int digiPotBits = 7;
int nDigiPots = 1;
float digiPotOhm = 10000;
float totalOhmMax = nDigiPots*digiPotOhm;
float OhmMin = 0;
int nOhmSteps = 127;
volatile int stepVal = 0;
int digiPotCS = 6;
byte digiPotAddress = 0x00;
int Ohmage;

short int triArray[128] = { };
short int sineArray[128] = { };
short int sawArray[128] = { };
short int rSawArray[128] = { };
short int sqrArray[128] = { };
float sinRad;

void setup()
{
  pinMode(blinkerLED, OUTPUT);
  pinMode(digiPotCS, OUTPUT);

  pinMode(tapInterruptPin, INPUT);
  pinMode(modeInterruptPin, INPUT);
  pinMode(encInterruptPin, INPUT);

  pinMode(encPinB, INPUT);

  attachInterrupt(digitalPinToInterrupt(tapInterruptPin), tapTempo, FALLING);
  attachInterrupt(digitalPinToInterrupt(modeInterruptPin), modeCycle, FALLING);
  attachInterrupt(digitalPinToInterrupt(encInterruptPin), encChange, FALLING);

  randomSeed(analogRead(0));

  for(int x = 0; x <= nOhmSteps; x++)
  {


	  sinRad = ((float)x/128.0000)*6.283;
	  Serial.print("sinRad: ");
	  Serial.print(sinRad);
	  Serial.print("\n");

	  sineArray[x] = round(sin(sinRad)*63.50 + 63.50); // 6.283 = 2*pi, 63.5 = 127/2
	  sawArray[x]= x;
	  rSawArray [x]= nOhmSteps - x;

	  if (x <= (nOhmSteps/2)){sqrArray[x] = nOhmSteps;}
	  else{sqrArray[x] = 0;}
  }

  SPI.begin();
  Serial.begin(9600);
  Serial.write("Program Start\n");
  //Serial.write("\nThis is even newer\n");

  //Serial.print("\ntdiffMax: ");
  //Serial.print(tdiffMax);
  //Serial.print("\ntdiffMin: ");
  //Serial.print(tdiffMin);

  //Serial.print("\nBPMmax: ");
  //Serial.print(BPMmax);
  //Serial.print("\nBPMmin: ");
  //Serial.print(BPMmin);

}

// The loop function is called in an endless loop
void loop(){
  Serial.write("hello from loop \n");
  if (mode == 0){
    if (BPM != BPMlast){
      Serial.write("\nBPM: ");
      Serial.print(BPM);
      Ohmage = BPMtoOhm(BPM);
      stepVal = ohmToStep(Ohmage);
      digiPotWrite(stepVal);

      digitalWrite(blinkerLED, HIGH);         // turn the LED on (HIGH is the voltage level)
      delay(round(tdiff/10));                  // wait for a second
      digitalWrite(blinkerLED, LOW);          // turn the LED off by making the voltage LOw
      delay(round((4*tdiff)/10));
    }
  }
  else if(mode > 0 || mode !=5){
    stepVal = expSeq();
    digiPotWrite(stepVal);

    delay(tdiff/nOhmSteps);					// a way to convert the range of steps to a period
    if (expSeq_i < round((float)nOhmSteps/5.00)){
      digitalWrite(blinkerLED, HIGH);
    }
    else {
      digitalWrite(blinkerLED, LOW);
    }
  }
  BPMlast = BPM;
}

void modeCycle(){
  if (mode < 5){mode++;}
  else{mode = 1;}
  expSeq_i = 0;
  //Serial.print("\nMode: ");
  //Serial.print(mode);
}

void tapTempo(){
  //Serial.write("hello from interrupt \n");
  if (mode == 0){
    tdiffMax = tdiffMaxMM;
    tdiffMin = tdiffMinMM;
  }
  else{
    tdiffMax = tdiffMaxExpSeq;
    tdiffMin = tdiffMinExpSeq;
  }
  float tInt0;
  tInt0 = millis() + 1;
  if (tTap1 == 1 || tdiff == tdiffRST){
    tTap1 = tInt0;
  }
  else {
    if ((tInt0 - tTap1) >= tdiffMax){
      if ((tInt0 - tTap1) >= tdiffRST){
        tTap1 = tInt0;
      }
      else{
        tdiff = tdiffMax;
      }
    }
    else{
      tTap2 = tInt0;
      tdiff = tTap2 - tTap1;
      if (tdiff <=tdiffMin){
        tdiff = tdiffMin;
      }
    }
  }
  BPM = (60000/tdiff);
}

void encChange(){
	Serial.print("\nHello from enc Change. BPM: ");
	Serial.print(BPM);
	Serial.print("\n");
	if (digitalRead(encInterruptPin) == digitalRead(encPinB)){
		BPM += 10;
	}
	else{
		BPM -= 10;
	}
	Serial.print("New BPM: ");
	Serial.print(BPM);
	Serial.print("\n");
	tdiff = 60000/BPM;
}

float BPMtoOhm(float convBPM){
  BPMmin = ((60.00*1000.00)/tdiffMaxMM);
  BPMmax = ((60.00*1000.00)/tdiffMinMM);
  float range = (BPMmax-BPMmin)/(totalOhmMax);
  float Ohm = convBPM*range;
  //Serial.print("\nfrom BPM to Ohm.");
  //Serial.print("\n Ohm:   ");
  //Serial.print(Ohm);
  //Serial.print("\n range:    ");
  //Serial.print(range);
  //Serial.print("\n");
  return Ohm;

}

int ohmToStep (float convOhm){
  int digiPotStep;
  float stepReso = (round(totalOhmMax)-OhmMin)/nOhmSteps;
  if (convOhm > totalOhmMax){
    //Serial.print("\nResistance is too high. Value can not be written to DigiPot.\n");
    digiPotStep = 0;
  }
  else{
    //Serial.print("\n from the else\n");
    digiPotStep = convOhm/stepReso;
  }
  //Serial.print("\nfrom ohm to Step. convOhm: ");
  //Serial.print(convOhm);
  //Serial.print("\ntotalOhmMax: ");
  //Serial.print(totalOhmMax);
  //Serial.print("\nstepReso: ");
  //Serial.print(stepReso);
  //Serial.print("\ndigiPotStep: ");
  //Serial.print(digiPotStep);
  //Serial.print("\nExiting ohmToStep\n");
  return digiPotStep;
}

int expSeq(){
  if (expSeq_i == 127){expSeq_i = 0;}
  switch(mode){
    case 1:                 				// sine Wave  \/\/
      stepVal = sineArray[expSeq_i];
      break;
    case 2:                 				// saw wave
      stepVal = sawArray[expSeq_i];
      break;
    case 3:                 				// Reverse Saw Wave /|
      stepVal = rSawArray[expSeq_i];
      break;
    case 4:                 				// Square wave    |_|
      stepVal = sqrArray[expSeq_i];
      break;
    case 5:                					// randomizer   |_-__--|
      if(expSeq_i == 0 || expSeq_i == nOhmSteps/2){
    	  stepVal = random(0,nOhmSteps-20);}
      break;
  }
  expSeq_i++;
  return stepVal;
}

void digiPotWrite(int value){
  //Serial.print("\\nHello from digiPotWrite. Value: ");
  //Serial.print(value);
  digitalWrite(digiPotCS, LOW);
  SPI.transfer(digiPotAddress);
  SPI.transfer(value);
  digitalWrite(digiPotCS, HIGH);
}
