/* Projeto - Grupo 3D - Joana Leiria, Jorge Silva - Maquina Lavar loica

    DESCRICAO:
    Este scipt funciona para um projeto de uma maquina de lavar loica.
    Com este script conseguimos simular as funcoes basicas de uma maquina de lavar loiça.
    Assim, conseguimos selecionar um dos programas de lavagem, por um temporizador,
    e simular aspetos como o fechar da porta, o verificar dos niveis de descalcificacao,
    rodar um motor e verificar a sua temperatura.

    Ao nivel das portas temos:
   - 1 display (portas 2 a 7) para fazer output de mensagens ao utlizador
   - 1 botao (porta 8) para o utilizador dar input: fechar a porta
   - 1 LED (porta 9)
   - 1 motor (portas 10 a 13) para a maquina
   - 1 sensor de temperatura (porta A2)
   - 1 sensor de infravermelhos (porta A3)


   Codigo com base em:
    - biblioteca LiquidCrystal : https://www.arduino.cc/en/Reference/LiquidCrystal
    - biblioteca IRremote : https://github.com/Arduino-IRremote/Arduino-IRremote

*/

//Bibliotecas
#include <IRremote.h>       // biblioteca para o recetor de infravermelhos
#include <LiquidCrystal.h>  // bibioteca para o display
#include <Stepper.h>        // biblioteca para o motor de passo

//DISPLAY----------------------
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 7, en = 6, d5 = 5, d4 = 4, d3 = 3, d2 = 2;
LiquidCrystal lcd(rs, en, d5, d4, d3, d2);
//fim display -----------------

//STEPPER
const int stepsPerRevolution = 100;  // change this to fit the number of steps per revolution for your motor 2038
// inicializa o motor para as entradas digitais 2,3,4 e 5
const int PIN_step1 = 10, PIN_step2 = 11, PIN_step3 = 12, PIN_step4 = 13;
Stepper motor(stepsPerRevolution, PIN_step1, PIN_step3, PIN_step2, PIN_step4);
//fim stepper ----------------------

//IR REMOTE--------------------
const int PIN_IRrec = A3;        // pino do recetor de infravermelho
IRrecv irrecv(PIN_IRrec);  //objeto do tipo IRrecv
decode_results results;    // objeto auxiliar para interpretar a tecla premida/sinal recebido

//BOTOES e LEDS----------------------
const int buttonDoor = 8;
const int LED = 9;

//SENSOR DE TEMPERATURA--------
// Pino do sensor
const int LM35 = A2;

//Condicoes extra e variaveis de controlo
const int N = 10;                    // numero de bits do arduino
const int Vs = 5;                    // tensao de alimentacao
bool overTemperature = false;  // variavel bool com o estado da temperatura
bool cancelOperation = false;  // variavel bool com o cancelamento de operacao
bool lowSalt = false;          // variavel bool com o nivel de sal
const int maxTemp = 80;        // temperatura maxima: 80 ºC
enum OPERATION {
    Eco,
    Intensivo,
    Diario,
    MeiaCarga,
    Rapido
};  // enumeracao das operacoes de lavagem
OPERATION desiredOperation;
int timer = 0;  // variavel que guarda o tempo para o temporizador

void setup() {
    /* Comecar por definir o modo dos pinos */
    pinMode(buttonDoor, INPUT);
    pinMode(LED, OUTPUT);
    digitalWrite(LED,LOW); // apagar o led


    /* Inicializacao de objetos */
    lcd.begin(16, 2);     // inicializar lcd de 16x2
    irrecv.enableIRIn();  // permite receber sinais infrabermelhos
    Serial.begin(9600);   // permite comunicacao com o computador

    verifyDescaling();  // verifica os niveis de descalcificacao
    if (lowSalt == true) {
        exit(0);
    }
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
            if (cancelOperation == false) {
                startWashing(desiredOperation);
            }
        }
    }
    irrecv.resume();  // prepara o recetor para o proximo sinal

}

// *************************
// FUNCOES *****************
// *************************

/*  COMANDO:
    Para o comando vamos fazer a seguinte correspondencia entre
    botao (do telecomando) e operacao (da maquina)
    
    { CH-, CH, CH+, PREV} -> selecionar os diferentes programas
    { PLAY/PAUSE } -> comecar e parar lavagem
    { EQ } -> anular qualquer operacao que esteja a decorrer ou que esteja programada
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
    PREV    - 16720605
    NEXT    - 16712445 
    PLAY    - 16761405
    VOL-    - 16769055
    VOL+    - 16754775
    EQ      - 16748655
    */

    Serial.println(results.value);  // para debug
    switch (results.value) {
        case 16753245:  //Eco
            desiredOperation = Eco;
            lcd.print("Eco");
            break;
        case 16736925:  //Intenso
            desiredOperation = Intensivo;
            lcd.print("Intensivo");
            break;
        case 16769565:  //Diario
            desiredOperation = Diario;
            lcd.print("Diario");
            break;
        case 16720605:  //Rapido
            desiredOperation = Rapido;
            lcd.print("Rapido");
            break;
        case 16712445:  //Meia carga
            desiredOperation = MeiaCarga;
            lcd.print("Meia Carga");
            break;
        default:
            // se recebemos uma outra tecla, instucao invalida
            validInstruction = false;
            lcd.print("Invalid");
            lcd.setCursor(0, 1);
            lcd.print("Operation");
            delay(1200);
            lcd.clear();
            break;
    }

    delay(1000);

    return validInstruction;
}

