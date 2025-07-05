"""
Sistema de Deteção e Rastreamento de Boias Vermelhas

Este programa foi desenvolvido para um Raspberry Pi 5 e destina-se à deteção
de objetos vermelhos circulares (boias) em tempo real usando a câmara. 
Quando uma boia é detetada, as suas coordenadas são enviadas para um Arduino
através de comunicação série (UART via pinos RX/TX).

"""

import cv2
import numpy as np
import serial
import time
from picamera2 import Picamera2


class RastreadorBoias:
    """
    Classe principal para deteção e rastreamento de boias vermelhas.
    
    Esta classe gere a captura de vídeo, processamento de imagem,
    deteção de objetos vermelhos e comunicação série com Arduino.
    """
    
    def __init__(self, porta_serial='/dev/ttyAMA0', baudrate=9600, 
                 resolucao=(1280, 720), limiar_area=600, limiar_circ=(0.75, 1.0)):
        """
        Inicializa o rastreador de boias.
        
        Args:
            porta_serial (str): Caminho para a porta série UART.
            baudrate (int): Taxa de transmissão da comunicação série.
            resolucao (tuple): Resolução da câmara (largura, altura).
            limiar_area (int): Área mínima (em píxeis) para considerar um contorno.
            limiar_circ (tuple): Intervalo de circularidade para identificar boias (min, max).
        """
        # Inicialização da câmara
        self.camera = Picamera2()
        self.camera.configure(self.camera.create_preview_configuration(
            main={"format": 'XRGB8888', "size": resolucao}
        ))
        
        '''Generalizado (Muito bom com pouca luminosidade)'''
        # Intervalos HSV para detetar cor vermelha (dois intervalos devido à natureza circular do espaço HSV)
        self.lower_red1 = np.array([0, 150, 100])     # Limite inferior do primeiro intervalo vermelho
        self.upper_red1 = np.array([10, 255, 255])    # Limite superior do primeiro intervalo vermelho
        self.lower_red2 = np.array([150, 110, 0])     # Limite inferior do segundo intervalo vermelho
        self.upper_red2 = np.array([179, 255, 255])   # Limite superior do segundo intervalo vermelho
    
        
        ''' Calibrado (Muito bom com boa luminosdade)
        # Intervalos HSV para detetar cor vermelha (dois intervalos devido à natureza circular do espaço HSV)
        self.lower_red1 = np.array([179, 255, 108])     # Limite inferior do primeiro intervalo vermelho
        self.upper_red1 = np.array([179, 125, 154])    # Limite superior do primeiro intervalo vermelho
        self.lower_red2 = np.array([150, 175, 0])     # Limite inferior do segundo intervalo vermelho
        self.upper_red2 = np.array([179, 255, 255])   # Limite superior do segundo intervalo vermelho
        '''
        
        # Definições para processamento de imagem
        self.fonte = cv2.FONT_HERSHEY_SIMPLEX        # Tipo de fonte para texto
        self.limiar_area = limiar_area               # Área mínima para considerar um objeto
        self.limiar_circularidade = limiar_circ      # Intervalo de circularidade para boias
        
        # Conexão série com Arduino
        self.arduino_serial = None
        self.porta_serial = porta_serial
        self.baudrate = baudrate
        self._iniciar_conexao_serial()
        
    def _iniciar_conexao_serial(self):
        """Estabelece conexão série com o Arduino."""
        try:
            self.arduino_serial = serial.Serial(self.porta_serial, self.baudrate, timeout=1)
            # Aguarda a inicialização do Arduino
            time.sleep(2)
            print(f"Conexão série estabelecida em {self.porta_serial} a {self.baudrate} bps")
        except serial.SerialException as e:
            print(f"Erro ao abrir a porta série {self.porta_serial}: {e}")
            self.arduino_serial = None
            
    def iniciar(self):
        """Inicia a câmara e prepara o sistema para rastreamento."""
        self.camera.start()
        print("Câmara inicializada com sucesso")
        
    def encerrar(self):
        """Liberta recursos e encerra as conexões."""
        self.camera.close()
        cv2.destroyAllWindows()
        if self.arduino_serial and self.arduino_serial.is_open:
            self.arduino_serial.close()
            print("Conexão série encerrada")
        print("Sistema encerrado com sucesso")
        
    def calcular_circularidade(self, contorno):
        """
        Calcula a circularidade de um contorno.
        
        A circularidade é uma medida de quão próximo um contorno está de um círculo perfeito.
        Um círculo perfeito tem circularidade = 1.0.
        
        Args:
            contorno: O contorno para calcular a circularidade.
            
        Returns:
            float: Valor de circularidade (entre 0 e 1).
        """
        perimetro = cv2.arcLength(contorno, True)
        area = cv2.contourArea(contorno)
        
        # Evitar divisão por zero
        if perimetro == 0 or area == 0:
            return 0
            
        # Fórmula da circularidade: 4π × (área/perímetro²)
        circularidade = 4 * np.pi * (area / (perimetro ** 2))
        return circularidade
        
    def detectar_objetos(self, frame):
        """
        Deteta objetos vermelhos no quadro da câmara.
        
        Args:
            frame: Imagem capturada da câmara.
            
        Returns:
            list: Lista de dicionários contendo informações sobre os objetos detetados.
        """
        # Converter imagem para espaço de cor HSV (matiz, saturação, valor)
        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
        
        # Criar máscaras para deteção da cor vermelha (em dois intervalos)
        mask1 = cv2.inRange(hsv, self.lower_red1, self.upper_red1)
        mask2 = cv2.inRange(hsv, self.lower_red2, self.upper_red2)
        mask = cv2.bitwise_or(mask1, mask2)  # Combinar as duas máscaras
        
        # Operações morfológicas para remover ruído e melhorar a deteção
        kernel = np.ones((5, 5), np.uint8)
        mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)   # Abertura: remoção de ruído
        mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)  # Fecho: preencher lacunas
        
        # Encontrar contornos na máscara
        contornos, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        
        objetos = []
        for i, contorno in enumerate(contornos):
            area = cv2.contourArea(contorno)
            
            # Ignorar contornos muito pequenos (provavelmente ruído)
            if area < self.limiar_area:
                continue
                
            # Calcular o centroide usando momentos
            M = cv2.moments(contorno)
            if M["m00"] == 0:  # Evitar divisão por zero
                continue
                
            cx = int(M["m10"] / M["m00"])  # Coordenada X do centroide
            cy = int(M["m01"] / M["m00"])  # Coordenada Y do centroide
            
            # Calcular circularidade para determinar se é uma boia
            circ = self.calcular_circularidade(contorno)
            
            # Adicionar objeto à lista
            objetos.append({
                "nome": f"Objeto {i+1}",
                "centroide": (cx, cy),
                "circularidade": circ,
                "contorno": contorno,
                "area": area
            })
            
        return objetos
        
    def desenhar_objetos(self, frame, objetos):
        """
        Desenha contornos, centróides e informações dos objetos na imagem.
        
        Args:
            frame: Quadro da câmara onde desenhar.
            objetos: Lista de objetos detetados.
        """
        for obj in objetos:
            contorno = obj["contorno"]
            cx, cy = obj["centroide"]
            circ = obj["circularidade"]
            nome = obj["nome"]
            area = obj["area"]
            
            # Preparar texto informativo
            texto = f"{nome}: ({cx},{cy}) C={circ:.2f} A={area:.0f}"
            
            # Desenhar contorno em verde
            cv2.drawContours(frame, [contorno], -1, (0, 255, 0), 2)
            
            # Marcar o centróide com um círculo azul
            cv2.circle(frame, (cx, cy), 4, (255, 0, 0), -1)
            
            # Adicionar texto com sombra para melhor leitura
            cv2.putText(frame, texto, (cx + 10, cy),
                        self.fonte, 0.8, (0, 0, 0), 6, cv2.LINE_AA)  # Sombra preta
            cv2.putText(frame, texto, (cx + 10, cy),
                        self.fonte, 0.8, (255, 255, 255), 2, cv2.LINE_AA)  # Texto branco
                        
    def enviar_serial(self, objetos):
        """
        Envia as coordenadas dos objetos circulares para o Arduino via série.
        
        Args:
            objetos: Lista de objetos detetados.
        """
        for obj in objetos:
            circ = obj["circularidade"]
            min_circ, max_circ = self.limiar_circularidade
            
            # Verificar se o objeto tem circularidade dentro do intervalo para boias
            if min_circ <= circ <= max_circ:
                cx, cy = obj["centroide"]
                mensagem = f"{cx},{cy}\n"
                
                print(f"A enviar: {mensagem.strip()}")
                
                # Enviar dados se a conexão estiver disponível
                if self.arduino_serial and self.arduino_serial.is_open:
                    try:
                        self.arduino_serial.write(mensagem.encode())
                    except Exception as e:
                        print(f"Erro ao enviar dados: {e}")
                        
    def executar(self):
        """
        Método principal para executar o ciclo de rastreamento em tempo real.
        """
        print("A iniciar rastreamento. Prima 'q' para sair.")
        
        # Criar e configurar janela de visualização
        nome_janela = "Deteção de Boias Vermelhas"
        cv2.namedWindow(nome_janela, cv2.WINDOW_NORMAL)
        cv2.resizeWindow(nome_janela, 960, 540)
        
        try:
            while True:
                # Capturar quadro da câmara
                frame = self.camera.capture_array()
                
                # Processar imagem e detetar objetos
                objetos = self.detectar_objetos(frame)
                
                # Visualizar resultados
                self.desenhar_objetos(frame, objetos)
                
                # Enviar coordenadas para o Arduino
                self.enviar_serial(objetos)
                
                # Mostrar imagem processada
                cv2.imshow(nome_janela, frame)
                
                # Verificar se a tecla 'q' foi pressionada para sair
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    break
                    
        except KeyboardInterrupt:
            print("\nInterrompido pelo utilizador")
        except Exception as e:
            print(f"Erro durante a execução: {e}")
        finally:
            self.encerrar()


if __name__ == "__main__":
    # Parâmetros de configuração
    CONFIGURACAO = {
        'porta_serial': '/dev/ttyAMA0',  # Porta UART do Raspberry Pi 5
        'baudrate': 9600,                # Taxa de comunicação com o Arduino
        'resolucao': (1280, 720),        # Resolução da câmara
        'limiar_area': 600,              # Área mínima de um objeto válido
        'limiar_circ': (0.75, 1.0)       # Circularidade mínima e máxima para boias
    }
    
    # Criar e iniciar o rastreador
    rastreador = RastreadorBoias(**CONFIGURACAO)
    rastreador.iniciar()
    
    # Executar o ciclo principal
    rastreador.executar()
