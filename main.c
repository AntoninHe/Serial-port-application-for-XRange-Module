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
#include <string.h> // needed for memset
#include <stdbool.h>

#define MSG_YES '!'
#define MSG_NO '?'
#define BUFFER_SIZE 200

void raw_mode (fd, old_term)
int fd;
struct termios *old_term;
{
    struct termios term;
    register int i;

    tcgetattr(fd, &term);
    tcgetattr(fd, old_term);

    /* mode RAW */
#if 0
    term.c_iflag = IGNBRK;
    term.c_lflag = 0;
    term.c_oflag = 0;
    term.c_cflag |= (CREAD | CRTSCTS);
    term.c_cc[VMIN] = 1; /* 1 char is enough ! */
    term.c_cc[VTIME] = 0;
    term.c_iflag &= ~(IXON|IXOFF|IXANY);
    term.c_cflag &= ~(PARENB | CSIZE);
#endif
    // Plus simple ! 
    cfmakeraw (&term);
    
    if(cfsetspeed(&term, B57600) != 0 )// 115200 baud
        printf("Error set speed\n");

    if(tcsetattr(fd, TCSANOW, &term) != 0)
        printf("Error set attr\n");
}

int readMsg(int fd, char *buffer){
    int i=0;
    char c;
    //while(read(fd, &c, 1) < 1); // read the space
    // printf("space read %c",c);
    c=0;
    while( c!= ' '){ 
        while(read(fd, &c, 1) < 1); // read the message
        buffer[i] = c; 
        i++;
        //printf("bcl :%c ",c);// DEBUG
        if(i + 2 > BUFFER_SIZE){
            printf("error read msg");
            break;
        }
    }
    buffer[ i++ ] = '\0'; // DEBUG
    printf("\n\ng%s \n", buffer);
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
    /*
    r = ' ';
    if ( write(fd, &r, 1)  != 1){
        printf("error");
        return -1; 
    }*/
    return 0;
}

int flushWithSpace(int fd){
    // TO DO add Flush
    char buffer[10]={' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};

    if (write(fd, buffer, 10) > 0){ // write YES
        return 0;
    }
    return -1;
}

int main(int argc,char** argv)
{
        struct termios stdio;
        int tty_fd;
        fd_set rdset;
        
	struct termios old;
        int i;
        char buffer[BUFFER_SIZE];

        char c='D';
        bool new_msg = true;

        char bufferW[10]={'1','2','3','4','5','6','7','8','9',' '};

        tty_fd = open(argv[1], O_RDWR );//| O_NONBLOCK);        // O_NONBLOCK might override VMIN and VTIME, so read() may return immediately.

        if(tty_fd == -1)
            return -1;

        printf("File descriptor : %d \n",tty_fd);
        raw_mode(tty_fd, &old);
        printf("Pass to raw mode\n");

        tcflush(tty_fd, TCIFLUSH);
        printf("Flush\n");

        while (c!='q')
        {
                if (read(tty_fd,&c,1) > 0 && (c == MSG_YES || c == MSG_NO )){
                    //printf("Caractère détecté %d %c \n",c);

                    if(c == MSG_YES || c == MSG_NO){
                        say_Y_N(tty_fd, new_msg);
                        //printf("responded to the device\n");
                        if(c == MSG_YES)
                        {
                            memset(&buffer, 0, sizeof(char) * BUFFER_SIZE); 
                            readMsg(tty_fd, buffer);
                            printf("MSG Yes received\n");
                        }
                        else //MSG_NO 
                        {
                            printf("MSG NO received\n");    
                        }
                        if( new_msg == true){
                            
                            if (write(tty_fd, bufferW, 10) == 10){ // write YES
                                printf("Msg Send\n");
                            }
                            else{
                                printf("fail send\n");
                            }
                        }

                    }
                    else{
                        printf("yes nor no\n");
                        //flushWithSpace(tty_fd);
                    }

                tcsetattr(tty_fd, TCSANOW, &old);
                }
        }
        close(tty_fd);
}
