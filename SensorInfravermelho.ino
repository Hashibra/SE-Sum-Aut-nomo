// Led de derrota
const int Led = 2;

// Sensor Infravermelho
const int sensor1 = 13; // define o pino do primeiro sensor
const int sensor2 = 12; // define o pino do segundo sensor

int est1 = LOW; // guarda se o sensor 1 já esteve fora da arena
int est2 = LOW; // guarda se o sensor 2 já esteve fora da arena

int lerIF1 = LOW;
int lerIF2 = LOW;

int luta = HIGH;

int confere(int infra1, int infra2){
  // confere leitura do primeiro sensor
  if(infra1 == HIGH)
  {
    if(est1 == LOW)
    {
      est1 = HIGH;
      if(est2 == HIGH)
      {
        return LOW;
      }
    }
  }
  // confere leitura do segundo sensor
  if(infra2 == HIGH)
  {
    if(est2 == LOW)
    {
      est2 = HIGH;
      if(est1 == HIGH)
      {
        return LOW;
      }
    }
  }
  return HIGH;
}

// Outros sensores e 

void setup()
{
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(Led, OUTPUT);
}

void loop()
{
  // .....
  // toda a lógica de combate do robô
  // .....
  
  lerIF1 = digitalRead(sensor1);
  lerIF2 = digitalRead(sensor2);
  luta = confere(lerIF1, lerIF2);
    
  if(luta == LOW)
  {
    while(1==1)
    {
      digitalWrite(Led, HIGH);
      delay(500);
      digitalWrite(Led, LOW);
      delay(500);
    }
  }
}