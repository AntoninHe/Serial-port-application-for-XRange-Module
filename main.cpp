#include "cpu.hpp"
#include "forwarder.hpp"
#include "serial_lora.hpp"

#include <thread> // std::thread
#include <iostream> // sdt::cout, sdt::cin, sdt::endl

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

int main(int argc, char *argv[]) {

    if (argc < 2) {
        std::cout << "Need the path to the node serial port" << std::endl;
        return -1;
    }

    // init packet forwarder
    parseCommandline(argc - 1, argv + 1);

    std::thread t1(thread_consummer);
    std::thread t2(thread_Cpu_data);
    std::thread t3(serial_exchange, argv[1], 200);
    // std::thread t3(thread_HIM);

    t1.join();
    t2.join();
    t3.join();

    return 0;
}
