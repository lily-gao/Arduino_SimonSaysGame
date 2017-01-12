
//For Sounds
#include <toneAC.h>

#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_C5 523

int successMel[] = {NOTE_C4,NOTE_E4,NOTE_G4,NOTE_C5};
const int successMelSize  = sizeof (successMel)/sizeof(int);
int successDur [] = {1000/7,1000/7,1000/7,1000/2} ;
int failMel[] = {NOTE_A4, NOTE_F4, NOTE_D4};
const int failMelSize = sizeof (failMel)/sizeof(int);
int failDur[] = {1000/6,1000/6,1000/2} ;
int vol = 5;
int dur = 1000/6;
const int redN = NOTE_C4;
const int yellowN = NOTE_E4;
const int blueN = NOTE_G4;
const int greenN = NOTE_C5;



//For myMillis()- A project requirement was to re-create the millis() function 
//Number of times timer0 overflows (=number of 1.024 ms ellapsed since the begining of the progrem)
unsigned long overflows = 0;
//multiplier counting number of timer to multiply 0.024 seconds by to get the actual error
int errorMult = 0;

//Setting numbers representing the colors, which is also their LED pin numbers
enum colors {RED = 3, YELLOW, BLUE, GREEN};
const colors red = RED;
const colors yellow = YELLOW;
const colors blue = BLUE;
const colors green = GREEN;
int color;
const int signale = 11;

//Button pin numbers for each color
const int redB = A0;
const int yellowB = A1;
const int blueB = A2;
const int greenB = A3;

//Variables related to interrupts
volatile bool interruptTrig = false;
volatile int dRed;
volatile int dYellow;
volatile int dBlue;
volatile int dGreen;

//Variables related to answering time
unsigned long int limT = 20000;
unsigned long int startT;
unsigned long int endT;
int diffT;

//Determines if it is "ok to record user input"
bool okToRec = false;

//Arrays to record output sequence and user input sequence along with the actual size we are using (level for output[] and inSize for input)
  //Arrays' real sizes are set to 150 (represents max level) because it is highly unlikely that a user reaches this level
int level = 1;
int output [150];
int input [150];
int inSize = 0;

void setup() {
//Setting up timer2 (myMillis () uses timer2)
  TIMSK2 = 0x01;        //Timer2 INT Reg: Timer2 Overflow Interrupt Enable
  TCCR2A = 0x00;        //Timer2 Control Reg A: Wave Gen Mode normal

  //LED pins set as outputs
  pinMode (red, OUTPUT);
  pinMode (yellow, OUTPUT);
  pinMode (blue, OUTPUT);
  pinMode (green, OUTPUT);
  pinMode (signale,OUTPUT);
  //Button pins set as inputs
  pinMode (redB, INPUT);
  pinMode (yellowB, INPUT);
  pinMode (blueB, INPUT);
  pinMode (greenB, INPUT);
  
  //Seeding the random() funtion
  pinMode (A5, INPUT);
  randomSeed (analogRead(A5));

  attachInterrupt (0, handler, FALLING);
  Serial.begin (9600);
}

void loop() {

  //output light sequence if conditions are met
  if ((inSize == 0) && !okToRec) {
    getOutput();
    displayOut();
    delay(200);
    okToRec = true;
    //signal to user to start input
    digitalWrite (signale, HIGH);
    startT = myMillis();
  }

  //Get user input if time limit has not been reached and number of button presses is lower and the number of elements in outputted light sequence
  while (((myMillis() - startT) < limT)&& okToRec) {
    if (inSize >= level) {
      break;
    }
    //Differentiating between buttons
    if (interruptTrig &&(dRed > 0)) {
      addToIns (RED);      
    } else if ((interruptTrig)&&(dYellow > 0)) {
      addToIns (YELLOW);
    } else if ((interruptTrig)&&(dBlue > 0)) {
      addToIns (BLUE);
    } else if ((interruptTrig)&&(dGreen > 0)) {
      addToIns (GREEN);
    }
  }

  //signal to user to stop input
  digitalWrite (signale, LOW);
  okToRec = false;
  endT = myMillis();
  //displayArr (input, inSize);       //UNCOMMENT IF DEBUGGING OR CHEATING

  //Check if user got the sequence right
  if (isSame ()) {
  //  Serial.println("Same");         //UNCOMMENT IF DEBUGGING OR CHEATING
    level++;
    playMel (successMel, successDur, successMelSize);
    //Design decision: user should at least be allowed 1.5 seconds per sequence element. 
      //This condition is not met by the initial 20 seconds time limit if user level exceeds 13, hence this 'if' statement
    if (level >= 14){
      limT= level*1500;
    }
  } else {
    level = 1;
    playMel (failMel, failDur, failMelSize);
    lights (signale, 300, 300, HIGH);
    lights (signale, 300, 300, HIGH);
  }
  if (endT-startT >= limT){
    lights (signale, 300, 300, HIGH);
    lights (signale, 300, 300, HIGH);
    
  }
  clearArrays();
  delay (1500);

}


