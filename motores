const int MOT_ESQ_A = 5;
const int MOT_ESQ_B = 10;
const int MOT_DIR_A = 6;
const int MOT_DIR_B = 11;

// Se um lado girar ao contrário no teste, troque para -1
const int INVERTE_ESQ = 1;
const int INVERTE_DIR = 1;

// PWM mínimo em que as rodas realmente começam a girar
// Precisamos descobrir a velocidade mínima depois (PRECISA TESTAR)
const int VEL_MINIMA = 1;

int ajustarVel(int valor_pedido) {
  int vel = constrain(valor_pedido, -255, 255);

  if (vel == 0) {
    return 0;
  } 

  int sinal;
    if (vel > 0) {
        sinal = 1;
    }   
    else {
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
    } 
  else if (vel < 0) {
    analogWrite(MOT_ESQ_A, 0);
    analogWrite(MOT_ESQ_B, -vel);
  } 
  
  else {
    analogWrite(MOT_ESQ_A, 0);
    analogWrite(MOT_ESQ_B, 0);
  }
}

void motorDireito(int vel) {
  vel = ajustarVel(vel) * INVERTE_DIR;
  if (vel > 0) {
    analogWrite(MOT_DIR_A, vel);
    analogWrite(MOT_DIR_B, 0);
  } 

  else if (vel < 0) {
    analogWrite(MOT_DIR_A, 0);
    analogWrite(MOT_DIR_B, -vel);
  } 

  else {
    analogWrite(MOT_DIR_A, 0);
    analogWrite(MOT_DIR_B, 0);
  }
}


void motores(int esq, int dir) {       // aceita -255 a 255 em cada lado
  motorEsquerdo(esq);
  motorDireito(dir);
}

void parar() {
  motores(0, 0);            // motor solto
}

void frente(int vel) {
  motores(vel, vel);
}

void tras(int vel) {
  motores(-vel, -vel);
}

void girarEsquerda(int vel) {
  motores(-vel, vel);       // no próprio eixo
}

void girarDireita(int vel) {
  motores(vel, -vel);       // no próprio eixo
}

void curvaEsquerda(int vel) {
  motores(vel / 2, vel);    // avança curvando
}

void curvaDireita(int vel) {
  motores(vel, vel / 2);    // avança curvando
}

void freio() {                                           // motor travado
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

void setup() {
  motoresIniciar();   // única linha dos motores no setup
  delay(3000);        // teste
}

void loop() { // teste bobo
  frente(150);        delay(1500);
  parar();            delay(500);
  tras(150);          delay(1500);
  parar();            delay(500);
  girarEsquerda(150); delay(1000);
  parar();            delay(500);
  girarDireita(150);  delay(1000);
  freio();            delay(2000);
}
