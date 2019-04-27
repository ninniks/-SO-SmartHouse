/**
 * @author  Nicola Giuseppe Ninni
 * @avr-gcc (GCC) 5.4.0
 * @avrdude version 6.3
 */

#include <stdio.h>
#include <stdint.h>
#include "serial.h"
#define SIZE 256

//volatile uint8_t * const data = (uint8_t *) 0x25;
//volatile uint8_t * const dir  = (uint8_t *) 0x24;

//macros for handling memory addresses of I/O ports
#define PORTA   (* (volatile uint8_t *) 0x22)
#define PORTB   (* (volatile uint8_t *) 0x25)
#define DDRA    (* (volatile uint8_t *) 0x21)
#define DDRB    (* (volatile uint8_t *) 0x24)

int8_t name_is_set = 0;
const char *token = ":";
const uint8_t sname[] = "sname";
const char *scname = "scname";
const char *scval = "scval";
const char *qych = "qych";

void parse_string(uint8_t* src, uint8_t* key, uint8_t* value){
  while(*src != ':'){
    uint8_t c = *src;
    *key = c;
    ++src;
    ++key;
  }
  *key='\0';

  ++src;

  while(*src && *src != ';'){
    uint8_t c = *src;
    *value = c;
    ++src;
    ++value;
  }
  *value='\0';
  return;
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
  uint8_t key[SIZE];
  uint8_t value[SIZE];

  serial_init();

  while(1){
    serial_get_string(buf);
    parse_string(buf, key, value);
    if(my_strcmp(key,sname) == 0 && name_is_set == 0){
      serial_put_string((uint8_t *) "name setted.");
    }
  }
}
