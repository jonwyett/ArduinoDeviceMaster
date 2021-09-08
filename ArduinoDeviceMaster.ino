/******************************************************************************
  Arduino Device Master
  Author: Jonathan Wyett
  Version: 1.1.2
  Date: 2021-09-08
/*****************************************************************************/


/*****************************************************************************/
/****** DEVICE CONFIG SECTION     ********************************************/
#define admDebug Serial //uncomment to see DeviceMaster debug info
//how many of each type of component, comment out for 0
#define TOTAL_LEDS 10 
#define TOTAL_BUTTONS 5
#define TOTAL_POTS 3
#define TOTAL_ROTARY_ENCODERS 1
//how many intervals do you need at once, comment out for 0
#define TOTAL_INTERVALS 5

/****** END DEVICE CONFIG SECTION     ****************************************/
/*****************************************************************************/

#define DEBOUNCE_DELAY 50 //for button debounce

//for setting the button modes
#define BUTTON_PRESS_HIGH 0 //default
#define BUTTON_PRESS_LOW 1
#define BUTTON_INPUT_PULLUP 3

/*****************************************************************************/

//Callbacks
typedef void (*basicCallback)();
typedef void (*oneParamCallback)(int);
  
#ifdef TOTAL_INTERVALS
  class Interval {
    public:
      //adds an interval to the local heap, returns false if no space
      bool addInterval(oneParamCallback 
                      callback,
                      int wait,
                      int count,
                      byte msg,
                      byte intervalName) {

        byte slot = findSlot();
        
        if (slot<255) {
            #ifdef admDebug
              admDebug.print("New interval on slot ");
              admDebug.print(slot);
              admDebug.print(" with msg=");
              admDebug.println(msg);
            #endif
            callbacks[slot] = callback;
            waits[slot] = wait;
            counts[slot] = count;
            msgs[slot] = msg;
            names[slot] = intervalName;
            return true;
        } else {
          return false;
          #ifdef admDebug
            admDebug.println("ERROR: No interval slot available.");
          #endif
        }
      }

      void clearInterval(byte intervalName) {
        for (byte i=0; i<TOTAL_INTERVALS; i++) {
          if (names[i]==intervalName) {
            resetInterval(i);
          }
        }
      }
      
      //CONSTRUCTOR
      Interval() {
        resetAll();
      }
  
      void update() {
        for (byte i=0; i<TOTAL_INTERVALS; i++) {
          //test if the interval is running
          //only process active intervals (-1 is inactive)
          if (counts[i]>=0) {
            //check if the time since the last run is >= the the wait
            if ((millis()-lastRuns[i])>=waits[i]) {
               callbacks[i](msgs[i]); //run the callback function
               lastRuns[i] = millis(); //update the last run
               //for intervals that only run a certain # of times (0 is infinit)
               if (counts[i]>0) { 
                counts[i]--; //decrement the count
                //now check if the count has reached 0
                if (counts[i]==0) {
                  resetInterval(i); //clear the interval
                }
               }
            }
          }
        }
      }
  
      #ifdef admDebug
        void printStatus() {
          admDebug.println("INTERVAL DEBUG:");
          for (byte i=0; i<TOTAL_INTERVALS; i++) {
            admDebug.print("(");
            admDebug.print(i);
            admDebug.print(") Count:" );
            admDebug.print(counts[i]);
            admDebug.print(" Wait:");
            admDebug.print(waits[i]);
            admDebug.print(" Msg:");
            admDebug.print(msgs[i]);
            admDebug.print(" Name:");
            admDebug.println(names[i]);
          }
          admDebug.println("-------------------------");
        }
      #endif
      
    private:
      oneParamCallback callbacks[TOTAL_INTERVALS]; //the callback functions
      int counts[TOTAL_INTERVALS]; //the count of each interval, -1 is no interval, 0 is infinit
      unsigned int waits[TOTAL_INTERVALS]; //how long to wait for each interval in ms
      unsigned long lastRuns[TOTAL_INTERVALS]; //the last time that interval was run
      byte msgs[TOTAL_INTERVALS]; //you can supply a value to be passed to the callback
      byte names[TOTAL_INTERVALS]; //use #define MY_INTERVAL_NAME 12 (example) to name intervals in your code

      //resets all intervals
      void resetAll() {
        for (byte i=0; i<TOTAL_INTERVALS; i++) {
          resetInterval(i);
        }
      }

      //resets individual interval
      void resetInterval(byte i) {
        counts[i] = -1;
        waits[i] = 0;
        lastRuns[i] = 0;
        msgs[i] = 0;
        names[i] = 0;
      }

      //find an empty interval slot
      byte findSlot() {
        for (byte i=0; i<TOTAL_INTERVALS; i++) {
          if (counts[i]==-1) {
            return i;
          }
        }
        //no empty slot found, return 255
        return 255;
      }

      //find an interval by name(byte)
      byte findByName(byte intervalName) {
        for (byte i=0; i<TOTAL_INTERVALS; i++) {
          if (names[i]=intervalName) {
            return i;
          }
        }
        return 0; //no name found
        #ifdef admDebug
          admDebug.println("ERROR: No interval found with that name");
        #endif
      }      
  };
