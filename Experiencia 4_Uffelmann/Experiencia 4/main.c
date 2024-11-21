#define F_CPU 16000000UL	// Usado para cálculo de retardos
#define is_high(p,b)	(p & _BV(b)) == _BV(b)
#define is_low(p,b)		(p & _BV(b)) == 0
#define tbi(p,b)		 p ^= _BV(b)
#define cbi(p,b)		 p &= ~(_BV(b))
#define sbi(p,b)		 p |= _BV(b)

#define P1 PC0
#define Z1 PC1
#define Z2 PC2
#define Z3 PC3
#define Alarma PC4

#define A PB4
#define B PB3
#define C PB2
#define D PB1

#define Q1 PD1
#define Q2 PD2
#define Q3 PD3

#include <avr/io.h>
#include <util/delay.h>		// Contiene macros para generar retardos.
#include <stdbool.h>

bool leerBoton (int puerto, int pin);

int main(void)
{
	DDRB = 0b11111111;	// PB1 al PB4 para decodificador. LED integrado en PB5.
	DDRC = 0b110000;	// P1 en PC0, Z1 en PC1, Z2 en PC2, Z3 en PC3, Relé en PC4
	DDRD = 0b11111111;	// Q1 en PD1, Q2 en PD2, Q3 en PD3

	unsigned int cont = 15000;
	unsigned int cuentaReg = 0;		// Cuenta regresiva del tiempo de 15 a 0 segundos, mostrado cuando la alarma está activada y se detecta presencia en Z1.
	int BP1_act = 1, BP1_ant = 1, flag_BP1 = 1;
	int BZ1_act = 1, BZ1_ant = 1;
	int BZ2_act = 1, BZ2_ant = 1, flag_BZ2 = 1;
	int BZ3_act = 1, BZ3_ant = 1, flag_BZ3 = 1;
	int Z1detecta = 0, Z2detecta = 0, Z3detecta = 0;
	int flagDisplay = 1;
	int MSD, LSD;
	int cuentaRegAlarma = 0;
	
	bool alarmaActivada = false;		// Por defecto, alarma desactivada.
	bool alarmaDisparada = false;		// Por defecto, alarma no disparada.
	
    while (1) {
		// Botón P1 -- ON-OFF alarma
		BP1_act = is_high(PINC,P1);
		if(BP1_act && BP1_ant)	{
			flag_BP1 = 1;
			cbi(PORTD, Q1);		// Si no se detecta nada en Z1, mantener displays apagados ?
			cbi(PORTD, Q2);
		}
		if(!BP1_act && !BP1_ant && flag_BP1){
			alarmaActivada = !alarmaActivada;
			cont = 0;
			flag_BP1 = 0;
		}
		BP1_ant = BP1_act;
		
 		// Detección de Zona 1
		BZ1_act = is_high(PINC, Z1);
		if(!BZ1_act && !BZ1_ant){
			// Ingresó un intruso por la zona 1. Empezar conteo.
			cont -= 10;
			Z1detecta = 1;
		} else {
			cont = 15000;
			Z1detecta = 0;
		}
		BZ1_ant = BZ1_act;
		
		// Detección en Zona 2
		BZ2_act = is_high(PINC, Z2);
		if(BZ2_ant && BZ2_act)	flag_BZ2 = 1;
		if(!BZ2_act && !BZ2_ant) {
			Z2detecta = 1;
			if(flag_BZ2) {
				// Ingresó un intruso por la zona 2. Disparar alarma.
				// alarmaDisparada = true; // Alarma activada.
				cuentaReg = 0;
				flag_BZ2 = 0;	
			}
		} else {
			Z2detecta = 0;
		}
		BZ2_ant = BZ2_act;
		
		// Detección en Zona 3
		BZ3_act = is_high(PINC, Z3);
		if(BZ3_ant && BZ3_act)	flag_BZ3 = 1;
		if(!BZ3_act && !BZ3_ant){ 
			if(flag_BZ3){
				// Ingresó un intruso por la zona 3. Disparar alarma.
				// alarmaDisparada = true; // Alarma activada.
				cuentaReg = 0;
				flag_BZ3 = 0;
				Z3detecta = 1;
			}
		} else {
			Z3detecta = 0;
		}
		BZ3_ant = BZ3_act;
		
		//	Solamente encender los displays de cuenta regresiva si la alarma está activada,
		//	y el contador  está entre 0 y 14990
		if (alarmaActivada && cont > 0 && cont < 15000) {
			// Escritura multiplexada en displays
			cuentaReg = cont / 1000;	// Paso de ms a s.
			MSD = cuentaReg / 10;	// Extraer el dígito más significativo (decena)
			LSD = cuentaReg % 10;	// Extraer el dígito menos significativo (unidad)
			// Escribir en el puerto
			flagDisplay = !flagDisplay;
			if (flagDisplay) {
				cbi(PORTD, Q2);
				PORTB = LSD;
				sbi(PORTD, Q1);
				} else {
				cbi(PORTD, Q1);
				PORTB = MSD;
				sbi(PORTD, Q2);
			}
		}
		if(cuentaReg == 0){ // Si la cuenta regresiva de presencia en Z1 llega a 0, disparar la alarma.
			cuentaRegAlarma = 10000; // Setear en 10 segundos, pero sólo 1 vez.
			alarmaDisparada = true;
			
			cuentaReg = 15000;		// Valor para que no sea cero, y no vuelva a entrar en este if.
			cbi(PORTD, Q1);			// Apagar displays de cuenta regresiva
			cbi(PORTD, Q2);
		}
		
		if (cuentaRegAlarma > 0) {
			cuentaRegAlarma -= 10;
		} else {
			// si llegó a cero, apagar alarma.
			alarmaDisparada = false;
		}
						
		if (!alarmaActivada) alarmaDisparada = false;
		if (alarmaActivada) sbi(PORTB, PB5); // Encender el LED integrado del Arduino (Pin 13) para indicar que la alarma está activada.
		else cbi(PORTB, PB5);
		
		// Activar o desactivar la alarma, según su bandera lo indique
		if(alarmaActivada && alarmaDisparada) {
			sbi(PORTC, Alarma);		// Encender LED y buzzer alarma
		} else {
			cbi(PORTC, Alarma);		// Apagar LED y buzzer alarma
		}
		
		// Lógica para mostrar el número de zona en display 3 (el de la izquierda).
		
		if (!alarmaActivada) {
			sbi(PORTD, Q3);		// Encender display 3, para mostrar la zona
			cbi(PORTD, Q2);		// Apagar displays 1 y 2
			cbi(PORTD, Q1);
			if(Z1detecta) PORTB = 1;
			if(Z2detecta) PORTB = 2;
			if(Z3detecta) PORTB = 3;
			if(!Z1detecta & !Z2detecta & !Z3detecta) cbi(PORTD, Q3);
		} else {
			cbi(PORTD, Q3);
		}
		
		
		_delay_ms(10);
	}
}