// SINAL PASSAGEM EM NIVEL - Versao 1.1
// By Clederson T. Przybysz - clederson_p@yahoo.com.br
// expressoarduino.blogspot.com
// Criação: Julho/2019 - Ultima Revisão: Julho/2019
//
// Release Note: 
// 1.0: Acionamento Luzes, Modulo Som Servos Cancela, CarSystem
// 1.1: Adicionados trimpots para ajuste angulo dos Servos da Cancela
//      Alterada a biblioteca de acionamento do modulo MP3
// 
//Copyright Sinal de Passagem em Nivel:
// O SOFTWARE É FORNECIDO "NO ESTADO EM QUE SE ENCONTRAM", SEM GARANTIA DE QUALQUER TIPO, EXPRESSA OU IMPLÍCITA, MAS NÃO SE LIMITANDO ÀS GARANTIAS DE COMERCIALIZAÇÃO.  
// EM NENHUMA CIRCUNSTÂNCIA, O AUTOR/TITULAR DE DIREITOS AUTORAIS SE RESPONSABILIZA POR QUALQUER RECLAMAÇÃO, DANOS OU OUTRA RESPONSABILIDADE, 
// SEJA EM AÇÃO DE CONTRATO, DELITO OU DE OUTRA FORMA, DECORRENDO DESTE SOFTWARE OU RELACIONADO AO SEU  USO.
//

#include <Servo.h>
#include <SoftwareSerial.h>
#include <DFPlayerMini_Fast.h>

#define pinSensor1 2
#define pinSensor2 3
#define pinSensor3 4
#define pinSensor4 5

#define pinServo1Cancela 6
#define pinServo2Cancela 7 
#define pinServo1CarSystem 8
#define pinServo2CarSystem 9

#define pinSinalA 10
#define pinSinalB 11

#define pinTxSoftSerial 12
#define pinRxSoftSerial A2 //Nao Utilizado

#define pinPotCancela1 A0
#define pinPotCancela2 A1

#define AnguloCarSystemAberto 0
#define AnguloCarSystemFechado 23

#define TempoCancela 100
#define TempoPisca 600

#define TimeroutSensores 5

Servo servoCancela1;
Servo servoCancela2;
Servo servoCarSystem1;
Servo servoCarSystem2;

//Variaveis Acionamento
byte Posicao=0;
byte Rst1=0;
byte Rst4=0;
unsigned long TimeLoopReset;

//Variaveis Cancela
unsigned long TimeLoopCancela;
byte AnguloCancela1;
byte AnguloCancela2;
byte AnguloNovoCancela1;
byte AnguloNovoCancela2;
byte AnguloCancela1Aberta;
byte AnguloCancela1Fechada;
byte AnguloCancela2Aberta;
byte AnguloCancela2Fechada;
byte CancelaFechada;

//Variaveis Pisca Sinalizacao
byte fasePisca=0;
unsigned long TimeLoopPisca;

SoftwareSerial mp3Serial(pinRxSoftSerial, pinTxSoftSerial); // RX, TX
DFPlayerMini_Fast mp3;

void setup() {
  Serial.begin(115200);
  mp3Serial.begin(9600);
  
  // Pinos Sensores
  pinMode(pinSensor1, INPUT);
  pinMode(pinSensor2, INPUT);
  pinMode(pinSensor3, INPUT);
  pinMode(pinSensor4, INPUT);
  
  //Pinos Sinal
  pinMode(pinSinalA, OUTPUT);
  pinMode(pinSinalB, OUTPUT);
  digitalWrite(pinSinalA,LOW);
  digitalWrite(pinSinalB,LOW);
  
  //Carrega Angulso Cancela
  CarregaAnguloCancela();
  AnguloCancela1=AnguloCancela1Aberta;
  AnguloNovoCancela1=AnguloCancela1Aberta;
  AnguloCancela2=AnguloCancela2Aberta;
  AnguloNovoCancela2=AnguloCancela2Aberta;
  CancelaFechada=0;
  
  //Inicia Modulo MP3
  mp3.begin(mp3Serial);
  mp3.volume(25);
  mp3.sleep();
  
  
  //Servos Cancela
  servoCancela1.attach(pinServo1Cancela);
  servoCancela2.attach(pinServo2Cancela);
  servoCancela1.write(AnguloCancela1);
  servoCancela2.write(AnguloCancela2);
  
  //Servos Car System
  servoCarSystem1.attach(pinServo1CarSystem);
  servoCarSystem2.attach(pinServo2CarSystem);
  servoCarSystem1.write(AnguloCarSystemAberto);
  servoCarSystem2.write(AnguloCarSystemAberto);
  Serial.println("Inicio");

  CancelaFechada=0;
}

