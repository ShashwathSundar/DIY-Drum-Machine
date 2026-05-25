#include <Keypad.h>
#include <TimerOne.h>
#include <LedControl.h>
#define home 0
#define editTempo 1
#define editBeatLen 2
#define editNotes 3

#define maxBeatLen 16
#define notesPerBeat 12
#define maxScreenBrightness 8
#define minTempo 30
#define maxTempo 240

int BPM=120; //tempo
int newbpm=BPM; //temp bpm memory for manual entry
byte mode=home;
bool tick=false;
byte bankA[maxBeatLen * notesPerBeat]={0};
byte bankB[maxBeatLen * notesPerBeat]={0};
byte beatnotes=0; //current beat for editing notes
bool altBank=false;
bool pause=false;
byte muteMask=0xff;
byte beatLen=maxBeatLen; // beat length
byte newblen=0; //temp beat length for manual entry
byte curChannel=0; //current channel being edited
byte stepCount=0; //step to be played next by the sequencer

byte screenBrightness=0;

char customKey='m'; //default to home mode on boot
const byte ROWS = 5;
const byte COLS = 5;
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'R','M','S','m','-'},
  {'B','1','2','3','N'},
  {' ','4','5','6','C'},
  {' ','7','8','9','n'},
  {' ','*','0','#','+'}
};
byte rowPins[ROWS] = {A4,A3,A2,A1,A0}; //connect to the row pinouts of the keypad M3;M4;M5;M6;M7
byte colPins[COLS] = {2,3,4,5,6}; //connect to the column pinouts of the keypad M8;M9;M10;M11;M12

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

//bitmaps for characters and numbers [4x5 matrix]
byte cAB[5]={0b00110101,0b01010111,0b00110101,0b01010101,0b00110010};
byte cCD[5]={0b00110110,0b01010001,0b01010001,0b01010001,0b00110110};
byte cEF[5]={0b00010111,0b00010001,0b00110011,0b00010001,0b01110111};
byte cGH[5]={0b01010111,0b01010101,0b01110101,0b01010001,0b01010110};
byte c1[5]={0b0111,0b0010,0b0010,0b0011,0b0010};
byte c2[5]={0b0111,0b0001,0b0111,0b0100,0b0111};
byte c3[5]={0b0111,0b0100,0b0111,0b0100,0b0111};
byte c4[5]={0b0100,0b0100,0b0111,0b0101,0b0101};
byte c5[5]={0b0111,0b0100,0b0111,0b0001,0b0111};
byte c6[5]={0b0111,0b0101,0b0111,0b0001,0b0111};
byte c7[5]={0b0100,0b0100,0b0100,0b0100,0b0111};
byte c8[5]={0b0111,0b0101,0b0111,0b0101,0b0111};
byte c9[5]={0b0100,0b0100,0b0111,0b0101,0b0111};
byte c0[5]={0b0111,0b0101,0b0101,0b0101,0b0111};

/* display connection:
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD */
LedControl lc=LedControl(12,11,10,2);

