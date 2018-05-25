#include <iostream>
#include <string>
#include <termios.h>

class SerialLora{
    public : 
        SerialLora(const std::string port);
        ~SerialLora();
        int send_msg(char msg[]);
        int read_msg(char msg[]);
        int read_msg(std::string msg);
        int serial_thread();//to move to private

    private : 
        //int fd = -1;
        //std::string message_recivied;
        std::string port;
        //struct termios termios_old;
        char *p_data_out = NULL;
        char *p_data_in = NULL;
};