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
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
//fim display ----------------------

//STEPPER
#include <Stepper.h>
const int stepsPerRevolution = 100;  // change this to fit the number of steps per revolution for your motor 2038
// inicializa o motor para as entradas digitais 2,3,4 e 5
Stepper motor(stepsPerRevolution, 2, 4, 3, 5);
//fim stepper ----------------------

//IR REMOTE
#include <IRremote.h>      // biblioteca para o recetor de infravermelhos
int PIN_IRrec = A3;        // pino do recetor de infravermelho
IRrecv irrecv(PIN_IRrec);  //objeto do tipo IRrecv
decode_results results;    // objeto auxiliar para interpretar a tecla premida/sinal recebido

//BOTOES
//Pinos DOS BOTOES
int button1 = 10;

//SENSOR DE TEMPERATURA
// Pino do sensor
const int LM35 = A3;

//MULTIPLADOR (74HC595)
int latchPin = 11;         // pino que define a entrada (sendo que sao 8 possiveis entradas)
int clockPin = 12;         // pino que controla o relógio do multiplador
int dataPin = 13;          // pino em que entram os dados 

//PROGRAMAS
byte programas = 0;    // variavel que controla que programas e funcoes extra estao on ou off
// os primeiros 5 bits controlam os programas, os ultimos 3 controlam as funcoes extra

//Condicoes extra e variaveis de controlo
int N = 10;  // numero de bits do arduino
int Vs = 5;  // tensao de alimentacao
bool overTemperature = false;
const int maxTemp = 80;  // temperatura maxima: 80 ºC
String buttons[21] = {"CH-", "CH", "CH+", "VOL-", "VOL+", "PLAY/PAUSE", "VOL-", "VOL+",
                        "EQ", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
enum operation { Eco,
                 Intensivo,
                 Diario,
                 Suave,
                 Rapido };  // enumeracao das operacoes de lavagem

void setup() {
    /* Comecar por definir o modo dos pinos */
    //o pino do botao e de input
    pinMode(button1, INPUT);
   
    //os pinos do multiplexador sao de output
    pinMode(latchPin, OUTPUT);
    pinMode(dataPin, OUTPUT);  
    pinMode(clockPin, OUTPUT);

    /* Inicializacao de coisas */
    lcd.begin(16, 2);     // inicializar lcd de 16x2
    irrecv.enableIRIn();  // permite receber sinais infrabermelhos

    /* Escrever mensagem amigável no ecrã */
    lcd.setCursor(0, 0);  // por o curso na primeira celula do display
    lcd.print("Dishwasher:");
}

void loop() {
    /*  O fluxo e':
        1. esperar por input do comando, que pode ser:
            a. lavar ja'
                1. esperar que a porta da maquina esteja fechada (botao1 pressed)
                2. esperar por input do tipo de programa (comando)
                3. iniciar lavagem
                    1. periodicamente verificar a temperatura;
                        1. se estiver a sobre aquecer, parar lavagem
                    2. se botao de pause clicado, parar a lavagem
                4. enviar mensagem para o display de lavagem concluida ou nao

            b. por um temporizador
                1. esperar que a porta da maquina esteja fechada (botao1 pressed) (por seguranca)
                2. esperar por input do tipo de programa (comando)
                3. por em wait
                4. SE a porta continua fechada (por seguranca), iniciar lavagem
                    1. periodicamente verificar a temperatura;
                        1. se estiver a sobreaquecer, parar lavagem
                    2. se botao de pause clicado, parar a lavagem
                5. enviar mensagem para o display de mensagem concluida ou nao

            c. opcoes de manutecao
                1. esperar que a porta da maquina esteja fechada (botao1 pressed)
                2. inicar operacao de manutencao
                    1. periodicamente verificar a temperatura;
                        1. se estiver a sobreaquecer, parar
                    2. se botao de pause clicado, parar
                3. enviar mensagem para o display de mensagem concluida ou nao
        
        OBS: passos intermedios tambem podem ter mensagens no display
    */

    if (irrecv.decode(&results)) {  // esperamos ate' que o recetor receba um sinal
        receberInstrucao();

    }
   
   // para selecionar o programa -> programas(0) -> selecionarPrograma() -> bitSet(programa, inteiro com o numero do programa) -> selecionarPrograma()
}

// *************************
// FUNCOES *****************
// *************************

/*  COMANDO:
    Para o comando estamos a pensar fazer a seguinte correspondencia entre
    botao (do telecomando) e operacao (da maquina)
    
    { CH-, CH, CH+ } -> operacoes de manutencao: abrilhantador, ....
    { PREV, NEXT } -> visualizar os diferentes programas
    { PLAY/PAUSE } -> comecar e parar lavagem
    { EQ } -> anular qualquer operacao que esteja a decorrer ou que esteja programada

    botao para propriamente ligar/desligar a maquina?
*/

void receberInstrucao() {
    /* Correspondecia
    tecla   - codigo
    -------------------
    CH-     - 16753245
    CH      - 16736925
    CH+     - 16769565
    PREV-   - 16720605
    NEXT-   - 16712445 
    PLAY-   - 16761405
    VOL--   - 16769055
    VOL+    - 16754775
    EQ      - 16748655
    */
    Serial.print(results.value);  //para debug
    switch (results.value) {
        case 16753245:
            /* code */
            // variavel especifica; falta esta parte
            break;
        case 16736925:
            break;
        case 16769565:
            break;
        case 16720605:
            break;
        case 16712445:
            break;
        case 16761405:  // play
            break;
        case 16769055:
            break;
        case 16754775:
            break;
        case 16748655:
            break;
        default:
            break;
    }
}

/* Funcao do temporizador */
/* OBS: talvez um delay nao seja a melhor ideia -> porque vai parar tudo,
  incluindo qualquer mensagem que esteja no display, e o recetor de infravermelhos
  (imagina que queria por um timer para 2 duas e so' pus para 1 hora; nao consigo anular)
  a solucao talvez seja esta funcao devolver um inteiro, e algures no codigo por
  while (tempo<sleep){...}, ou entao, fazer contas com a funcao millis();
  O melhor deve ser uar o millis();
*/
void sleep(int hours) {
    long timer = hours * 3600 * 1000;

    long startingTime = millis();

    while (millis() - startingTime < timer) {
        // bloco (por agora) vazio; suficiente para ficar parado neste ciclo enquanto nao se tiver
        // passado o tempo desejado
        // PORMENOR: tenho que considerar o caso em que clico no comando para parar o programa
        // SOLUCAO: por aqui dentro: if (received Play/pause) break
    }
}

/* Funcao para selecionar um programa 
Seguindo a lógica do multiplexer, esta função coloca o latchPin em LOW, depois com a função do 
Arduino 'shiftOut', altera o conteúdo da variável programas no registo e volta a mudar o
latchPin para HIGH novamente

Esta função vai servir para os 5 programas base e as 3 funções extra
(possivel porque o multiplexer permite 8 entradas)
*/
void selectProgram() {
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, programas);
   digitalWrite(latchPin, HIGH);
}

