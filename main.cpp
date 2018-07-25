#include "SerialLora.hpp"
#include "forwarder.hpp"
#include "serial_lora.hpp"

#include <thread> // std::thread

int main(int argc, char *argv[]) {

    if (argc < 2) {
        std::cout << "Need the path to the node serial port" << std::endl;
        return -1;
    }

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
