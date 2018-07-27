#include "cpu.hpp"
#include "forwarder.hpp"
#include "serial_lora.hpp"

#include <iostream> // sdt::cout, sdt::cin, sdt::endl
#include <thread>   // std::thread

using std::cin;
using std::cout;
using std::endl;
using std::string;

void thread_Cpu_data(SerialLora &my_serial_port) {
    cout << "thread cpu started" << endl;
    const auto size_buffer = 200;
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto cpuUsage = Get_cpu();
        auto my_buff = SerialBuffer(size_buffer);
        for (auto &e : cpuUsage) {
            my_buff.msg[my_buff.size] = e;
            my_buff.size++;
        }
        my_serial_port.write_serial_Lora(my_buff);
    }
}

void thread_consummer(SerialLora &my_serial_port) {
    cout << "thread consumer started" << endl;
    while (true) {
        auto my_buffer = my_serial_port.read_serial_Lora();
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
    auto my_port = std::string(argv[1]);

    SerialLora my_serial_port(my_port, 200);

    std::thread t1(thread_consummer, std::ref(my_serial_port));
    std::thread t2(thread_Cpu_data, std::ref(my_serial_port));

    t1.join();
    t2.join();

    return 0;
}
