/*#include <FastIO.h>
//#include <LiquidCrystal_I2C_ByVac.h>
//#include <LiquidCrystal_SI2C.h>
//#include <LiquidCrystal_SR.h>
//#include <LiquidCrystal_SR1W.h>
//#include <LiquidCrystal_SR2W.h>
//#include <LiquidCrystal_SR3W.h>
//#include <SI2CIO.h>
//#include <SoftI2CMaster.h>
*/
#include <I2CIO.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Stepper.h>
#include <LedControl.h>


//pin 7 is connected to the DataIn(DIN)
//pin 6 is connected to LOAD(CS)
//pin 5 is connected to the CLOCK(CLK)
LedControl lmc=LedControl(7,5,6,1);

// Define LCD pinout
const int  en = 2, rw = 1, rs = 0, d4 = 4, d5 = 5, d6 = 6, d7 = 7, bl = 3;
 
// Define I2C Address - change if reqiuired
const int i2c_addr = 0x27;

//initialize I2C connected LCD (POSITIVE/NEGATIVE = backlight ON/OFF)
LiquidCrystal_I2C lcd(i2c_addr, en, rw, rs, d4, d5, d6, d7, bl, POSITIVE);

//save button pin and vars for use in its mode selection function
const int buttonPin=4;
int buttonState;            //for the current reading from input pin
int triggerState;           //duplicating because I don't want to ruin the first loop button mojo
int lastButtonState = HIGH; //the previous(default) reading from the input pin

//motor vars
const int stepsPerRevolution = 2048;  //with actual exact gearing ratio may be: 2038, or 4076 in half-step mode
const int rotationsPerMinute = 8;         // Adjustable range of 28BYJ-48 stepper is 0~17 rpm but over 10 seems to lead to more skipping...
// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);

//variables
bool blockSymbol = false;     //used to keep symbol from being displayed on led matrix, blocks input at this time as well
int chomp = 0;                //number of treats in current run dispensed
int chompd = chomp;           //something to keep track of chomp to monitor change
int missedCheck = chomp;      //something to use for counting missed rewarded click opportunities
int bar = 0;                  //counter for loading bar position
int clicks = 1;               //the currently selected stage (the number of button presses + 1)
int modeCount = 4;            //the number of active modes
bool loaded = false;          //a toggle to control program's switch from stage select loop to test run loop
//for run stage specific timing and display
int right = 1;                //value to associate with symbol that is rewarded
int wrong = 0;                //value to associate with symbol that isn't rewarded
unsigned long stageLoops = 0; //a simple counter of stageLoops (currently used to capture stageStart time only during first stage loop)
bool displayStartPhase = true;     //a toggle for timing symbol display
bool waitStartPhase  = true;      //a toggle for timing wait time periods
long baseWait = 5000;         //the default wait time between symbol displays
long baseDisplay = 3000;      //the base, max display time of a symbol without a button press
int wrongCount = 0;           //the number of times the button was clicked on the wrong symbol
int whiffCount = 0;           //the number of times the button was pressed with no symbol being shown at all 
int missCount = 0;            //the number of times the correct symbol was shown and timed out with no response
double rightTimes[15] = {};   //an array to store the reaction times of correct presess
double wrongTimes[30] = {};   //an array to store wrong presses, might need to expand/make a reasonable cutt off where enough wrongs finishes the stage anyway

//timing
unsigned long previousTime = 0; //previousTime storage for stage select button checking
unsigned long previousTime2 = 0; //and extra var for holding a time stamp in cases where the OG previousTime still needs to be compared
const long loadInterval = 100;  //the loading bar spacing across millis()
unsigned long lastDebounceTime=0;   //last time the output pin was toggled
unsigned long lastDebounceTime2 =0; //using a seperate var to hold click time for whiff clicks until I've double checked simplifying these two vars won't affect data collection/calculation
unsigned long debounceDelay = 50;   //the debounce time; increase if the output flickers
unsigned long motorActionDelay = 500; // pause for treat dispense (presumably munching as well)
unsigned long stageStart = 0;       //variable to store the exact time the run stage starts
unsigned long stageEnd = 0;         //variable to store time when final click is made
unsigned long totalTime = 0;        //variable to store total time to complete a stage
unsigned long toggleDelay = 1000;    //delay for animation on winScreen, longer now to also toggle total time with average response time in stage 2

