/*
 * author: Antonin Héréson
 * Serial port courses from :
 * http://pficheux.free.fr/articles/lmf/serial/
 * https://en.wikibooks.org/wiki/Serial_Programming/termios
 */

#ifndef SERIAL_LORA_HPP
#define SERIAL_LORA_HPP

#include <memory>

class SerialBuffer {
  public:
    int size = 0;
    std::unique_ptr<char[]> msg;
    SerialBuffer(int size_msg);
};

int serial_exchange(const char *port, int size_data_in);
void write_serial_Lora(SerialBuffer &buff);
SerialBuffer read_serial_Lora();
#endif