//Interrupt service routine:
void blinkLED(void)
{ tick=!tick;

  if(tick && !pause){
   
    byte val=0;
    
    //set step value based on the current bank selected:
    if(!altBank)
      val=bankA[stepCount];
    else
      val=bankB[stepCount];
      
  //apply masking for mute and solo effect:
      val&=muteMask;

      //Shift register commections:: D8-latch ; D7-DIN ; D9-clock
    //shift out LSB first
    for(byte i=0;i<8;i++)
      { 
        //set data bit:
        if(1&(val>>i))
          digitalWrite(7,HIGH);
        else
          digitalWrite(7,LOW);
          
        //toggle clock to shift the bit
        digitalWrite(9,HIGH); 
        digitalWrite(9,LOW);
        
        }
        //toggle latch:
        digitalWrite(8,HIGH);
        digitalWrite(8,LOW);
        
    //increment step counter ; set to 0 on overflow
    stepCount+=1;
    if(stepCount>=(beatLen*notesPerBeat))
      {stepCount=0;
      }
  }
  else{
    //reset output
    //shift out zero:
    for(byte i=0;i<8;i++)
      { 
        digitalWrite(7,LOW);
        //toggle clock
        digitalWrite(9,HIGH);
        digitalWrite(9,LOW);
        
        }
    //toggle latch
    digitalWrite(8,HIGH);
    digitalWrite(8,LOW);
    }
    
    //LED for showing the clock ticks:
    digitalWrite(13, tick);
    
}
unsigned long BPMtoDelay(int bpm)
{
  // (bpm*24)/60 : number of ticks per sec for 24 notes per beat
  // 1000000/(ticks per sec) : microsecond delay for each tick
  return (2500000/bpm);
  
}
void displayNum(byte disp,byte number,bool leadingZero){
      
  byte numLen[5]={0};
  
  byte num=number/10;
  switch (num){
    case 0:
       for(byte i=0;i<5;i++)
         if(leadingZero)
         numLen[i]=c0[i];
        break;
    case 1:
       for(byte i=0;i<5;i++)
         numLen[i]=c1[i];
        break;
    case 2:
       for(byte i=0;i<5;i++)
         numLen[i]=c2[i];
        break;
    case 3:
       for(byte i=0;i<5;i++)
         numLen[i]=c3[i];
        break;
    case 4:
       for(byte i=0;i<5;i++)
         numLen[i]=c4[i];
        break;
    case 5:
       for(byte i=0;i<5;i++)
         numLen[i]=c5[i];
        break;
    case 6:
       for(byte i=0;i<5;i++)
         numLen[i]=c6[i];
        break;
    case 7:
       for(byte i=0;i<5;i++)
         numLen[i]=c7[i];
        break;
    case 8:
       for(byte i=0;i<5;i++)
         numLen[i]=c8[i];
        break;
    case 9:
       for(byte i=0;i<5;i++)
         numLen[i]=c9[i];
        break;
  }
  num=number%10;
    switch(num){
    case 1:
       for(byte i=0;i<5;i++)
         numLen[i] |= c1[i]<<4;
        break;
    case 2:
       for(byte i=0;i<5;i++)
         numLen[i] |= c2[i]<<4;
        break;
    case 3:
       for(byte i=0;i<5;i++)
         numLen[i] |= c3[i]<<4;
        break;
    case 4:
       for(byte i=0;i<5;i++)
         numLen[i] |= c4[i]<<4;
        break;
    case 5:
       for(byte i=0;i<5;i++)
         numLen[i] |= c5[i]<<4;
        break;
    case 6:
       for(byte i=0;i<5;i++)
         numLen[i] |= c6[i]<<4;
        break;
    case 7:
       for(byte i=0;i<5;i++)
         numLen[i] |= c7[i]<<4;
        break;
    case 8:
       for(byte i=0;i<5;i++)
         numLen[i] |= c8[i]<<4;
        break;
    case 9:
       for(byte i=0;i<5;i++)
         numLen[i] |= c9[i]<<4;
        break;
    case 0:
       for(byte i=0;i<5;i++)
         numLen[i] |= c0[i]<<4;
        break;
    }
    for(byte i=0;i<5;i++)
         lc.setRow(disp,i+3,numLen[i]);
      }
void displayChannel(){
  byte bankval[5]={0};
  
  switch (altBank){
    case 0:
       for(byte i=0;i<5;i++)
         bankval[i]=c0[i];
        break;
    case 1:
       for(byte i=0;i<5;i++)
         bankval[i]=c1[i];
        break;
  }
  switch (curChannel){
    case 0:
      for(byte i=0;i<5;i++)
         lc.setRow(0,i+3,(cAB[i]&0x0f)<<4 | bankval[i]);
         break;
    case 1:
      for(byte i=0;i<5;i++)
         lc.setRow(0,i+3,(cAB[i]&0xf0) | bankval[i]);
         break;
    case 2:
      for(byte i=0;i<5;i++)
         lc.setRow(0,i+3,(cCD[i]&0x0f)<<4 | bankval[i]);
         break;
    case 3:
      for(byte i=0;i<5;i++)
         lc.setRow(0,i+3,(cCD[i]&0xf0) | bankval[i]);
         break;
    case 4:
      for(byte i=0;i<5;i++)
         lc.setRow(0,i+3,(cEF[i]&0x0f)<<4 | bankval[i]);
         break;
    case 5:
      for(byte i=0;i<5;i++)
         lc.setRow(0,i+3,(cEF[i]&0xf0) | bankval[i]);
         break;
    case 6:
      for(byte i=0;i<5;i++)
         lc.setRow(0,i+3,(cGH[i]&0x0f)<<4 | bankval[i]);
         break;
    case 7:
      for(byte i=0;i<5;i++)
         lc.setRow(0,i+3,(cGH[i]&0xf0) | bankval[i]);
         break;
  }
 
  }
void displayHome(){
  displayChannel();
  displayNum(1,beatLen,true);

  }
  void displayTempo(){
    displayNum(0,BPM/100,false);
    displayNum(1,BPM%100,true);
    }
 void displayNotes(){
  int noteSet=0;
  for (byte i=0;i<12;i++)
    if (!altBank)
    noteSet |= (((bankA[beatnotes-notesPerBeat+i]>> curChannel) & 1) <<i); //get step for current beat; shift and mask for current channel; shift to note position ans set the noteSet bit;
    else
    noteSet |= (((bankB[beatnotes-notesPerBeat+i]>> curChannel) & 1) <<i); //get step for current beat; shift and mask for current channel; shift to note position ans set the noteSet bit;
    
    noteSet= noteSet<<2;
    
    lc.setRow(0,1,noteSet | 1); // OR bit mask just for display
    lc.setRow(1,1,noteSet>>8 | 0x80); //OR bit mask just for display
    lc.setRow(0,0,0b00000001); //just for display
    lc.setRow(1,0,0b10000000); //just for display
    
  displayChannel();
  displayNum(1,beatnotes/notesPerBeat,true);
  }
 