/* Funcao do temporizador
    OBS: para o temporizador um delay nao era a melhor ideia, porque para tudo,
    incluindo qualquer mensagem que esteja no display, e o recetor de infravermelhos
    (imagina que queria por um timer para 2 duas mas so' pus para 1 hora; nao conseguiria anular)
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
        delay(5000);

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

/* Mensagens para o display */
void dispMessages(String message) {
}

/* Funcao para comecar o programa de lavagem
Esta funcao identifica o programa de lavagem, definindo a velocidade para o motor,
...tempo e temperatura de lavagem.
Para de correr (para a lavagem) quando se concluir o tempo de lavagem, ou, por seguranca,
...se o sensor de temperatura acusar sobre aquecimento. Ou ainda, se alguem cancelar a
...lavagem a meio.
Envia para o display quanto tempo falta para concluir a lavagem
*/
void startWashing(OPERATION desiredProgram) {
    int motorSpeed;        // variavel que controla a velocidade do motor (que corresponde a um delay)
    int cycleTime;         //tempo do programa
    int cycleTemperature;  //temperatura do programa

    switch (desiredProgram) {
        case Eco:
            cycleTime = 200;
            cycleTemperature = 50;
            motorSpeed = 60;
            break;
        case Intensivo:
            cycleTime = 120;
            cycleTemperature = 70;
            motorSpeed = 200;
            break;
        case Diario:
            cycleTime = 120;
            cycleTemperature = 50;
            motorSpeed = 120;
            break;
        case MeiaCarga:
            cycleTime = 100;
            cycleTemperature = 50;
            motorSpeed = 120;
        case Rapido:
            cycleTime = 30;
            cycleTemperature = 65;
            motorSpeed = 200;
            break;
    }

    long startingTime = millis();                 // registar o instante do inicio do programa
    long cycleInSeconds = cycleTime * 60;  //forcar a ser um long

    lcd.clear();
    lcd.print("Washing...");

    motor.setSpeed(motorSpeed);  // definimos a velocidade do motor

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
        long remainingTime = startingTime/1000 + cycleInSeconds - millis()/1000;
        lcd.setCursor(0, 1);
        int hours = remainingTime / (3600);
        int minutes = remainingTime/60 - hours*60;
        lcd.print(String(hours) + ":" + String(minutes) + " ");  // com padding zeros
        motor.step(100);

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
        Serial.print("hours"); Serial.println(hours);
        Serial.print("minutes"); Serial.println(minutes);

        if (overTemperature == true) {
            lcd.clear();
            lcd.write("Sobreaquecimento");
            digitalWrite(LED,HIGH); // Acender o LED como forma de aviso
            delay(10000);
            break;
        }

        if (irrecv.decode(&results)) {
            if (receivedCancellation()) {
                break;  // se for preciso cancelar o timer, fazemos break
            }
        }
        irrecv.resume();
    }
}

/* Funcao com instrucoes para parar o programa de lavagem*/
void stopWashing() {
    motor.setSpeed(0);
    motor.step(0);
}

/* Funcao para verificar a temperatura
Lemos o estado da porta ligada ao sensor de temperatura atraves
... da funcao analogRead(). Pela datasheet do sensor, convetermos
... o estado da porta para um valor de temperatura.
Comparamos esse valor com um valor de referencia de temperatura maxima.
*/
bool checkTemperature() {
    int ADC_read = analogRead(LM35);
    int temperature = ADC_read * Vs / (pow(2, N) - 1) / 0.01;  // N e' o numero de bits do ADC
    Serial.println("-----");
    Serial.print("Temperature: ");
    Serial.println(temperature);

    if (temperature >= maxTemp) {
        // sobreaquecimento; parar o motor
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
    bool doorIsClosed = digitalRead(buttonDoor);

    if (cancelOperation == false) {
        while (!doorIsClosed) {
            //esperamos ate' que seja precionado o botao que simula o fechar da porta

            doorIsClosed = digitalRead(buttonDoor);
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
        if (doorIsClosed) {
            lcd.print("Door is closed");
        }
    }
    lcd.clear();  // limpar o ecra
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
/* Funcao para verificar se e preciso meter sal amaciador
   - Retira-se um numero aleatorio de 0 a 100, se for igual a 50 temos de repor o sal
   - Para repor o sal, enviamos mensagem pelo lcd para o utilizador desligar a camara e repor o sal, consoante
   o nivel da dureza da agua que utiliza
   - Esta funcao e utilizada no inicio do loop
*/
void verifyDescaling() {
    long x = random(100);
    Serial.println(x);
    if (x == 50) {
        lcd.clear();  // apagar qualquer mensagem anterior no display (por causa dos padding chars)
        lcd.print("Falta de sal");
        lcd.setCursor(0, 1);
        lcd.print("Desligar e repor");
        delay(2000);
        lowSalt = true;
    }
}

