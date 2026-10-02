#include <NewPing.h>

// PRIMEIRA ETAPA: PINOS

// Motores
const int MOT_ESQ_A = 5; // IN1 (lado esquerdo)
const int MOT_ESQ_B = 10; // IN2 (lado esquerdo)
const int MOT_DIR_A = 6; // IN3 (lado direito)
const int MOT_DIR_B = 11; // IN4 (lado direito)

// Se um lado girar ao contrário no TESTE, PRECISA TROCAR para -1
const int INVERTE_ESQ = 1; // 1 = sentido normal, -1 = inverte o lado esquerdo
const int INVERTE_DIR = 1;

// PWM mínimo em que as rodas realmente começam a girar.
const int VEL_MINIMA = 1; // PRECISA TESTAR qual a velocidade mínima

// LED de derrota
const int Led = 2;

// Sensores Infravermelhos
// HIGH = sensor vendo fora da arena
const int sensor1 = 13; // primeiro IR
const int sensor2 = 12; // segundo IR

int est1 = LOW; // memória: o sensor 1 já esteve fora da arena? (LOW = não)
int est2 = LOW; // memória: o sensor 2 já esteve fora da arena?

int lerIF1 = LOW; // guarda a leitura atual do sensores
int lerIF2 = LOW; 

int luta = HIGH; // HIGH = a luta continua, LOW = o sumô perdeu :/

// Sensor Ultrassônico
#define TRIG_PIN 22 // pino que dispara o pulso de som
#define ECHO_PIN 23 // pino que recebe o eco
#define DISTANCIA_MAXIMA 200 // distância máxima

NewPing sonar(TRIG_PIN, ECHO_PIN, DISTANCIA_MAXIMA);   // cria o objeto "sonar" com esses pinos e limite

// Deixe false enquanto o sensor não estiver ligado. Quando estiver, troque para true.
const bool TEM_ULTRASSONICO = false;   // chave que liga/desliga o uso do ultrassônico

unsigned long ultimoTempoMedicao = 0; // quando foi a última medição
const unsigned long INTERVALO_MEDICAO = 50; // só mede de novo depois de 50 ms

float distanciaAtual = DISTANCIA_MAXIMA; // última distância medida

const int DIST_ATAQUE = 35; // oponente a menos de 35 cm

// Velocidades e limites
const int VEL_ATAQUE = 255; // velocidade ao atacar (máxima)
const int VEL_FRENTE = 200; // velocidade de avanço da estratégia 1
const int VEL_GIRO = 150; // velocidade ao girar procurando o oponente

const unsigned long TEMPO_RECUO = 300; // ms dando ré ao ver a borda
const unsigned long TEMPO_GIRO_BORDA = 250; // ms girando depois de recuar
const unsigned long TEMPO_DESVIO_GIRO = 250; // estratégia 2: ms girando no início
const unsigned long TEMPO_DESVIO_FRENTE = 400; // estratégia 2: ms avançando depois do giro

// ETAPA DOIS: ESTRATÉGIAS
int estrategia = 1; // qual estratégia o robô vai usar

unsigned long tempoInicio = 0; // guarda o momento em que a luta começou

// DEBUG (podemos apagar depois de TESTAR)
const bool DEBUG = true; // true = imprime leituras no computador
unsigned long ultimoTempoPrint = 0; // quando foi o último print
const unsigned long INTERVALO_PRINT = 200; // imprime a cada 200 ms

// ETAPA TRÊS: MOTORES 

// AJUSTE DE VELOCIDADE
int ajustarVel(int valor_pedido) { // recebe a velocidade pedida e devolve a ajustada
  int vel = constrain(valor_pedido, -255, 255); // limita o valor entre -255 e 255

  if (vel == 0) { // se pediram parado...
    return 0; // ...devolve 0 e encerra a função
  }

  int sinal; // vai guardar o sentido: 1 (frente) ou -1 (ré)
  if (vel > 0) { // se for positivo...
    sinal = 1; // ...sentido é frente
  } 
  
  else { // senão (negativo)...
    sinal = -1; // ...sentido é ré
  }

  int modulo = map(abs(vel), 1, 255, VEL_MINIMA, 255); // tira o sinal e converte para a faixa VEL_MINIMA–255

  return sinal * modulo; // devolve intensidade ajustada com o sentido de volta
}

// MOTOR ESQUERDO
void motorEsquerdo(int vel) { // controla o lado esquerdo (-255 a 255)
  vel = ajustarVel(vel) * INVERTE_ESQ; // ajusta a velocidade e aplica a inversão, se houver
  if (vel > 0) { // positivo = frente
    analogWrite(MOT_ESQ_A, vel); // pino A recebe PWM com a intensidade
    analogWrite(MOT_ESQ_B, 0); // pino B fica desligado
  } 
  
  else if (vel < 0) { // negativo = ré
    analogWrite(MOT_ESQ_A, 0); // pino A desligado
    analogWrite(MOT_ESQ_B, -vel); // pino B recebe PWM (o "-" tira o sinal negativo)
  } 
  
  else { // zero = parado
    analogWrite(MOT_ESQ_A, 0); // os dois pinos desligados
    analogWrite(MOT_ESQ_B, 0);
  }
}

