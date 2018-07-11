/* 
 * author: Antonin Héréson
 * Serial port courses from :
 * http://pficheux.free.fr/articles/lmf/serial/
 * https://en.wikibooks.org/wiki/Serial_Programming/termios
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h> // needed for memset
#include <stdbool.h>


#include <condition_variable>   // std::condition_variable
#include <iostream>             // sdt::cout, sdt::cin, sdt::endl
#include <list>                 // std::list
#include <mutex>                // std::mutex, std::unique_lock
#include <queue>                // std::queue
#include <tuple>                // std::tuple

#include "mbedtls/base64.h"

#define MSG_YES '!'
#define MSG_NO '?'

#define BASE64BUFFERSIZE 500

bool new_msg = false;

extern std::mutex mutex_serial_port_read;
extern std::condition_variable cv_serial_port;
extern int done_serial_port;

extern std::queue < std::tuple<char *,int> >msg_queue_r;

extern char *p_msg_user;
extern int msg_size_user;

extern std::mutex mutex_serial_port_read_send;
extern std::condition_variable cv_serial_port_send;
extern std::queue < std::tuple<char *,int> >msg_queue_s;

using std::cout;
using std::cin;
using std::endl;

void raw_mode (int fd, struct termios *old_term)
{
    struct termios term;

    tcgetattr(fd, &term);
    tcgetattr(fd, old_term);

    /* mode RAW */
    cfmakeraw (&term);
    
    if(cfsetspeed(&term, B57600) != 0 )
        cout << "Error set speed" << endl;

    if(tcsetattr(fd, TCSANOW, &term) != 0)
        cout << "Error set attr" << endl;
}

int read_msg(int fd, char *buffer, size_t buffer_size){
    int i=0;
    char c=0;
    while( c!= ' '){ 
        while(read(fd, &c, 1) < 1); // read the message
        buffer[i] = c; 
        i++;
        if(  (size_t)i + 2 > buffer_size){
            cout << "Error read msg overflow" << endl;
            break;
        }
    }
    i--; //purpose: ignore the space at the end of the buffer later
    {
        std::lock_guard<std::mutex> lk(mutex_serial_port_read);
        char *p_msg_return = (char *)malloc( (i+1)*sizeof(char) );
        memcpy( (void *)p_msg_return, (void *)buffer, i);
        msg_queue_r.push( std::make_tuple(p_msg_return,i) );
    }
    done_serial_port = 1;
    cv_serial_port.notify_one();
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
        cout << "Error write" << endl;
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

        tty_fd = open(port, O_RDWR);

        if(tty_fd == -1){
            cout << "opening failed" << endl;
            return -1;
        }
        cout << "File descriptor : " << tty_fd << endl;
        raw_mode(tty_fd, &old);
        cout << "Pass to raw mode" << endl;

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
                        read_msg(tty_fd, p_data_out, size_data_out);
                    }
                    if( new_msg == true){// Write msg
                        std::unique_lock<std::mutex> locker(mutex_serial_port_read_send);
                        unsigned char buffer_write[BASE64BUFFERSIZE];
                        size_t olen;
//p_msg_user, size_data_in
mbedtls_base64_encode(buffer_write,BASE64BUFFERSIZE,&olen,(unsigned char *)p_msg_user,size_data_in);

                        if (write(tty_fd, p_msg_user, size_data_in) == (ssize_t)size_data_in){
                            new_msg = false;
                            free(p_msg_user);
                            p_msg_user = NULL;
                            cv_serial_port_send.notify_one();
                        }
                        else{
                            cout << "Sending failed" << endl;
                        }
                    }
                }
                else{
                    printf("yes nor no\n");
                }
            }
        }
        cout << "Close port" << endl;
        tcsetattr(tty_fd, TCSANOW, &old);
        close(tty_fd);
        return 0;
}
