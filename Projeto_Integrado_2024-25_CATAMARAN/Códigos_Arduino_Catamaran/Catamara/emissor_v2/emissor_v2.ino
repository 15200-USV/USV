//Adding Libraries

#include <SPI.h>       // to handle the communication interface with the modem
#include <nRF24L01.h>  // to handle this particular modem driver
#include <RF24.h>      // the library which helps us to control the radio modem

RF24 radio(9, 10);                // Creating instance 'radio'  ( CE , CSN )   CE -> D9 | CSN -> D10
const byte Address[6] = "00001";  // Address to which data to be transmitted

char msg, buffer[5];
int NBytes;
int dados;


void setup() {
  Serial.begin(9600);
  radio.begin();                   // Activate the modem
  radio.openWritingPipe(Address);  // Sets the address of transmitter to which program will send the data
  radio.setPALevel(RF24_PA_MIN);   //???
  radio.stopListening();           // Setting modem in transmission mode
}

void loop() {

  if (Serial.available()) {

    msg = Serial.read();

    if (msg == 'A') {
      
      NBytes = Serial.readBytesUntil('\r\n', buffer, 5);

      dados = 10000 + atoi(buffer);

      radio.write(&dados, sizeof(dados)); // Sending data over NRF 24L01
      Serial.print("Transmitted msg : ");
      Serial.println(dados);

      Serial.print("msg_A (valor1): ");
      Serial.println(dados - 10000);
      Serial.println("----------------------");
    }

    if (msg == 'B') {
      
      NBytes = Serial.readBytesUntil('\r\n', buffer, 5);

      dados = 20000 + atoi(buffer);

      radio.write(&dados, sizeof(dados)); // Sending data over NRF 24L01
      Serial.print("Transmitted msg : ");
      Serial.println(dados);

      Serial.print("msg_B (valor2): ");
      Serial.println(dados - 20000);
      Serial.println("----------------------");
    }
  }
}