/*
Librería escornabot por Prudencio Luna y Pedro Ruiz
V 0.20 (04/09/2020): sexta versión, se le pueden colocar sensores como ultrasonidos e infrarrojos.
V 0.15 (16/04/2019): quinta versión, se puede mover los motores mediante procedimiento de medio paso (tipo 3) y se corrigen fallos en constructor con parámetros para elegir modos de paso
V 0.14 (14/04/2019): cuarta versión del programa, cambiado nombre de procedimiento stop por Stop, se arregla procedimiento versión en .h, cambios en procedimiento pushButton, cambios en archivo de ejemplo.
V 0.13 (07/03/2018): tercera versión del programa, incorpora control de motores paso a paso (avances, retrocesos, giros, parada)
, elección del tipo de excitación de bobinas, control de leds, zumbador, botonera y bluetooth.
*/

#include "Arduino.h"
#include "rupertobot.h"

// Declaración y asignación de variables privadas

	int step [8][4] =//matriz que describe puesta en marcha de bobinas por defecto (4 posiciones)
		{
	  	{1, 0, 0, 0},
	  	{0, 1, 0, 0},
	  	{0, 0, 1, 0},
	  	{0, 0, 0, 1}
		};

	int stepsLap=2048;//nº de pasos que da en una vuelta (en paso completo)
	int stepsDone=0;//cuenta los pasos dados
	int coilPosition=0;// devuelve la posición de la bobina en cada paso (4 posiciones)
	int nSteps=4;//numero de pasos que da por ciclo completo
	int tstep1rpm=29297;//velocidad en rpm, 29297 es el nº de microsegundos que tardaría en dar 1 paso a 1rpm, 60/2048 y pasarlo a microsegundos

/*Pinout*/
	const int pinMotor[8]={2,3,4,5,6,7,8,9};//pines de motores
	const int buzz = 12; //pin del zumbador	



/*
 rupertobot constructor sin pasar parámetros
 */

rupertobot::rupertobot() //si no se pasan parámetros al constructor por defecto coge el paso 1 (1 sóla bobina a la vez)
{
//se inicializa las comunicaciones serie a 9600 baudios (para bluetooth)
	Serial.begin (9600);//iniciamos las comunicaciones
//se definen los pines de motores de escornabot como de salida
	for (int i=0;i<8;i++) {
		pinMode(pinMotor [i],OUTPUT);
	}

//se define el pin del zumbador como salida
    pinMode(buzz, OUTPUT);


}
//rupertobot

/*
 rupertobot constructor pasando tipo de paso de bobina 1, 2 o 3.( 1 y 2 de paso completo 3 de medio paso)
 */

rupertobot::rupertobot(int kindStep) //aquí se construye el objeto escornabot con el tipo de paso (excitación de bobinas) 2 (paso completo con dos bobinas) o 3 (medio paso)
{
//se inicializa las comunicaciones serie a 9600 baudios (para bluetooth)
	Serial.begin (9600);//iniciamos las comunicaciones
//se definen los pines de motores de escornabot como de salida
	for (int i=0;i<8;i++) {
		pinMode(pinMotor [i],OUTPUT);
	}

//se define el pin del zumbador como salida
   	pinMode(buzz, OUTPUT);

//comprueba el parámetro pasado de tipo de paso de bobina


	if (kindStep==2) {// se excitan dos bobinas a la vez en paso completo (más par más consumo)
		/* Ahora la matriz debe cambiar a
		{1, 1, 0, 0},
	  	{0, 1, 1, 0},
	  	{0, 0, 1, 1},
	  	{1, 0, 0, 1}*/
		step [0][1] =1;step [1][2]=1;step[2][3]=1;step [3][0]=1;//se cambian los 0 por 1 en la matriz para el paso completo con una bobina por paso
	}

	else if (kindStep==3) {//medio paso consumo y par intermedio a los anteriores movimiento más suave
		/* Ahora la matriz de medio paso debe cambiar a
		{1, 0, 0, 0},
		{1, 1, 0, 0},
		{0, 1, 0, 0},
		{0, 1, 1, 0},
		{0, 0, 1, 0},
		{0, 0, 1, 1},
		{0, 0, 0, 1},
		{1, 0, 0, 1}*/
		step[1][0]=1;step[2][1]=1;step[2][2]=0;//se cambian los 0 por 1 y los 1 por 0 en la matriz para el medio paso
		step[3][1]=1;step[3][2]=1;step[3][3]=0;
		step[4][2]=1;step[5][2]=1;step[5][3]=1;
		step[6][3]=1;step[7][0]=1;step[7][3]=1;
		stepsLap=4096;
		nSteps=8;
		tstep1rpm=14648;//es el nº de microsegundos que tardaría en dar 1 paso a 1rpm, 60/4096 y pasarlo a microsegundos
	}
}//rupertobot


