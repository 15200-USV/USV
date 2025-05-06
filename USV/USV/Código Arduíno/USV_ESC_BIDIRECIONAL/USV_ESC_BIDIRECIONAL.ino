#include <Servo.h>  // Biblioteca que permite controlar ESCs/motores via sinais PWM

Servo motorA, motorB;  // Criamos dois objetos Servo para os dois motores

const int pwmPinA = 7;  // Motor A conectado ao pino digital 7\const int pwmPinB = 5; | Motor B conectado ao pino digital 5

int pwmValA = 1500;  // Valor inicial neutro para motor A (parado)
int pwmValB = 1500;  // Valor inicial neutro para motor B (parado)

char msg, buffer[15];  // Variáveis para armazenar o comando ('A' ou 'B') e a mensagem lida
int NBytes;            // Quantidade de caracteres lidos da Serial

void setup() {
  motorA.attach(pwmPinA);  // Liga motor A ao pino 7
  motorB.attach(pwmPinB);  // Liga motor B ao pino 5

  motorA.writeMicroseconds(pwmValA);  // Envia sinal neutro para motor A
  motorB.writeMicroseconds(pwmValB);  // Envia sinal neutro para motor B

  Serial.begin(9600);  // Inicializa a comunicação Serial a 9600 bps

  Serial.println("Motores A e B inicializados no valor neutro (1500 μs)." );
}

void loop() {
  // Se há dados na porta Serial (recebendo comandos)
  if (Serial.available()) {
    msg = Serial.read();  // Lê o primeiro caractere: deve ser 'A' ou 'B'
    
    memset(buffer, 0, sizeof(buffer));  // Limpa o buffer para evitar lixo anterior

    // Lê os próximos caracteres até encontrar '\n' (fim da linha)
    NBytes = Serial.readBytesUntil('\n', buffer, sizeof(buffer) - 1);

    int val = atoi(buffer);       // Converte texto para inteiro
    val = constrain(val, 1000, 2000);  // Garante que valor esteja entre 1000 e 2000

    switch (msg) {
      case 'A':  // Se comando é para motor A
        pwmValA = val;  // Atualiza valor de controle do motor A
        motorA.writeMicroseconds(pwmValA);  // Envia valor para motor A
        Serial.print("Motor A pwmVal: ");
        Serial.println(pwmValA);
        break;

      case 'B':  // Se comando é para motor B
        pwmValB = val;
        motorB.writeMicroseconds(pwmValB);  // Envia valor para motor B
        Serial.print("Motor B pwmVal: ");
        Serial.println(pwmValB);
        break;

      default:
        // Se o caractere não é reconhecido
        Serial.println("Comando inválido. Use 'A' ou 'B'.");
    }
  }
}
