#include "SerialLora.hpp"
#include "cpu.hpp"
#include "forwarder.hpp"
#include "serial_lora.hpp"

#include <string.h> // memcpy

#include <condition_variable> // std::condition_variable
#include <iostream>           // sdt::cout, sdt::cin, sdt::endl
#include <list>               // std::list
#include <mutex>              // std::mutex, std::unique_lock
#include <queue>              // std::queue
#include <thread>             // std::thread
#include <tuple>              // std::tuple

#include <chrono>

#define MSG_YES '!'
#define MSG_NO '?'
#define MSG_END '*'

using std::cin;
using std::cout;
using std::endl;
using std::string;

extern bool new_msg;

/////////////////////////////////////////////////
std::mutex mutex_serial_port_read;
std::condition_variable cv_serial_port;
int done_serial_port;

std::queue<std::tuple<char *, int>> msg_queue_r;
/////////////////////////////////////////////////

/////////////////////////////////////////////////
std::mutex mutex_serial_port_read_send;
std::condition_variable cv_serial_port_send;

std::queue<std::tuple<char *, int>> msg_queue_s;
char *p_msg_user;
int msg_size_user;
/////////////////////////////////////////////////

SerialLora::SerialLora(const std::string port) {

    this->port = string(port);
    const size_t size_buffer = 200;

    p_data_in = (char *)calloc(size_buffer, sizeof(char));
    p_data_out = (char *)calloc(size_buffer, sizeof(char));
}

SerialLora::~SerialLora() {
    if (p_data_out != NULL)
        free(p_data_out);

    if (p_data_in != NULL)
        free(p_data_in);
}

// void thread_HIM() { need to update this with to new function
//     while (1) {
//         cout << "enter msg to be send by your lora node" << endl;
//         auto user_msg = string();
//         cin >> user_msg;
//         msg_size_user = user_msg.length();
//         {
//             std::unique_lock<std::mutex> locker(mutex_serial_port_read_send);
//             p_msg_user = (char *)malloc((msg_size_user) * sizeof(char));
//             memcpy((void *)p_msg_user, (void *)user_msg.c_str(),
//             msg_size_user); new_msg = true; cv_serial_port_send.wait(locker,
//             []() { return new_msg == false; });
//         }
//     }
// }

// void thread_Cpu_data() {
//     cout << "cpu start" << endl;
//     while (1) {
//         // 100ms pause
//         std::this_thread::sleep_for(std::chrono::seconds(1));
//         cout << "sleep done" << endl;
//         auto cpuUsage = Get_cpu();

//         {
//             std::unique_lock<std::mutex> locker(mutex_serial_port_read_send);
//             msg_size_usee = cpuUsage.size();
//             auto i = 0;
//             p_msg_user = (char *)malloc((msg_size_user) * sizeof(char));
//             for (auto &e : cpuUsage) {
//                 p_msg_user[i] = e;
//                 cout << e << " et i" << i << endl;
//                 i++;
//             }
//             new_msg = true;
//             cv_serial_port_send.wait(locker, []() { return new_msg == false;
//             });
//         }
//     }
// }

void thread_Cpu_data() {
    cout << "cpu start" << endl;
    const auto size_buffer = 200;
    while (1) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        cout << "sleep done" << endl;
        auto cpuUsage = Get_cpu();
        auto my_buff = SerialBuffer(size_buffer);
        for (auto &e : cpuUsage) {
            my_buff.msg[my_buff.size] = e;
            my_buff.size++;
        }
        write_serial_Lora(my_buff);
    }
}

void thread_consummer() {
    while (1) {
        std::unique_lock<std::mutex> locker(mutex_serial_port_read);
        cv_serial_port.wait(locker, []() { return done_serial_port == 1; });
        done_serial_port = 0;

        while (!msg_queue_r.empty()) {
            // cout << std::get<0>(msg_queue_r.front()) <<
            // std::get<1>(msg_queue_r.front())<< endl;

            testForwarder(std::get<0>(msg_queue_r.front()),
                          std::get<1>(msg_queue_r.front()));
            msg_queue_r.pop();
        }
    }
}

// std::unique_ptr<char[]> p_msg, int msg_size
void read_serial_Lora();

int SerialLora::serial_thread() {

    if (p_data_in == NULL || p_data_out == NULL)
        return -1;
    std::thread t1(thread_consummer);
    std::thread t2(serial_exchange, this->port.c_str(), 200);
    // std::thread t3(thread_HIM);
    std::thread t3(thread_Cpu_data);

    t1.join();
    t2.join();
    t3.join();

    return 0;
}

int SerialLora::send_msg(char msg[]) { return -1; }

int SerialLora::read_msg(char msg[]) { return -1; }

int SerialLora::read_msg(std::string msg) { return -1; }
