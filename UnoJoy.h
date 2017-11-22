/*  UnoJoy.h
 *   Alan Chatham - 2012
 *    RMIT Exertion Games Lab
 *
 *  This library gives you a standard way to create Arduino code that talks
 *   to the UnoJoy firmware in order to make native USB game controllers.
 *  Functions:
 *   setupUnoJoy()
 *   getBlankDataForController()
 *   setControllerData(dataForController_t dataToSet)
 *
 *   NOTE: You cannot use pins 0 or 1 if you use this code - they are used by the serial communication.
 *         Also, the setupUnoJoy() function starts the serial port at 38400, so if you're using
 *         the serial port to debug and it's not working, this may be your problem.
 *   
 *   === How to use this library ===
 *   If you want, you can move this file into your Arduino/Libraries folder, then use it like a normal library.
 *   However, since you'll need to refer to the details of the dataForController_t struct in this file, I would suggest you use
 *    it by adding it to your Arduino sketch manually (in Arduino, go to Sketch->Add file...)
 *
 *  To use this library to make a controller, you'll need to do 3 things:
 *   Call setupUnoJoy(); in the setup() block
 *   Create and populate a dataForController_t type variable and fill it with your data
 *         The getBlankDataForController() function is good for that.
 *   Call setControllerData(yourData); where yourData is the variable from above,
 *         somewhere in your loop(), once you're ready to push your controller data to the system.
 *         If you forget to call sendControllerData in your loop, your controller won't ever do anything
 *
 *  You can then debug the controller with the included Processing sketch, UnoJoyProcessingVisualizer
 *  
 *  To turn it into an actual USB video game controller, you'll reflash the
 *   Arduino's communication's chip using the instructions found in the 'Firmware' folder,
 *   then unplug and re-plug in the Arduino. 
 * 
 *  Details about the dataForController_t type are below, but in order to create and use it,
 *   you'll declare it like:
 *
 *      dataForController_t sexyControllerData;
 *
 *   and then control button presses and analog stick movement with statements like:
 *      
 *      sexyControllerData.triangleOn = 1;   // Marks the triangle button as pressed
 *      sexyControllerData.squareOn = 0;     // Marks the square button as unpressed
 *      sexyControllerData.leftStickX = 90;  // Analog stick values can range from 0 - 255
 */

#ifndef UNOJOY_H
#define UNOJOY_H
    #include <stdint.h>
    #include <util/atomic.h>
    #include <Arduino.h>
// STRUCTURE
	typedef struct dataForController_t  {    //use the setControllerData function to send that data out.
		uint8_t triangleOn : 1;             // Each of these member variables control if a button is off or on
		uint8_t circleOn : 1;     
		uint8_t squareOn : 1;              // For the buttons, 0 is off, 1 is on
		uint8_t crossOn : 1;       
		uint8_t l1On : 1;          
		uint8_t l2On : 8;     // 1 originally  acc
		uint8_t l3On : 1;                  // The : 1 here just tells the compiler to only have 1 bit for each variable.
		uint8_t r1On : 1;        
		uint8_t r2On : 8;     // 1 originally  brk
		uint8_t r3On : 1;
		uint8_t selectOn : 1;
		uint8_t startOn : 1;
		uint8_t homeOn : 1;
		uint8_t dpadLeftOn : 1;
		uint8_t dpadUpOn : 1;
		uint8_t dpadRightOn : 1;
		uint8_t dpadDownOn : 1;
        uint8_t padding : 1;     // We end with 1 (7 originally) bytes of padding to make sure we get our data aligned in bytes
		uint8_t leftStickX : 8;    // Each of the analog stick values can range from 0 to 255
		uint8_t leftStickY : 8;    //  0 is fully left or up!  255 is fully right or down!  128 is centered!
		uint8_t rightStickX : 8;   //  Important - analogRead(pin) returns a 10 bit value, so if you're getting strange
		uint8_t rightStickY : 8;   //  results from analogRead, you may need to do (analogRead(pin) >> 2) to get good data  
	} dataForController_t;
