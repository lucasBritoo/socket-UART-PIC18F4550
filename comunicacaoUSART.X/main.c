#include	<xc.h>
#include <stdio.h>
#include <pic18f4550.h>
#include "config.h"
#include "lcd.h"


#define _XTAL_FREQ 20000000
#define led PORTBbits.RB0
#define botao PORTBbits.RB1
#define led2 PORTBbits.RB2
char serial;
char envia = 'H';
//int x =0;

void __interrupt(high_priority) tmr (void){
    if(RCIF){
        
        char serial = RCREG;
        lcd_putch(serial);
        //x = x+1;
        RCIF =0;
        
        TXREG = envia;
    }
}

void main(void) {
    configura();
    PORTD = 0;
    PORTE = 0;
    
    lcd_init();     //inicia o lcd
    lcd_clear();
    led = 0;
    led2 = 0;
    
    //lcd_puts("OIEE");
    
    while(1){
        //lcd_puts("OIEE");
        led =1;
        __delay_ms(500);
        led = 0;
        __delay_ms(500);
    }
    
}


