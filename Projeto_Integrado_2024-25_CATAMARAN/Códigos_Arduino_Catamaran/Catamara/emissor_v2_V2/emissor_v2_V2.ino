//Adding Libraries

#include <SPI.h>       // to handle the communication interface with the modem
#include <nRF24L01.h>  // to handle this particular modem driver
#include <RF24.h>      // the library which helps us to control the radio modem

RF24 radio(9, 10);                // Creating instance 'radio'  ( CE , CSN )   CE -> D9 | CSN -> D10
const byte Address[6] = "00001";  // Address to which data to be transmitted

const int potPinA = A1;
const int potPinB = A0;
int potValueY = 0;
int potValueX = 0;



int vel_MC;
int vel_Turn; 

char msg, buffer[5];
int NBytes;
int dados;


void setup() {
  pinMode(potPinA, INPUT);
  pinMode(potPinB, INPUT);
  Serial.begin(9600);
  radio.begin();                   // Activate the modem
  radio.openWritingPipe(Address);  // Sets the address of transmitter to which program will send the data
  radio.setPALevel(RF24_PA_MIN);   //???
  radio.stopListening();           // Setting modem in transmission mode
}

void loop() {
  potValueY = analogRead(potPinA);

  if ((potValueY > 515) && (potValueY < 525)) {

    potValueY = 512;
  }

  potValueX = analogRead(potPinB);
//serial.print("potValueX")

  if ((potValueX > 445) && (potValueX < 455)) {

    potValueX = 512;
  }

  vel_MC = map(potValueY, 0, 1023, 1200, 1800);
  vel_Turn = map(potValueX, 0, 1023, -200, 200);

  Serial.print("vel_MC: ");
  Serial.println(vel_MC);

  Serial.print("vel_Turn: ");
  Serial.println(vel_Turn);

  Serial.print("potValueY = ");
  Serial.println(potValueY);

  Serial.print("potValueX = ");
  Serial.println(potValueX);

  dados = 10000 + vel_MC + vel_Turn;

  radio.write(&dados, sizeof(dados));  // Sending data over NRF 24L01
  Serial.print("Transmitted msg : ");
  Serial.println(dados);

  Serial.print("msg_A (valor): ");
  Serial.println(dados - 10000);
  Serial.println("----------------------");

  dados = 20000 + vel_MC - vel_Turn;

  radio.write(&dados, sizeof(dados));  // Sending data over NRF 24L01
  Serial.print("Transmitted msg : ");
  Serial.println(dados);

  Serial.print("msg_B (valor): ");
  Serial.println(dados - 20000);
  Serial.println("----------------------");

  if (Serial.available()) {

    msg = Serial.read();

    if (msg == 'A') {

      NBytes = Serial.readBytesUntil('\r\n', buffer, 5);

      dados = 10000 + atoi(buffer);

      radio.write(&dados, sizeof(dados));  // Sending data over NRF 24L01
      Serial.print("Transmitted msg : ");
      Serial.println(dados);

      Serial.print("msg_A (valor1): ");
      Serial.println(dados - 10000);
      Serial.println("----------------------");
    }

    if (msg == 'B') {

      NBytes = Serial.readBytesUntil('\r\n', buffer, 5);

      dados = 20000 + atoi(buffer);

      radio.write(&dados, sizeof(dados));  // Sending data over NRF 24L01
      Serial.print("Transmitted msg : ");
      Serial.println(dados);

      Serial.print("msg_B (valor2): ");
      Serial.println(dados - 20000);
      Serial.println("----------------------");
    }
  }
  delay (450);
}
