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

//Bibliotecas
#include <IRremote.h>       // biblioteca para o recetor de infravermelhos
#include <LiquidCrystal.h>  // bibioteca para o display
#include <Stepper.h>        // biblioteca para o motor de passo

//DISPLAY----------------------
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
//fim display ----------------------

//STEPPER
const int stepsPerRevolution = 100;  // change this to fit the number of steps per revolution for your motor 2038
// inicializa o motor para as entradas digitais 2,3,4 e 5
const int PIN_step1 = 10, PIN_step2 = 11, PIN_step3 = 12, PIN_step4 = 13;
Stepper motor(stepsPerRevolution, PIN_step1, PIN_step3, PIN_step2, PIN_step4);
//fim stepper ----------------------

//IR REMOTE
int PIN_IRrec = A3;        // pino do recetor de infravermelho
IRrecv irrecv(PIN_IRrec);  //objeto do tipo IRrecv
decode_results results;    // objeto auxiliar para interpretar a tecla premida/sinal recebido

//BOTOES
//Pinos DOS BOTOES
int button1 = 8;

//SENSOR DE TEMPERATURA
// Pino do sensor
const int LM35 = A2;

//MULTIPLADOR (74HC595)
int latchPin = 11;  // pino que define a entrada (sendo que sao 8 possiveis entradas)
int clockPin = 12;  // pino que controla o relógio do multiplador
int dataPin = 13;   // pino em que entram os dados

//PROGRAMAS
byte programas = 0;  // variavel que controla que programas e funcoes extra estao on ou off
// os primeiros 5 bits controlam os programas, os ultimos 3 controlam as funcoes extra

//Condicoes extra e variaveis de controlo
int N = 10;  // numero de bits do arduino
int Vs = 5;  // tensao de alimentacao
bool overTemperature = false;
bool cancelOperation = false;
const int maxTemp = 80;  // temperatura maxima: 80 ºC
String buttons[21] = {"CH-", "CH", "CH+", "VOL-", "VOL+", "PLAY/PAUSE", "VOL-", "VOL+",
                      "EQ", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
enum OPERATION { Eco,
                 Intensivo,
                 Diario,
                 Rapido };  // enumeracao das operacoes de lavagem
OPERATION desiredOperation;
int timer = 0;

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
    Serial.begin(9600);
}

