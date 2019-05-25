/**
 * @author  Nicola Giuseppe Ninni
 * @avr-gcc (GCC) 5.4.0
 * @avrdude version 6.3
 */

#include <stdio.h>
#include <stdint.h>
#include "serial.h"
#define SIZE 256
#define DIGITAL_OUT 0
#define DIGITAL_IN 1
#define ANALOG 2
#define INPUT_TYPE 0
#define CHANNEL 1
#define VALUE 2
#define ACK "ack"
#define NACK "nack"

//macros for handling memory addresses of I/O ports

#define PORTA   (* (volatile uint8_t *) 0x22) //PORTA is used for digital in
#define PORTB   (* (volatile uint8_t *) 0x25) //PORTB is used for digital out
#define DDRA    (* (volatile uint8_t *) 0x21) //Data Direction Register to setup I/O
#define DDRB    (* (volatile uint8_t *) 0x24)
#define PINA    (* (volatile uint8_t *) 0x20) //PORTA input pins
#define ADMUX   (* (volatile uint8_t *) 0x7C) //ADC Multiplexer section register
#define ADCSRA  (* (volatile uint8_t *) 0x7A) //ADC Status and Control Register
#define ADC     (* (volatile uint16_t*) 0x78) //ADC Data Register 16 bit


#define REFS0  6 //ADMUX bit 0 select voltage reference for ADC
#define ADPS0  0 //ADCSRA bits. These bits determine the division factor between the XTAL frequency and the input clock to the ADC.
#define ADPS1  1
#define ADPS2  2
#define ADEN   7 //ADCSRA bit 7 set to 1 enables the ADC
#define ADSC   6 //ADC start conversion

uint8_t o_mask;
uint8_t i_mask;

void set_output_channels(void){
  const uint8_t out = 255;
  DDRB |= out;
}

void set_input_channels(void){
  const uint8_t in = 0x00;
  DDRA = in;
}


void ADC_init(void){
  // Select Vref=AVcc
  ADMUX |= (1<<REFS0);
  //set prescaller to 128 and enable ADC
  ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);

}

uint16_t adc_read(uint8_t channel){
  //select ADC channel
  ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
  //single conversion mode
  ADCSRA |= (1<<ADSC);
  // wait until ADC conversion is complete
  while(ADCSRA & (1<<ADSC));

  return ADC;
}


void parse_string(uint8_t* src, uint8_t* type, uint8_t* channel, uint8_t* value, uint8_t* chksum){
  int word = 0;

  while(*src != ';'){
    uint8_t c = *src;
    if(*src == ':'){
      ++src;
      c = *src;
      word+=1;
    }

    switch(word){
      case INPUT_TYPE:
        *type = c;
        ++src;
        ++type;
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
      case CHECKSUM:
        *chksum = c;
        ++src;
        ++chksum;
        break;
    }
  }
  *type = '\0';
  *channel = '\0';
  *value = '\0';
  *chksum = '\0';
}

int8_t my_strcmp(const uint8_t* s1, const uint8_t* s2){
  while((*s1 != '\0') && (*s1 == *s2)){
    s1++;
    s2++;
  }
  return (int8_t)*s1 - (int8_t)*s2;
}

int main(void){
  uint8_t buf[SIZE];
  uint8_t channel[SIZE];
  uint8_t value[SIZE];
  uint8_t type[SIZE];

  char mychar[5];
  serial_init();
  set_output_channels();
  set_input_channels();
  ADC_init();


  while(1){
    serial_get_string(buf);
    parse_string(buf, type, channel, value, checksum);

    uint16_t adc_value;
    int t = type[0]-'0';
    int bit = channel[0]-'0';

    switch(t){
      case DIGITAL_OUT:
        o_mask = (1<<bit);
        if(my_strcmp((uint8_t *)"1",value) == 0){
          PORTB |= (1<<bit);
          serial_put_string((uint8_t *) ACK);
        }else if(my_strcmp((uint8_t *)"0", value) == 0){
          PORTB &= ~(1 << (bit));
          serial_put_string((uint8_t *) ACK);
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
  }
}
