/*
(Pai nosso que estais no céu...)
 *  - 2 motores controlados por ponte H (PWM)
 *  - 1 sensor infravermelho (detecta a borda da arena)
 *  - 1 sensor ultrassônico com biblioteca NewPing (detecta o oponente)
 *  - 3 botões para escolher a estratégia antes da luta
 *
 *  Organização do arquivo:
 *    1. Pinos e constantes
 *    2. Variáveis globais
 *    3. Controle dos motores
 *    4. Sensores e botões
 *    5. Comportamentos (borda e estratégias)
 *    6. Depuração
 *    7. setup() e loop() */

#include <NewPing.h>

// TODOS OS PINOS E MOTORES

// # MOTORES #
// Pinos dos motores
const int MOT_ESQ_A = 6;
const int MOT_ESQ_B = 10;
const int MOT_DIR_A = 11;
const int MOT_DIR_B = 9;

// Inversão do sentido de cada motor
const int INVERTE_ESQ = 1;
const int INVERTE_DIR = 1;

// Velocidade mínima (PWM) em que o motor realmente começa a girar
const int VEL_MINIMA = 100;

// # INFRAVERMELHO #
const int sensor1 = 2;

// # BOTÕES #
const int BOTAO_EST1 = 3;
const int BOTAO_EST2 = 4;
const int BOTAO_EST3 = 5;

// Debounce
const int NUM_BOTOES = 3;
const int PINOS_BOTOES[NUM_BOTOES] = {BOTAO_EST1, BOTAO_EST2, BOTAO_EST3};
const unsigned long TEMPO_DEBOUNCE = 50;  // ms (aumente se o botão "tremer")

// # ULTRASSÔNICO
#define TRIG_PIN 12
#define ECHO_PIN 13
#define DISTANCIA_MAXIMA 200                       

const unsigned long INTERVALO_MEDICAO = 50; // ms entre medições
const int DIST_ATAQUE = 35; // cm: abaixo disso, ataca

NewPing sonar(TRIG_PIN, ECHO_PIN, DISTANCIA_MAXIMA);

// VELOCIDADES
const int VEL_ATAQUE = 255;
const int VEL_FRENTE = 200;
const int VEL_GIRO = 150;

// TEMPO DAS MOVIMENTAÇÕES
const unsigned long TEMPO_RECUO = 300; // recuo ao achar a borda
const unsigned long TEMPO_GIRO_BORDA = 250; // giro após o recuo
const unsigned long TEMPO_DESVIO_GIRO = 250; // estratégia 2: giro inicial
const unsigned long TEMPO_DESVIO_FRENTE = 400; // estratégia 2: avanço inicial

// DEBUG (nosso salvador)
const bool DEBUG = true;
const unsigned long INTERVALO_PRINT = 200; // ms entre prints

// OUTRAS VARIÁVEIS
// Estado dos botões (debounce no estilo do exemplo "Debounce" da Arduino)
int estadoBotao[NUM_BOTOES];                    // estado estável atual (HIGH = apertado)
int ultimoEstadoBotao[NUM_BOTOES];              // leitura anterior
unsigned long ultimoTempoDebounce[NUM_BOTOES];  // última vez que a leitura mudou

// Estado do sensor ultrassônico
unsigned long ultimoTempoMedicao = 0;
float distanciaAtual = DISTANCIA_MAXIMA;

// Estado da luta
int estrategia = 1;
unsigned long tempoInicio = 0; // momento em que a luta começou

// Estado da depuração
unsigned long ultimoTempoPrint = 0;


// CONTROLE DOS MOTORES

// # AJUSTE DE VELOCIDADE
int ajustarVel(int valor_pedido) {
  int vel = constrain(valor_pedido, -255, 255);

  if (vel == 0) {
    return 0;
  }

  int sinal;
  if (vel > 0) {
    sinal = 1;
  } else {
    sinal = -1;
  }

  int modulo = map(abs(vel), 1, 255, VEL_MINIMA, 255);

  return sinal * modulo;
}

// # Motor esquerdo: vel > 0 = frente, vel < 0 = trás, 0 = parado
void motorEsquerdo(int vel) {
  vel = ajustarVel(vel) * INVERTE_ESQ;
  if (vel > 0) {
    analogWrite(MOT_ESQ_A, vel);
    analogWrite(MOT_ESQ_B, 0);
  } else if (vel < 0) {
    analogWrite(MOT_ESQ_A, 0);
    analogWrite(MOT_ESQ_B, -vel);
  } else {
    analogWrite(MOT_ESQ_A, 0);
    analogWrite(MOT_ESQ_B, 0);
  }
}

