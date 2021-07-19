/* Grupo 3D - Joana Leiria, Jorge Silva
   PCI - Folha 9 - Exercicio 2 - Controlar velocidade de um motor com um potenciometro

   Descricao:
   Este script foi criado para variar a velocidade de um motor de passo em funcao da posição de um
   potenciometro (ligado à porta A3).
   Quando o potenciometro esta' a meio, paramos o motor. Se rodamos o potenciometro para a direita
   o motor roda mais depressa no sentido dos ponteiros do relogio. Se rodarmos para a esquerda
   roda mais depressa em CCW.
   O programa le um numero 'm' da porta A3, e através da funcao map() convertemos esse numero para uma
   velocidade tal que (para o ADC de 10 bits, e uma tolerancia na posicao do potenciometro de 40):
   - m < 460 : roda no sentido contrario ao ponteiro do relogio
   - m > 540 : roda no sentido dos ponteiros do relogio
   - 460 < m < 540 : para

   Codigo baseado em:
   https://www.arduino.cc/en/reference/stepper - biblioteca para o motor de passo
*/

//Motor de passo
#include <Stepper.h> // biblioteca do motor de passo
const int stepsPerRevolution = 10000;  // change this to fit the number of steps per revolution


// inicializar o motor para as entradas digitais 2,3,4 e 5
Stepper myStepper(stepsPerRevolution, 10, 12, 11, 13);

//Potencionmetro
const int PIN_POT = A3;

void setup() {
  Serial.begin(9600);
}

void loop(){
  
  int m_potenciometro = analogRead(PIN_POT); // lemos o valor do potenciometro
  Serial.println(m_potenciometro);

  // mapear a velocidade: um numero entre 0 e 1023 lido pelo ADC, para uma velocidade entre -100 e 100
  int motorSpeed = map(m_potenciometro, 0, 1023, -100, 100);
  motorSpeed = abs(motorSpeed); // a funcao setSpeed() so' aceita valores positivos;
  myStepper.setSpeed(200); // mudamos a velocidade do motor
  Serial.print("motorSpeed");
  Serial.println(motorSpeed);

  if (m_potenciometro < 460) {
    myStepper.step(-stepsPerRevolution);
    //myStepper.step(1);
  }
  else if (m_potenciometro > 540) {
    myStepper.step(stepsPerRevolution);
    //myStepper.step(1);
  }
  else {
    myStepper.step(0);
  }

}
