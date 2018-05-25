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

#include <iostream>             // sdt::cout, sdt::cin, sdt::endl
#include <mutex>                // std::mutex, std::unique_lock
#include <condition_variable>   // std::condition_variable

#define MSG_YES '!'
#define MSG_NO '?'

extern std::mutex myMutex;
extern std::string msg_string;
extern std::condition_variable cv;
extern int done;

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
        if(i + 2 > buffer_size){
            cout << "Error read msg overflow" << endl;
            break;
        }
    }
    buffer[ i++ ] = '\0'; // DEBUG
    {
        std::lock_guard<std::mutex> lk(myMutex);
        msg_string  = std::string(buffer);
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
        bool new_msg = false;

        tty_fd = open(port, O_RDWR);

        if(tty_fd == -1)
            return -1;

        cout << "File descriptor : " << tty_fd << endl;
        raw_mode(tty_fd, &old);
        cout << "Pass to law mode" << endl;

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
                            //printf("Msg Send\n"); // DEBUG
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
