#include <stdio.h>
#include <string.h>
#include "avr_client_serial.h"
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "loop.h"

#define SNAME "sname:"
#define SCNAME "scname:"
#define SCVAL "scval:"
#define QYCH "qych:"

int fd;
const char *ack = "ok";
const char *handshake = "ACMino";
const char *portname = "/dev/ttyACM0";
char buf = '\0';
char string[256];
char *name = NULL;
//builtin functions of smart house

int set_name(char **args);
int set_channel_name(char **args);
int set_channel_value(char **args);
int query_channels(char **args);
int help(char **args);

char *builtin_str[] = {
  "set_name",
  "set_channel_name",
  "set_channel_value",
  "query_channels",
  "help"
};

int (*builtin_func[]) (char **) = {
  &set_name,
  &set_channel_name,
  &set_channel_value,
  &query_channels,
  &help
};

int avr_connection_init(){

  fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
  if(fd < 0){
    fprintf(stderr, "Error opening device: %s\n", strerror(errno));
    return 0;
  }

  serial_init(fd, B19200, 0);
  set_blocking(fd, 0);

  /*write(fd, handshake, sizeof(handshake));

  read(fd, &buf, sizeof(ack));

  if(buf == *ack)
    return 0;*/
  return 1;
}

int avr_client_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int set_name(char **args){
  if(args[1] == NULL){
    fprintf(stderr, "avr_client: expected argument to \"set_name\"\n");
  }else{
    snprintf(string, sizeof string, "%s%s%s%s", SNAME, args[1],";","\r");
    write(fd,string, strlen(string));
  }
}

int set_channel_name(char **args){
  if(args[1] == NULL){
    fprintf(stderr, "avr_client: expected argument to \"set_channel_name\"\n");
  }else{

  }
}

int set_channel_value(char **args){
  if(args[1] == NULL){
    fprintf(stderr, "avr_client: expected argument to \"set_channel_value\"\n");
  }else{
    fprintf(stderr, "avr_client: no device name set. Please use \"set_name\" \n ");
  }
}

int query_channels(char **args){
  if(args[1] == NULL){
    fprintf(stderr, "avr_client: expected argument to \"query_channels\"\n");
  }
}

int help(char **args){
  if(args[1] == NULL){
    fprintf(stderr, "avr_client: expected argument to \"set_name\"\n");
  }
}


int avr_client_execute(char **args){
  int i;

  if(args[0] == NULL){
    return 1;
  }

  for (i = 0; i < avr_client_num_builtins(); i++) {
   if (strcmp(args[0], builtin_str[i]) == 0) {
     return (*builtin_func[i])(args);
   }
 }

 return 1;
}
