/**
 * @author  Nicola Giuseppe Ninni
 * @avr-gcc (GCC) 5.4.0
 * @avrdude version 6.3
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "serial.h"
#include "utils.h"

uint8_t o_mask;
uint8_t i_mask;

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
void parse_string(uint8_t* src, uint8_t* name, uint8_t* i_type, uint8_t* channel, uint8_t* value){
  int word = 0;

  while(*src != ';'){
    uint8_t c = *src;
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

int main(void){
  uint8_t name[SIZE];
  uint8_t buf[SIZE];
  uint8_t disp[SIZE];
  uint8_t channel[SIZE];
  uint8_t value[SIZE];
  uint8_t type[SIZE];

  char mychar[5];
  serial_init();
  serial_get_string((uint8_t *) buf);
  strcpy((char *) name,(char *) buf);

  set_output_channels();
  set_input_channels();
  ADC_init();

  while(1){
    serial_get_string(buf);
    parse_string(buf, disp, type, channel, value);

    uint16_t adc_value;
    int t = type[0]-'0';
    int bit = channel[0]-'0';
    strcat((char *)disp, "\r");

    if(my_strcmp(disp,name) == 0){
      switch(t){
        case DIGITAL_OUT:
          o_mask = (1<<bit);
          if(strcmp("1",(char *)value) == 0){
            PORTB |= (1<<bit);
          }else if(strcmp("0",(char *)value) == 0){
            PORTB &= ~(1 << (bit));
          }
        break;

        case DIGITAL_IN:
          i_mask = (1<<bit);
          if((PINA & (1<< bit)) == i_mask){
            serial_put_string((uint8_t *) "1");
          }else{
            serial_put_string((uint8_t *) "0");
          }
        break;

        case ANALOG:
          adc_value = adc_read(bit);
          sprintf(mychar,"%04d",adc_value);
          serial_put_string((uint8_t *) *(&mychar));
        break;
      }
    }else{
      serial_put_string((uint8_t *) NACK);
    }
  }
}
