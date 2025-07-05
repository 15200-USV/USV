/**
 * Sistema de Controlo para Barco Autónomo
 * 
 * Este programa permite controlar um barco autónomo com dois modos de operação:
 * - Modo Manual: Controlo através de comando sem fios (RF24) - 3 motores
 * - Modo Automático: Navegação baseada na deteção visual de boias (dados da Raspberry Pi) - 2 motores
 * 
 * A imagem processada pela Raspberry Pi tem uma resolução de 1280x720 píxeis.
 * A Raspberry envia as coordenadas do centróide (cx,cy) e massa da boia detetada.
 * 
 */

#include <Servo.h>        // Biblioteca para controlo de servomotores
#include <SPI.h>          // Comunicação SPI para o módulo RF24
#include <nRF24L01.h>     // Definições para o módulo NRF24L01
#include <RF24.h>         // Funcionalidade principal do módulo RF24

//---- DEFINIÇÃO DE CONSTANTES ----//

// Pinos e configuração do módulo RF24
#define PINO_CE 9
#define PINO_CSN 10
const byte ENDERECO_RF24[6] = "00001";

// Pinos para os motores (servos)
#define PINO_MOTOR_A 7
#define PINO_MOTOR_B 6
#define PINO_MOTOR_C 5

// Pinos para LED RGB
#define PINO_LED_AMARELO 47  // Componente amarelo do LED RGB
#define PINO_LED_VERDE 46  // Componente verde do LED RGB

// Valores para modos de operação
#define MODO_MANUAL 0
#define MODO_AUTOMATICO 1

// Comandos recebidos por RF24
#define CMD_MOTOR_A 1       // Prefixo para comandos do motor A
#define CMD_MOTOR_B 2       // Prefixo para comandos do motor B
#define CMD_MOTOR_C 3       // Prefixo para comandos do motor C
#define CMD_MODO 4          // Prefixo para mudança de modo (mudou de 3 para 4)
#define VAL_MODO_MANUAL 100 // Valor para selecionar modo manual
#define VAL_MODO_AUTO 200   // Valor para selecionar modo automático

// Parâmetros para controlo automático
#define CENTRO_IMAGEM_X 640     // Centro horizontal da imagem (1280x720)
#define PWM_CENTRO 1500         // PWM neutro para os motores
#define TOLERANCIA_CENTRADO 40  // Tolerância para considerar a boia centrada (em píxeis)
#define Y_MINIMO_PROXIMO 650    // Valor mínimo de Y para considerar boia próxima

// Parâmetros para controlo proporcional
#define GANHO_KP 0.8            // Ganho proporcional para o controlador

// Tempo para piscar o LED (em milissegundos)
#define INTERVALO_PISCAR 500

//---- OBJETOS GLOBAIS ----//
RF24 radio(PINO_CE, PINO_CSN);  // Cria objeto para comunicação RF
Servo motorA;                   // Servo para o motor A
Servo motorB;                   // Servo para o motor B
Servo motorC;                   // Servo para o motor C

//---- VARIÁVEIS GLOBAIS ----//

// Variáveis para comunicação RF
uint16_t dados = 0;                  // Dados recebidos pelo RF24
int valor = 0;                  // Valor extraído dos dados recebidos
bool erro_detetado = false;

// Controlo de modo de operação
int modo = MODO_MANUAL;         // Modo atual (inicia em manual)
int modo_anterior = -1;         // Controlo para detetar mudanças de modo

// Controlo de LED piscante
unsigned long ultima_piscada = 0;
bool estado_led = false;

// Variáveis para dados da Raspberry Pi
String dados_raspberry = "";    // String de dados recebidos
int cx = 0;                     // Coordenada X do centróide
int cy = 0;                     // Coordenada Y do centróide
int massa = 0;                  // Massa do objeto detetado

// Variáveis para controlo proporcional
int erro = 0;                   // Erro entre posição atual e desejada
int termo_p = 0;                // Termo proporcional
float Kp = GANHO_KP;            // Ganho do controlador proporcional
bool modo_posicionamento = false;

// Valores PWM para os motores
int pwm_a = PWM_CENTRO;
int pwm_b = PWM_CENTRO;
int pwm_c = PWM_CENTRO;

/**
 * Configura o hardware e inicializa os componentes do sistema
 */
