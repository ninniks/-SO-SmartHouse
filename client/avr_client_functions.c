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
#include <stdbool.h>

#define SNAME "sname:"
#define SCNAME "scname:"
#define SCVAL "scval:"
#define QYCH "qych:"

int fd;
const char *ack = "ok";
const char *handshake = "ACMino";
const char *portname = "/dev/ttyACM0";
char buf = '\0';
char string[512];
char name[256];
bool name_is_set = false;
//builtin functions of smart house

int set_name(char **args);
int set_channel_name(char **args);
int set_channel_value(char **args);
int query_channels(char **args);
int help(char **args);


char *user_switches_name[8] = { NULL };

char *default_switches_name[] = {
  "switch_0",
  "switch_1",
  "switch_2",
  "switch_3",
  "switch_4",
  "switch_5",
  "switch_6",
  "switch_7"
};

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

int avr_default_switches_num(){
  return sizeof(default_switches_name)/ sizeof(char *);
}

int avr_user_switches_num(){
  return sizeof(user_switches_name)/sizeof(char *);
}

int avr_get_conf_switch(char **args){
  int i;
  for(i = 0; i < avr_user_switches_num(); i++){
    if(user_switches_name[i] != NULL){
      if(strcmp(args[1], user_switches_name[i]) == 0)
        return i;
    }
  }
  return -1;
}

int avr_check_switches_name(char **args){
  int i;
  for(i = 0; i < avr_default_switches_num(); i++){
    if(strcmp(args[1], default_switches_name[i]) == 0){
      return i;
    }
  }

  return -1;
}

void print_user_channels(){
  int c;
  for(c = 0; c < avr_user_switches_num(); c++){
    if(user_switches_name[c] == NULL){
      printf("empty\n");
    }else{
      printf("%s\n", user_switches_name[c]);
    }
  }
}

int set_name(char **args){
  if(args[1] == NULL){
    fprintf(stderr, "avr_client: expected argument to \"set_name\"\n");
  }else{
    //snprintf(string, sizeof string, "%s%s%s%s", SNAME, args[1],";","\r");
    //write(fd,string, strlen(string));
    snprintf(name, sizeof name, "%s", args[1]);
    name_is_set = true;
  }
}

int set_channel_name(char **args){
  int p;
  char *name;
  if(args[1] == NULL){
    fprintf(stderr, "avr_client: expected argument to \"set_channel_name\"\n");
  }else if(name_is_set){
    if(args[2] != NULL){
      p = avr_check_switches_name(args);
      if(p != -1){
        if(user_switches_name[p] == NULL){
          user_switches_name[p] = (char *) malloc(sizeof(args[2])*sizeof(char));
          strcpy(user_switches_name[p], args[2]);
        }
      }else{
        fprintf(stderr,"avr_client: no channnel named: %s\n",args[1]);
      }
    }else {
      fprintf(stderr, "avr_client: \"set_channel_name\" usage: <switch_(n)> <name>\n");
    }
  }else{
    fprintf(stderr, "avr_client: please set a name to the device. Use \"help\" for info\n");
  }
}

int set_channel_value(char **args){
  int pos;
  if(args[1] == NULL){
    fprintf(stderr, "avr_client: expected argument to \"set_channel_value\"\n");
  }else if(name_is_set){
    pos = avr_get_conf_switch(args);
    printf("%d", pos);
    if(pos != -1){
      if((strcmp(args[2],"0") != 0) && (strcmp(args[2], "1")!= 0)){
        fprintf(stderr, "avr_client: invalid switch value.\n");
      }else{
        char c[256];
        sprintf(c,"%d",pos);
        snprintf(string, sizeof string, "%s%s%s%s%s", c , ":", args[2], ";","\r");
        write(fd, string, strlen(string));
     }
    }else{
      fprintf(stderr, "avr_client: no switch with name: %s", args[1]);
    }
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
  }else{
      print_user_channels();
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
