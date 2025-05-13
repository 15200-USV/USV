//// Adicionar Bibliotecas
#include <SPI.h>      // para gerir a interface de comunicação com o modem
#include <nRF24L01.h> // para controlar este driver de modem específico
#include <RF24.h>     // biblioteca que nos ajuda a controlar o modem de rádio
#include <Servo.h>

// Objetos e Variáveis
Servo motorA, motorB, motorC;
int pwmPinA = 7, pwmPinB = 6, pwmPinC = 5; // Definir o valor para pwmPinC quando souberes a porta
int pwmVal = 1500;                         // Valor neutro do servo
const int ledPinA = 47, ledPinB = 46;      // LEDs para mostrar se está em modo automático ou não

// Rádio
RF24 radio(9, 10);                         // Criar instância 'radio' ( CE , CSN )   CE -> D9 | CSN -> D10
const byte Address[6] = "00001";           // Endereço do qual os dados serão recebidos
int dados;                                 // Variável que vai receber os dados transmitidos
int valor = 0;                             // Variável que recebe o número para executar as ações

void setup() 
{
  Serial.begin(9600);

  radio.begin();                           // Inicializar o rádio
  radio.openReadingPipe(1, Address);       // Configurar o canal de receção
  radio.setPALevel(RF24_PA_MIN);           // Definir a potência da transmissão
  radio.startListening();                  // Colocar o rádio em modo de escuta

  // Inicializar os LEDs
  pinMode(ledPinA, OUTPUT);
  pinMode(ledPinB, OUTPUT);

  // Inicializar o motor A
  motorA.attach(pwmPinA);
  motorA.writeMicroseconds(pwmVal);

  // Inicializar o motor B
  motorB.attach(pwmPinB);
  motorB.writeMicroseconds(pwmVal);

  // Inicializar o motor C
  motorC.attach(pwmPinC);
  motorC.writeMicroseconds(pwmVal);
}

void loop() 
{
  if (radio.available()) 
  {

    radio.read(&dados, sizeof(dados));

    Serial.print("Mensagem recebida: "); 
    Serial.println(dados);

    switch (dados / 10000) 
    {
      case 1:  // Caso seja o motor A (definir a posição)
        
        valor = dados % 10000;  // O resto da divisão inteira dá o valor a extrair (ou "valor = dados - 10000")
        motorA.writeMicroseconds(valor);  // Enviar a velocidade para o motor

        // Verificação do valor recebido
        Serial.print("msg_A (valor1): ");
        Serial.println(valor);
        Serial.println("------------------");
        break;

      case 2:  // Caso seja o motor B (definir a posição)
        
        valor = dados % 20000;  // O resto da divisão inteira dá o valor a extrair (ou "valor = dados - 20000")
        motorB.writeMicroseconds(valor);  // Enviar a velocidade para o motor

        // Verificação do valor recebido
        Serial.print("msg_B (valor2): ");
        Serial.println(valor);
        Serial.println("------------------");
        break;

      case 3:  // Caso seja o motor C (definir a posição)
        
        valor = dados % 30000;  // O resto da divisão inteira dá o valor a extrair (ou "valor = dados - 30000")
        motorC.writeMicroseconds(valor);  // Enviar a velocidade para o motor

        // Verificação do valor recebido
        Serial.print("msg_C (valor3): ");
        Serial.println(valor);
        Serial.println("------------------");
        break;

      case 4:  // Mostrar se está em modo controlo (ON) ou não (OFF)
        
        valor = dados % 40000;
        if (valor == 100) 
        {
          digitalWrite(ledPinA, HIGH);  // Acende o LED A
          digitalWrite(ledPinB, LOW);   // Apaga o LED B
        } 
        else 
        {
          digitalWrite(ledPinA, LOW);   // Apaga o LED A
          digitalWrite(ledPinB, HIGH);  // Acende o LED B
        }
        break;
        
      default:
        // Nenhuma ação definida
        break;
    }
  }
}
