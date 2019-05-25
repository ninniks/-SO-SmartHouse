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

#define DIGITAL_OUT "0"
#define DIGITAL_IN  "1"
#define ANALOG "2"
#define ACK "ack"

const char *ack = "ok";
const char *handshake = "ACMino";
const char *portname = "/dev/ttyACM0";
char buf = '\0';
char string[512];
char name[256];
bool name_is_set = false;
int fd;
//builtin functions of smart house

int set_name(char **args);
int set_channel_name(char **args);
int set_channel_value(char **args);
int query_channels(char **args);
int get_channel_value(char **args);
int get_adc_channel_value(char **args);


char *user_dout_channels_names[8] = { NULL };
char *user_din_channels_names[8] = { NULL };
char *user_adc_channels_names[8] = { NULL };

char *default_adc_channels_name[] = {
  "analog_in_0",
  "analog_in_1",
  "analog_in_2",
  "analog_in_3",
  "analog_in_4",
  "analog_in_5",
  "analog_in_6",
  "analog_in_7",
};

char *default_din_channels_name[] = {
  "digital_in_0",
  "digital_in_1",
  "digital_in_2",
  "digital_in_3",
  "digital_in_4",
  "digital_in_5",
  "digital_in_6",
  "digital_in_7"
};

char *default_dout_channels_name[] = {
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
  "get_channel_value",
  "get_adc_channel_value",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &set_name,
  &set_channel_name,
  &set_channel_value,
  &query_channels,
  &get_channel_value,
  &get_adc_channel_value
};

int avr_connection_init(){


  fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
  if(fd < 0){
    fprintf(stderr, "Error opening device: %s\n", strerror(errno));
    return 0;
  }

  serial_init(fd, B19200, 0);
  set_blocking(fd, 0);
  return 1;
}

int avr_client_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int avr_channels_num(char **channels){
  return sizeof(channels)/ sizeof(char );
}

int avr_get_conf_switch(char **args, char **channels){
  int i;
  for(i = 0; i < avr_channels_num(channels); i++){
    if(channels[i] != NULL){
      if(strcmp(args[1], channels[i]) == 0)
        return i;
    }
  }
  return -1;
}

int avr_check_switches_name(char **args, char **channels){
  int i;
  for(i = 0; i < avr_channels_num(channels); i++){
    if(strcmp(args[1], channels[i]) == 0){
      return i;
    }
  }

  return -1;
}

int avr_chech_user_channel_input(char **args, char *channel_name, int size){
  if(args[2] != NULL && (strncmp(channel_name, args[1], size) == 0)){
    return 0;
  }
  return 1;
}

void avr_write_channel_name(char **args, char *channels[], int p){
  if(channels[p] == NULL){
    channels[p] = (char *) malloc(sizeof(args[2])*sizeof(char));
    strcpy(channels[p], args[2]);
    printf("Done.\n");
  }
}

void print_channels(char *channels[]){
  int c;
  for(c = 0; c < avr_channels_num(channels); c++){
    if(channels[c] == NULL){
      printf("empty\n");
    }else{
      printf("%s\n", channels[c]);
    }
  }
}

int set_name(char **args){
  if(args[1] == NULL){
    fprintf(stderr, "avr_client: expected argument to \"set_name\"\n");
  }else{
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
    if(avr_chech_user_channel_input(args, "switch_", 7) == 0){
      p = avr_get_conf_switch(args, default_dout_channels_name);
      if(p != -1){
        avr_write_channel_name(args, user_dout_channels_names, p);
      }else{
        fprintf(stderr,"avr_client: no channnel named: %s\n",args[1]);
      }
    }else if(avr_chech_user_channel_input(args, "digital_in_", 11) == 0) {
      p = avr_get_conf_switch(args, default_din_channels_name);
      if(p != -1){
        avr_write_channel_name(args, user_din_channels_names, p);
      }else{
        fprintf(stderr,"avr_client: no channnel named: %s\n",args[1]);
      }
    }else if(avr_chech_user_channel_input(args, "analog_in_", 10) == 0){
      p = avr_get_conf_switch(args, default_adc_channels_name);
      if(p != -1){
        avr_write_channel_name(args, user_adc_channels_names, p);
      }else{
        fprintf(stderr,"avr_client: no channnel named: %s\n",args[1]);
      }
    }else{
      fprintf(stderr, "avr_client: \"set_channel_name\" usage: <channelDefaultName_(n)> <name>. \n");
    }
  }else{
    fprintf(stderr, "avr_client: please set a name to the device. \n");
  }
}

int set_channel_value(char **args){
  int pos;
  if(args[1] == NULL){
    fprintf(stderr, "avr_client: expected argument to \"set_channel_value\"\n");
  }else if(name_is_set){
    pos = avr_get_conf_switch(args, user_dout_channels_names);
    if(pos != -1){
      if((strcmp(args[2],"0") != 0) && (strcmp(args[2], "1")!= 0)){
        fprintf(stderr, "avr_client: invalid switch value.\n");
      }else{
        char *s_checksum = malloc(4*sizeof(char));
        char c[256];
        sprintf(c,"%d",pos);
        snprintf(string, sizeof string, "%s%s%s%s%s%s%s", DIGITAL_OUT, ":", c , ":", args[2], ";", "\r");
        write(fd, string, strlen(string));
        read(fd, string, 4)
     }
    }else{
      fprintf(stderr, "avr_client: no switch with name: %s", args[1]);
    }
  }else{
    fprintf(stderr, "avr_client: no device name set. Please use \"set_name\" \n ");
  }
}

int get_channel_value(char **args){
  int pos;
  if(args[1] == NULL){
    fprintf(stderr, "avr_client: expected argument to \"get_channel_value\"\n");
  }else if(name_is_set){
    pos = avr_get_conf_switch(args, user_din_channels_names);
    if(pos != -1){
        char c[256];
        sprintf(c,"%d",pos);
        snprintf(string, sizeof string, "%s%s%s%s%s%s%s", DIGITAL_IN, ":", c , ":", args[2], ";","\r");
        write(fd, string, strlen(string));
        read(fd, string, sizeof(1));
     }
    }else{
      fprintf(stderr, "avr_client: no switch with name: %s", args[1]);
    }
}

int get_adc_channel_value(char **args){
  int pos;
  if(args[1] == NULL){
    fprintf(stderr, "avr_client: expected argument to \"get_channel_value\"\n");
  }else if(name_is_set){
    pos = avr_get_conf_switch(args, user_adc_channels_names);
    if(pos != -1){
        char c[256];
        char res[3];
        sprintf(c,"%d",pos);
        snprintf(string, sizeof string, "%s%s%s%s%s%s%s", ANALOG, ":", c , ":", "0", ";","\r");
        write(fd, string, strlen(string));
        read(fd, res, 4);
        printf("%s", res);
        memset(&res, 0, sizeof(res));
     }
    }else{
      fprintf(stderr, "avr_client: no switch with name: %s", args[1]);
    }
}

int query_channels(char **args){
  printf("\n DIGITAL OUT:\n");
  print_channels(user_dout_channels_names);
  printf("\n DIGITAL IN:\n");
  print_channels(user_din_channels_names);
  printf("\n ANALOG IN:\n");
  print_channels(user_adc_channels_names);
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

 fprintf(stderr, "avr_client: command not found. Please use \"help\" for info.\n");

 return 1;
}
