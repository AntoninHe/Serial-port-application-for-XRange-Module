#ifndef SERIAL_LORA_CLASS_HPP
#define SERIAL_LORA_CLASS_HPP

#include <iostream>
#include <string>
#include <termios.h>

class SerialLora {
  public:
    SerialLora(const std::string port);
    int serial_thread(); // to move to private

  private:
    std::string port;
};

#endif
