/* Grupo 3D - Joana Leiria, Jorge Silva
    PCI - Folha 9 - Exercicio 3 - Escrever numeros num LCD usando um comando de Infravermelhos

    Descricao:
    Este script foi escrito para uma pequena montagem de Arduino para escrever numeros de ate'
    4 digitos num LCD com 4 digitos de 7 segmentos.
    Quando clicamos uma tecla do telecomando, enviamos um sinal para um sensor de infravermelhos
    (porta A3).
    Cada tecla envia um sinal diferente, pelo que, ao lermos o valor na porta, sabemos qual a tecla premida.
    Vamos guardando os varios digitos ate' que seja premida a tecla Play/Pause. E enviamos o numero completo
    para o display.

    Codigo baseado em:
    https://github.com/Arduino-IRremote/Arduino-IRremote - biblioteca IRremote
    https://github.com/DeanIsMe/SevSeg - biblioteca SevSeg para o display
*/

//Bibliotecas
#include <SevSeg.h> // biblioteca para o display de 4 digitos
#include <IRremote.h>  // biblioteca para o recetor de infravermelhos

//PINOS
int PIN_IRrec = A3; // pino do recetor de infravermelho
// Pinos do Display
int pinA = 2;
int pinB = 3;
int pinC = 4;
int pinD = 5;
int pinE = 6;
int pinF = 7;
int pinG = 8;
int pinP = 9;
int D1 = 10; // pino do primeiro digito
int D2 = 11; // pino do segundo digito
int D3 = 12; // pino do terceiro digito
int D4 = 13; // pino do quarto digito

// Variaveis auxiliares
int digitsCounter = 0; // conta o numero de digitos guardados
int number = 0; // guarda o numero (ate' 4 digitos) final
int recebido; // guarda o ultimo digito recebido

// Objetos
SevSeg sevseg; // objeto do tipo SevSeg
IRrecv irrecv(PIN_IRrec); //objeto do tipo IRrecv
decode_results results;// objeto auxiliar para interpretar a tecla premida/sinal recebido

void setup() {
  // Configuracao inicial do display
  byte numDigits = 4;
  byte digitPins[] = {D1, D2, D3, D4};
  byte segmentPins[] = {pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinP};

  byte hardwareConfig = COMMON_ANODE;
  //bool resistorsOnSegments = true;

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, true);

  Serial.begin(9600);
  irrecv.enableIRIn(); // permite receber sinais

  sevseg.refreshDisplay();
  sevseg.setNumber(1234, 1); // debug
  sevseg.refreshDisplay();
}

void loop() {


  if (irrecv.decode(&results)) { // esperamos ate' que o recetor receba um sinal
    Serial.print(results.value);
    if (receivedPlayPause() == true) {
      if (digitsCounter > 4) {
        sendErrorMessage(); // numero demasiado grande
      }
      else {
        sendToDisplay(); // enviar o numero para o display
      }
      digitsCounter = 0; // zerar o contador de digitos recebidos
      number = 0; // zerar o numero ( conjunto de digitos recebidos)
    }
    else {
      receberNumero(); // reconhecer o digito
      guardarDigito(); // guardar o digito
    }

    irrecv.resume(); // prepara o sensor para receber o proximo sinal
  }
  sevseg.refreshDisplay();

}

void receberNumero() {
  // um switch case para guardar o valor recebido pelo IR receiver
  switch (results.value) {
    case 16738455: // recebe 0 ;0xFFB04F
      recebido = 0;
      break;
    case 16724175: // recebe 1
      recebido = 1;
      break;
    case 16718055: // recebe 2
      recebido = 2;
      break;
    case 16743045: // recebe 3
      recebido = 3;
      break;
    case 16716015: // recebe 4
      recebido = 4;
      break;
    case 16726215: // recebe 5
      recebido = 5;
      break;
    case 16734885: // recebe 6
      recebido = 6;
      break;
    case 16728765: // recebe 7
      recebido = 7;
      break;
    case 16730805: // recebe 8
      recebido = 8;
      break;
    case 16732845: // recebe 9
      recebido = 9;
      break;
    default:
      Serial.println("Not a Number");
      recebido = -1;
      break ;
  }
}

boolean receivedPlayPause() {
  if (results.value == 16761405) {
    Serial.println("Play/Pause= TRUE");
    return true;
  }
  else {
    return false;
  }
}

void guardarDigito() {
  if (recebido >= 0) { // se recebemos um sinal valido
    number = 10 * number + recebido; //...aumentamos uma ordem de grandeza ao numero que tinhamos e adicionamos o novo digito;
    digitsCounter++; //... e atualizamos o contador de numeros recebidos
  }
}

void sendErrorMessage() {
  sevseg.setChars("Error");
  Serial.println("Error");
}

void sendToDisplay() {
  if (digitsCounter > 0) {
    sevseg.setNumber(number);
  }
  else {
    sevseg.blank();
  }
}