//Timer 2 overflow interrupt routine, used for myMillis()
   //This ISR counts the number of timer2 overflows (at the default prescaler, this takes 1.024 milliseconds, 
  // this is 3/125 more than 1 millisecond) and after each 125 overflows, the system corrects its error by adding 
  // 3 to the number of overflows (which corresponds to the number of milliseconds ellapsed since the begining
  //of the program). This means that myMillis can have at most an error of 2.976 milliseconds (accumulated over 124 overflows),
  //if we don't take into account the error on the timer 2 frequency...
  //This error is slightly higher than that of the original millis() function, which is around 1 millisecond.
ISR (TIMER2_OVF_vect){
  overflows++;
  errorMult ++;
  if (errorMult==125){
    overflows = overflows+3;
    errorMult = 0;
  }
}

unsigned long myMillis(){
  return overflows;
}

//Interrupt handler
void handler () {
 if (okToRec){
  dRed = analogRead(redB);
  dYellow = analogRead(yellowB);
  dBlue = analogRead(blueB);
  dGreen = analogRead(greenB);
  interruptTrig = true;
  }
}


//Plays any melody
void playMel (int mel[], int dur[], int melSize){
  delay(500);
  for (int i=0; i < melSize ;i++){
    toneAC(mel[i], 5, dur[i], true);
    delay (dur[i]);
  }
  noToneAC();
}


//Produces ramdom sequences depending on user level, and stores them into the output array
void getOutput() {
  for (int i = 0; i < level; i++) {
    color = (int) random (3, 7);
    output[i] = color;
  }
}



//void displayArr(int inOut [], int arrSize) {                //UNCOMMENT THIS SECTION IF DEBUGGING OR CHEATING
//  for (int i = 0; i < arrSize; i++) {
//    Serial.print (inOut [i]);
//    Serial.print (" ");
//  }
//  Serial.println ("--");
//}

//Makes lights change state and go back to previous state
void lights (int pin, int lightL ,int delayL, bool writeFirst){
  digitalWrite(pin, writeFirst);
  delay (lightL);
  digitalWrite (pin, !writeFirst);
  delay (delayL);
}

//Diplays sequence to user in the form of lights and sounds
void displayOut() {
  for (int i = 0; i < level; i++) {
//    Serial.print (output [i]);              //UNCOMMENT THIS SECTION IF DEBUGGING OR CHEATING
//    Serial.print (" ");

    switch (output [i]){
      case red :
        toneAC (redN, vol, dur, true);
        delay(8);
        break;
      case yellow :
        toneAC (yellowN, vol, dur, true);
        delay(8);
        break;
      case blue :
        toneAC (blueN, vol, dur, true);
        delay(8);
        break;
      case green :
        toneAC (greenN, vol, dur, true);
        delay(8);
        break;
    }
    lights(output[i], 650, 500, HIGH);

  }
  Serial.println ("--");
}


//Adds user input to input array while giving user feedback in the form of lights and sounds
//(ex: red 'tone' and light if user presses red button)
void addToIns (colors color){
  input[inSize++] = color;
  interruptTrig = false;
  digitalWrite (color,HIGH);
  switch (color){
      case red :
        toneAC (redN, vol, dur, true);
        delay(8);
        break;
      case yellow :
        toneAC (yellowN, vol, dur, true);
        delay(8);
        break;
      case blue :
        toneAC (blueN, vol, dur, true);
        delay(8);
        break;
      case green :
        toneAC (greenN, vol, dur, true);
        delay(8);
        break;
    }
   delay (380); 
   digitalWrite (color,LOW);
  dRed = 0;
  dYellow = 0;
  dBlue = 0;
  dGreen = 0;
}


//Checks if inout and output arrays are the same in size and in each index's element
bool isSame () {
  if (level != (inSize)) {
    Serial.println ("Size problem");
    return false;
  }
  for (int i = 0; i < level; i++) {
    if (output [i] != input [i]) {
      Serial.println ("element problem");
      return false;
    }
  }
  return true;
}


//Clears input and output array, sets input array size to 0 for next sequence
void clearArrays () {
  for (int i = 0 ; i < level ; i++) {
    output[i] = 0;
  }
  for (int i = 0 ; i <= inSize ; i++) {
    input[i] = 0;
  }
  inSize = 0;
}