//FUNCTIONS CALLS  
    void setupUnoJoy(void);   // Call setupUnoJoy in the setup block of your program. It sets up the hardware UnoJoy needs to work properly
    void setupUnoJoy(int);        // You can also call the set
    void setControllerData(dataForController_t);  
              // This sets the controller to reflect the button and joystick positions you input (as a dataForController_t).
              // The controller will just send a zeroed (joysticks centered) signal until you tell it otherwise with this function.
    dataForController_t getBlankDataForController(void);    // Fresh dataForController_t:  No buttons pressed, joystick centered
                                                            //   call: myControllerData = getBlankDataForController();

//IMPLEMENTATION  
  dataForController_t controllerDataBuffer;  // Used to store the controller data to send out. DO NOT CHANGE IT!
                                //  UnoJoy on the ATmega8u2 regularly polls the Arduino chip for individual bytes of a dataForController_t.
  void setControllerData(dataForController_t controllerData){
    ATOMIC_BLOCK(ATOMIC_FORCEON){                                 // Maybe uneces., guarantees the data gets copied to buffer all at once.
      controllerDataBuffer = controllerData;
    }
  }
  
  volatile int serialCheckInterval = 1;   // serialCheckInterval governs how many ms between checks to the serial port for data (<20).
  int serialCheckCounter = 0;             // This is an internal counter variable to count ms between serial check times
  
  void setupUnoJoy(void){                 // This is the setup function - it sets up the serial communication and the timer interrupt for actually sending the data back and forth. 
    controllerDataBuffer = getBlankDataForController();     // First, let's zero out our controller data buffer (center the sticks)
    Serial.begin(9600);
    OCR0A = 128;      // Now set up the Timer 0 compare register A so that Timer0 (used for millis() and such) also fires an interrupt when it's equal to 128, not just on overflow.
    TIMSK0 |= (1 << OCIE0A);  // This will fire our timer interrupt almost every 1 ms (1024 us to be exact).
  }
  
  void setupUnoJoy(int interval){     // If you really need to change the serial polling interval, use this function to initialize UnoJoy.
    serialCheckInterval = interval;   //  Interval is the polling frequency, in ms.
    setupUnoJoy();
  }
  
  ISR(TIMER0_COMPA_vect){    // polling serial  // This interrupt gets called approximately once per ms. It counts how many ms between serial port polls, and if it's been long enough,
    serialCheckCounter++;                       //  polls the serial port to see if the UnoJoy firmware requested data. If it did, it transmits the appropriate data back.
    if (serialCheckCounter >= serialCheckInterval){         // If there is incoming data stored in the Arduino serial buffer
      serialCheckCounter = 0;   
      while (Serial.available() > 0) {
        pinMode(13, OUTPUT);                                                //digitalWrite(13, HIGH);
        byte inByte = Serial.read();                  // Get incoming byte from the ATmega8u2. That number tells us which byte of the dataForController_t struct to send out.
        Serial.write(((uint8_t*)&controllerDataBuffer)[inByte]);           //digitalWrite(13, LOW);
      }
    }
  }

  dataForController_t getBlankDataForController(void){
    dataForController_t controllerData;
    controllerData.triangleOn = 0;        // Make the buttons zero
    controllerData.circleOn = 0;
    controllerData.squareOn = 0;
    controllerData.crossOn = 0;
    controllerData.l1On = 0;
    controllerData.l2On = 128;
    controllerData.l3On = 0;
    controllerData.r1On = 0;
    controllerData.r2On = 128;
    controllerData.r3On = 0;
    controllerData.dpadLeftOn = 0;
    controllerData.dpadUpOn = 0;
    controllerData.dpadRightOn = 0;
    controllerData.dpadDownOn = 0;  
    controllerData.selectOn = 0;
    controllerData.startOn = 0;
    controllerData.homeOn = 0;
    controllerData.leftStickX = 128;      // sticks centered
    controllerData.leftStickY = 128;
    controllerData.rightStickX = 128;
    controllerData.rightStickY = 128; 
    return controllerData;
  }
#endif
