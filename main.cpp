#include "SerialLora.hpp"

int main (int argc, char *argv[]){
    if(argc < 2)
        return -1;

    std::string port_name(argv[1]);
    std::cout << argv[1] << std::endl;
    std::cout << port_name << std::endl;
        
    SerialLora my_LoRa(port_name);
    my_LoRa.serial_thread();
    return 0;
}