// MOTOR DIREITO
void motorDireito(int vel) { // controla o lado direito (igual ao esquerdo)
  vel = ajustarVel(vel) * INVERTE_DIR; // ajusta a velocidade e aplica a inversão direita
  if (vel > 0) { // positivo = frente
    analogWrite(MOT_DIR_A, vel); // pino A com PWM
    analogWrite(MOT_DIR_B, 0); // pino B desligado
  } 
  
  else if (vel < 0) { // negativo = ré
    analogWrite(MOT_DIR_A, 0); // pino A desligado
    analogWrite(MOT_DIR_B, -vel); // pino B com PWM
  } 
  
  else { // zero = parado
    analogWrite(MOT_DIR_A, 0); // os dois pinos desligados
    analogWrite(MOT_DIR_B, 0);
  }
}

// AMBOS OS MOTORES
void motores(int esq, int dir) { // comanda os dois lados de uma vez
  motorEsquerdo(esq); // manda a velocidade do lado esquerdo
  motorDireito(dir); // manda a velocidade do lado direito
}

// PARAR
void parar() {
  motores(0, 0); // os dois lados em 0: motores soltos
}

// FRENTE
void frente(int vel) {
  motores(vel, vel); // os dois lados para frente, mesma velocidade
}

// RÉ
void tras(int vel) {
  motores(-vel, -vel); // os dois lados para trás
}

// GIRO ESQUERDA
void girarEsquerda(int vel) {
  motores(-vel, vel); // esquerdo para trás, direito para frente: gira no lugar
}

// GIRO DIREITA
void girarDireita(int vel) {
  motores(vel, -vel); // esquerdo para frente, direito para trás: gira no lugar
}

// CURVA ESQUERDA
void curvaEsquerda(int vel) {
  motores(vel / 2, vel); // esquerdo na metade da velocidade: avança curvando à esquerda
}

// CURVA DIREITA
void curvaDireita(int vel) {
  motores(vel, vel / 2); // direito na metade da velocidade: avança curvando à direita
}

// FREIO
void freio() {
  digitalWrite(MOT_ESQ_A, HIGH); // os quatro pinos em HIGH travam os motores
  digitalWrite(MOT_ESQ_B, HIGH);
  digitalWrite(MOT_DIR_A, HIGH);
  digitalWrite(MOT_DIR_B, HIGH);
}

// CONFIGURAÇÃO DE INICIALIZAÇÃO
void motoresIniciar() {
  pinMode(MOT_ESQ_A, OUTPUT);
  pinMode(MOT_ESQ_B, OUTPUT);
  pinMode(MOT_DIR_A, OUTPUT);
  pinMode(MOT_DIR_B, OUTPUT);
  parar(); // garante que o robô começa parado
}

// SENSORES INFRAVERMELHOS
// INICIALIZAÇÃO
void sensoresIniciar() {
  pinMode(sensor1, INPUT); // IR esquerdo é entrada (lê o sensor)
  pinMode(sensor2, INPUT); // IR direito é entrada
  pinMode(Led, OUTPUT); // LED de derrota é saída
}

// SENSOR 1
bool foraEsquerda() {
  return digitalRead(sensor1) == HIGH; // true se o IR esquerdo está vendo fora da arena
}

// SENSOR 2
bool foraDireita() {
  return digitalRead(sensor2) == HIGH; // true se o IR direito está vendo fora da arena
}

// Confere derrota
int confere(int infra1, int infra2) { // recebe as leituras; devolve HIGH (continua) ou LOW (perdeu)
  if (infra1 == LOW && infra2 == LOW) { // os dois sensores dentro da arena...
    est1 = LOW; // ...zera a memória do sensor 1
    est2 = LOW; // ...zera a memória do sensor 2
    return HIGH; // a luta continua
  }

  // confere leitura do primeiro sensor
  if (infra1 == HIGH) {  // sensor 1 está fora da arena
    if (est1 == LOW) { // e ainda não estava marcado como fora
      est1 = HIGH; // marca que o sensor 1 saiu
      if (est2 == HIGH) { // se o sensor 2 também já tinha saído...
        return LOW; // ...os dois saíram: derrota
      }
    }
  }

  // confere leitura do segundo sensor
  if (infra2 == HIGH) {                   
    if (est2 == LOW) {                                   
      est2 = HIGH;                                    
      if (est1 == HIGH) {                                
        return LOW;                                      
      }
    }
  }
  return HIGH; // nenhum caso de derrota: a luta continua
}

// Derrota: para os motores e pisca o LED para sempre
void derrota() {
  parar(); // para os motores antes de tudo
  while (1 == 1) { // laço infinito
    // pisca pisca :D
    digitalWrite(Led, HIGH); 
    delay(500); 
    digitalWrite(Led, LOW); 
    delay(500); 
  }
}


