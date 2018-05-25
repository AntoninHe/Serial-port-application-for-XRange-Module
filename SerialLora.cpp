#include "SerialLora.hpp"
#include <iostream>             // sdt::cout, sdt::cin, sdt::endl
#include <list>                 // sdt::list
#include <thread>               // std::thread
#include <mutex>                // std::mutex, std::unique_lock
#include <condition_variable>   // std::condition_variable
using std::string;
using std::cout;
using std::cin;
using std::endl;

std::mutex myMutex;
std::mutex myNotifier;
std::list<std::string>myList;
std::condition_variable cv;
int done;


#include "serial_lora.hpp"

SerialLora::SerialLora(const std::string port){
    
    this->port = string(port);
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
}

void thread_consummer(){
    while(1){
        std::unique_lock<std::mutex> locker(myMutex);
        cv.wait(locker, [](){return done == 1;});
        done = 0;
        if( !myList.empty() ){
            string myString(myList.front());
            myList.pop_front();
            cout << myString << endl; 
        }
    }
}


int SerialLora::serial_thread(){

    //bool new_msg = true;
    if(p_data_in == NULL || p_data_out == NULL)
        return -1;

    std::thread t1(serialExchange,this->port.c_str(), p_data_in, 200, p_data_out, 200);
    std::thread t2(thread_consummer);

    t1.join();
    t2.join();

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

