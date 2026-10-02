#include <NewPing.h>

const int MOT_ESQ_A = 5;
const int MOT_ESQ_B = 10;
const int MOT_DIR_A = 6;
const int MOT_DIR_B = 11;

const int INVERTE_ESQ = 1;
const int INVERTE_DIR = 1;

const int VEL_MINIMA = 1;

const int Led = 2;

// Único sensor infravermelho (HIGH = fora da arena)
const int sensor1 = 13;

// Botões de estratégia (um botão entre o pino e o GND, com INPUT_PULLUP)
// Troque os pinos se ligar em outros
const int BOTAO_EST1 = 30;
const int BOTAO_EST2 = 31;
const int BOTAO_EST3 = 32;

// Debounce dos botões
const int NUM_BOTOES = 3;
const int PINOS_BOTOES[NUM_BOTOES] = {BOTAO_EST1, BOTAO_EST2, BOTAO_EST3};
const unsigned long TEMPO_DEBOUNCE = 50;  // ms

bool ultimaLeituraBotao[NUM_BOTOES];            // true = apertado (leitura crua)
unsigned long ultimaMudancaBotao[NUM_BOTOES];   // momento da última mudança

#define TRIG_PIN 22
#define ECHO_PIN 23
#define DISTANCIA_MAXIMA 200

NewPing sonar(TRIG_PIN, ECHO_PIN, DISTANCIA_MAXIMA);

const bool TEM_ULTRASSONICO = false;

unsigned long ultimoTempoMedicao = 0;
const unsigned long INTERVALO_MEDICAO = 50;

float distanciaAtual = DISTANCIA_MAXIMA;

const int DIST_ATAQUE = 35;

const int VEL_ATAQUE = 255;
const int VEL_FRENTE = 200;
const int VEL_GIRO = 150;

const unsigned long TEMPO_RECUO = 300;
const unsigned long TEMPO_GIRO_BORDA = 250;
const unsigned long TEMPO_DESVIO_GIRO = 250;
const unsigned long TEMPO_DESVIO_FRENTE = 400;

int estrategia = 1;

unsigned long tempoInicio = 0;

const bool DEBUG = true;
unsigned long ultimoTempoPrint = 0;
const unsigned long INTERVALO_PRINT = 200;

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

void motores(int esq, int dir) {
  motorEsquerdo(esq);
  motorDireito(dir);
}

void parar() {
  motores(0, 0);
}

void frente(int vel) {
  motores(vel, vel);
}

void tras(int vel) {
  motores(-vel, -vel);
}

void girarEsquerda(int vel) {
  motores(-vel, vel);
}

void girarDireita(int vel) {
  motores(vel, -vel);
}

void curvaEsquerda(int vel) {
  motores(vel / 2, vel);
}

void curvaDireita(int vel) {
  motores(vel, vel / 2);
}

void freio() {
  digitalWrite(MOT_ESQ_A, HIGH);
  digitalWrite(MOT_ESQ_B, HIGH);
  digitalWrite(MOT_DIR_A, HIGH);
  digitalWrite(MOT_DIR_B, HIGH);
}

void motoresIniciar() {
  pinMode(MOT_ESQ_A, OUTPUT);
  pinMode(MOT_ESQ_B, OUTPUT);
  pinMode(MOT_DIR_A, OUTPUT);
  pinMode(MOT_DIR_B, OUTPUT);
  parar();
}

void sensoresIniciar() {
  pinMode(sensor1, INPUT);
  pinMode(Led, OUTPUT);

  for (int i = 0; i < NUM_BOTOES; i++) {
    pinMode(PINOS_BOTOES[i], INPUT_PULLUP);
    ultimaLeituraBotao[i] = false;
    ultimaMudancaBotao[i] = millis();
  }
}

// Retorna true se o botão i está apertado de forma estável por TEMPO_DEBOUNCE ms.
// Com INPUT_PULLUP, botão apertado = LOW.
bool botaoApertado(int i) {
  bool leitura = (digitalRead(PINOS_BOTOES[i]) == LOW);

  // Se a leitura mudou, reinicia a contagem
  if (leitura != ultimaLeituraBotao[i]) {
    ultimaLeituraBotao[i] = leitura;
    ultimaMudancaBotao[i] = millis();
  }

  // Só vale se estiver apertado e estável pelo tempo mínimo
  return leitura && (millis() - ultimaMudancaBotao[i] >= TEMPO_DEBOUNCE);
}

// Espera um dos 3 botões ser apertado e devolve o número da estratégia (1, 2 ou 3).
int esperarBotaoEstrategia() {
  while (true) {
    for (int i = 0; i < NUM_BOTOES; i++) {
      if (botaoApertado(i)) {
        return i + 1;
      }
    }
  }
}

bool foraDaArena() {
  return digitalRead(sensor1) == HIGH;
}

bool lerDistancia() {
  if (!TEM_ULTRASSONICO) {
    return false;
  }

  unsigned long tempoAtual = millis();

  if (tempoAtual - ultimoTempoMedicao >= INTERVALO_MEDICAO) {
    ultimoTempoMedicao = tempoAtual;

    unsigned int distancia = sonar.ping_cm();

    if (distancia == 0) {
      distanciaAtual = DISTANCIA_MAXIMA;
    } else {
      distanciaAtual = distancia;
    }

    return true;
  }

  return false;
}

float getDistancia() {
  return distanciaAtual;
}

bool oponenteVisto() {
  lerDistancia();
  return getDistancia() < DIST_ATAQUE;
}

// Com apenas 1 sensor não dá para saber de que lado está a borda,
// então o robô recua e gira sempre para a direita.
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

void estrategia1() {
  frente(VEL_FRENTE);
}

void procurarGirando() {
  if (oponenteVisto()) {
    frente(VEL_ATAQUE);
  } else {
    girarDireita(VEL_GIRO);
  }
}

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

void estrategia3() {
  procurarGirando();
}

void depurar() {
  unsigned long tempoAtual = millis();

  if (tempoAtual - ultimoTempoPrint >= INTERVALO_PRINT) {
    ultimoTempoPrint = tempoAtual;

    Serial.print("IR: ");
    Serial.print(digitalRead(sensor1));
    Serial.print(" | distancia: ");
    Serial.print(getDistancia());
    Serial.println(" cm");
  }
}

void setup() {
  Serial.begin(9600);
  motoresIniciar();
  sensoresIniciar();

  // Aguarda o botão da estratégia escolhida e só então começa a contagem
  estrategia = esperarBotaoEstrategia();
  Serial.print("Estrategia escolhida: ");
  Serial.println(estrategia);

  delay(5000);
  tempoInicio = millis();
}

void loop() {
  if (DEBUG) {
    depurar();
  }

  if (tratarBorda()) {
    return;
  }

  if (estrategia == 1) {
    estrategia1();
  } else if (estrategia == 2) {
    estrategia2();
  } else if (estrategia == 3) {
    estrategia3();
  }
}