void setup(){
  Timer1.initialize(BPMtoDelay(BPM)); // microseconds
  Timer1.attachInterrupt(blinkLED);
  //pin setup for shift register:
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(9,OUTPUT);
  digitalWrite(7,LOW);
  digitalWrite(8,LOW);
  digitalWrite(9,LOW);
  
  lc.shutdown(0,false);
  lc.shutdown(1,false);
  /* Set the brightness to lowest value */
  lc.setIntensity(0,0);
  lc.setIntensity(1,0);
  /* and clear the display */
  lc.clearDisplay(0);
  lc.clearDisplay(1);
    
}
  
void loop(){
  if (customKey){
  switch (customKey){
    case 'M': //tempo mode toggle
      mode=editTempo;
      newbpm=0;
      break;
    case 'm' :  //home (channel)
      mode=home;
      break;
    case 'N' :  // edit beat
      mode=editBeatLen;
      newblen=0;
      break;
    case 'n' :  // edit notes for selected beat
      mode=editNotes;
      beatnotes=notesPerBeat;//first 12 notes OR first beat
      break;
    case 'S' :  // play pause
      pause=!pause;
      break;
    case 'B' :  // change bank
      altBank=!altBank;
      break;
    case 'R':
      stepCount=0;
      break;
    case 'C':
      if (mode==home)
          for(byte i=0;i<192;i++)
            if(!altBank)
              bankA[i] &= ~(1<<curChannel);
            else
              bankB[i] &= ~(1<<curChannel);
      if (mode==editNotes)
          for(byte i=0;i<12;i++)
            if(!altBank)
              bankA[beatnotes-12+i] &= ~(1<<curChannel);
            else
              bankB[beatnotes-12+i] &= ~(1<<curChannel);
      break;
    case '1':
      if (mode==editNotes){
        if(!altBank)
          bankA[beatnotes-12] ^= 1<<curChannel;     //toggle 1st note of current beat for current channel
        else
          bankB[beatnotes-12] ^= 1<<curChannel;}
      if (mode==editTempo)
        newbpm=(newbpm*10)+1;
      if (mode==editBeatLen)
        newblen=(newblen*10)+1;
      if (mode==home)
        curChannel=0;
      break;
     case '2':
      if (mode==editNotes){
        if(!altBank)
          bankA[beatnotes-11] ^= 1<<curChannel;     //toggle 2nd note of current beat for current channel
        else
          bankB[beatnotes-11] ^= 1<<curChannel;}
       if (mode==editTempo)
       newbpm=(newbpm*10)+2;
       if (mode==editBeatLen)
        newblen=(newblen*10)+2;
       if (mode==home)
        curChannel=1;
      break;
     case '3':
      if (mode==editNotes){
        if(!altBank)
          bankA[beatnotes-10] ^= 1<<curChannel;     //toggle 3rd note of current beat for current channel
        else
          bankB[beatnotes-10] ^= 1<<curChannel;}
       if (mode==editTempo)
        newbpm=(newbpm*10)+3;
        if (mode==editBeatLen)
        newblen=(newblen*10)+3;
       if (mode==home)
        curChannel=2;
      break;
     case '4':
        if (mode==editNotes){
        if(!altBank)
          bankA[beatnotes-9] ^= 1<<curChannel;      //toggle 4th note of current beat for current channel
        else
          bankB[beatnotes-9] ^= 1<<curChannel;}
       if (mode==editTempo)
        newbpm=(newbpm*10)+4;
        if (mode==editBeatLen)
        newblen=(newblen*10)+4;
       if (mode==home)
        curChannel=3;
      break;
     case '5':
      if (mode==editNotes){
        if(!altBank)
          bankA[beatnotes-8] ^= 1<<curChannel;      //toggle 5th note of current beat for current channel
        else
          bankB[beatnotes-8] ^= 1<<curChannel;}
      if (mode==editTempo)
        newbpm=(newbpm*10)+5;
        if (mode==editBeatLen)
        newblen=(newblen*10)+5;
      if (mode==home)
        curChannel=4;
      break;
     case '6':
      if (mode==editNotes){
        if(!altBank)
          bankA[beatnotes-7] ^= 1<<curChannel;      //toggle 6th note of current beat for current channel
        else
          bankB[beatnotes-7] ^= 1<<curChannel;}
      if (mode==editTempo)
        newbpm=(newbpm*10)+6;
        if (mode==editBeatLen)
        newblen=(newblen*10)+6;
      if (mode==home)
        curChannel=5;
      break;
     case '7':
      if (mode==editNotes){
        if(!altBank)
          bankA[beatnotes-6] ^= 1<<curChannel;      //toggle 7th note of current beat for current channel
        else
          bankB[beatnotes-6] ^= 1<<curChannel;}
      if (mode==editTempo)
        newbpm=(newbpm*10)+7;
        if (mode==editBeatLen)
        newblen=(newblen*10)+7;
      if (mode==home)
        curChannel=6;
      break;
     case '8':
      if (mode==editNotes){
        if(!altBank)
          bankA[beatnotes-5] ^= 1<<curChannel;      //toggle 8th note of current beat for current channel
        else
          bankB[beatnotes-5] ^= 1<<curChannel;}
      if (mode==editTempo)
        newbpm=(newbpm*10)+8;
        if (mode==editBeatLen)
        newblen=(newblen*10)+8;
      if (mode==home)
        curChannel=7;
      break;
     case '9':
      if (mode==editNotes){
        if(!altBank)
          bankA[beatnotes-4] ^= 1<<curChannel;      //toggle 9th note of current beat for current channel
        else
          bankB[beatnotes-4] ^= 1<<curChannel;}
      if (mode==editTempo)
        newbpm=(newbpm*10)+9;
        if (mode==editBeatLen)
        newblen=(newblen*10)+9;
      
      break;
     case '*':
      if (mode==editNotes){
        if(!altBank)
          bankA[beatnotes-3] ^= 1<<curChannel;      //toggle 10th note of current beat for current channel
        else
          bankB[beatnotes-3] ^= 1<<curChannel;}
      if(mode==home)                                //solo current channel
        muteMask=1<<curChannel;
      break;
     case '0':
      if (mode==editNotes){
        if(!altBank)
          bankA[beatnotes-2] ^= 1<<curChannel;      //toggle 11th note of current beat for current channel
        else
          bankB[beatnotes-2] ^= 1<<curChannel;}
      if (mode==editTempo)
        newbpm=(newbpm*10)+0;
        if (mode==editBeatLen)
        newblen=(newblen*10)+0;
      if(mode==home)
        muteMask=0xff;                            //unmute all channels
     
      break;
     case '+':
      if (mode==editTempo && BPM<maxTempo)
        {BPM+=1;
          Timer1.setPeriod(BPMtoDelay(BPM));
            }
      if (mode==editBeatLen && beatLen<maxBeatLen)
          beatLen+=1;
      if (mode==editNotes && beatnotes<192)
        beatnotes+=notesPerBeat;
      if(mode==home && screenBrightness<maxScreenBrightness)
        screenBrightness+=1;
      break;
     case '-':
      if (mode==editTempo && BPM>minTempo)
        {BPM-=1;
          Timer1.setPeriod(BPMtoDelay(BPM));
            }
     if (mode==editBeatLen && beatLen>1)
          beatLen-=1;
     if (mode==editNotes && beatnotes>12)
        beatnotes-=12;
     if(mode==home && screenBrightness>0)
        screenBrightness-=1;
      break;
     case '#':
      if (mode==editNotes){
        if(!altBank)
          bankA[beatnotes-1] ^= 1<<curChannel;      //toggle 12th note of current beat for current channel
        else
          bankB[beatnotes-1] ^= 1<<curChannel;}
      if (mode==editTempo)
        {if (newbpm>=minTempo && newbpm<=maxTempo)
          {BPM=newbpm;
          Serial.println(BPM);
          Timer1.setPeriod(BPMtoDelay(BPM));}
          newbpm=0;}
      if (mode==editBeatLen)
        {if(newblen>0 && newblen<=maxBeatLen)
          beatLen=newblen;
          newblen=0;}
      if(mode==home)                                //toggle mute current channel
        muteMask^=(1<<curChannel);
      break;
    }
        lc.setIntensity(0,screenBrightness);
        lc.setIntensity(1,screenBrightness);
        //blank bottom 3 rows on both displays
        lc.setRow(0,0,0);
        lc.setRow(0,1,0);
        lc.setRow(0,2,0);
        lc.setRow(1,0,0);
        lc.setRow(1,1,0);
        lc.setRow(1,2,0);
    if(mode==home)
      {displayHome();
      if(muteMask& (1<<curChannel))
        lc.setRow(0,1,0b01110000);
      else{
        lc.setRow(0,1,0b01010000);
        lc.setRow(0,0,0b01110000);}
        }
    if (mode==editBeatLen)
      { displayHome();
        lc.setRow(1,1,0b00111110);
        }
     if (mode==editTempo)
      {displayTempo();
        }
     if (mode==editNotes)
      {displayNotes();
        } 
  }
  //detect button press
  customKey = customKeypad.getKey();
}