//byte data for LCD customChars
//The heart symbol for the 'health bar' that depicts the number of treats left as hollow squares on the runscreen
byte customHeart[]={
  B00000,
  B01010,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};
//The only hiragana character to make the katakana on boot more readable
byte customNo[]={
  B00000,
  B00000,
  B01110,
  B10101,
  B10101,
  B11101,
  B00010,
  B00000
};

//LED Matrix Characters
byte lgCircleHole[8]={
  B00111100,
  B01100110,
  B11000011,
  B10000001,
  B10000001,
  B11000011,
  B01100110,
  B00111100
};
byte largeX[8]={
  B10000001,
  B01000010,
  B00100100,
  B00011000,
  B00011000,
  B00100100,
  B01000010,
  B10000001
  };
byte smiley[8]={
  B01111110,
  B10000001,
  B10100101,
  B10000001,
  B10100101,
  B10011001,
  B10000001,
  B01111110
  };
byte fontX[8]={
  B11100111,
  B01000010,
  B00100100,
  B00011000,
  B00100100,
  B00100100,
  B01000010,
  B11100111
  };
byte beer[8]={
  B01111000,
  B11111100,
  B11111110,
  B01000101,
  B01000101,
  B01000110,
  B01000100,
  B01111100
  };

//functions for LCD screens
void bootScreen() {
  //prints bootscreen
  byte bootTop[]={
    B01111110,
    B11111110,
    B11111110,
    B11001000,
    B10111010,
    B00000001,
    B01101111,
    B01111000,
    B10111001,
    B11011110,
    B10110000,
    B11010001,
    B00000001,
    B11111110,
    B11111110,
    B01111111
  };
  byte bootBot[]={
    B01111110,
    B11111110,
    B11000100,
    B11011010,
    B10110000,
    B11000110,
    B11011101,
    B10111000,
    B11011110,
    B11001111,
    B10111100,
    B10110000,
    B11011101,
    B00100001,
    B11111110,
    B01111111
  };
  lcd.setCursor(0,0);
  for(int col =0;col<16;col++) {
    lcd.write(bootTop[col]);
  }
  lcd.setCursor(0,1);
  for(int col =0;col<16;col++) {
    lcd.write(bootBot[col]);
  }
}
//specific, simplified stage one variant
void runScreen1(){
  //prints run screen
  byte runTop[]={
    B00000010,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011
  };
  //blank i guess..
  byte runBot[]={
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110
  };

  //remove treat sprites in health bar
  //not sure why the actively replaced sprite fades out sorta but it looks rad
  //I now pronounce this bug a feature
  if(chomp >= 1 && chomp <= 15){
    for(int i=(16-chomp);i<=15;i++){
      lcd.setCursor(i,0);
      lcd.write("-");
    }
  }
  //only print top row up until lost sprites
  lcd.setCursor(0,0);
  for(int col =0;col<(16-chomp);col++) {
    lcd.write(runTop[col]);
  }
  
  //clear bottom row
  if(chomp == 0){
      lcd.setCursor(0,1);
      for(int col =0;col<16;col++) {
      lcd.write(runBot[col]);
      }
  }
  
  //keep the single digit count from being repeatedly cleared
  if(chomp >=1 && chomp <10){
    lcd.setCursor(1,1);
    for(int col =1;col<16;col++) {
      lcd.write(runBot[col]);
    }
  }else{
    //keep the two digit count from repeatedly clearing
      lcd.setCursor(2,1);
      for(int col =2;col<16;col++) {
      lcd.write(runBot[col]);
      }
  }
  
  //checks if chomp has changed
  if(chompd != chomp){
    //level the two vars
    chompd = chomp;
    //print number dispensed if it has indeed changed
    lcd.setCursor(0,1);
    lcd.print(chomp);
  }
}

