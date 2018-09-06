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
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

class SerialLora {
  public:
    SerialLora(const std::string &port);
    ~SerialLora();
    void write_serial_Lora(std::string &buff);
    std::string &&read_serial_Lora();

  private:
    int tty_fd = -1;
    struct termios old = {};
    std::thread my_thread;
    void serial_exchange();
    void raw_mode(int fd, struct termios *old_term);
    int read_msg(int fd);
    int say_Y_N(int fd, bool new_msg);
    int flush_with_space(int fd);
    int write_msg(int fd);

    /////////////////////////////////////////////////
    std::mutex mutex_serial_port_read;
    std::condition_variable cv_serial_port;
    std::string my_buffer_R = std::string();
    bool done_serial_port = false;
    /////////////////////////////////////////////////

    /////////////////////////////////////////////////
    std::mutex mutex_serial_port_read_send;
    std::condition_variable cv_serial_port_send;
    std::string my_buffer_W = std::string();
    /////////////////////////////////////////////////

    bool new_msg = false;


};

class openException: public std::exception {
    public:
      virtual const char* what() const throw() {
        return "Port opening failed";
      }
};

#endif
