#include "SerialLora.hpp"
#include <iostream>

using std::string;
using std::cout;
using std::cin;
using std::endl;

extern "C" {
#include "serial_lora.h"
}

SerialLora::SerialLora(const std::string port){
    
    this->port = string(port);
    fd = open_port(this->port.c_str(), &termios_old);
    if(fd < 0 )
        cout << "Wrong port" << endl; 
          
}

SerialLora::~SerialLora(){
    if(fd > 0)
        if(close_port(port.c_str(), &termios_old, fd) != 0)
            cout << "error" << endl; 

}

void SerialLora::serial_thread(){
    if(fd < 0 )
        return;

    bool new_msg = false;
    const size_t size_buffer = 200;

    char *p_data_in  = (char *)calloc(size_buffer , sizeof(char) );
    char *p_data_out  = (char *)calloc(size_buffer , sizeof(char) );


    //while(1){ //TODO
        exchange(fd, p_data_in, size_buffer, p_data_out, size_buffer,new_msg);
    //}
    string output(p_data_out,size_buffer);  

    free(p_data_in);
    free(p_data_in);
}

int SerialLora::send_msg(char msg[]){

    return -1;
}

int SerialLora::read_msg(char msg[]){

    return -1;
}

int SerialLora::read_msg(std::string msg){

    return -1;
}