void setup() {
  // Inicializa comunicações seriais
  Serial.begin(9600);           // Para depuração
  Serial3.begin(9600);          // Para comunicação com a Raspberry Pi
  
  // Configura o módulo de rádio RF24
  radio.begin();
  radio.openReadingPipe(0, ENDERECO_RF24);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();

  // Configura os servomotores
  motorA.attach(PINO_MOTOR_A);
  motorB.attach(PINO_MOTOR_B);
  motorC.attach(PINO_MOTOR_C);
  
  // Inicializa os servos na posição neutra
  motorA.writeMicroseconds(PWM_CENTRO);
  motorB.writeMicroseconds(PWM_CENTRO);
  motorC.writeMicroseconds(PWM_CENTRO);

  // Configura os pinos do LED RGB
  pinMode(PINO_LED_AMARELO, OUTPUT);
  pinMode(PINO_LED_VERDE, OUTPUT);
  
  // Inicia com LED amarelo (modo manual)
  definir_led_amarelo();

  Serial.println(">> Sistema iniciado com 3 motores. À espera de dados...");
}

/**
 * Processa os comandos de mudança de modo recebidos pelo RF24
 * @param valor O valor do comando recebido
 * @return true se o modo for válido, false caso contrário
 */
bool processar_mudanca_modo(int valor) {
  bool modo_valido = true;
  
  // Determina o novo modo com base no valor recebido
  if (valor == VAL_MODO_MANUAL) {
    modo = MODO_MANUAL;
    erro_detetado = false;
  }
  else if (valor == VAL_MODO_AUTO) {
    modo = MODO_AUTOMATICO;
    erro_detetado = false;
  }
  else {
    modo_valido = false;
    erro_detetado = true;
  }

  // Atualiza indicadores e notifica mudança, apenas se o modo alterou
  if (modo != modo_anterior) {
    if (modo == MODO_MANUAL) {
      Serial.println(">> Comando: MODO MANUAL");
      definir_led_amarelo();
    } 
    else if (modo == MODO_AUTOMATICO) {
      Serial.println(">> Comando: MODO AUTOMÁTICO");
      definir_led_verde();
      // Para o motor C quando muda para modo automático
      motorC.writeMicroseconds(PWM_CENTRO);
      Serial.println(">> Motor C parado (modo automático)");
    }
    modo_anterior = modo;
  }
  
  return modo_valido;
}

/**
 * Define o LED RGB para cor amarela
 */
void definir_led_amarelo() {
  digitalWrite(PINO_LED_AMARELO, HIGH);
  digitalWrite(PINO_LED_VERDE, LOW);
}

/**
 * Define o LED RGB para cor verde
 */
void definir_led_verde() {
  digitalWrite(PINO_LED_AMARELO, LOW);
  digitalWrite(PINO_LED_VERDE, HIGH);
}

/**
 * Faz o LED RGB piscar em amarelo para indicar erro
 */
void piscar_led_amarelo() {
  unsigned long agora = millis();
  if (agora - ultima_piscada >= INTERVALO_PISCAR) {
    ultima_piscada = agora;
    estado_led = !estado_led;
    
    if (estado_led) {
      definir_led_amarelo();
    } else {
      digitalWrite(PINO_LED_AMARELO, LOW);
      digitalWrite(PINO_LED_VERDE, LOW);
    }
  }
}

/**
 * Processa os comandos para o modo manual recebidos pelo RF24
 * @param comando O identificador do comando (1 para motor A, 2 para motor B, 3 para motor C)
 * @param valor O valor PWM a aplicar ao motor
 */
void processar_comando_manual(int comando, int valor) {
  if (comando == CMD_MOTOR_A) {
    motorA.writeMicroseconds(valor);
    Serial.print(">> PWM motor A: ");
    Serial.println(valor);
  }
  else if (comando == CMD_MOTOR_B) {
    motorB.writeMicroseconds(valor);
    Serial.print(">> PWM motor B: ");
    Serial.println(valor);
  }
  else if (comando == CMD_MOTOR_C) {
    motorC.writeMicroseconds(valor);
    Serial.print(">> PWM motor C: ");
    Serial.println(valor);
  }
}

/**
 * Extrai os dados da string recebida da Raspberry Pi
 * @param dados_str A string recebida no formato "cx,cy,massa"
 * @return true se os dados foram extraídos com sucesso, false caso contrário
 */
bool extrair_dados_raspberry(String dados_str) {
  dados_str.trim();  // remove espaços e \r

  int separador = dados_str.indexOf(',');

  if (separador > 0 && separador < dados_str.length() - 1) {
    // Tem vírgula num lugar válido
    String parte1 = dados_str.substring(0, separador);
    String parte2 = dados_str.substring(separador + 1);

    parte1.trim();
    parte2.trim();

    cx = parte1.toInt();
    cy = parte2.toInt();

    Serial.print("Raspberry - cx: ");
    Serial.print(cx);
    Serial.print(" | cy: ");
    Serial.println(cy);

    return true;

  } else {
    Serial.print(">> ERRO: Formato inválido! Dados recebidos: ");
    Serial.println(dados_str);
    erro_detetado = true;
    return false;
  }
}

