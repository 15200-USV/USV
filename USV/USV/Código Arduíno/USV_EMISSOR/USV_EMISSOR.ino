//Adding Libraries
#include <SPI.h>       // to handle the communication interface with the modem
#include <nRF24L01.h>  // to handle this particular modem driver
#include <RF24.h>      // the library which helps us to control the radio modem

//Variáveis

//Rádio
RF24 radio(9, 10);                // Creating instance 'radio'  ( CE , CSN )   CE -> D9 | CSN -> D10
const byte Address[6] = "00001";  // Address to which data to be transmitted

// Fazer teste através do serial monitor
char msg, buffer[5];
int NBytes;

// Entradas e Saídas
const int switchPin = 3;            // O Switch que indica se está no modo Autonomo/Controlado
const int ledPinA = 5, ledPinB = 6; // Os LED's
const int JEY = A3, JEX = A0; // JEY - JoyStick Esquerdo Y
const int JDY = A2, JDX = A1; // JDY - JoyStick Direita Y

// para guardar os valores do JoyStick
int Valor_JEY, Valor_JEX, Valor_JDY, Valor_JDX;
uint16_t mensagem; // Valor enviado via radio~
int m1, m2, m3;

// Switch
int switchState = 0;

void setup() {

    // Inicialização do Switch (Auto/Control)
  pinMode(switchPin, INPUT_PULLUP);
  
  //Inicialização dos LED's
  pinMode(ledPinA, OUTPUT);
  pinMode(ledPinB, OUTPUT);
  // Indicar que está ligado com os LED's
  digitalWrite(ledPinA, LOW);
  digitalWrite(ledPinB, HIGH);

  // Rádio
  Serial.begin(19200);
  radio.begin();                   // Activate the modem
  radio.openWritingPipe(Address);  // Sets the address of transmitter to which program will send the data
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  pinMode(JEY, INPUT); // JoyStick Esquerdo Y
  pinMode(JEX, INPUT); // JoyStick Esquerdo X
  pinMode(JDY, INPUT); // JoyStick Direito Y
  pinMode(JDX, INPUT); // JoyStick Direito X
}

//Função para obter os valores do joystick de x
int obterValor(int joyStick, int eixo){
  int valor = 0;
  int x, y = 0;

  valor = analogRead(joyStick);

  // Fazer verificação se está no meio
  if ((valor > 515) && (valor < 525)) {

    valor = 512;
  }

  // Conversão - Uso o 1 para identificar que quer obter o valor de Y e 2 para o X  
  if(eixo == 1){
    valor = map(valor, 0, 1023, 200, -200); // Y
  }
  else if(eixo == 2){
    valor = map(valor, 0, 1023, 1800, 1200); // X
  }
  else {
    valor = 0; // Não existe
  }

  return valor;
}

int mandarValor(int motor, int x , int y){

  // Variável
  int dados = 0;      // Dados para fazer as contas
  int ultimoDado = 0; // Dados que serão enviados para o recetor

  //Casos 
  if(motor == 1){
    
    dados =  x - y;

    if(y > 170 && x < 1600 && x > 1400){
      dados = 1300;
    }
    else if(y < -170 && x < 1600 && x > 1400){
      dados = 1700;
    }

    ultimoDado = 10000 + dados;

  }
  else if(motor == 2) {

    dados = x - y;
    
    if(y > 170 && x < 1600 && x > 1400){
      dados = 1700;
    }
    else if(y < -170 && x < 1600 && x > 1400){
      dados = 1300;
    }

    ultimoDado = 20000 + dados;

  }
  else if(motor == 3){

    dados = x;
    ultimoDado = 30000 + dados;

  }
  else{
    ultimoDado = 0;
  }    

  //Print
  Serial.print("Motor :");
  Serial.println(motor);
  Serial.println("Dados :");
  Serial.println(dados);
  /*Serial.println("X :");
  Serial.println(x);
  Serial.println("Y :");
  Serial.println(y);*/
  delay(1000);

  return ultimoDado;
}

//Função para obter os valores do joystick de x

void loop() {
  
  switchState = digitalRead(switchPin);

  if (switchState == LOW) {
    digitalWrite(ledPinA, HIGH);
    digitalWrite(ledPinB, LOW);
    
    //Enviar os dados para recetor sovre o LED
    mensagem = 40100;
    radio.write(&mensagem, sizeof(mensagem));  // Sending data over NRF 24L01
    Serial.print("Transmitted msg (Modo Manual): ");
    Serial.println(mensagem);

    // Obter os Valores do JoyStick
    Valor_JEY = obterValor(JEY, 1);
    Valor_JEX = obterValor(JEX, 2);
    Valor_JDY = obterValor(JDY, 1);
    Valor_JDX = obterValor(JDX, 2);

    // Mandar os valores para           || Caso queiras ler os valores coloca os delays e tira o comentário dos print's na função
    m1 = mandarValor(1, Valor_JDX, Valor_JDY);
    //delay(1000);
    m2 = mandarValor(2, Valor_JDX, Valor_JDY);
    //delay(1000);
    m3 = mandarValor(3, Valor_JEX, Valor_JEY);
    //delay(1000);
    //mandarValor(4, Valor_JEX, Valor_JEY)

    // mandar para recetor
    radio.write(&m1, sizeof(m1)); // Mandar a mensagem para o recetor
    radio.write(&m2, sizeof(m2)); // Mandar a mensagem para o recetor
    radio.write(&m3, sizeof(m3)); // Mandar a mensagem para o recetor

    if (Serial.available()) {

      msg = Serial.read();

      if (msg == 'A') {

        NBytes = Serial.readBytesUntil('\r\n', buffer, 5);

        mensagem = 10000 + atoi(buffer);

        radio.write(&mensagem, sizeof(mensagem));  // Sending data over NRF 24L01
        Serial.print("Transmitted msg : ");
        Serial.println(mensagem);

        Serial.print("msg_A (valor1): ");
        Serial.println(mensagem - 10000);
        Serial.println("----------------------");
      }

      if (msg == 'B') {

        NBytes = Serial.readBytesUntil('\r\n', buffer, 5);

        mensagem = 20000 + atoi(buffer);

        radio.write(&mensagem, sizeof(mensagem));  // Sending data over NRF 24L01
        Serial.print("Transmitted msg : ");
        Serial.println(mensagem);

        Serial.print("msg_B (valor2): ");
        Serial.println(mensagem - 20000);
        Serial.println("----------------------");
      }
    }
    delay(450);

  } else {

    digitalWrite(ledPinA, LOW);
    digitalWrite(ledPinB, HIGH);
    
    mensagem = 40200;
    radio.write(&mensagem, sizeof(mensagem));  // Sending data over NRF 24L01
    Serial.print("Transmitted msg (Modo Automatico): ");
    Serial.println(mensagem);
  }
  
}
