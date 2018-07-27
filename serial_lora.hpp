/*
 * author: Antonin Héréson
 * Serial port courses from :
 * http://pficheux.free.fr/articles/lmf/serial/
 * https://en.wikibooks.org/wiki/Serial_Programming/termios
 */

#ifndef SERIAL_LORA_HPP
#define SERIAL_LORA_HPP

#include <memory>
#include <termios.h>
#include <thread>

class SerialBuffer {
  public:
    int size = 0;
    std::unique_ptr<char[]> msg;
    SerialBuffer(int size_msg);
};

class SerialLora {
  public:
    SerialLora(const std::string port, const int size_data_in);
    ~SerialLora();
    void write_serial_Lora(SerialBuffer &buff);
    SerialBuffer read_serial_Lora();

  private:
    int tty_fd = -1;
    int size_max = 0;
    struct termios old = {};
    std::thread my_thread;
    void serial_exchange();
    void raw_mode(int fd, struct termios *old_term);
    int read_msg(int fd, int buffer_size);
    int say_Y_N(int fd, bool new_msg);
    int flush_with_space(int fd);
    int write_msg(int fd);
};

#endif
