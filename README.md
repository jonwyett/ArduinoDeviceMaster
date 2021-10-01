# ArduinoDeviceMaster
A simple framework for controlling buttons, LEDs, pots, etc. from an Arduino. 

The framework extracts away all of the details of handling simple electrical components so you can focus on functionality. It currently supports:

1. Buttons
2. LEDs (including RGB and dimming support)
3. Potentiometers
4. Rotary Encoders
5. An interval function that is similar to Javascript's setInterval()

## Installation ##

1. Download or clone the ArduinoDeviceMaster.ino file from the repo.
2. Open the ArduinoDeviceMaster.ino file with the Arduino IDE.
3. Save-As a new file for your project.
4. Edit the "DEVICE CONFIG SECTION" with the correct number of each type of electrical component. (Seriously, this is soooooo important. I wrote this thing and most of the times when my program isn't working it's because I didn't included enough of each type of component...)
5. Add and configure your components in the void setup() function.
5. Create your program in the "USER PROGRAM SECTION" section.
6. (optional) uncomment the "#define admDebug Serial" line so that you can see any errors and verify that your program is working with the framework properly.

## Hello World Example ##  

Your program goes before the default setup function and your device setup goes in the setup function:

```c++
    void helloWorld() {
        Serial.println("Hello World");
    }

    void setup() {
        Serial.begin(9600);

        device.addButton("btn1", 3); //name, pin
        //pass the helloWorld() function by reference
        device.button("btn1").onPress(helloWorld);
    }
```

## Simple Example ##

This will light an LED only when a button is pressed

```c++
    void ledOn() {
        device.led("red").turnOn();
    }

    void ledOff() {
        device.led("red").turnOff();
    }

    void setup() {
        Serial.begin(9600);

        device.addButton("btn1", 3); 
        device.button("btn1").onPress(ledOn);
        device.button("btn1").onRelease(ledOff);

        device.addLED("red", 5);
    }
```
---
For efficiency the framework uses preprocessor directives to control how many of each type of component your device has. It's required that you set this:

```c++
//how many of each type of component, comment out for 0
#define TOTAL_LEDS 5 
#define TOTAL_BUTTONS 4
#define TOTAL_POTS 3
#define TOTAL_ROTARY_ENCODERS 2

//how many intervals do you need at once, comment out for 0
#define TOTAL_INTERVALS 1
```
By default the framework has several of each type of component, probably more than you need for most projects, but the most common mistake is to not have enough of each component specified so it made sense to have extra. This wastes space though, so lower the numbers if needed.  

If you comment out a device type completely the framework wont even include the code needed to support it, saving a few extra bytes. (Does the Arduino complier optimize enough to not include class code if the class is never initialized? If so you can just set it to 0 I guess... either way....)

You can also enable debug output by un-commenting this line:

```c++
//#define admDebug Serial
```
(This will also increase program size significantly). 

---
## How to use the framework ##

After setting the number of each type of component in the preprocessor section, the next steps are to setup the device itself in the default void setup() section and to create your actual program.  
Add a component using the add[COMPONENT_TYPE] function of "device". All the add functions require a name and a pin number (or multiple pins for RGBs or Rotary Encoders).  

EX:

```c++
void setup() {
    device.addButton("My Button", 2);
    device.addPot("My Pot", 4);
    device.addLED("internal", 13);
}
```

Your program will consist of various functions that you attach to events associated with the appropriate component. (all in the void setup() routine). Once a component has been added you reference it's properties by supplying it's name to the appropriate device lookup function, such as "device.button(name)" or "device.led(name)".

This program has two buttons on pins 2 and 3. When you press the buttons a counter is incremented or decremented and printed

EX:

```c++
//my program
int num=0;

void printIt() {
    Serial.println(num);
}

void numUp() {
    num++;
    printIt();
}

void numDown() {
    num--;
    printIt();
}

void setup() {
    Serial.begin(9600); 

    //add the buttons
    device.addButton("btnUp", 2);
    device.addButton("btnDown", 3);
    
    //associate the functions you created above with the buttons
    //use the names you specified above to indicate which one.
    device.button("btnUp").onPress(numUp);
    device.button("btnDown").onPress(numDown);
}
```

Most of the components have at least one property that can be changed after you create it. For example Potentiometers have a "range" capability so that they return values from 0-num instead of their native value (useful for 0-100 or 0-255, etc.)

EX:

```c++
    void setup() {
        device.addPot("pot1", 6);
        device.pot("pot1").range = 100;
    }
```

---

## The "device" Class ##
All interactions with the framework use the "device" class. It's very simple to use.

__Adding Components__
All components are added using the corresponding _add_ function. The all require a name and one or more pin numbers as appropriate.
* addButton(name, PIN) - add a button
* addPot(name, PIN) - add a potentiometer
* addRotaryEncoder(name, PIN_SW, PIN_DT, PIN CLK) - add a rotary encoder
* addLED(name, PIN) - add a regular LED
* addLED(name, PIN_R, PIN_G, PIN_B) - add an RGB LED
* addInterval(callback, delay) - add a simple, infinite interval
* addInterval(callback, delay, repeat) - add an interval that only fires a certain number of times
* addInterval(callback, delay, repeat, message) - add an interval that provides a message to the callback
* addInterval(callback, delay, repeat, message, name) - add an interval with a repeat, message and name (names are needed if you want to clear/stop the interval later)

__Configuring Components__
Once a component is added you can set it up or modify it by using the appropriate lookup function.
* device.button(name)
* device.led(name)
* device.pot(name)
* device.rotaryEncoder(name)
So to modify an LED type you would use:
```c++
    device.led("red").isDimmable = true;
    device.led("red").turnOn();
```

__Intervals__
In addition to the various addInterval() functions, Intervals have a unique function called device.clearInterval(name) that clears the associated interval.

---
# Component Types #  

## Button ##  
__Create with:__
```c++
 device.addButton("name", PIN);
 //or
 device.addButton("name", PIN, pressMode);
```
_pressMode_ defines if the button is HIGH or LOW when pressed and/or whether the internal pullup resistor should be enabled.
Values: 
* BUTTON_PRESS_HIGH 
* BUTTON_PRESS_LOW 
* BUTTON_INPUT_PULLUP (default)

EX:
```c++
    //both buttons will be "pressed" when LOW
    device.button("btn1", 4); 
    device.button("btn2", 3, BUTTON_INPUT_PULLUP);
```

__Events__
* onPress()
* onRelease()  

---

## Potentiometer ##  
__Create with:__
```c++
 device.addPot("name", PIN);
```

__Events__
* onChange(value)

__Properties__ 
* min
* max
* range
* delta
* avg

__min, max and range__ are all used together to convert the native value of the pot to a user specified range. By default min=0 and max=1024. If your pot is running on 3.3v and only returns values up to 715 for example, than set max to 715 or the range calculation will be wrong. You can also use min/max if you want a dead-zone as well (but only with range).

Ex:
```c++
    void setup() {
        device.addPot("myPot", 3);
        device.pot("myPot").range = 255;
        device.pot("myPot").max = 715;
    }
```
This will cause a 3.3v pot to return values from 0-255

__delta__ is the minimum change that will fire an onChange event, so if you set it to __3__ then the pot will have to increase/decrease by 3 units to fire a change. You only need to use this if your pot is "floating" too much or if you want the pot to return a bracket of values.

EX:
```c++
    void setup() {
        device.addPot("myPot", 3);
        device.pot("myPot").range = 100;
        device.pot("myPot").delta = 25;
    }
```
This will create a pot that only fires 0,25,50,75,100

__avg__ is used to average out the value of a pot to prevent floating. It defines how many readings should be averaged to determine the value. Default is __10__. There is a 1 millisecond loop delay with the framework, so the higher you set this the longer of a delay between a physical change and when an onChange event will trigger. You also run the risk of overflowing the int value that holds the state if it's too high. __10__ should be sufficient, particularly if you're using the __range__ option. You can also combine with a small delta if needed.  

__NOTE:__ You can use this class with any type of variable resistor! Works well with photoresistors  for example.


---


## Rotary Encoder ##  
__Create with:__
```c++
 device.addRotaryEncoder("name", PIN_SW, PIN_DT, PIN_CLK);

```

__Events__
* onPress()
* onRelease()  
* onClockwise()
* onCounterClockwise()

---

## LED ##  
__Create with:__
```c++
 device.addButton("name", PIN);
 //or for RGB 
 device.addButton("name", PIN_R, PIN_G, PIN_B);
```

__properties__
* isDimmable
If set to true the LED can use the setLevel() command to dim the LED (requires a PMW pin). Only for regular LEDs, not RGB.
* commonAnode
If true (default) the RGB is common anode type (common pin is hot). Set to false if yours is of the Common Cathode type.

__Commands__
* turnOn()
* turnOff()
* flip()
* blink(delay), blink(delay, count)
* pulse(delay, count, low, high)
* setLevel(0-255)
* setColor(0-255, 0-255, 0-255)

__turnOn & turnOff__ turns on and off the LED, regardless of color or dim setting. __turnOff__ also stops a blinking LED.  
__flip__ flips the LED state from on to off and visa versa.  
__blink(delay, optional count)__ blinks the light, delay is the total time on & off, count is how many times to blink, __0__ for unlimited. If you set the blink to unlimited you can use turnOff() to stop the blinking.  
__pulse(delay, count, low, high)__ pulses the LED from low to high. Delay is the length of 1 on-off pulse and count is the number of pulses (on-off).  
__setLevel(level)__ level 0-255. Does not explicitly turn on the LED. You can set the level when the LED is off and it will be that level when you turn it back on.  
__setColor(R,G,B)__ RGB 0-255. Sets the color, but only if you provide the RGB PINS. Like setLevel you can set the color when the light is off and it will be that color when you turn it back on.  

---

## Intervals! ##

Intervals are an incredibly useful feature that allows you to have something happen one of more times in the future. Many arduino tutorials start off by having a void loop() that does something and then has a delay at the end, maybe reading a temperature probe, something like that. But what if you want to do many different things at different times? Intervals to the rescue!

__Creating an Interval:__

```c++
    device.addInterval(callback, delay);
    device.addInterval(callback, delay, repeat);
    device.addInterval(callback, delay, repeat, msg);
    device.addInterval(callback, delay, repeat, msg, intervalName);
```

* callback: the function to run on the interval
* delay: how long in milliseconds to wait between each interval
* repeat: how many times to repeat the interval, __0__ for unlimited, __1__ to run once. (So really it's how many times to run the interval more than how many times to "repeat" it).
* msg: 0-255, a message to pass to the function. Useful if multiple intervals use the same function and you want to change the function behavior depending on the message.
* name: If you give an interval a name you can also use the clearInterval(name) function to stop an interval.

__Basic Use Example:__
```c++
    //this is the function to run when the interval happens
    //you need to include the msg param
    void readTempProbe(byte msg) {
        doTheThingThatReadsTheTemperature();
    }

    void setup() {
        device.addInterval(readTempProbe, 1000);
    }
```

This will run the readTempProbe function every second indefinitely.

__Example #2:__
```c++
    //use preprocessor directives for the msgs so it's easier to read
    #define LED_RED 1
    #define LED_BLUE 2

    //we need two intervals (minimum) so modify that as well
    #define TOTAL_INTERVALS 2

    void turnOnRedLED() {
        device.led("red").turnOn();
        device.addInterval(turnOffLED, 500, 1, LED_RED);
    }

    void turnOnBlueLED() {
        device.led("red").turnOn();
        device.addInterval(turnOffLED, 500, 1, LED_RED);
    }

    void turnOffLed(byte msg) {
        if (msg==LED_RED) {
            device.led("red").turnOff();
        } else if (msg==LED_BLUE) {
            device.led("blue").turnOff();
        }
    }

    void setup() {
        device.addLED("red", 5);
        device.addLed("blue", 6);
        device.addButton("turnOnRed", 3);
        device.addButton("turnOnBlue", 4);

        device.button("turnOnRed").onPress(turnOnRedLED);
        device.button("turnOnBlue").onPress(turnOnBlueLED);
    }
```
This program will turn on the LEDs when the corresponding button is pressed and then turn that LED off after 500ms. A nice feature to have for user-feedback since the LED will light for a half second even when the user just taps the button.

Explanation:
```c++
    #define LED_RED 1
    #define LED_BLUE 2
```
An interval can pass a "msg" to it's callback function, but the msg is just a byte (0-255) so to improve code reusability I used a #define macro. You don't have to do this.

```c++
    #define TOTAL_INTERVALS 2
```
There's an array that holds the intervals, this line sizes that array. You can run this program with TOTAL_INTERVALS 1, but if you press both buttons within 500ms (the interval length) then the second interval wont be created since there's no room left in the array for it. If you only want to press one button at once it wont be a problem. In general you should leave yourself a buffer so there's always room for all required intervals, but intervals are removed from the array when they're completed so depending on how your program works you may be able to have a smaller number of total intervals to save program space. It's really total _simultaneous_ intervals.

```c++    
    void turnOnRedLED() {
        device.led("red").turnOn();
        device.addInterval(turnOffLED, 500, 1, LED_RED);
        //device.addInterval(Callback, delay, repeat, msg);
    }

    void turnOffLed(byte msg) {
        if (msg==LED_RED) {
            device.led("red").turnOff();
        } else if (msg==LED_BLUE) {
            device.led("blue").turnOff();
        }
    }
```

When the turnOnRedLED function is run it turns on the LED and then creates an interval with the msg LED_RED. That same msg is then used by the turnOffLed() function to determine which LED to turn off.


## Example #3 ###
This demonstrates the clearInterval(name) function.

```c++
     void readTempProbe(byte msg) {
        doTheThingThatReadsTheTemperature();
    }

    void start(byte msg) {
        //device.addInterval(callback, delay, repeat, msg, name);
        device.addInterval(readTempProbe, 5000, 0, 0, "temp");
    }

    void stop(byte msg) {
        device.clearInterval("temp");
    }

    void setup() {
        device.addButton("start", 4);
        device.addButton("stop", 5);

        device.button("start").onPress(start);
        device.button("stop").onPress(stop);
    }
```
When the button on pin 4 is pressed this will read the temperature probe every 5 seconds indefinitely until the button on pin 5 is pressed, clearing the interval.


---