// SENSOR ULTRASSÔNICO
// Atualiza a medida a cada INTERVALO_MEDICAO. Devolve true se mediu agora.
bool lerDistancia() {
  if (!TEM_ULTRASSONICO) { // se o sensor está desligado na chave...
    return false; // ...não mede nada
  }

  unsigned long tempoAtual = millis(); // lê o relógio do Arduino

  if (tempoAtual - ultimoTempoMedicao >= INTERVALO_MEDICAO) { // já passaram 50 ms desde a última medição?
    ultimoTempoMedicao = tempoAtual; // anota o momento desta medição

    unsigned int distancia = sonar.ping_cm(); // mede a distância em cm (0 se não voltou eco)

    if (distancia == 0) { // sem eco = nada dentro do alcance
      distanciaAtual = DISTANCIA_MAXIMA; // considera "bem longe"
    } else { // houve eco
      distanciaAtual = distancia; // guarda a distância medida
    }

    return true; // avisa que fez uma medição nova
  }

  return false; // ainda não deu o intervalo: não mediu
}

float getDistancia() {
  return distanciaAtual; // devolve a última distância guardada
}

// true se há algo perto o bastante para atacar
bool oponenteVisto() {
  lerDistancia(); // atualiza a medida se já passou o intervalo
  return getDistancia() < DIST_ATAQUE; // true se a distância é menor que 35 cm
}

// TRATAMENTO DA BORDA DO SUMÔ
// Se algum IR viu a borda, recua e gira para longe dela.
// Devolve true se tratou a borda
bool tratarBorda() {
  bool esq = foraEsquerda(); // o IR esquerdo está vendo a borda?
  bool dir = foraDireita(); // o IR direito está vendo a borda?

  if (!esq && !dir) { // nenhum dos dois viu a borda...
    return false; // ...não faz nada e avisa que não tratou
  }

  tras(255); // dá ré na velocidade máxima
  delay(TEMPO_RECUO); // continua em ré por 300 ms

  if (esq) { // se foi o sensor esquerdo que viu...
    girarDireita(255); // ...foge girando para a direita
  } else { // senão (foi o direito)...
    girarEsquerda(255); // ...foge girando para a esquerda
  }
  delay(TEMPO_GIRO_BORDA); // continua girando por 250 ms

  return true; // avisa que tratou a borda
}

// ESTRATÉGIAS (3)
// 1) Segue para frente o tempo todo
void estrategia1() {
  frente(VEL_FRENTE);
}

// Parte comum de 2 e 3: gira procurando e ataca quando vê
void procurarGirando() {
  if (oponenteVisto()) { // tem algo a menos de 35 cm?
    frente(VEL_ATAQUE); // ataca em velocidade máxima
  } else { // não vê nada
    girarDireita(VEL_GIRO); // continua girando para procurar
  }
}

// 2) Desvia para um lado e depois procura o oponente girando
void estrategia2() {
  unsigned long passou = millis() - tempoInicio; // quanto tempo passou desde o início da luta

  if (passou < TEMPO_DESVIO_GIRO) { // primeiros 250 ms...
    girarDireita(200); // ...desvia girando para a direita
  } else if (passou < TEMPO_DESVIO_GIRO + TEMPO_DESVIO_FRENTE) { // depois, por mais 400 ms...
    frente(200); // ...avança
  } else { // passado esse tempo todo...
    procurarGirando(); // ...procura o oponente girando
  }
}

// 3) Gira no lugar procurando o oponente
void estrategia3() {
  procurarGirando(); // só gira e ataca quando vê
}

// O DEBUG do sensor ultrassônico
void depurar() {
  unsigned long tempoAtual = millis(); // lê o relógio

  if (tempoAtual - ultimoTempoPrint >= INTERVALO_PRINT) { // já passaram 200 ms desde o último print?
    ultimoTempoPrint = tempoAtual; // anota o momento deste print

    Serial.print("IR esq: "); 
    Serial.print(digitalRead(sensor1)); 
    Serial.print(" | IR dir: ");
    Serial.print(digitalRead(sensor2)); 
    Serial.print(" | distancia: ");
    Serial.print(getDistancia()); 
    Serial.println(" cm");                             
  }
}


void setup() {                                          
  Serial.begin(9600);                                    
  motoresIniciar();                                      
  sensoresIniciar();                                     

  delay(5000);                                          
  tempoInicio = millis(); // início da luta                          
}

void loop() {                                            
  if (DEBUG) { // DEBUG                                       
    depurar();                                       
  }

  // 1º: lê os IR e confere se perdeu
  lerIF1 = digitalRead(sensor1); 
  lerIF2 = digitalRead(sensor2);   
  luta = confere(lerIF1, lerIF2); // derrota?

  if (luta == LOW) {                                     
    derrota();                                          
  }

  // 2º: a borda sempre tem prioridade
  if (tratarBorda()) {                              
    return;                                              
  }

  // 3º: executa a estratégia escolhida
  if (estrategia == 1) {                              
    estrategia1();                                
  } else if (estrategia == 2) {                      
    estrategia2();                                       
  } else if (estrategia == 3) {                          
    estrategia3();                                    
  }
}