#endif

#ifdef TOTAL_POTS
  class Pot {
    public:
      int state = 0;
      int oldState = 0;
      byte pin;
      const char *name;
      unsigned int max = 1024; //the max value of the pot (native)
      int min = 0; //the min value of the pot (native)
      int range = 0; //the scaler, so range=100 means 0-100, range=255 means 0-255, etc.
      byte delta = 1; //the required delta before a change is fired
      byte avg = 10; //the number of readings to average
      
      void init(const char *newName, byte newPin) {
        name = newName;
        pin = newPin;
  
        #ifdef admDebug
          admDebug.print("Setup new POT: '");
          admDebug.print(name);
          admDebug.print("' on pin ");
          admDebug.println(pin);
        #endif
      }
  
      void onChange(oneParamCallback callback) {
        hasChangeFunc = true;
        changed = callback;
      }
  
      void update() {
        if (range>0) { 
          average((analogRead(pin)/(max/range)));
        } else {
          average(analogRead(pin));
        }

        if (count==0) { //the state is only checked if the avg count is reset
          //clamp state high/low
          if (state<delta) {
            state = 0;
          } else if (range>0 && state>range-delta) {
            state=range;
          } else if (range>0 && state>range) {
            state = range; 
          } else if (state>max-delta) {
            state = max;
          }
          if (state>=oldState+delta || state<=oldState-delta) {
            oldState = state;
            
            #ifdef admDebug
              admDebug.print("Pot: ");
              admDebug.print(name);
              admDebug.print(" value changed to ");
              admDebug.println(state);
            #endif
            if (hasChangeFunc) {
              changed(state);
            }
          }
          //reset the state because we need to reset the average
          state = 0; 
        }  
      }
  
      private:
        bool hasChangeFunc = false;
        oneParamCallback changed;
        byte count = 0; //for the avg function

        void average(int newVal) {
          state += newVal;
          count++;
          if (count == avg) {
            state /= avg;
            count = 0;
          } 
        }
  };
#endif

#if defined TOTAL_BUTTONS || defined TOTAL_ROTARY_ENCODERS
  class Button {
    public:
      byte state = LOW;
      byte oldState = LOW;
      byte pin;
      byte pressMode = BUTTON_PRESS_HIGH; 
      const char *name;
      
      void init(const char *newName, byte newPin, byte newMode) {
        name = newName;
        pin = newPin;
        pressMode = newMode;
        if (pressMode==BUTTON_INPUT_PULLUP) {
          pinMode(pin, INPUT_PULLUP);  
        } else {
          pinMode(pin, INPUT);
        }
        
        #ifdef admDebug
          admDebug.print("Setup new button: '");
          admDebug.print(name);
          admDebug.print("' on pin ");
          admDebug.println(pin);
        #endif
      }
  
      void onPress(basicCallback callback) {
        hasPressFunc = true;
        pressed = callback;
      }
  
      void onRelease(basicCallback callback) {
        hasReleaseFunc = true;
        released = callback;
      }
      
      void update() {
        checkForPress();  
      }
  
      protected:
        bool hasPressFunc = false;
        bool hasReleaseFunc = false;
        basicCallback pressed;
        basicCallback released;  
        unsigned long lastDebounceTime; 

        void checkForPress() {
          //get state
          state = digitalRead(pin);
          if (state!=oldState) {
            if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
              oldState = state;
              lastDebounceTime = millis();
              
              #ifdef admDebug
                admDebug.print("Button: ");
                admDebug.print(name);
                admDebug.print(" state changed to ");
                admDebug.println(state);
              #endif
              bool isPressed = false;
              if (pressMode==BUTTON_PRESS_HIGH) {
                isPressed = (state==HIGH) ? true : false;
              } else {
                isPressed = (state==LOW) ? true : false;
              }    
              if(isPressed==true && hasPressFunc) {
                pressed();    
              } else if (isPressed==false && hasReleaseFunc) {
                released();
              }
            } else {
              #ifdef admDebug
                admDebug.print("BUTTON DEBOUNCE PROTECTION ON ");
                admDebug.println(name);
              #endif
            }
          }   
        }
  };