// # Motor direito: vel > 0 = frente, vel < 0 = trás, 0 = parado
void motorDireito(int vel) {
  vel = ajustarVel(vel) * INVERTE_DIR;
  if (vel > 0) {
    analogWrite(MOT_DIR_A, vel);
    analogWrite(MOT_DIR_B, 0);
  } else if (vel < 0) {
    analogWrite(MOT_DIR_A, 0);
    analogWrite(MOT_DIR_B, -vel);
  } else {
    analogWrite(MOT_DIR_A, 0);
    analogWrite(MOT_DIR_B, 0);
  }
}

// # Controla os dois motores de uma vez
void motores(int esq, int dir) {
  motorEsquerdo(esq);
  motorDireito(dir);
}

// MOVIMENTOS

void parar() {
  motores(0, 0);
}

void frente(int vel) {
  motores(vel, vel);
}

void tras(int vel) {
  motores(-vel, -vel);
}

// Giro no próprio eixo
void girarEsquerda(int vel) {
  motores(-vel, vel);
}

void girarDireita(int vel) {
  motores(vel, -vel);
}

// Curva (um lado com metade da velocidade)
void curvaEsquerda(int vel) {
  motores(vel / 2, vel);
}

void curvaDireita(int vel) {
  motores(vel, vel / 2);
}

// Freio: os dois lados de cada ponte H em HIGH
void freio() {
  digitalWrite(MOT_ESQ_A, HIGH);
  digitalWrite(MOT_ESQ_B, HIGH);
  digitalWrite(MOT_DIR_A, HIGH);
  digitalWrite(MOT_DIR_B, HIGH);
}

// Inicialização para o SETUP
void motoresIniciar() {
  pinMode(MOT_ESQ_A, OUTPUT);
  pinMode(MOT_ESQ_B, OUTPUT);
  pinMode(MOT_DIR_A, OUTPUT);
  pinMode(MOT_DIR_B, OUTPUT);
  parar();
}


// # SENSORES E BOTÕES #

// # Sensores (inicialização)
// (Os pinos TRIG/ECHO são configurados pela própria NewPing)
void sensoresIniciar() {
  pinMode(sensor1, INPUT);

  for (int i = 0; i < NUM_BOTOES; i++) {
    pinMode(PINOS_BOTOES[i], INPUT);   // pull-down externo
    estadoBotao[i] = LOW;
    ultimoEstadoBotao[i] = LOW;
    ultimoTempoDebounce[i] = 0;
  }
}

// # Botões (debounce)
// Debounce no estilo do exemplo da Arduino.
// Retorna true UMA vez, no momento em que o botão i passa a ficar
// apertado (HIGH) de forma estável. Com pull-down externo, apertado = HIGH.
bool botaoApertado(int i) {
  bool apertou = false;

  // Lê o estado do botão
  int leitura = digitalRead(PINOS_BOTOES[i]);

  // Se a leitura mudou (ruído ou aperto), reinicia o contador do debounce
  if (leitura != ultimoEstadoBotao[i]) {
    ultimoTempoDebounce[i] = millis();
  }

  // Se a leitura está estável há mais que TEMPO_DEBOUNCE, vale como estado real
  if ((millis() - ultimoTempoDebounce[i]) > TEMPO_DEBOUNCE) {

    // Se o estado estável mudou
    if (leitura != estadoBotao[i]) {
      estadoBotao[i] = leitura;

      // Só considera "apertou" se o novo estado for HIGH
      if (estadoBotao[i] == HIGH) {
        apertou = true;
      }
    }
  }

  // Guarda a leitura para a próxima chamada
  ultimoEstadoBotao[i] = leitura;

  return apertou;
}