void loadScreen() {
  byte loadTop[]={
    B11111110,
    B11111110,
    B10111101,
    B11000011,
    B10110000,
    B10111100,
    B11011110,
    B00111010,
    B11111110,
    B00111100,
    B00111100,
    B00110001, //single digit at twelfth place
    B00111110,
    B00111110,
    B11111110,
    B11111110
  };
  byte loadBot[]={
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110
  };
  lcd.setCursor(0,0);
  for(int col=0;col<16;col++) {
    lcd.write(loadTop[col]);
  }
  //clear bottom row
  lcd.setCursor(0,1);
  for(int col=0;col<16;col++) {
    lcd.write(loadBot[col]);
  }
  //write in currently selected stage
  lcd.setCursor(11,0);
  lcd.print(clicks);
}
//right now only shows time completed (stage one specific)
void winScreen(int toggle) {
  //screen to print upon test completion
  lcd.setCursor(0,0);
  //simple two phase animation
  if(toggle == 0){
    lcd.print("!*!*CONGRATS*!*!");
  }else{
    lcd.print("$!$!CONGRATS!$!$");
  }
  lcd.setCursor(0,1);
  lcd.print("Time: ");
  lcd.print(totalTime);
  lcd.print("ms");
}

//function to check for button press during run, and count chomp/return updated last debounce time (time of click) too
unsigned long clickChecknChomp(){
  if(blockSymbol == false){
    if(digitalRead(buttonPin) == LOW && triggerState == 0){
      if((millis() - lastDebounceTime)>debounceDelay){
       
        triggerState = 1;
        chomp++;
        lastDebounceTime = millis();

        //save reaction time of valid press into array
        rightTimes[chomp - 1] = lastDebounceTime - previousTime; //previousTime reset in displayStartPhase
        
        lmc.clearDisplay(0);     //remove symbol 
        blockSymbol = true;      //prevent display until motorActionDelay satisfied

        //fire after clearing screen (only noticeable difference, delays while motor moves)
        myStepper.step(stepsPerRevolution/16); //clockwise
        
        return lastDebounceTime; //function spits out time of detected press (return gotta go last, duh)
      }
    }else{
      if(triggerState == 1 && digitalRead(buttonPin) == HIGH){
        triggerState = 0;
      }
    }
  }
}

//function to check for button press during times when no LED symbol is displayed
unsigned long whiffClickCheck(){
  if(blockSymbol == true){
    if(digitalRead(buttonPin) == LOW && triggerState == 0){
      if((millis() - lastDebounceTime2)>debounceDelay){
       
        triggerState = 1;
        whiffCount++;
        lastDebounceTime2 = millis();
      
        return lastDebounceTime2; //function spits out time of detected press (return gotta go last, duh)
      }
    }else{
      if(triggerState == 1 && digitalRead(buttonPin) == HIGH){
        triggerState = 0;
      }
    }
  }
}

//function for displaying 'O' on LED Matrix
void circleLm() {
  if(blockSymbol == false){
    for(int row=0;row<8;row++) {
      lmc.setRow(0,row,lgCircleHole[row]);
    } 
  } 
}

//function for displaying 'X' on LED Matrix
void xNormLm() {
  if(blockSymbol == false){
    for(int row=0;row<8;row++) {
      lmc.setRow(0,row,largeX[row]);
    } 
  } 
}

//function for toggling through LED Matrix candidates for all stages on boot
void symbolMenu(long symbolPause){
  for(int row =0;row<8;row++) {
    lmc.setRow(0,row,lgCircleHole[row]);
  }
  delay(symbolPause);
  for(int row =0;row<8;row++) {
    lmc.setRow(0,row,largeX[row]);
  }
  delay(symbolPause);
  for(int row =0;row<8;row++) {
    lmc.setRow(0,row,smiley[row]);
  }
  delay(symbolPause);
  for(int row =0;row<8;row++) {
    lmc.setRow(0,row,fontX[row]);
  }
  delay(symbolPause);
  for(int row =0;row<8;row++) {
    lmc.setRow(0,row,beer[row]);
  } 
  delay(symbolPause);
}

//another oddly specific function I use to simply toggle the click count back to one when all modes have been visited
int clickCycle(int x){
  if (clicks>x){
      clicks=1;
    }
  return clicks;
}

//I might be missing something about passing char variables/using strings in conditionals so I'll try not using strings
//instead the int right = 1; int wrong =0; as seen up above
//and it worked, for future reference my attempt was:
/*
 * double movingAverage(char rightORwrong){
 * double sum = 0;
 * if(rightORwrong == "right"){
 * blahblah} blah blah}
 * 
 * movingAverage("right");
 * 
 * and the sum was always 0.00 (so it wasn't triggering the conditional properly I guess)
 */


