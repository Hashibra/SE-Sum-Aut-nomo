#include <NewPing.h>

#define TRIG_PIN 12
#define ECHO_PIN 13
#define DISTANCIA_MAXIMA 200

NewPing sonar(TRIG_PIN, ECHO_PIN, DISTANCIA_MAXIMA);

unsigned long ultimoTempoMedicao = 0;
const unsigned long INTERVALO_MEDICAO = 50;

float distanciaAtual = 0.0;

bool lerDistancia() {
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
