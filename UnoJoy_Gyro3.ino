#include<Wire.h>
#include "UnoJoy.h"
const int MPU=0x68;                 // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
boolean on = true;
int switchState = 0;
int acc; int brk; int lef; int rig; int stick; int thh = 10000; int thl = 1000; int i=-10500;

void setup(){
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial.begin(9600);
  setupPins();
  setupUnoJoy();  // per vedere dati su monitor, serial 38400->9600
}

void loop(){
  if (on) { 
      Wire.beginTransmission(MPU);
      Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
      Wire.endTransmission(false);
      Wire.requestFrom(MPU,14,true);  // request a total of 14 regi
      AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
      AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
      AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
      //Serial.println(AcX);
      AcX = AcX - 2700;
           if (AcY > thh)   { acc = thh;   brk = 0; }      // top acceleration
      else if (AcY > thl)   { acc = AcY;  brk = 0; }      // acceleration
      else if (AcY < -thl)  { acc = 0;    brk = AcY; }    // brakingsters
      else if (AcY < -thh)  { acc = 0;    brk = thh; }     // top braking  
      else                  { acc = 0;    brk = 0;}       // 

           if (AcX >  thh)  { lef = thh;  rig = 0;}       // asse X
      else if (AcX >  thl)  { lef = AcX;  rig = 0;}
      else if (AcX < -thh)  { lef = 0;    rig = -thh; }     
      else if (AcX <  thl)  { lef = 0;    rig = AcX; }
      else                  { lef = 0;    rig = 0;}
//i=i+500;
    acc   = map( acc,     -thh, +thh, 255, 0 ); 
    brk   = map( brk,     -thh, +thh, 255, 0 ); 
    stick = map( lef+rig, -thh, +thh, 255, 0 ); 
    
    dataForController_t controllerData = getControllerData();
    setControllerData(controllerData);
    //Serial.print("\t acc "); Serial.print    ( acc );   //Serial.print("\t "); Serial.print  ( controllerData.l2On ); 
    //Serial.print("\t brk "); Serial.print    ( brk );   //Serial.print("\t "); Serial.print  ( controllerData.r2On ); 
    //Serial.print("\t stk "); Serial.println( stick ); //Serial.print("\t "); Serial.println( controllerData.leftStickX ); 
    //delay(10);                                        //    Serial.print("A0"); Serial.println(A0);
  } else {
    delay(1000);
  }
}

void setupPins(void){     //// Set all the digital pins with the pull-up enabled, except for the inputs two serial line pins
  for (int i = 2; i <= 12; i++){ 
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);
  }
  /*  default in unojoy
    pinMode(A4, INPUT);    digitalWrite(A4, HIGH);    pinMode(A5, INPUT);    digitalWrite(A5, HIGH);     */
}

dataForController_t getControllerData(void){  //// Set up controller data. Use the getBlankDataForController() function to avoid random values
 dataForController_t controllerData = getBlankDataForController();

  controllerData.l2On         = acc;             //analogRead(acc);
  controllerData.r2On         = brk;             //analogRead(brk);
  controllerData.leftStickX   = stick;               
  controllerData.rightStickY  = acc-brk;      //analogRead(Acz/1000);
  
  return controllerData;      
}

////////////////////// THE END ///////////////////////////////////////////////// 
/*



analogRead(128+lef/thh);    // Set the analog sticks, since analogRead(pin) returns a 10 bit value, bit shift operation
  controllerData.leftStickY   = analogRead(A1) >> 2;               // 0 left/UP  255 right/DOWN
  controllerData.rightStickX  = analogRead(acc-brk) >> 2;
  controllerData.rightStickY  = 128;      //analogRead(Acz/1000);
 



  controllerData.triangleOn       = !digitalRead(2);   // Since our buttons are all held high and pulled low when pressed, we use the "!"  operator to invert the readings from the pins
  controllerData.circleOn         = !digitalRead(3);
  controllerData.squareOn         = !digitalRead(4);
  controllerData.crossOn          = !digitalRead(5);
  controllerData.dpadUpOn         = !digitalRead(6);
  controllerData.dpadDownOn       = !digitalRead(7);
  controllerData.dpadLeftOn       = !digitalRead(8);
  controllerData.dpadRightOn      = !digitalRead(9);
  controllerData.l1On             = !digitalRead(10);
  controllerData.r1On             = !digitalRead(11);

  controllerData.selectOn         = !digitalRead(12);
  controllerData.startOn          = !digitalRead(A4);
  controllerData.homeOn           = !digitalRead(A5);
*/
