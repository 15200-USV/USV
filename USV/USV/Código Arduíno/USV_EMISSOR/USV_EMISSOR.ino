// Inclusão de bibliotecas
#include <SPI.h>       // para lidar com a interface de comunicação com o modem
#include <nRF24L01.h>  // para lidar com o driver específico do modem
#include <RF24.h>      // biblioteca que facilita o controlo do rádio modem

// Rádio
RF24 radio(9, 10);                // Criação da instância 'radio'  ( CE , CSN )   CE -> D9 | CSN -> D10
const byte Address[6] = "00001";  // Endereço para o qual os dados serão transmitidos
char msg, buffer[5];
int NBytes, dados;

// Entradas e Saídas
const int potPinA = A1, potPinB = A0;  // Entradas dos potenciómetros
const int switchPin = 3;               // Botão ON/OFF do comando
const int ledPinA = 5, ledPinB = 6;    // Entradas ou Saídas? para os LED's

// Variáveis e objetos
int potValueY = 0, potValuex = 0, switchState = 0;
int vel_MC, vel_Turn, vel_C;


void setup() 
{
  Serial.begin(19200);

  // Inicialização do Switch (ON/OFF)
  pinMode(switchPin, INPUT_PULLUP);

  //Inicialização dos LED's
  pinMode(ledPinA, OUTPUT);
  pinMode(ledPinB, OUTPUT);

  // Indicar que está ligado através dos LED's
  digitalWrite(ledPinA, LOW);
  digitalWrite(ledPinB, HIGH);

  pinMode(potPinA, INPUT);
  pinMode(potPinB, INPUT);

  // Inicialização do rádio
  radio.begin();                   // Ativa o modem
  radio.openWritingPipe(Address);  // Define o endereço do transmissor para onde os dados serão enviados
  radio.setPALevel(RF24_PA_MIN);   // Define o nível de potência (mínimo)
  radio.stopListening();           // Coloca o modem em modo de transmissão
}

void loop() 
{

  switchState = digitalRead(switchPin);  // Lê o estado do botão ON/OFF

  if (switchState == LOW)  // Neste caso está LIGADO (ON)
  {
    // Acende os LED's
    digitalWrite(ledPinA, HIGH);
    digitalWrite(ledPinB, LOW);

    // Envia os dados
    dados = 30100;
    radio.write(&dados, sizeof(dados));  // Envia dados via NRF24L01

    // Verificação
    Serial.println("LOW");
    Serial.print("Transmitted msg : ");
    Serial.println(dados);

  } 
  else  //Neste caso está OFF
  {

    // Colocar os LED's OFF
    digitalWrite(ledPinA, LOW);
    digitalWrite(ledPinB, HIGH);

    // Envia os dados
    dados = 40200;
    radio.write(&dados, sizeof(dados));  // Envia dados via NRF24L01

    // Verificação de quê?
    Serial.println("HIGH");
    Serial.print("Transmitted msg : ");
    Serial.println(dados);
  }

  if (switchState == LOW) 
  {

    // Leitura do joystick
    potValueY = analogRead(potPinY);
    potValueX = analogRead(potPinX);

    // Corrigir zona morta
    if (potValueY > 510 && potValueY < 515)
      potValueY = 512;

    if (potValueX > 510 && potValueX < 515)
      potValueX = 512;

    // Mapeamento
    vel_MC = map(potValueY, 0, 1023, 1800, 1200);   // Frente/trás
    vel_Turn = map(potValueY, 0, 1023, 200, -200);  // Curva baseada no mesmo eixo Y
    vel_C = map(potValueX, 0, 1023, 1800, 1200);    // Movimento lateral motor C

    // Verificação
    Serial.print("vel_MC: ");
    Serial.println(vel_MC);
    Serial.print("vel_Turn: ");
    Serial.println(vel_Turn);
    Serial.print("vel_C: ");
    Serial.println(vel_C);
    Serial.print("potValueY = ");
    Serial.println(potValueY);
    Serial.print("potValueX = ");
    Serial.println(potValueX);

    // Motor A = velocidade + curva
    dados = 10000 + vel_MC + vel_Turn;
    radio.write(&dados, sizeof(dados)); // Envia dados via NRF24L01

    // Verificação
    Serial.print("Transmitted msg : ");
    Serial.println(dados);
    Serial.print("msg_A (valor): ");
    Serial.println(dados - 10000);
    Serial.println("----------------------");

    // Motor B = velocidade - curva
    dados = 20000 + vel_MC - vel_Turn;
    radio.write(&dados, sizeof(dados)); // Envia dados via NRF24L01

    // Verificação
    Serial.print("Transmitted msg : ");
    Serial.println(dados);
    Serial.print("msg_B (valor): ");
    Serial.println(dados - 20000);
    Serial.println("----------------------");

    // Motor C = movimento lateral
    dados = 30000 + vel_C;
    radio.write(&dados, sizeof(dados)); // Envia dados via NRF24L01

    // Verificação
    Serial.print("Transmitted msg : ");
    Serial.println(dados);
    Serial.print("Motor C: ");
    Serial.println(vel_C);
    Serial.println("----------------------");
  }

  // Fazer Manualmente o controlo dos motores
  if (Serial.available()) 
  {

    msg = Serial.read();

    if (msg == 'A') 
    {

      NBytes = Serial.readBytesUntil('\r\n', buffer, 5);

      dados = 10000 + atoi(buffer);

      radio.write(&dados, sizeof(dados));  // Sending data over NRF 24L01
      Serial.print("Transmitted msg : ");
      Serial.println(dados);
      Serial.print("msg_A (valor1): ");
      Serial.println(dados - 10000);
      Serial.println("----------------------");
    }

    if (msg == 'B') 
    {

      NBytes = Serial.readBytesUntil('\r\n', buffer, 5);
      dados = 20000 + atoi(buffer);

      radio.write(&dados, sizeof(dados));  // Sending data over NRF 24L01
      Serial.print("Transmitted msg : ");
      Serial.println(dados);
      Serial.print("msg_B (valor2): ");
      Serial.println(dados - 20000);
      Serial.println("----------------------");
    }

    // Fazer um delay
    delay(450);
  }
}