double movingAverage(int rightORwrong){
  //some conditionals so this average can find and return either moving average
  double sum = 0;
  if(rightORwrong == 1){
    //find sum of all currently stored reaction times (equal to chomp because it counts successes, caps at 15)
    for(int i = 0; i<chomp;i++){
      sum = sum + rightTimes[i];
    }
    //return average
    return sum/chomp;
  }

  if(rightORwrong == 0){
    for(int i = 0; i < wrongCount; i++){
      //find sum of all stored wrong reaction times, equal to wrongCount and potentially numerous
      sum = sum + wrongTimes[i];
    }
    return sum/wrongCount;
  }
}

//stage two run screen, should basically be the same for all following stages accept without a wrong count because there is only symbol being tested
void runScreen2(){
  //prints run screen
  byte runTop[]={
    B00000010,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011,
    B11011011
  };
  //blank i guess..
  byte runBot[]={
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110
  };

  //remove treat sprites in health bar
  //not sure why the actively replaced sprite fades out sorta but it looks rad
  //I now pronounce this bug a feature
  if(chomp >= 1 && chomp <= 15){
    for(int i=(16-chomp);i<=15;i++){
      lcd.setCursor(i,0);
      lcd.write("-");
    }
  }
  //only print top row up until lost sprites
  lcd.setCursor(0,0);
  for(int col =0;col<(16-chomp);col++) {
    lcd.write(runTop[col]);
  }
  
  //checks if chomp has changed
  if(chompd != chomp){
    //level the two vars
    chompd = chomp;
    //update moving average
    lcd.setCursor(0,1);
    lcd.print(movingAverage(right));
  }

  //print the M (miss) count label
  lcd.setCursor(7,1);
  lcd.print("M");
  lcd.print(missCount);
  //skip two spaces to leave two digit count space
  lcd.setCursor(10,1);
  //print the W(wrong) count label
  lcd.print("W");
  lcd.print(wrongCount);
  //skip two spaces to leave two digit count space
  lcd.setCursor(13,1);
  //print the "?" for whiff count label
  lcd.print("?");
  lcd.print(whiffCount);
  
//  //clear bottom row
//  if(chomp == 0){
//      lcd.setCursor(0,1);
//      for(int col =0;col<16;col++) {
//      lcd.write(runBot[col]);
//      }
//  }
//  
//  //keep the single digit count from being repeatedly cleared
//  if(chomp >=1 && chomp <10){
//    lcd.setCursor(1,1);
//    for(int col =1;col<16;col++) {
//      lcd.write(runBot[col]);
//    }
//  }else{
//    //keep the two digit count from repeatedly clearing
//      lcd.setCursor(2,1);
//      for(int col =2;col<16;col++) {
//      lcd.write(runBot[col]);
//      }
//  }
  

}


void setup() {
  //initialized for testing and debugging print outs (may also leave in prints to serial monitor for more fine grained data collection)
  Serial.begin(9600);

  //set the stepper speed 
  myStepper.setSpeed(rotationsPerMinute);
  
  // The MAX72XX is in power-saving mode on startup, so we have to do a wakeup call
  lmc.shutdown(0,false);
  //even at (0,1) I couldn't stare at it too long, figured a cat with oodles of extra eye rods would appreciate it if I toned it down even further
  lmc.setIntensity(0,0);
  //clear the display
  lmc.clearDisplay(0);

  //set button as an input
  pinMode(buttonPin, INPUT_PULLUP);

  //setup lcd 
  lcd.begin(16,2);
  //make customChars
  lcd.createChar(2,customHeart);
  lcd.createChar(1,customNo); //storing in 0 is some special case, was to lazy to figure it out so just bumped up what byte code they were linked to 

  //boot screen
  bootScreen();

  //hold here until any user input gets things started
  while(digitalRead(buttonPin)==HIGH){
    //run through symbols while waiting
    //symbolMenu(1500); 
  }

  loadScreen(); //print load screen at stage 1 (int clicks = 1 to start)
  delay(200);   //avoid tail end of button debounce
  previousTime=millis(); //set the reference time for loading bar animation timing
}


