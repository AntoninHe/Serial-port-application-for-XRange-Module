#include "SerialLora.hpp"
#include "cpu.hpp"
#include "forwarder.hpp"
#include "serial_lora.hpp"

#include <iostream> // sdt::cout, sdt::cin, sdt::endl
#include <list>     // std::list
#include <string.h> // memcpy
#include <thread>   // std::thread

#include <chrono>

#define MSG_YES '!'
#define MSG_NO '?'
#define MSG_END '*'

using std::cin;
using std::cout;
using std::endl;
using std::string;

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
        auto my_buffer = read_serial_Lora();
        forwarder(my_buffer.msg.get(), my_buffer.size);
    }
}

// int SerialLora::serial_thread() {

//     std::thread t1(thread_consummer);
//     std::thread t2(serial_exchange, this->port.c_str(), 200);
//     // std::thread t3(thread_HIM);
//     std::thread t3(thread_Cpu_data);

//     t1.join();
//     t2.join();
//     t3.join();

//     return 0;
// }