#endif

#ifdef TOTAL_ROTARY_ENCODERS
  class RotaryEncoder: public Button {
    public:
      byte DT, CLK;

      void init(const char *newName, byte swPin, byte dtPin, byte clkPin) {
        name = newName;
        pin = swPin; 
        DT = dtPin;
        CLK = clkPin;
        pinMode(pin, INPUT);
        pinMode(DT, INPUT);
        pinMode(CLK, INPUT);

        lastStateCLK = digitalRead(CLK);
        
        #ifdef admDebug
          admDebug.print("Setup new Rotary Encoder: '");
          admDebug.print(name);
          admDebug.print("' SW:");
          admDebug.print(pin);
          admDebug.print("' DT:");
          admDebug.print(DT);
          admDebug.print("' CLK:");
          admDebug.println(CLK);
        #endif
      }

      void onClockwise(basicCallback callback) {
        hasCWFunc = true;
        CW = callback;
      }

      void onCounterClockwise(basicCallback callback) {
        hasCCWFunc = true;
        CCW = callback;
      }
      
      void update() {
        checkForPress();
        
        currentStateCLK = digitalRead(CLK);
        
        // If last and current state of CLK are different, then pulse occurred
        // React to only 1 state change to avoid double count
        if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){
      
          // If the DT state is different than the CLK state then
          // the encoder is rotating CCW
          if (digitalRead(DT) != currentStateCLK) {
            if (hasCCWFunc) {
              CCW();
            }
            #ifdef admDebug
              admDebug.print("Encoder '");
              admDebug.print(name);
              admDebug.println("' CCW");
            #endif
          } else { // Encoder is rotating CW
            if (hasCWFunc) {
              CW();
            }
            #ifdef admDebug
              admDebug.print("Encoder '");
              admDebug.print(name);
              admDebug.println("' CW");
            #endif
          }
        }
      
        // Remember last CLK state
        lastStateCLK = currentStateCLK;
      }

      private:
        byte currentStateCLK;
        byte lastStateCLK;
        bool hasCWFunc = false;
        bool hasCCWFunc = false;
        basicCallback CW;
        basicCallback CCW;  
  };
#endif

