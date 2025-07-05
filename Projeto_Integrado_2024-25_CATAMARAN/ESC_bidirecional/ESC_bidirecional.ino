
// Ligar fio branco do ESC ao pino 3 arduino e o fio preto ao GND
// O fio vermelho do ESC fornece 5V (não ligar aos 5V do arduino)
// Depois de ligar a alimentação do circuito, procurar o ponto de inicialização do ESC


#include <Servo.h>

Servo motor;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pwmPin = 3;
int pwmVal = 1500;  // variable to store the servo position

char msg, buffer[15];
int NBytes;


void setup() {

  //motor.attach(pwmPin,1000, 2000);  // attaches the servo on pin 3 to the servo object
  motor.attach(pwmPin);  // attaches the servo on pin 3 to the servo object
  motor.writeMicroseconds(pwmVal);

  Serial.begin(9600, SERIAL_8N1);

  Serial.print("pwmVal: ");
  Serial.println(pwmVal);
  Serial.println("Motor inicializado");
}


void loop() {

  if (Serial.available()) {

    msg = Serial.read();

    if (msg == 'A') {

      NBytes = Serial.readBytesUntil('\r\n', buffer, 15);
      pwmVal = atoi(buffer);

      motor.writeMicroseconds(pwmVal);

      Serial.print("pwmVal: ");
      Serial.println(pwmVal);
    }
  }
}
