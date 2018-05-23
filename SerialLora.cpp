#include "SerialLora.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <list>

using std::string;
using std::cout;
using std::cin;
using std::endl;

std::mutex myMutex;
std::list<std::string>myList;

#include "serial_lora.hpp"

SerialLora::SerialLora(const std::string port){
    
    this->port = string(port);
    /*
     fd = open_port(this->port.c_str(), &termios_old);
    if(fd < 0 )
        cout << "Wrong port" << endl; 
    */

    const size_t size_buffer = 200;

    p_data_in  = (char *)calloc(size_buffer , sizeof(char) );
    //if(p_data_in == NULL)
        //return -1;

    p_data_out  = (char *)calloc(size_buffer , sizeof(char) );
    //if(p_data_out == NULL)
        //return -1;
}

SerialLora::~SerialLora(){
    if(p_data_out != NULL)
        free(p_data_out);

    if(p_data_in != NULL)
        free(p_data_in);
    /*
    if(fd > 0)
        if(close_port(port.c_str(), &termios_old, fd) != 0)
            cout << "error" << endl; 
    */
}

int SerialLora::serial_thread(){

    //bool new_msg = true;
    if(p_data_in == NULL || p_data_out == NULL)
        return -1;

    std::thread t1(serialExchange,this->port.c_str(), p_data_in, 200, p_data_out, 200);

    //std::thread t2(reader);

    t1.join();
    //t2.join();

/*
    p_data_out[0] = 'a';
    p_data_out[1] = 'b';
    p_data_out[2] = 'c';
    p_data_out[3] = ' ';
*/

    return 0;
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