void loop() {
  if (CancelaFechada==1) {
    //Pisca Sinalizacao Cancela
    if (millis()-TimeLoopPisca>TempoPisca){
      if (fasePisca==0) {
        digitalWrite(pinSinalA,HIGH);
        digitalWrite(pinSinalB,LOW); 
        fasePisca=1;
      }
      else
      {
        digitalWrite(pinSinalA,LOW);
        digitalWrite(pinSinalB,HIGH); 
        fasePisca=0;
      }
      TimeLoopPisca=millis();
    }
    // Abre/Fecha Cancelas
    if (millis()-TimeLoopCancela>TempoCancela){
      //Cancela 1
      if (AnguloCancela1<AnguloNovoCancela1) {
        AnguloCancela1++;
        servoCancela1.write(AnguloCancela1);
      }
      else if (AnguloCancela1>AnguloNovoCancela1)
      {
        AnguloCancela1--;  
        servoCancela1.write(AnguloCancela1);
      }
      //Cancela 2
      if (AnguloCancela2<AnguloNovoCancela2) {
        AnguloCancela2++;
        servoCancela2.write(AnguloCancela2);
      }
      else if (AnguloCancela2>AnguloNovoCancela2)
      {
        AnguloCancela2--;  
        servoCancela2.write(AnguloCancela2);
      }
      //Se as duas cancelas abriram Desliga Luzes e Libera CarSystem
      if (AnguloCancela1==AnguloCancela1Aberta&&AnguloCancela2==AnguloCancela2Aberta) {
        Serial.println("Cancela Aberta");
        digitalWrite(pinSinalA,LOW);
        digitalWrite(pinSinalB,LOW);
        CancelaFechada=0;
        servoCarSystem1.write(AnguloCarSystemAberto);
        servoCarSystem2.write(AnguloCarSystemAberto);
        //Para Sino
        mp3.sleep();
      }
      TimeLoopCancela = millis();
    }
    
  }

  //Verifica Sensores 
  
  //Incrementa Timerout Sensores de Entrada (S1 e S4)
  if (millis()-TimeLoopReset>1000){
    if (Rst1>0) Rst1++;
    if (Rst4>0) Rst4++;
    TimeLoopReset=millis();
  }

  //Le Valor Sensores
  byte vSensor1 = digitalRead(pinSensor1);
  byte vSensor2 = digitalRead(pinSensor2);
  byte vSensor3 = digitalRead(pinSensor3);
  byte vSensor4 = digitalRead(pinSensor4);
  

  //Valida Sensores e Posicao
    //Entra S1
  if (vSensor1==1&&Posicao==0) {
    Serial.println("Entra S1");
    Rst1=1;
    Posicao=1;
    FechaPassagem();
    return;
  }
    //Sai S1 (Timeout Sensor 1)
  if (Posicao==1&&Rst1>TimeroutSensores) {
    Serial.println("Sai S1 (Timeout Sensor 1)");
    Rst1=0;
    Posicao=0;
    AbrePassagem();
    return;
  }
  //Reset Timeout Sensor 1
  if (Posicao==1&&vSensor1==1) {
    //Serial.println("Reset Timeout S1");
    Rst1=1;
    return;
  }
  //Vai de S1 para S2
  if (Posicao==1&&vSensor1==0&&vSensor2==1) {
    Serial.println("Vai de S1 para S2");
    Posicao=2;
    return;
  }
  //Vai de S2 para S1
  if (Posicao==2&&vSensor1==1&&vSensor2==0) {
    Serial.println("Vai de S2 para S1");
    Posicao=1;
    Rst1=1;
    return;
  }
  //Vai de S2 para S3
  if (Posicao==2&&vSensor2==0&&vSensor3==1) {
    Serial.println("Vai de S2 para S3");
    Posicao=3;
    return;
  }
  //Vai de S3 para S2
  if (Posicao==3&&vSensor2==1&&vSensor3==0) {
    Serial.println("Vai de S3 para S2");
    Posicao=2;
    return;
  }
  //Vai de S3 para S4
  if (Posicao==3&&vSensor3==0&&vSensor4==1) {
    Serial.println("Vai de S3 para S4");
    Posicao=4;
    Rst4=1;
    return;
  }
  //Vai de S4 para S3
  if (Posicao==4&&vSensor3==1&&vSensor4==0) {
    Serial.println("Vai de S4 para S3");
    Posicao=3;
    return;
  }
  //Sai S4 (Timeout Sensor 4)
  if (Posicao==4&&Rst4>TimeroutSensores) {
    Serial.println("Sai S4 (Timeout Sensor 4)");
    Posicao=0;
    Rst4=0;
    AbrePassagem();
    return;
  }
  //Entra S4
  if (Posicao==0&&vSensor4==1) {
    Serial.println("Entra S4");
    Posicao=4;
    Rst4=1;
    FechaPassagem();
    return;
  }
  //Reset Timeout Sensor 4
  if (Posicao==4&&vSensor4==1) {
    //Serial.println("Reset Timeout S4");
    Rst4=1;
    return;
  }
}

void FechaPassagem() {
  //Serial.println("Fecha Passagem");
  //Toca Sino
  if (CancelaFechada==0) {
    mp3.startRepeatPlay();
    mp3.play(1);
  }
  //Aciona Luzes
  CancelaFechada=1;
  //Define Fechamento Cancela
  AnguloNovoCancela1=AnguloCancela1Fechada;
  AnguloNovoCancela2=AnguloCancela2Fechada;
  
  //Fecha Car System
  servoCarSystem1.write(AnguloCarSystemFechado);
  servoCarSystem2.write(AnguloCarSystemFechado);
  //Serial.println("Fecha Passagem2");
}

void AbrePassagem() {
  AnguloNovoCancela1=AnguloCancela1Aberta;
  AnguloNovoCancela2=AnguloCancela2Aberta;
}

void CarregaAnguloCancela() {
  int v1=analogRead(pinPotCancela1);
  int v2=analogRead(pinPotCancela2);
  AnguloCancela1Fechada=map(v1, 0, 1023, 10, 60); 
  AnguloCancela2Fechada=map(v2, 0, 1023, 10, 60); 
  Serial.print("C1:");
  Serial.print(AnguloCancela1Fechada);
  Serial.print(" C2:");
  Serial.println(AnguloCancela2Fechada);
  AnguloCancela1Aberta=3;
  AnguloCancela2Aberta=3;

  //Teste Fixo
  //AnguloCancela1Fechada=39; 
  //AnguloCancela2Fechada=35; 
}