// Espera um dos 3 botões ser apertado e devolve o número da estratégia (1, 2 ou 3).
// Enquanto espera, imprime a leitura crua dos botões (para depuração).
int esperarBotaoEstrategia() {
  unsigned long ultimoPrint = 0;

  while (true) {
    for (int i = 0; i < NUM_BOTOES; i++) {
      if (botaoApertado(i)) {
        return i + 1;
      }
    }

    if (millis() - ultimoPrint >= 500) {
      ultimoPrint = millis();
      Serial.print("Botoes D3/D4/D5: ");
      Serial.print(digitalRead(BOTAO_EST1));
      Serial.print(" ");
      Serial.print(digitalRead(BOTAO_EST2));
      Serial.print(" ");
      Serial.println(digitalRead(BOTAO_EST3));
    }
  }
}

// # Infravermelho
// true se o sensor IR detecta que o robô está fora da arena
bool foraDaArena() {
  return digitalRead(sensor1) == HIGH;
}

// # Ultrassônico

// Faz uma nova medição a cada INTERVALO_MEDICAO ms e guarda em distanciaAtual.
// Retorna true se mediu agora, false se ainda não deu o intervalo.
bool lerDistancia() {
  unsigned long tempoAtual = millis();

  if (tempoAtual - ultimoTempoMedicao >= INTERVALO_MEDICAO) {
    ultimoTempoMedicao = tempoAtual;

    int cm = sonar.ping_cm();   // retorna 0 se não houver eco

    if (cm == 0) {
      // Sem eco: considera que não há nada à frente
      distanciaAtual = DISTANCIA_MAXIMA;
    } else {
      distanciaAtual = cm;
    }

    return true;
  }

  return false;
}

// Última distância medida (cm)
float getDistancia() {
  return distanciaAtual;
}

// true se há um oponente dentro da distância de ataque
bool oponenteVisto() {
  lerDistancia();
  return getDistancia() < DIST_ATAQUE;
}


// # COMPORTAMENTOS E ESTRATÉGIAS #

// # Borda
// Retorna true se tratou a borda (o loop deve recomeçar).
bool tratarBorda() {
  if (!foraDaArena()) {
    return false;
  }

  tras(255);
  delay(TEMPO_RECUO);

  girarDireita(255);
  delay(TEMPO_GIRO_BORDA);

  return true;
}

// # Procurar inimigo
// Gira procurando o oponente; se o vir, ataca de frente
void procurarGirando() {
  if (oponenteVisto()) {
    frente(VEL_ATAQUE);
  } else {
    girarDireita(VEL_GIRO);
  }
}

// # Estratégias
// Estratégia 1: avança reto o tempo todo
void estrategia1() {
  frente(VEL_FRENTE);
}

// Estratégia 2: gira e avança um pouco (desvio inicial) e depois procura girando
void estrategia2() {
  unsigned long passou = millis() - tempoInicio;

  if (passou < TEMPO_DESVIO_GIRO) {
    girarDireita(200);
  } else if (passou < TEMPO_DESVIO_GIRO + TEMPO_DESVIO_FRENTE) {
    frente(200);
  } else {
    procurarGirando();
  }
}

// Estratégia 3: procura girando desde o início
void estrategia3() {
  procurarGirando();
}


// # DEBUG # (muita reza)

// Imprime no Serial o estado dos sensores a cada INTERVALO_PRINT ms
void depurar() {
  unsigned long tempoAtual = millis();

  if (tempoAtual - ultimoTempoPrint >= INTERVALO_PRINT) {
    ultimoTempoPrint = tempoAtual;

    lerDistancia();   // garante que a distância é atualizada em qualquer estratégia

    Serial.print("IR: ");
    Serial.print(digitalRead(sensor1));
    Serial.print(" | distancia: ");
    Serial.print(getDistancia());
    Serial.println(" cm");
  }
}


// # SETUP E LOOP #

void setup() {
  Serial.begin(9600);
  motoresIniciar();
  sensoresIniciar();

  Serial.println("Aguardando botao...");

  // Aguarda o botão da estratégia escolhida e só então começa a contagem
  estrategia = esperarBotaoEstrategia();
  Serial.print("Estrategia escolhida: ");
  Serial.println(estrategia);

  // Espera de 5 segundos antes da luta começar
  delay(5000);
  tempoInicio = millis();
}

void loop() {
  if (DEBUG) {
    depurar();
  }

  // Prioridade máxima: não cair da arena
  if (tratarBorda()) {
    return;
  }

  // Executa a estratégia escolhida
  if (estrategia == 1) {
    estrategia1();
  } else if (estrategia == 2) {
    estrategia2();
  } else if (estrategia == 3) {
    estrategia3();
  }
}