void loop() {
  //stage selection loop
  if(loaded == false){
    //read button state into local variable
    int reading = digitalRead(buttonPin);
    //if switch changed at all (noise or legit press)
    if(reading != lastButtonState){
      //reset the debouncing timer
      lastDebounceTime = millis();
    }

    if((millis()-lastDebounceTime)>debounceDelay){
      //if the button state has changed:
      if(reading != buttonState){
        buttonState = reading;
        //only execute mode changes if the new button state is LOW
        if(buttonState == LOW){
          clicks++;             //record the click
          clickCycle(modeCount);//reset click if stage options have been cycled
          loadScreen();         //print the right load screen to lcd
          bar=0;                //reset bar counter, begin loading/timer over again
        }
      }
    }

    lastButtonState = reading;
    
    //loading incrementation
    unsigned long currentTime = millis();
    if((currentTime-previousTime)>=loadInterval){
      //save last time bar segment was printed
      previousTime=currentTime;
      lcd.setCursor(bar,1);
      lcd.write(B11111111);
      bar++;
      if(bar>17){   //increased above 15 for visual bar completion and a little extra stall
        loaded = true;
      }  
    }
  }

  //stage select has been locked in and test begins
  if(loaded==true){
    
    //Stage 1: Only simple X is shown, constantly, simple X is rewarded
    //constantly lit "X" (symbol temporarily clears for motor operation until system is ready again)
    if(clicks == 1){
      runScreen1(); //update screen
      xNormLm(); //display x
      
      if(stageLoops == 0){
        stageStart = millis(); //save stage start time upfront
      }
      
      //check for click and respond to hits (within function)
      unsigned long hitTime = clickChecknChomp(); //also outputs lastDebounceTime (starts randomly, but after intial value represents the time of click)
      //after motorActionDelay, enable button checking/symbol display
      if((millis()-hitTime)>motorActionDelay){
        blockSymbol = false;
        xNormLm();
      }
      //completion condition and winscreen
      if(chomp>=15){
        stageEnd = millis();                //store time stage was completed
        totalTime = stageEnd - stageStart;  //calculate total time
        int toggle = 0;                     //random toggle var with crap placement
        unsigned long toggleTime = millis();//grabs updated time stamp for tracking toggleDelay

        //rotate stepper one final time to reset? (not necessary but might keep rotations more repeatable w/ engineering inaccuracies atm)
         myStepper.step(stepsPerRevolution/16);
         
        while(true){
          //twiddles the screen forever while displaying winScreen with animation and totalTime
          winScreen(toggle);
          if((millis()-toggleTime)>toggleDelay){
            toggle = !toggle;
            toggleTime = millis();
          }
        }
      }
      stageLoops++;   //ya
    }

    /*
     * Stage 2: Only simple X is shown (3 seconds), nothing is shown (5 seconds), simple X is rewarded
     */
    if(clicks ==2){
      
      if(stageLoops == 0){
        stageStart = millis(); //save stage start time upfront
       //clean out bottom row of lcd for runscreen 
        lcd.setCursor(0,1);
        lcd.print("                ");
      }
      
      runScreen2(); //update screen
      /*
       * might count miss clicks before first symbol is even displayed, 
       * but will have to make blocksymbol=true upfront to make it work and reset 
       * blocksymbol = false strategically farther down the conditional flow.
       * 
       * The cat will most likely be excitedly clicking prior to the run beginning which isn't necessarily
       * useful to include in the final whiff count if the LED is the initial visual cue and is considered the start of the run
       * Eventually, if possible, this data could be used to help train the cat 
       * to be more precise in both waiting for the formal beginning of the stage/clicking actively only with the LED display illuminated.
       */
      //whiffClickCheck();
      
      //the stage starts with a smaller delay without symbol, I use baseDisplay out of convenience
      if(millis()-stageStart >= baseDisplay){
        if(displayStartPhase == true){
                    
          previousTime = millis();  //take the time the symbol begins
          xNormLm();                //the symbol begins
          displayStartPhase = false;    //turn off for now (to avoid taking time and displaying every loop

          missedCheck = chomp; //record the chomp count at beginning of display time
        }

        if(displayStartPhase == false){
          
          //executing a check for successful clicks while within display time interval
          if(millis()-previousTime<= baseDisplay){
            
            //check for click and respond to hits (within function)
            unsigned long hitTime = clickChecknChomp(); //also outputs lastDebounceTime (starts randomly, but after intial value represents the time of click)
            //blocks input upon a click

            //check for whiff clicks during the remainder of the display time after a successful click and before the full wait time begins
            whiffClickCheck();
            
            //no need for motorActionDelay, as the default delay between symbols is plenty of time 
          }
          
          if(millis()-previousTime>baseDisplay){

            //we have passed the display interval
            if(waitStartPhase == true){

              //take a time stamp for beginning of wait time
              previousTime2 = millis();
              lmc.clearDisplay(0);
              blockSymbol = true;     //block symbol and inputs

              //after display time has elapsed, count missed if no successful click
              if(missedCheck == chomp){
                missCount++; //put it in this conditional so it only counts once after a baseDisplay time out
              }
              
              //block repeating the above until next wait
              waitStartPhase = false;
            }

            //counting miss clicks while waiting
            if(millis()-previousTime2 < baseWait){
              whiffClickCheck();
            }
            
            //upon the wait time being satisfied
            if(millis()-previousTime2 >= baseWait){
              
              //allow display phase to start
              displayStartPhase = true;
              //allow display and input
              blockSymbol = false;
              //prime the waitStartPhase for eventual lapse in symbol display time
              waitStartPhase = true;
            }
          }
          
        }

        
      }
      
      //completion condition and winscreen
      if(chomp>=15){
        stageEnd = millis();                //store time stage was completed
        totalTime = stageEnd - stageStart;  //calculate total time

        runScreen2(); //last update of runscreen to include last response time and stuff
        
        int toggle = 0;                     //random toggle var with crap placement
        unsigned long toggleTime = millis();//grabs updated time stamp for tracking toggleDelay

        //rotate stepper one final time to reset? (not necessary but might keep rotations more repeatable w/ engineering inaccuracies atm)
        myStepper.step(stepsPerRevolution/16);
       
        delay(5000);//A DELAY!!! ya, total time already calculate so we can leave some time to catch the runScreen data

        lcd.setCursor(0,1);
        lcd.print("                "); //clear bottom row for totalTime print
        while(true){
          //twiddles the screen forever while displaying winScreen with animation and totalTime
          winScreen(toggle);
          if((millis()-toggleTime)>toggleDelay){
            toggle = !toggle;
            toggleTime = millis();
          }
        }
      }
      
      stageLoops++;   //ya
    
    }

    /*
     * Stage 3: at random, simple X or large O is shown (3 seconds), nothing is shown (5 seconds), simple x is rewarded
     */
    if(clicks ==3){
    lcd.setCursor(0,0);
    lcd.print("THIS RUNSCREEN 3");
    }

    /*
     * Stage 4: Stage 3 but the wait time is now a random variable that is 
     * normally distributed (two parameters, maybe keep mean = 5 seconds w/ a narrow standard deviation).
     * Will need to definitley play around with timing as the specific cat's learning rate/style is observed. 
     */
    if(clicks ==4){
    lcd.setCursor(0,0);
    lcd.print("THIS RUNSCREEN 4");
    }

    /*
     * Ideas for stages beyond the above:
     * -All at once, or one at a time, transition the 'x' and 'o' to the slightly more complex AND more similar 'smiley' and 'fontX'
     * -Test adaptive learning by switching out the complex x or complex o with a novel symbol that differs from both like 'beer' (square, filled and hollow, off center)
     * -Test more fine grained descrimination: simple x vs. font x, or large O vs. smiley
     * -Test response to reverting on symbol to a simpler/past symbol that was also compared against the current antagonist
     * -Test switching/blocking: flip what symbol is rewarded completely (x like or O like)
     *  this seems like it could be an insightful measure of intelligent identification/discrimination,
     *  but there is a tension between rewarding an 'x-like' or 'o-like' symbol for a long time that a solid
     *  association can be used as a solid foundation for more complex tasks, and switching the reward early during 
     *  simpler tasks when it may be easier to transition completely, if possible at all. Likely this can't be determined
     *  until one sees how fast a cat clears levels and their apptitude for distinguishing symbols. Several months of rewarding 'X' 
     *  for example would seem to make the transition to rewarding 'O' extremely difficult, but the cats behavior may have been
     *  reliably shaped that it will reliably select 'x' in much more complex arrangements (e.g. the holy grail: tic tac toe) 
     * -Test discrimination between more than two symbols
     * -Reward selection of a specific sequence of numbers (so you can pretend your cat can do sums for guests, or something of the like)
     * -Test reaction time: (slowly shorten the display time/when a click can happen, maybe increase treat pay out if personal records are smashed)
     *  This would be particularly interesting if paired with symbol discrimination. 
     * -AND PROBABLY A LOT MORE (e.g. you get an RGB LED matrix and can train color recognition, keeping in mind cats have much fewer cones and see in blue-violet/yellow-green ranges)
     */
    
  }
}
