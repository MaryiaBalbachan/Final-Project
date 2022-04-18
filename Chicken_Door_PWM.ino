#include <Servo.h>
Servo myservo;                           // create servo object to control a servo
int controlPin = 0;                      // analog pin used to receive signal from pin 16 on the RPi
int val;                                 // variable to read the value from the analog pin A0
void setup() {
  myservo.attach(9);                     // attaches the servo on pin 9 to the servo object
}
void loop() {
  val = analogRead(controlPin);          // reads the value of the analogue pin A0 (value between 0 and 1023)
  
  val = map(val, 0, 1023, 180, 0);       // scale it to use it with the servo (value between 0 and 180) Mapping 0 to 180 and 1023 to 0,
                                         // to ensure that the door will open when RPi pin 16 goes HIGH
        
  myservo.write(val);                    // sets the servo position according to the scaled value


delay(10);                               // waits for the servo to get there
}
