#include "SerialLora.hpp"
#include "forwarder.hpp"

int main(int argc, char *argv[]) {

    if (argc < 2) {
        std::cout << "Need the path to the node serial port" << std::endl;
        return -1;
    }

    std::string port_name(argv[1]);
    std::cout << argv[1] << std::endl;
    std::cout << port_name << std::endl;

    parseCommandline(argc - 1, argv + 1);

    /////    testForwarder();

    SerialLora my_LoRa(port_name);
    my_LoRa.serial_thread();

    return 0;
}