#ifdef TOTAL_LEDS
  class LED {
    public:
      byte state = LOW;
      byte pin, pinG, pinB;
      byte R, G, B;
      bool isRGB = false;
      bool isDimmable = false;
      bool commonAnode = true;
      byte level = 0;
      const char *name;

      //Regular LEDs
      void init(const char *newName, byte newPin) {
        name = newName;
        pin = newPin;
        pinMode(pin, OUTPUT);
        setState(LOW);
        #ifdef admDebug
          admDebug.print("Setup new LED: '");
          admDebug.print(name);
          admDebug.print("' on pin ");
          admDebug.println(pin);
        #endif 
      }

      //RGB LEDs
      void init(const char *newName, byte newPinR, byte newPinG, byte newPinB) {
        name = newName;
        pin = newPinR;
        pinMode(pin, OUTPUT);
        pinG = newPinG;
        pinMode(pinG, OUTPUT);
        pinB = newPinB;
        pinMode(pinB, OUTPUT);
        isRGB = true;
        setState(LOW);
        #ifdef admDebug
          admDebug.print("Setup new RGB LED: '");
          admDebug.print(name);
          admDebug.print("' on pins ");
          admDebug.print(pin);
          admDebug.print(",");
          admDebug.print(pinG);
          admDebug.print(",");
          admDebug.println(pinB);
        #endif 
      }
      
      void turnOn() {
        setState(HIGH);
      }
      
      void turnOff() {
        clearBlink();
        setState(LOW);
      }
      
      void setLevel(byte newLevel) {
        level = newLevel;
        //setState() also writes the state to the LED
        //se we want to set the new level if the state is already high        
        if (state==HIGH) {
          setState(HIGH); 
        }
        #ifdef admDebug
          admDebug.print("New LED level: ");
          admDebug.println(level);
        #endif
      }
      
      void setColor(byte newR, byte newG, byte newB) {
        R = newR;
        G = newG;
        B = newB;
        if (commonAnode) {
          R = 255-R;
          G = 255-G;
          B = 255-B;
        }
        #ifdef admDebug
          admDebug.print("New RGB Color: ");
          admDebug.print(R);
          admDebug.print(",");
          admDebug.print(G);
          admDebug.print(",");
          admDebug.println(B);
        #endif
        //setState() also writes the state to the LED
        //se we want to set the new color if the state is already high        
        if (state==HIGH) {
          setState(HIGH); 
        }
      }
      
      void flip() {
        if(state==HIGH) {
          setState(LOW);
        } else {
          setState(HIGH);
        }
      }  

      void blink(int delay) {
        initBlink(delay, 0);
      }
      
      void blink(int delay, int count) {
        initBlink(delay, count);  
      }
  
      void update() {
        //run the blink routine
        runBlink();
      }
  
      private:
        int blinkCount = 0;
        int blinkMax = 0;
        unsigned long lastBlinkTime = 0;
        int blinkDelay = 0;
        bool blinking = false;

        void setState(byte newState) {
          state = newState;
          if (isRGB) {
            if (state==LOW) {
              byte val = 0;
              if (commonAnode) {
                val = 255;
              }
              analogWrite(pin, val);
              analogWrite(pinG, val);
              analogWrite(pinB, val);
            } else {
              analogWrite(pin, R);
              analogWrite(pinG, G);
              analogWrite(pinB, B);  
              #ifdef admDebug
                admDebug.print("New RGB: ");
                admDebug.print(R);
                admDebug.print(",");
                admDebug.print(G);
                admDebug.print(",");
                admDebug.println(B);
              #endif
            }
          } else if (isDimmable) {
            analogWrite(pin, (state==HIGH) ? level : LOW); 
            #ifdef admDebug
              admDebug.print("LED set to: ");
              admDebug.println(level);
            #endif
          } else {
            digitalWrite(pin, state);
          } 
        }
        void initBlink(int delay, int count) {
          blinkCount = 0;
          blinkMax = (count *2)+1; //since each blink is 2 flips  
          lastBlinkTime = 0;
          blinking = true;
          blinkDelay = delay/2; //since we need to flip
          setState(HIGH); //blink should illuminate immediately  
         }

        void clearBlink() {
          blinkCount = 0;
          blinkMax = 0;
          lastBlinkTime = 0;
          blinkDelay = 0;
          blinking = false;  
        }
         
        void runBlink() {
          if (blinking) {
            //Serial.print(millis()-lastBlinkTime);
            //Serial.print("/");
            //Serial.println(blinkDelay);
            if (millis()-lastBlinkTime>=blinkDelay) {
              lastBlinkTime = millis();
              blinkCount++;
              flip();
              if (blinkCount>=blinkMax && blinkMax>0) {
                turnOff();
                blinking = false;
              }
            }
          }  
        }
  };
#endif

class Device {
  public:
    #ifdef TOTAL_BUTTONS
      //Buttons
      Button buttons[TOTAL_BUTTONS];
      //reference a button based on its name
      Button &button(const char *searchName) {
        for (byte i=0; i<totalSetupButtons; i++) {
          if (buttons[i].name == searchName) { 
            return buttons[i];
          } 
        }  
      }

      //MAYBE TODO: delegate overloaded constructors to private init()
      void addButton(const char *newName, byte newPin) {
        if (totalSetupButtons<TOTAL_BUTTONS) {
          buttons[totalSetupButtons].init(newName, newPin, BUTTON_PRESS_HIGH);
          totalSetupButtons++;
        } else {
          #ifdef admDebug
            admDebug.println("ERROR: Too many buttons added.");
          #endif
        }  
      }
      
      void addButton(const char *newName, byte newPin, byte newMode) {
        if (totalSetupButtons<TOTAL_BUTTONS) {
          buttons[totalSetupButtons].init(newName, newPin, newMode);
          totalSetupButtons++;
        } else {
          #ifdef admDebug
            admDebug.println("ERROR: Too many buttons added.");
          #endif
        }
      }
    #endif

    #ifdef TOTAL_ROTARY_ENCODERS
      //Rotary Encoders
      RotaryEncoder rotaryEncoders[TOTAL_ROTARY_ENCODERS];
      //reference a rotary encoder based on its name
      RotaryEncoder &rotaryEncoder(const char *searchName) {
        for (byte i=0; i<totalSetupRotaryEncoders; i++) {
          if (rotaryEncoders[i].name == searchName) { 
            return rotaryEncoders[i];
          } 
        }  
      }
  
