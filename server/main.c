/**
 * @author  Nicola Giuseppe Ninni
 * @avr-gcc (GCC) 5.4.0
 * @avrdude version 6.3
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "uart.h"
#include <avr/io.h>

uint8_t o_mask;
uint8_t i_mask;


struct UART* uart;

void printString(char* s){
  int l=strlen(s);
  for(int i=0; i<l; ++i, ++s)
    UART_putChar(uart, (uint8_t) *s);
}


void set_output_channels(void){ //setting port to output mode
  const uint8_t out = 255;
  DDRB |= out;
}

void set_input_channels(void){ //setting input mode channels
  const uint8_t in = 0x00;
  DDRA = in;
}

//adc function with interrupts

void ADC_init(void){
  // Select Vref=AVcc
  ADMUX |= (1<<REFS0);
  //set prescaller to 128 and enable ADC
  ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);

}

uint16_t adc_read(uint8_t channel){
  channel &= 0b00000111;  // AND operation with 7
  ADMUX = (ADMUX & 0xF8)|channel; // clears the bottom 3 bits before ORin
  //single conversion mode
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1<<ADSC);
  // wait until ADC conversion is complete
  while(ADCSRA & (1<<ADSC));

  return ADC;
}

//my string parser
void parse_string(char* src, char* name, char* i_type, char* channel, char* value){
  int word = 0;

  while(*src != ';'){
    char c = *src;
    if(*src == ':'){
      ++src;
      c = *src;
      word+=1;
    }

    switch(word){
      case NAME:
        *name = c;
        ++src;
        ++name;
        break;
      case INPUT_TYPE:
        *i_type = c;
        ++src;
        ++i_type;
        break;
      case CHANNEL:
        *channel = c;
        ++src;
        ++channel;
        break;
      case VALUE:
        *value = c;
        ++src;
        ++value;
        break;
    }
  }
  *name = '\0';
  *i_type = '\0';
  *channel = '\0';
  *value = '\0';
}

int8_t my_strcmp(const uint8_t* s1, const uint8_t* s2){
  while((*s1 != '\0') && (*s1 == *s2)){
    s1++;
    s2++;
  }
  return (int8_t)*s1 - (int8_t)*s2;
}

void getString(char* string){
  string[0] = 0;
  int size = 0;
  while(1){
    uint8_t c = UART_getChar(uart);
    string[size] = c;
    ++size;
    string[size] = 0;
    if (c == 0xFF ||c == '\n' || c=='\r' || c==0) {
      break;
    }
  }
}

void pwm_init(void){
  TCCR1A=TCCRA_MASK;
  TCCR2A=TCCRA1_MASK;
  TCCR1B=TCCRB_MASK;
  TCCR2B=TCCRB1_MASK;

  OCR2A = 255;
  OCR1BL = 255;
  OCR1AL = 255;
  OCR1CL = 255;
}


int main(void){
   uart = UART_init("uart_0", 115200);

  char buf[SIZE];
  char name[SIZE];
  char disp[SIZE];
  char channel[SIZE];
  char value[SIZE];
  char type[SIZE];
  char mychar[5];
 // serial_init();

  getString(buf);
  strcpy(name, buf);

  set_output_channels();
  set_input_channels();
  ADC_init();
  pwm_init();

  while(1){
    //serial_get_string(buf);
    getString(buf);
    //printString(buf);
    parse_string(buf, disp, type, channel, value);
    //printString(disp);
    /* printString(type);
    printString(channel);
    printString(value); */

    uint16_t adc_value;
    int t = type[0]-'0';
    int bit = channel[0]-'0';
    int val = atoi(value);
    strcat(disp, "\r");

    if(strcmp(disp,name) == 0){
      switch(t){
        case DIGITAL_OUT:
          if(bit < 4){
            printString(value);
            if(val > 0){
              PORTB |= (1<<bit);
            }else{
              PORTB &= ~(1 << (bit));
            }
          }else if(bit == 4){
            OCR2A = val;
          }else if(bit == 5){
            OCR1AL = val;
          }else if(bit == 6){
            OCR1BL = val;
          }else{
            OCR1CL = val;
          }
        break;

        case DIGITAL_IN:
          i_mask = (1<<bit);
          if((PINA & (1<< bit)) == i_mask){
            //serial_put_string((uint8_t *) "1");
            printString("1");
          }else{
            //serial_put_string((uint8_t *) "0");
            printString("0");
          }
        break;

        case ANALOG:
          adc_value = adc_read(bit);
          sprintf(mychar,"%04d",adc_value);
          //serial_put_string((uint8_t *) *(&mychar));
          printString(mychar);
        break;
      }
    }else{
      printString(NACK);
    }
  }
}
