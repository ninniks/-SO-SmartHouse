/**
 * @author  Nicola Giuseppe Ninni
 * @gcc (Ubuntu 8.2.0-7ubuntu1) 8.2.0
 * Copyright (C) 2018 Free Software Foundation, Inc.
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include <stdlib.h>
#include <stdio.h>
#include "shell.h"
#include "avr_client_functions.h"

int main(int argc, char const *argv[]){

  if(avr_connection_init() == 0){
    fprintf(stderr, "Connection problem to the host\n");
    return EXIT_FAILURE;
  }
  avr_client_loop();

  return EXIT_SUCCESS;
}
