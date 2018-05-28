#include "SerialLora.hpp"
#include "serial_lora.hpp"

#include <iostream>             // sdt::cout, sdt::cin, sdt::endl
#include <thread>               // std::thread
#include <mutex>                // std::mutex, std::unique_lock
#include <condition_variable>   // std::condition_variable

using std::string;
using std::cout;
using std::cin;
using std::endl;

std::mutex mutex_serial_port;
std::condition_variable cv_serial_port;
int done_serial_port;

//std::mutex mutex_forwarder;
//std::condition_variable cv_forwarder;
//int done_forwarder;

std::string msg_string;

SerialLora::SerialLora(const std::string port){
    
    this->port = string(port);
    const size_t size_buffer = 200;

    p_data_in  = (char *)calloc(size_buffer , sizeof(char) );
    p_data_out  = (char *)calloc(size_buffer , sizeof(char) );
}

SerialLora::~SerialLora(){
    if(p_data_out != NULL)
        free(p_data_out);

    if(p_data_in != NULL)
        free(p_data_in);
}


void thread_consummer(){
    while(1){
        std::unique_lock<std::mutex> locker(mutex_serial_port);
        cv_serial_port.wait(locker, [](){return done_serial_port == 1;});
        done_serial_port = 0;
        if( !msg_string.empty() ){
            string myString(msg_string);
            msg_string.clear();
            cout << myString << endl; 
        }
    }
}

int SerialLora::serial_thread(){

    if(p_data_in == NULL || p_data_out == NULL)
        return -1;

    std::thread t1(thread_consummer);
    std::thread t2(serial_exchange,this->port.c_str(), p_data_in, 200, p_data_out, 200);

    t1.join();
    t2.join();

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