/**
 * Calcula os valores PWM para os motores no modo automático
 * com base na posição da boia detetada
 * (Apenas motores A e B são usados no modo automático)
 */
void calcular_controlo_automatico() {
  // Erros brutos
  int erroX = CENTRO_IMAGEM_X - cx;
  int erroY = Y_MINIMO_PROXIMO - cy;

  // Verifica se está centrado
  bool centradoX = abs(erroX) < TOLERANCIA_CENTRADO_X;
  bool centradoY = abs(erroY) < TOLERANCIA_CENTRADO_Y;

  // Define se entra em modo de posicionamento
  if (cy >= Y_MINIMO_PROXIMO && centradoX && centradoY) {
    modo_posicionamento = true;
  } else {
    modo_posicionamento = false;
  }

  // Se não estiver centrado em X, corrige
  if (!centradoX) {
    // mantém erroX
  } else {
    erroX = 0;
  }

  // Se for modo de posicionamento, sempre corrige Y para segurar
  // Se for aproximação, só corrige se fora da tolerância
  if (!modo_posicionamento && centradoY) {
    erroY = 0;
  }

  // Ganhos
  float KpX = modo_posicionamento ? 0.5 : 0.5;
  float KpY = modo_posicionamento ? 0.5 : 0.7;

  // Controlos
  int sinalVirar = erroX * KpX;
  int sinalAvancar = erroY * KpY;

  int motorA_output = PWM_CENTRO + sinalAvancar + sinalVirar;
  int motorB_output = PWM_CENTRO + sinalAvancar - sinalVirar;

  // Limites
  motorA_output = constrain(motorA_output, 1000, 2000);
  motorB_output = constrain(motorB_output, 1000, 2000);

  // Aplica
  motorA.writeMicroseconds(motorA_output);
  motorB.writeMicroseconds(motorB_output);

  // Debug
  Serial.print("MODO: ");
  Serial.print(modo_posicionamento ? "POSICIONAMENTO" : "APROXIMACAO");
  Serial.print(" | erroX: ");
  Serial.print(erroX);
  Serial.print(" | erroY: ");
  Serial.print(erroY);
  Serial.print(" | PWM A: ");
  Serial.print(motorA_output);
  Serial.print(" | PWM B: ");
  Serial.println(motorB_output);

  // delay(500); // Debug
}


/**
 * Função principal do programa, executada continuamente
 */
void loop() {
  // Verifica se há dados disponíveis do módulo RF24
  if (radio.available()) {
    radio.read(&dados, sizeof(dados));
    
    // Extrai o tipo de comando (prefixo dos dados)
    int comando = dados / 10000;
    
    // Processa comando conforme o tipo
    switch (comando) {
      
      case CMD_MODO:
        // Processa comando de mudança de modo (agora é 40000)
        valor = dados % 40000;
        if (!processar_mudanca_modo(valor)) {
          Serial.println(">> Modo inválido recebido. Luz amarela a piscar.");
          erro_detetado = true;
        }
        break;
        
      case CMD_MOTOR_A:
      case CMD_MOTOR_B:
      case CMD_MOTOR_C:
        // Processa comandos de motor (apenas em modo manual)
        if (modo == MODO_MANUAL) {
          valor = dados % (comando * 10000);
          processar_comando_manual(comando, valor);
        }
        break;
        
      default:
        // Comando desconhecido
        Serial.println(">> ERRO: Comando desconhecido recebido!");
        erro_detetado = true;
        break;
    }
  }
  
  // Gestão do LED em caso de erro
  if (erro_detetado) {
    piscar_led_amarelo();
  }
  // Se não há erro, mantém o LED conforme o modo atual
  else {
    if (modo == MODO_MANUAL) {
      definir_led_amarelo();
    } else if (modo == MODO_AUTOMATICO) {
      definir_led_verde();
    }
  }
  
  // Modo automático: processa dados da Raspberry Pi
  if (modo == MODO_AUTOMATICO && !erro_detetado) {
  if (Serial3.available()) {
    String dados = Serial3.readStringUntil('\n');
    dados.trim(); // remove \r e espaços

    if (dados.length() > 0) {
      if (extrair_dados_raspberry(dados)) {
        calcular_controlo_automatico();
      } else {
        // Dado inválido: limpa o buffer residual
        while (Serial3.available()) Serial3.read();
        }
      }
    }
  }
}