void loop() {
    /* Escrever mensagem amigável no ecrã */
    lcd.setCursor(0, 0);  // limpa o display e poe o cursor na primeira celula
    lcd.print("Dishwasher");
    lcd.setCursor(0, 1);
    lcd.print("Select operation");

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
        
        OBS: passos intermedios tambem podem ter mensagens no display */

    if (irrecv.decode(&results)) {  // esperamos ate' que o recetor receba um sinal
        if (receberInstrucao()) {
            askTimer();
            sleep();
            waitForDoorClose();  // consideramos que a porta fica bloqueada durante a lavagem
            startWashing(desiredOperation);
        }
    }
    irrecv.resume();  // prepara o recetor para o proximo sinal

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

bool receberInstrucao() {
    bool validInstruction = true;  // variavel de controlo; valor true se foi clicado um botao com uma operacao valida
    lcd.clear();
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

    Serial.println(results.value);  //para debug
    switch (results.value) {
        case 16753245:  //Eco
            desiredOperation = Eco;
            lcd.print("Eco");
            Serial.println("Eco");
            break;
        case 16736925:  //Intenso
            desiredOperation = Intensivo;
            lcd.print("Intensivo");
            Serial.println("Intensivo");
            break;
        case 16769565:  //Diario
            desiredOperation = Diario;
            lcd.print("Diario");
            Serial.println("Diario");
            break;
        //case 16720605:  //Suave
        //    desiredOperation = Suave;
        //    lcd.print("Suave");
        //    break;
        case 16712445:  //Rapido
            desiredOperation = Rapido;
            lcd.print("Rapido");
            Serial.println("Rapido");
            break;
        //case 16761405:  // play
        //break;
        case 16769055:
            break;
        case 16754775:
            break;
        case 16748655:
            break;
        default:
            // se recebemos uma outra tecla, instucao invalida
            validInstruction = false;
            lcd.print("Invalid");
            lcd.setCursor(0, 1);
            lcd.print("Operation");
            Serial.println("Invalid Operation");
            delay(200);
            break;
    }

    delay(1000);
    lcd.clear();


    return validInstruction;
}

/* Funcao do temporizador
    OBS: para o temporizador um delay nao era a melhor ideia, porque para tudo,
    incluindo qualquer mensagem que esteja no display, e o recetor de infravermelhos
    (imagina que queria por um timer para 2 duas mas so' pus para 1 hora; nao consigo anular)
    A melhor solucao e' usar o millis() para saber quanto tempo ja' passou.
*/
void sleep() {
    long timerInMillis = timer * 60 * 1000L;  //forcar a ser um long
    long startingTime = millis();

    Serial.println("----");
    Serial.print("timerInMillis:");
    Serial.println(timerInMillis);
    Serial.print("startingTime:");
    Serial.println(startingTime);

    if (timer > 0) {
        delay(10000);

        lcd.clear();
        lcd.print("Starting in");
    }

    while ((millis() - startingTime) / 1000 < timer * 60) {
        /* Mostrar no display quanto tempo falta
            OBS: a maneira que podia parecer mais simples era:
            lcd.print(remainingTime / 1000);
            mas, esta maneira tinha uma problema que e' sempre que o tempo restante baixava de uma
            unidade de grandeza, ficava erradamente um zero a mais.
            Por exemplo: 102, 101, 100, 900, 890, 880 (ficava um zero a mais erradamente)
            A solucao implementada foi por isso enviar uma String com um padding space que vai apagar
            aquele zero que esta' a mais.

        */
        long remainingTime = startingTime + timerInMillis - millis();
        lcd.setCursor(0, 1);
        lcd.print(String(remainingTime / 1000) + " ");

        Serial.print("remainingTime: ");
        Serial.println(remainingTime / 1000);

        if (irrecv.decode(&results)) {
            if (receivedCancellation()) {
                break;  // se for preciso cancelar o timer, fazemos break
            }
        }
        irrecv.resume();
    }
    timer = 0;  // fazer reset do temporizador

    lcd.clear();  // limpamos o lcd
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

/* Funcao que espera que se clique num botao */
void waitForButton() {
    bool buttonPressed = false;  // variavel de controlo; valor false enquanto nenhum botao e' premido

    while (!buttonPressed) {
        if (button1 == LOW) {
            buttonPressed = true;
        }
        /*
        else if (button2 == LOW) {
            buttonPressed = true;

        } else if (button3 == LOW) {
            buttonPressed = true;
        }
        */
    }  // FIM while()

}  // FIM waitForButton()

/* Funcao para comecar o programa de lavagem
Esta funcao identifica o programa de lavagem, definindo a velocidade para o motor,
...tempo e temperatura de lavagem.
Para de correr (para a lavagem) quando se concluir o tempo de lavagem, ou, por seguranca,
...se o sensor de temperatura acusar sobre aquecimento. Ou ainda, se alguem cancelar a
...lavagem a meio.
Envia para o display quanto tempo falta para concluir a lavagem
*/
void startWashing(OPERATION desiredProgram) {
    int motorSpeed;  // velocidade do motor
    int cycleTime;   //tempo do programa
    int cycleTemperature;

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
        //case Suave:
        //    cycleTime = 120;
        //    cycleTemperature = 65;
        //    motorSpeed = 3;
        //    break;
        case Rapido:
            cycleTime = 30;
            cycleTemperature = 65;
            motorSpeed = 10;
            break;
    }

    // Verificar: "ERRO" COM O remaingingTime

    long startingTime = millis();                 // registar o instante do inicio do programa
    long cycleInMillis = cycleTime * 60 * 1000L;  //forcar a ser um long

    motor.setSpeed(motorSpeed);  // definimos a velocidade do motor
    lcd.clear();
    lcd.print("Washing...");
    while ((millis() - startingTime) / 1000 < cycleTime * 60) {
        overTemperature = checkTemperature();

        /* Mostrar no display quanto tempo falta
            OBS: a maneira que podia parecer mais simples era:
            lcd.print(remainingTime / 1000);
            mas, esta maneira tinha uma problema que e' sempre que o tempo restante baixava de uma
            unidade de grandeza, ficava erradamente um zero a mais.
            Por exemplo: 102, 101, 100, 900, 890, 880 (ficava um zero a mais erradamente)
            A solucao implementada foi por isso enviar uma String com um padding space que vai apagar
            aquele zero que esta' a mais.

        */
        long remainingTime = startingTime + cycleInMillis - millis();
        lcd.setCursor(0, 1);
        lcd.print(String(remainingTime / 1000) + " ");

        Serial.print("remainingTime: ");
        Serial.println(remainingTime / 1000);

        //Debug
        Serial.println("----");
        Serial.print("cycleTime: ");
        Serial.println(cycleTime);
        Serial.print("startingTime: ");
        Serial.println(startingTime);
        Serial.print("millis ");
        Serial.print(millis());
        Serial.print("remainingTime: ");
        Serial.println(remainingTime);
        //Serial.println(remainingTime / 1000 / 60);

        if (overTemperature == true) {
            break;
        }

        if (irrecv.decode(&results)) {
            if (receivedCancellation()) {
                break;  // se for preciso cancelar o timer, fazemos break
            }
        }
        irrecv.resume();
    }
    lcd.clear();
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
bool checkTemperature() {
    int ADC_read = analogRead(LM35);
    int temperature = ADC_read * Vs / (pow(2, N) - 1);  // N e' o numero de bits do ADC
    Serial.println("-----");
    Serial.print("Temperature: ");
    Serial.println(temperature);

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
    lcd.clear();
    lcd.print("Close the door!");
    bool doorIsClosed = digitalRead(button1);

    while (!doorIsClosed) {
        //esperamos ate' que seja precionado o botao que simula o fechar da porta

        doorIsClosed = digitalRead(button1);
        Serial.println("----");
        Serial.print("doorIsClosed: ");
        Serial.println(doorIsClosed);

        if (irrecv.decode(&results)) {
            if (receivedCancellation()) {
                break;  // se for preciso cancelar o timer, fazemos break
            }
        }
        irrecv.resume();
    }
    lcd.clear();  // limpar o display
    if (doorIsClosed) {
        lcd.print("Door is closed");
    }
}

/* Funcao especifica para saber se recebemos o botao PLAY/PAUSE */
bool receivedPlayPause() {
    if (results.value == 16761405) {
        Serial.println("Received Play/Pause order");
        return true;
    } else {
        return false;
    }
}
/* Funcao especifica para saber se recebemos o botao de cancelar */
bool receivedCancellation() {
    if (results.value == 16748655) {
        Serial.println("Received Cancellation order");
        lcd.clear();
        lcd.print("Cancelled");
        delay(1000);
        cancelOperation = true;
    } else {
        cancelOperation = false;
    }
    return cancelOperation;
}

/* Funcao auxliar para receber um digito pelo telecomando */
int receberNumero() {
    int recebido;  // variavel que guarda o numero recebido
    // um switch case para guardar o valor recebido pelo IR receiver
    switch (results.value) {
        case 16738455:  // recebe 0 ;0xFFB04F
            recebido = 0;
            break;
        case 16724175:  // recebe 1
            recebido = 1;
            break;
        case 16718055:  // recebe 2
            recebido = 2;
            break;
        case 16743045:  // recebe 3
            recebido = 3;
            break;
        case 16716015:  // recebe 4
            recebido = 4;
            break;
        case 16726215:  // recebe 5
            recebido = 5;
            break;
        case 16734885:  // recebe 6
            recebido = 6;
            break;
        case 16728765:  // recebe 7
            recebido = 7;
            break;
        case 16730805:  // recebe 8
            recebido = 8;
            break;
        case 16732845:  // recebe 9
            recebido = 9;
            break;
        default:
            Serial.println("Not a Number");
            recebido = -1;
            break;
    }
    return recebido;
}

/* Funcao auxiliar para acumular o tempo para o temporizador */
void guardarDigito(int recebido) {
    if (recebido >= 0) {                // se recebemos um sinal valido
        timer = 10 * timer + recebido;  //...aumentamos uma ordem de grandeza ao numero que tinhamos e adicionamos o novo digito;
        //digitsCounter++;                  //... e atualizamos o contador de numeros recebidos
    }
}
/* Funcao que espera input do utilizador sobre o temporizador a definir
    - Comecamos por limpar qualquer mensagem anterior do display (por causa dos padding chars),
    e passamos a mostrar a mensagem "Timer:"
    - Esperamos que o utilizador uso o telecomando para enviar o temporizador desejado
    - Tem ainda em atencao que o utilizador pode querer anular a operacao; nesse caso faz break
*/
void askTimer() {
    lcd.clear();  // apagar qualquer mensagem anterior no display (por causa dos padding chars)
    lcd.print("Timer:");

    irrecv.resume();  // prepara o sensor para receber o proximo sinal

    while (true) {  // while true
        if (irrecv.decode(&results)) {
            //while (!receivedPlayPause) {
            // pode acontecer querer cancelar a operacao; nesse caso, fazemos break
            if (receivedCancellation()) {
                timer = 0;  // fazer reset do temporizador
                break;
            }
            if (receivedPlayPause()) {
                break;
            }
            int recebido = receberNumero();
            guardarDigito(recebido);
            // falta aparecer no display o nosso input
            lcd.setCursor(0, 1);
            lcd.print(timer);
            Serial.println("----------");
            Serial.print("results.value: ");
            Serial.println(results.value);
            Serial.print("recebido: ");
            Serial.println(recebido);
            Serial.print("timer: ");
            Serial.println(timer);

            irrecv.resume();  // prepara o sensor para receber o proximo sinal
        }
    }
    irrecv.resume();
}

/*
    Sons:
    - bip curto ao fim da lavagem completa
    - bip longo em casos de erro
*/