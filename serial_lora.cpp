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

#include <iostream>             // sdt::cout, sdt::cin, sdt::endl
#include <list>                 // sdt::list
#include <mutex>                // std::mutex, std::unique_lock
#include <condition_variable>   // std::condition_variable

#define MSG_YES '!'
#define MSG_NO '?'

extern std::mutex myMutex;
extern std::list<std::string>myList;
extern std::condition_variable cv;
extern int done;

void raw_mode (int fd, struct termios *old_term)
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

int read_msg(int fd, char *buffer, size_t buffer_size){
    int i=0;
    char c=0;
    while( c!= ' '){ 
        while(read(fd, &c, 1) < 1); // read the message
        buffer[i] = c; 
        i++;
        if(i + 2 > buffer_size){
            printf("error read msg overflow");
            break;
        }
    }
    buffer[ i++ ] = '\0'; // DEBUG
    {
        std::lock_guard<std::mutex> lk(myMutex);
        std::string mystring(buffer);
        myList.push_back(mystring);
    }
    done = 1;
    cv.notify_one();
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
        printf("error write");
        return -1;
    }
    return 0;
}

int flush_with_space(int fd){
    char c = ' ';
    if (write(fd, &c, 1) > 0){ 
        return 0;
    }
    return -1;
}

int serial_exchange(const char *port, char *p_data_in, size_t size_data_in, char *p_data_out, size_t size_data_out){
        int tty_fd;
        
	struct termios old;

        char c='D';
        bool new_msg = false;

        printf("try open port\n");

        tty_fd = open(port, O_RDWR);

        if(tty_fd == -1)
            return -1;

        printf("File descriptor : %d \n",tty_fd);
        raw_mode(tty_fd, &old);
        printf("Pass to law mode\n");

        tcflush(tty_fd, TCIFLUSH);
        flush_with_space(tty_fd);
        while (c!='q')
        {
            c=0;
            if (read(tty_fd,&c,1) > 0 ){
                if(c == MSG_YES || c == MSG_NO){
                    usleep(10000);
                    say_Y_N(tty_fd, new_msg);
                    if(c == MSG_YES)
                    {
                        //memset(p_data_out, 0, sizeof(char) * size_data_out); 
                        //printf("MSG Yes received\n"); // DEBUG
                        read_msg(tty_fd, p_data_out, size_data_out);
                    }
                    else //MSG_NO 
                    {
                        //printf("MSG NO received\n"); // DEBUG   
                    }
                    if( new_msg == true){
                        
                        if (write(tty_fd, p_data_in, size_data_in) == size_data_in){ // write YES
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
            }
        }
        printf("close port\n");
        tcsetattr(tty_fd, TCSANOW, &old);
        close(tty_fd);
        return 0;
}