      void addRotaryEncoder(const char *newName, byte SW, byte DT, byte CLK) {
        if (totalSetupRotaryEncoders<TOTAL_ROTARY_ENCODERS) {
          rotaryEncoders[totalSetupRotaryEncoders].init(newName, SW, DT, CLK);
          totalSetupRotaryEncoders++;
        } else {
          #ifdef admDebug
            admDebug.println("ERROR: Too many Rotary Encoders added.");
          #endif
        }
      }
    #endif

    #ifdef TOTAL_LEDS
      //LEDs
      LED LEDs[TOTAL_LEDS];
      //reference a LED based on its name
      LED &led(const char *searchName) {
        for (byte i=0; i<totalSetupLEDs; i++) {
          if (LEDs[i].name == searchName) {
            return LEDs[i];
          } 
        }
      }
  
      void addLED(const char *newName, byte newPin) {
        if (totalSetupLEDs<TOTAL_LEDS) {
          LEDs[totalSetupLEDs].init(newName, newPin);
          totalSetupLEDs++;
        } else {
          #ifdef admDebug
            admDebug.println("ERROR: Too many LEDs added.");
          #endif
        }
      }

      void addLED(const char *newName, byte newPinR, byte newPinG, byte newPinB) {
        LEDs[totalSetupLEDs].init(newName, newPinR, newPinG, newPinB);
        totalSetupLEDs++;
      }
    #endif

    #ifdef TOTAL_POTS
      //Pots
      Pot pots[TOTAL_POTS];
      //reference a pot based on its name
      Pot &pot(const char *searchName) {
        for (byte i=0; i<totalSetupPots; i++) {
          if (pots[i].name == searchName) {
            return pots[i];
          } 
        }  
      }
  
      void addPot(const char *newName, byte newPin) {
        if (totalSetupPots<TOTAL_POTS) {
          pots[totalSetupPots].init(newName, newPin);
          totalSetupPots++;
        } else {
          #ifdef admDebug
            admDebug.println("ERROR: Too many pots added.");
          #endif
        }
      }
    #endif
    
    #ifdef TOTAL_INTERVALS
      //Intervals
      Interval intervals;

      void addInterval(oneParamCallback callback, int wait) {
        intervals.addInterval(callback, wait, 0, 0, 0);  
      }
      void addInterval(oneParamCallback callback, int wait, int repeat) {
        intervals.addInterval(callback, wait, repeat, 0, 0);  
      }

      void addInterval(oneParamCallback callback, int wait, int repeat, byte msg) {
        intervals.addInterval(callback, wait, repeat, msg, 0);  
      }
      
      void addInterval(oneParamCallback callback, int wait, int repeat, byte msg, byte intervalName) {
        intervals.addInterval(callback, wait, repeat, msg, intervalName);
      }

      void clearInterval(byte intervalName) {
        intervals.clearInterval(intervalName);
      }
    #endif
      
    void update() {
      #ifdef TOTAL_LEDS
        //LEDs
        for (byte i=0; i<totalSetupLEDs; i++) {
          LEDs[i].update();
        }
      #endif
      
      #ifdef TOTAL_BUTTONS
        //BUTTONS
        for (byte i=0; i<totalSetupButtons; i++) {
          buttons[i].update();
        }
      #endif

      #ifdef TOTAL_ROTARY_ENCODERS
        //Rotary Encoders
        for (byte i=0; i<totalSetupRotaryEncoders; i++) {
          rotaryEncoders[i].update();
        }
      #endif
      
      #ifdef TOTAL_POTS
        //Pots
        for (byte i=0; i<totalSetupPots; i++) {
          pots[i].update();
        }
      #endif
   
      #ifdef TOTAL_INTERVALS
        //Intervals
        intervals.update();
      #endif
    }
    
  private:
    //To track total components added
    #ifdef TOTAL_LEDS
      byte totalSetupLEDs = 0;
    #endif
    
    #ifdef TOTAL_BUTTONS
      byte totalSetupButtons = 0;
    #endif

    #ifdef TOTAL_ROTARY_ENCODERS
      byte totalSetupRotaryEncoders = 0;
    #endif
    
    #ifdef TOTAL_POTS
      byte totalSetupPots = 0;
    #endif
};

//DEVICE DECLARATION
Device device; 

/*****************************************************************************/
/******      USER PROGRAM SECTION   ******************************************/
/*****************************************************************************/
/*****************************************************************************/


//put any user code, callbacks, etc here. This is your program.

/*****************************************************************************/

void setup() {
  // init  serial
  Serial.begin(9600);
}

void loop() {  
  device.update();
}
