/* 
 * author: Antonin Héréson
 * Serial port courses from :
 * http://pficheux.free.fr/articles/lmf/serial/
 * https://en.wikibooks.org/wiki/Serial_Programming/termios
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h> 
#include <stdbool.h>

#include "serial_lora.h"

#define MSG_YES '!'
#define MSG_NO '?'
#define BUFFER_SIZE 200

void raw_mode (fd, old_term)
int fd;
struct termios *old_term;
{
    struct termios term;

    tcgetattr(fd, &term);
    tcgetattr(fd, old_term);

    /* mode RAW */
    cfmakeraw (&term);
    
    if(cfsetspeed(&term, B57600) != 0 )
        printf("Error set speed\n");

    if(tcsetattr(fd, TCSANOW, &term) != 0)
        printf("Error set attr\n");
}

int readMsg(int fd, char *buffer, int size){
    int i=0;
    char c;
    c=0;
    while( c!= ' '){ 
        while(read(fd, &c, 1) < 1); // read the message
        buffer[i] = c; 
        i++;
        if(i + 2 > size){
            printf("error read msg overflow");
            break;
        }
    }
    buffer[ i++ ] = '\0'; // DEBUG
    printf("\n\n%s \n", buffer);// DEBUG
    return i;
}

int say_Y_N(int fd, bool new_msg){
    char r;
    if(new_msg == true) {
        r = MSG_YES;
    }
    else {
        r = MSG_NO;
    }
    if( write(fd, &r, 1) != 1){
        printf("error");
        return -1;
    }
    return 0;
}

int flushWithSpace(int fd){
    char c = ' ';
    if (write(fd, &c, 1) > 0){ 
        return 0;
    }
    return -1;
}

// ------------------------------------------------------------------------- //

int exchange(const int fd, const char *data_in, const size_t size_data_in, char *data_out, size_t size_data_out, const bool new_msg){
    char c;
    int size=-1;

    if (read(fd,&c,1) > 0 && (c == MSG_YES || c == MSG_NO )){

        if(c == MSG_YES || c == MSG_NO){
            say_Y_N(fd, new_msg);
            if(c == MSG_YES)
            {
                memset(&data_out, 0, sizeof(char) * size_data_out); 
                size = readMsg(fd, data_in, size_data_in);
                printf("MSG Yes received\n"); // DEBUG
            }
            else //MSG_NO 
            {
                printf("MSG NO received\n"); // DEBUG   
                size = 0;
            }
            if( new_msg == true){
                    
                if (write(fd, data_out, size_data_out) == size_data_out){ // write YES
                    printf("Msg Send\n"); // DEBUG
                    return size;
                }
                else{
                    printf("Fail send\n");
                    return -2;
                }
            }
        }
        else{
            printf("yes nor no\n");
            return -1;
        }
    }
    return 0;
}
 
int open_port(const char *p_port, struct termios *p_old){
    int tty_fd;

    tty_fd = open(p_port, O_RDWR );

    if(tty_fd == -1)
        return -1;

    printf("File descriptor : %d \n",tty_fd);

    raw_mode(tty_fd, p_old);

    printf("Pass to raw mode\n");

    tcflush(tty_fd, TCIFLUSH);
    flushWithSpace(tty_fd);

    usleep(100);
    printf("Flush\n");
    
    return tty_fd;
}

int close_port(const char p_port[], struct termios *p_old, int fd){
    tcsetattr(fd, TCSANOW, p_old);

    if(close(fd) == 0) 
        return 0;
    return -1;
}

// ------------------------------------------------------------------------- //


int serialExchange(char *port, char *data_in, size_t size_data_in, char *data_out, size_t size_data_out){

        int tty_fd;
        
	struct termios old;
        //char buffer[BUFFER_SIZE]={'5'};

        char c='D';
        bool new_msg = false;

        printf("try open port\n");

        tty_fd = open(port, O_RDWR );

        if(tty_fd == -1)
            return -1;

        printf("File descriptor : %d \n",tty_fd);
        raw_mode(tty_fd, &old);
        printf("Pass to raw mode\n");

        tcflush(tty_fd, TCIFLUSH);
        flushWithSpace(tty_fd);
        usleep(100);
        printf("Flush\n");

        while (c!='q')
        {
                if (read(tty_fd,&c,1) > 0 && (c == MSG_YES || c == MSG_NO )){

                    if(c == MSG_YES || c == MSG_NO){
                        say_Y_N(tty_fd, new_msg);
                        if(c == MSG_YES)
                        {
                            memset(&data_out, 0, sizeof(char) * size_data_out); 
                            readMsg(tty_fd, data_out, size_data_out);
                            printf("MSG Yes received\n"); // DEBUG
                        }
                        else //MSG_NO 
                        {
                            printf("MSG NO received\n"); // DEBUG   
                        }
                        if( new_msg == true){
                            
                            if (write(tty_fd, data_in, size_data_in) == size_data_in){ // write YES
                                printf("Msg Send\n"); // DEBUG
                            }
                            else{
                                printf("Fail send\n");
                            }

                        }
                    }
                    else{
                        printf("yes nor no\n");
                    }
                tcsetattr(tty_fd, TCSANOW, &old);
                }
        }
        close(tty_fd);
        return 0;
}