/* Mensagens para o display */
void dispMessages(String message) {
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

/* Funcao para comecar o programa de lavagem */
void startWashing(operation desiredProgram) {
    // instrucoes para por o motor a funcionar
    // e imprimir no display mensagem quanto tempo falta
    int motorSpeed;  // velocidade do motor
    int cycleTime;   //tempo do programa
    int cycleTemperature;
    /*!!!
    O compilador da' erro aqui porque o switch case so' aceita numeros inteiros; nao aceita strings
    Ja' estou 'a procura de solucao
    */
    switch (desiredProgram) {
        case Eco:
            cycleTime = 200;
            cycleTemperature = 50;
            motorSpeed = 3;
            break;
        case Intensivo:
            cycleTime = 120;
            cycleTemperature = 70;
            motorSpeed = 10;
            break;
        case Diario:
            cycleTime = 120;
            cycleTemperature = 65;
            motorSpeed = 6;
            break;
        case Suave:
            cycleTime = 120;
            cycleTemperature = 65;
            motorSpeed = 3;
            break;
        case Rapido:
            cycleTime = 30;
            cycleTemperature = 65;
            motorSpeed = 10;
            break;
    }
}

/* Funcao para parar o programa de lavagem*/
void stopWashing() {
    // instrucoes para parar o motor
    // e imprimir no display mensagem stopped
    motor.setSpeed(0);
    motor.step(0);
}
/* Funcao para verificar o nivel de calcario */
/* ....*/

/* Funcao para verificar a temperatura */
/* 2 opcoes:
  verificamos se esta' a uma temperatura maior do que uma pre-definida
  ... ou 
  verificamos se aumentou muito desde a ultima vez que verificámos:
  - se sim, e' melhor aumentar a taxa com que medimos a temperatura
  - se nao, podemos baixar

- variavel delta_t;
- ou entao, se for do tipo temperatura=0.8*maxTemp, repetir leitura 

  OBS: ainda nao sei se faz sentido este metodo ser void ou outra coisa
*/
bool checkTemperature(int lastCheck) {
    int ADC_read = analogRead(LM35);
    int temperature = ADC_read * Vs / (pow(2, N) - 1);  // N e' o numero de bits do ADC

    if (temperature >= maxTemp) {
        // sobre aquecimento; parar o motor
        return true;  // break variable is overTemperature
    } else {
        return false;
    }
}
/* Funcao para verificar se a porta esta' aberta
    - enquanto a porta esta' aberta, esperamos; */
void waitForDoorClose() {
    bool doorIsOpened = digitalRead(button1);
    while (doorIsOpened) {
        //esperamos ate' que seja precionado o botao que simula o fechar da porta
        // talvez por aqui o tal if(Play.isPressed){break}
    }
}

/* Tendo em conta que ja' temos uma funcao que se chama startWashing(),
o nome desta funcao nao e' muito feliz
O que esta funcao faz e' passar a velocidade para o motor, e deixa-o a
correr enquanto nao se passar o tempo de duracao do ciclo, a nao ser que
o motor comece a sobreaquecer. Neste ultimo caso, forca a paragem.
*/
void wash(int motorSpeed, int timer, int cycleTemperature) {
    long startingTime = millis();  // registar o instante do inicio do programa

    motor.setSpeed(motorSpeed);
    while (millis() - startingTime < timer) {
        overTemperature = checkTemperature;
        if (overTemperature == true) {
            break;
        }
    }
}