/* Grupo 3D - Joana Leiria, Jorge Silva
  PCI - Folha 9 - Exercicio 1 - Termometro analogico

  Descricao:
  Este scipt foi criado para utilizar um sensor de temperatura LM35 (porta A2) e um servo
  motor SG90 (porta ~9) como um termometro analogico.
  O arduino faz uma leitura da porta onde esta ligado o sensor de temperatura. Pelas
  especificacoes dos dispositivos, criamos uma relacao unica entre o valor lido e o angulo
  que deve ter o servo motor.
  O valor do angulo e' enviado para o servo motor, fazendo-o mover ate' ao referido angulo,
  de forma a funcionar como um termometro analogico.

  Codigo baseado em:
  https://www.arduino.cc/reference/en/libraries/servo/ - biblioteca servo motor
*/

// Temperatura minima e maxima; e atual
const float T_min = 20; // tempearatura minima em ºC
const float T_max = 50; // tempearatura maxima em ºC
float Temp; // variavel que guarda a temperatura atual

//Pinos a utilizar
const int PIN_Servo = 9; // pino do servo motor
const int PIN_LM = A2; // pino do LM35

// Caracterisiticas do LM35
const float LM_conversao = 0.01; // conversao de 0.01 V/ºC
const float LM_offset = 0;

// --- Variaveis auxiliares ---
// Tensoes: de alimentacao; aos 20º e aos 50º
const float Vs = 5; // tensao de alimentacao 5V
const float V_min = T_min * LM_conversao + LM_offset;
const float V_max = T_max * LM_conversao + LM_offset;

// Numeros m do arduino
int m; //variavel que guarda o numero m lido no momento
const int m_min = V_min / Vs * (pow(2, 10) - 1);
const int m_max = V_max / Vs * (pow(2, 10) - 1);

//Angulo alfa e conversao entre numero m e angulo
int alfa; // variavel que regista o angulo
const float k = 180.0 / (m_max - m_min); // a conversao entre o numero m e o angulo e' uma relacao linear alfa=k*m+b
const float b = -180 * m_min / (m_max - m_min);

#include <Servo.h> // Biblioteca com comandos para controlar o servo motor
Servo myservo;  // Objeto do tipo Servo para controlar o novo servo


void setup() {
  myservo.attach(PIN_Servo);  // atribui o pino ao objeto do servo
  myservo.write(0); // por o servo na posicao zero
  delay(10000);
  Serial.begin(9600);

  //DEBUG
  Serial.print("Tmin "); Serial.println(T_min);
  Serial.print("Tmax "); Serial.println(T_max);
  Serial.print("Vmin "); Serial.println(V_min);
  Serial.print("Vmax "); Serial.println(V_max);
  Serial.print("m_min "); Serial.println(m_min);
  Serial.print("m_max "); Serial.println(m_max);
  Serial.print("k "); Serial.println(k);
  Serial.print("b "); Serial.println(b);
  myservo.write(10);


}

void loop() {
  m = analogRead(PIN_LM);// le a tensao que corresponde 'a temperatura neste momento
  float temperatura = ( m * Vs / (pow(2, 10) - 1)) / LM_conversao; // calcula a temperatura (debug)

  // Calcular o novo angulo para o servo motor

  if ( m < m_min ) {
    // se lemos um valor que corresponde a menos de 20ºC: posicao 0;
    alfa = 0;
  }
  else if ( m > m_max ) {
    // se lemos um valor que corresponde a mais do que 50ºC: posicao 180;
    alfa = 180;
  }
  else {
    //em condicoes de temperatura normais : alfa =  m * 2.9 - 116;
    alfa = m * k + b;
    // Debug
    Serial.print("m "); Serial.println(m);
    Serial.print("k "); Serial.println(k);
    Serial.print("b "); Serial.println(b);
    Serial.print("angulo "); Serial.println(m * k + b);
    Serial.print("alfa "); Serial.println(alfa);
    delay(10000);
  }

  myservo.write(alfa); // Atualizar a posicao do servo motor

  delay(1000); // refazer a leitura so' a cada 1 segundo


  /*
    Serial.print("Temperatura: ");
    Serial.println(temperatura);
    Serial.print("m: ");
    Serial.print(m);

    Serial.print("Angulo: ");
    Serial.print(alfa);
  */
}
