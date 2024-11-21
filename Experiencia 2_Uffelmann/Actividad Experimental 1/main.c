#define F_CPU 16000000UL   // Frecuencia del reloj para cálculos de delay
#include <avr/io.h>        // Definiciones estándar (puertos, etc.)
#include <util/delay.h>    // Macros para delay

// Constantes para el motor y los LEDs
#define SAL_MOT PB0
#define BOTON_START_STOP PB1
#define BOTON_INC PB2
#define BOTON_DEC PB3

// Valores de tiempo en incrementos de 10 ms (modificado para 10 ms)
#define T1 500      // 5 segundos
#define T2 1000     // 10 segundos
#define T3 2000     // 20 segundos
#define T4 6000     // 1 minuto
#define T5 12000    // 2 minutos

// Macros para verificar el estado de los botones
#define es_alto(p,b)    ((p & _BV(b)) == _BV(b))
#define es_bajo(p,b)    ((p & _BV(b)) == 0)
#define tbi(p,b)        (p ^= _BV(b))
#define cbi(p,b)        (p &= ~(_BV(b)))
#define sbi(p,b)        (p |= _BV(b))

// Declaración de funciones
void inicializar_io();
void actualizar_leds(int T);
void apagar_leds();

int main(void) {
	// Inicialización de los puertos de entrada/salida
	inicializar_io();
	unsigned int cont = 0;        // Contador para el temporizador
	unsigned int T = T1;          // Tiempo seleccionado inicialmente
	unsigned int temp = T;        // Tiempo actual seleccionado en unidades de 10 ms
	int motor_encendido = 0;      // Estado del motor

	// Variables de estado de los botones
	int B1_act, B1_ant = 1, flag_B1 = 1;
	int B2_act, B2_ant = 1, flag_B2 = 1;
	int B3_act, B3_ant = 1, flag_B3 = 1;

	while (1) {
		
		actualizar_leds(T);
		// Manejo del botón de start/stop (PB1)
		B1_act = es_alto(PINB, BOTON_START_STOP);
		if (B1_act && B1_ant)
		flag_B1 = 1;
		if (!B1_act && !B1_ant && flag_B1) {
			motor_encendido = !motor_encendido;
			if (motor_encendido) {
				sbi(PORTB, SAL_MOT);	// Encender el motor
				cont = 0;
				} else {
				cbi(PORTB, SAL_MOT);	// Apagar el motor
				apagar_leds();			// Apagar los LEDs cuando se apaga el motor
				T = T1;                // Restablecer la temporización a T1
				temp = T;              // Actualizar la variable temp también
			}
			flag_B1 = 0;
		}
		B1_ant = B1_act;

		// Manejo del botón de incremento de tiempo (PB2)
		B2_act = es_alto(PINB, BOTON_INC);
		if (B2_act && B2_ant)
		flag_B2 = 1;
		if (!B2_act && !B2_ant && flag_B2) {
			if (!motor_encendido && T < T5) {
				T = (T == T1) ? T2 : (T == T2) ? T3 : (T == T3) ? T4 : T5;
				temp = T;
				actualizar_leds(T);  // Actualizar los LEDs con el nuevo tiempo seleccionado
			}
			flag_B2 = 0;
		}
		B2_ant = B2_act;

		// Manejo del botón de decremento de tiempo (PB3)
		B3_act = es_alto(PINB, BOTON_DEC);
		if (B3_act && B3_ant)
		flag_B3 = 1;
		if (!B3_act && !B3_ant && flag_B3) {
			if (!motor_encendido && T > T1) {
				T = (T == T5) ? T4 : (T == T4) ? T3 : (T == T3) ? T2 : T1;
				temp = T;
				actualizar_leds(T);  // Actualizar los LEDs con el nuevo tiempo seleccionado
			}
			flag_B3 = 0;
		}
		B3_ant = B3_act;

		if (motor_encendido) {
			if (cont >= temp) {
				motor_encendido = 0;
				cbi(PORTB, SAL_MOT);  // Apagar el motor cuando termina el tiempo
				apagar_leds();         // Apagar todos los LEDs
				T = T1;                // Restablecer la temporización a T1
				temp = T;              // Actualizar la variable temp también
				} else {
				cont++;  // Incrementa el contador de tiempo en 10 ms

				// Determina el valor de tiempo_restante para los LEDs
				unsigned int tiempo_restante = temp - cont;

				if (tiempo_restante > 6000) {
					T = T5;  // Entre 1 y 2 minutos, todos los LEDs encendidos
					} else if (tiempo_restante > 2000) {
					T = T4;  // Entre 20 segundos y 1 minuto, LEDs T1 a T4 encendidos
					} else if (tiempo_restante > 1000) {
					T = T3;  // Entre 10 y 20 segundos, LEDs T1 a T3 encendidos
					} else if (tiempo_restante > 500) {
					T = T2;  // Entre 5 y 10 segundos, LEDs T1 a T2 encendidos
					} else {
					T = T1;  // Entre 0 y 5 segundos, solo LED T1 encendido
				}

				// Reutiliza la función actualizar_leds para encender los LEDs según T
				actualizar_leds(T);
			}
		}

		_delay_ms(10);  // Delay de 10 ms (100 * 10 ms = 1 segundo)
	}
}

// Inicialización de los puertos de entrada/salida
void inicializar_io() {
	DDRB = 0xF1;  // PB0 como salida (motor), PB1, PB2, PB3 como entradas (botones)
	DDRC = 0xFF;  // PC0 a PC4 como salidas (LEDs)
	PORTB = 0x00; // Inicializar pines del motor y botones
	PORTC = 0x00; // Inicializar LEDs
}

// Actualizar los LEDs según el tiempo seleccionado
void actualizar_leds(int T) {
	apagar_leds();  // Apagar los LEDs antes de actualizar
	switch (T) {
		case T1: sbi(PORTC, 0); break;
		case T2: sbi(PORTC, 0); sbi(PORTC, 1); break;
		case T3: sbi(PORTC, 0); sbi(PORTC, 1); sbi(PORTC, 2); break;
		case T4: sbi(PORTC, 0); sbi(PORTC, 1); sbi(PORTC, 2); sbi(PORTC, 3); break;
		case T5: sbi(PORTC, 0); sbi(PORTC, 1); sbi(PORTC, 2); sbi(PORTC, 3); sbi(PORTC, 4); break;
	}
}

// Apagar todos los LEDs
void apagar_leds() {
	PORTC = 0x00;  // Apagar todos los LEDs
}