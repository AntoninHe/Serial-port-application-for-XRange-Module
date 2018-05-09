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
        void serial_thread();//to move to private

    private : 
        int fd = -1;
        std::string message_recivied;
        std::string port;
        struct termios termios_old;
};
