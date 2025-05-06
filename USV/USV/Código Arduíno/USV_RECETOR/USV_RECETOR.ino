//// Adding Libraries
#include <SPI.h>      /* to handle the communication interface with the modem*/
#include <nRF24L01.h> /* to handle this particular modem driver*/
#include <RF24.h>     /* the library which helps us to control the radio modem*/

#include <Servo.h>
Servo motorA;

int pwmPinA = 7; //Motor Esquerda
int pwmVal = 1500;

Servo motorB;

int pwmPinB = 6;

RF24 radio(9, 10);               /* Creating instance 'radio'  ( CE , CSN )   CE -> D9 | CSN -> D10 */
const byte Address[6] = "00001"; /* Address from which data to be received */

int dados;

const int ledPinA = 47;
const int ledPinB = 46;


int valor = 0;

void setup() {

  Serial.begin(9600);

  radio.begin();                      // Activate the modem
  radio.openReadingPipe(1, Address);  // Sets the address of receiver from which program will receive the data
  radio.setPALevel(RF24_PA_MIN);      //???
  radio.startListening();             //Setting modem in Receiver mode

  pinMode(ledPinA, OUTPUT);
  pinMode(ledPinB, OUTPUT);

  motorA.attach(pwmPinA);  // attaches the servo on pin 3 to the servo object
  motorA.writeMicroseconds(pwmVal);

  motorB.attach(pwmPinB);
  motorB.writeMicroseconds(pwmVal);
}

void loop() {

  if (radio.available()) {

    radio.read(&dados, sizeof(dados));

    //motor = char(dados[0]);
    //SPEED = int8_t(dados[1]);

    Serial.print("Received msg: ");
    Serial.println(dados);

    switch (dados / 10000) {  //dados é uma variável inteira ---então--> dados/1000 dará 1, 2, 3, etc.
      case 1:
        valor = dados % 10000;  // resto da divisão inteira dá o valor a extrair (ou "valor=dados-1000")

        motorA.writeMicroseconds(valor);

        Serial.print("msg_A (valor1): ");
        Serial.println(valor);
        Serial.println("------------------");
        break;
      case 2:
        valor = dados % 20000;  // resto da divisão inteira dá o valor a extrair (ou "valor=dados-2000")

        motorB.writeMicroseconds(valor);

        Serial.print("msg_B (valor2): ");
        Serial.println(valor);
        Serial.println("------------------");
        break;
      case 3:
        valor = dados % 30000;
        if (valor == 100){

          digitalWrite(ledPinA, HIGH);
          digitalWrite(ledPinB, LOW);

        } else {

          digitalWrite(ledPinA, LOW);
          digitalWrite(ledPinB, HIGH);
        }
        
      default:
        // statements
        break;
    }
  }
}
