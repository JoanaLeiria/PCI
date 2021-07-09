/* Projeto - Joana Leiria, Jorge Silva

   DESCRICAO:
   Este scipt funciona para um projeto de uma maquina de lavar louça.
   Temos:
   - X botões (portas ...) para o utilizador dar input: Start/Stop;
        selecionar programa de lavagem; ...
   - 1 display (portas ...) para fazer output de mensagens ao utlizador
   - 1 motor (portas ...) para a maquina
   - X LEDs (portas ...)
   - 1 sensor de temperatura (porta ...)
   - ...


   Codigo com base em:
    - biblioteca LiquidCrystal :
    - biblioteca Stepper :  https://www.arduino.cc/en/reference/stepper
    - biblioteca IRremote : https://github.com/Arduino-IRremote/Arduino-IRremote

*/

//DISPLAY----------------------
#include <LiquidCrystal.h>  // bibioteca para o display
LiquidCrystal lcd;
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
//fim display ----------------------

//STEPPER
#include <Stepper.h>
const int stepsPerRevolution = 100;  // change this to fit the number of steps per revolution for your motor 2038
// inicializa o motor para as entradas digitais 2,3,4 e 5
Stepper myStepper(stepsPerRevolution, 2, 4, 3, 5);
//fim stepper ----------------------

//IR REMOTE
#include <IRremote.h>      // biblioteca para o recetor de infravermelhos
int PIN_IRrec = A3;        // pino do recetor de infravermelho
IRrecv irrecv(PIN_IRrec);  //objeto do tipo IRrecv
decode_results results;    // objeto auxiliar para interpretar a tecla premida/sinal recebido

//BOTOES
//Pinos DOS BOTOES
int button1 = 10;
int button2 = 11;
int button3 = 12;

//SENSOR DE TEMPERATURA
// Pino do sensor
const int LM35 = A3;

void setup() {
    /* Comecar por definir o modo dos pinos */
    //os pinos dos botoes sao de input
    pinMode(button1, INPUT);
    pinMode(button2, INPUT);
    pinMode(button3, INPUT);

    /* Inicializacao de coisas */
    lcd.begin(16, 2);  // inicializar lcd de 16x2

    /* Escrever mensagem amigável no ecrã */
    lcd.setCursor(0, 0);  // por o curso na primeira celula do display
    lcd.print("Dishwasher:");
}

void loop() {
    // put your main code here, to run repeatedly:
}

// *************************
// FUNCOES *****************
// *************************

/* Funcao do temporizador */
/*
  OBS: talvez um delay nao seja a melhor ideia -> porque vai parar tudo,
  incluindo qualquer mensagem que esteja no display;
  a solucao talvez seja esta funcao devolver um inteiro, e algures no codigo por
  while (tempo<sleep){...}
*/
void sleep(int hours) {
    delay(hours * 3600 * 1000);  // parar o programa x millisegundos
}

/* Funcao para selecionar um programa */
void selectProgram() {
}

/* Mensagens para o display */
void dispMessages() {
}

/* Funcao que espera que se clique num botao*/
void waitForButton() {
    bool buttonPressed = false;  // variavel de controlo; valor false enquanto nenhum botao e' premido

    while (!buttonPressed) {
        if (button1 == LOW) {
            buttonPressed = true;

        } else if (button2 == LOW) {
            buttonPressed = true;

        } else if (button3 == LOW) {
            buttonPressed = true;
        }

    }  // FIM while()

}  // FIM waitForButton()

/* Funcao para comecar o programa de lavagem*/
void startWashing(String desiredProgram) {
    // instrucoes para por o motor a funcionar
    // e imprimir no display mensagem quanto tempo falta
    int motorSpeed;  // velocidade do motor
    int cycleTime;   //tempo do programa
    switch (desiredProgram) {
        case "Eco":
            motorSpeed = 10;
            cycleTime = ;
            break;
    }
    case "":
        break;
}

/* Funcao para parar o programa de lavagem*/
void stopWashing() {
    // instrucoes para parar o motor
    // e imprimir no display mensagem stopped
}

/* Funcao para verificar o nivel de calcario */
/* ....*/

/* Funcao para verificar a temperatura */
/*
  2 opcoes:
  verificamos se esta' a uma temperatura maior do que uma pre-definida
  ... ou 
  verificamos se aumentou muito desde a ultima vez que verificámos:
  - se sim, e' melhor aumentar a taxa com que medimos a temperatura
  - se nao, podemos baixar

  OBS: ainda nao sei se faz sentido este metodo ser void ou outra coisa
*/
void checkTemperature(int lastCheck) {
    int ADC_read = analogRead(LM35);
    int temperature = ADC_read * Vs / (pow(2, N) - 1);

    if (temperature > lastCheck) {
        /* break variable*/
    }
}