/*
Procedimientos para avanzar, retroceder y girar
*/


void rupertobot::avanza(float laps, int speed)  {
	stepsDone=0;
			while (int(laps*stepsLap)>=stepsDone)  {
     		stepsDone ++;
     		coilPosition=stepsDone % nSteps;
      		digitalWrite(pinMotor[0], step[coilPosition][0]);
      		digitalWrite(pinMotor[4], step[coilPosition][3]);
      		digitalWrite(pinMotor[1], step[coilPosition][1]);
      		digitalWrite(pinMotor[5], step[coilPosition][2]);
      		digitalWrite(pinMotor[2], step[coilPosition][2]);
      		digitalWrite(pinMotor[6], step[coilPosition][1]);
      		digitalWrite(pinMotor[3], step[coilPosition][3]);
      		digitalWrite(pinMotor[7], step[coilPosition][0]);
      		delayMicroseconds(tstep1rpm/speed);
    	}

}
void rupertobot::atras(float laps, int speed)  {
	stepsDone=0;
		while (int(laps*stepsLap)>=stepsDone)  {
     		stepsDone ++;
     		coilPosition=stepsDone % nSteps;//calcula el resto para saber en la posición de bobina que está
		//cuando un pin está en una fila de la matriz el del otro motor está a la inversa pin 2 col 4 pin 6 col 1 por ejemplo
		digitalWrite(pinMotor[0], step[coilPosition][3]);//pin 2
      		digitalWrite(pinMotor[4], step[coilPosition][0]);//pin 6
      		digitalWrite(pinMotor[1], step[coilPosition][2]);//pin 3
      		digitalWrite(pinMotor[5], step[coilPosition][1]);//pin 7
      		digitalWrite(pinMotor[2], step[coilPosition][1]);//pin 4
      		digitalWrite(pinMotor[6], step[coilPosition][2]);//pin 8
      		digitalWrite(pinMotor[3], step[coilPosition][0]);//pin 5
      		digitalWrite(pinMotor[7], step[coilPosition][3]);//pin 9
      		delayMicroseconds(tstep1rpm/speed);
    	}
}

void rupertobot::avanzacm(float cm, int speed){
	rupertobot::avanza((cm/25.1327),speed);
}
void rupertobot::atrascm(float cm, int speed){
	rupertobot::atras((cm/25.1327),speed);
}

void rupertobot::alto () {

	for (int i=0; i<8; i++) {
	digitalWrite (pinMotor[i],LOW);
	}

}//stop



/*
Procedimientos de giro
*/


