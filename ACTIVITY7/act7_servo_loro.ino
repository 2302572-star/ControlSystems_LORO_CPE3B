#include <Servo.h>

Servo myServo;

const int servoPin = 10;

void setup() {
  myServo.attach(servoPin);
  
  Serial.begin(9600);
  
  myServo.write(0);
}

void loop() {

  if (Serial.available() > 0) {

    int angle = Serial.parseInt();

    if (Serial.read() == '\n') {

      angle = constrain(angle, 0, 180);
      

      myServo.write(angle);

      Serial.print("Servo moved to: ");
      Serial.println(angle);
    }
  }
}