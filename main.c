#include "serial_lora.h"
#include <termios.h>
#include <stdbool.h>
#include <stdio.h>
int main(){
    char port[] = "/dev/ttyACM0";
    //struct termios termios_old;
    //int fd;
    size_t size_buffer=200;
    char p_data_in[200];
    char p_data_out[200];
    // bool new_msg = false;
    printf("sdfasdf\n");

    serialExchange(port, p_data_in, size_buffer, p_data_out,size_buffer);

    printf("sdfasdf\n");
    /*fd = open_port(port, &termios_old);


    while(1){
        exchange(fd, p_data_in, size_buffer, p_data_out, size_buffer,new_msg);
    }

    close_port(port, &termios_old, fd);
    */
}