void rupertobot::izquierda(float laps, int speed){
	stepsDone=0;
			while (int(laps*stepsLap)>=stepsDone)  {
			stepsDone ++;
			coilPosition=stepsDone % nSteps;
			digitalWrite(pinMotor[0], step[coilPosition][3]);
			digitalWrite(pinMotor[4], step[coilPosition][3]);
			digitalWrite(pinMotor[1], step[coilPosition][2]);
			digitalWrite(pinMotor[5], step[coilPosition][2]);
			digitalWrite(pinMotor[2], step[coilPosition][1]);
			digitalWrite(pinMotor[6], step[coilPosition][1]);
			digitalWrite(pinMotor[3], step[coilPosition][0]);
			digitalWrite(pinMotor[7], step[coilPosition][0]);
			delayMicroseconds(tstep1rpm/speed);
		}
}
void rupertobot::derecha(float laps, int speed){
	stepsDone=0;
			while (int(laps*stepsLap)>=stepsDone)  {
			stepsDone ++;
			coilPosition=stepsDone % nSteps;
			digitalWrite(pinMotor[0], step[coilPosition][0]);
			digitalWrite(pinMotor[4], step[coilPosition][0]);
			digitalWrite(pinMotor[1], step[coilPosition][1]);
			digitalWrite(pinMotor[5], step[coilPosition][1]);
			digitalWrite(pinMotor[2], step[coilPosition][2]);
			digitalWrite(pinMotor[6], step[coilPosition][2]);
			digitalWrite(pinMotor[3], step[coilPosition][3]);
			digitalWrite(pinMotor[7], step[coilPosition][3]);
			delayMicroseconds(tstep1rpm/speed);
		}
}



void rupertobot::giraIzquierda (float grados, int speed){
	rupertobot:izquierda(grados/180,speed);
}
void rupertobot::giraDerecha(float grados,int speed){
	rupertobot::derecha(grados/180,speed);
}

/*
  version() procedimiento que devuelve la versión de la librería
*/
float rupertobot::version(){
  return 0.20;
}

//A propuesta de Antonio Gómez

/*
infrared procedimiento que configura los pines de los sensores infrarrojos
*/ 
void rupertobot::IR(int der, int izq){
	_IRL=izq;
	_IRR=der;
	pinMode(_IRL,INPUT);
	pinMode(_IRR,INPUT);
}

/*
us procedimiento que configura los pines del sensor de ultrasonidos
*/ 
void rupertobot::us(int trig, int echo){
	_trig=trig;
	_echo=echo;
	pinMode(_trig,OUTPUT);
	pinMode(_echo,INPUT);
}


/*
distance() procedimiento que devuelve la distancia en cm del sensor de ultrasonidos
*/ 
long rupertobot::distancia(){

	long espacio;
	long tiempo;
	digitalWrite(_trig,LOW);
	delayMicroseconds(4);
	digitalWrite(_trig,HIGH);
	delayMicroseconds(10);
	digitalWrite(_trig,LOW);
	tiempo=pulseIn(_echo,HIGH);
	espacio=tiempo/58.309037901;
	return espacio;
}

/*
blackRight procedimiento que nos devuelve true o false si el sensor de ir derecho está a negro
*/ 
bool rupertobot::negroDerecha(){
	bool negro;
	if (digitalRead(_IRR)==1){
		negro=true;
	}
	else{
		negro=false;
	}
	return negro;
}

/*
blackLeft procedimiento que nos devuelve true o false si el sensor de ir izquierdo está a negro
*/ 
bool rupertobot::negroIzquierda(){
	bool negro;
	if (digitalRead(_IRL)==1){
		negro=true;
	}
	else{
		negro=false;
	}
	return negro;
}

/*
whiteRight procedimiento que nos devuelve true o false si el sensor de ir derecho está a blanco
*/ 
bool rupertobot::blancoDerecha(){
	bool blanco;
	if (digitalRead(_IRR)==1){
		blanco=false;
	}
	else{
		blanco=true;
	}
	return blanco;
}


/*
whiteLeft procedimiento que nos devuelve true o false si el sensor de ir izquierdo está a blanco
*/ 
bool rupertobot::blancoIzquierda(){
	bool blanco;
	if (digitalRead(_IRL)==1){
		blanco=false;
	}
	else{
		blanco=true;
	}
	return blanco;
}

/*
buzzer procedimiento para indicar el pin al que se conecta un zumbador
*/
void rupertobot::zumbador(int pin){
	_buzz=pin;
	pinMode(_buzz,OUTPUT);
}

/*
tono procedimiento para indicar la frecuencia y duración del sonido del zumbador
*/
void rupertobot::tono(int frequency, int time){
	tone(_buzz, frequency,time);
	delay(time);
	noTone(_buzz);
}
