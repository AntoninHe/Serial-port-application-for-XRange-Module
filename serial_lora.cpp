/*
 * author: Antonin Héréson
 * Serial port courses from :
 * http://pficheux.free.fr/articles/lmf/serial/
 * https://en.wikibooks.org/wiki/Serial_Programming/termios
 */

#include <condition_variable> // std::condition_variable
#include <iostream>           // sdt::cout, sdt::cin, sdt::endl
#include <memory>             // unique_ptr
#include <mutex>              // std::mutex, std::unique_lock
#include <thread>             // std::thread
#include <exception>

extern "C" {
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "mbedtls/base64.h"
}

#include "serial_lora.hpp"

const char MSG_YES = '!';
const char MSG_NO = '?';
const char MSG_END = '*';

#define BASE64BUFFERSIZE 500

/////////////////////////////////////////////////
std::mutex mutex_serial_port_read;
std::condition_variable cv_serial_port;
auto my_buffer_R = SerialBuffer(0);
auto done_serial_port = false;
/////////////////////////////////////////////////

/////////////////////////////////////////////////
std::mutex mutex_serial_port_read_send;
std::condition_variable cv_serial_port_send;
auto my_buffer_W = SerialBuffer(0);
/////////////////////////////////////////////////
auto new_msg = false;

using std::cin;
using std::cout;
using std::endl;

const int SIZE_MAX_BUFER = 200;
SerialBuffer::SerialBuffer(int size_msg) {
    this->msg = std::unique_ptr<char[]>(new char[size_msg]);
}

class openException: public std::exception {
      virtual const char* what() const throw() {
        return "Port opening failed";
      }
} open_exception ;

SerialLora::SerialLora(const std::string port,
                       const int size_data_in = SIZE_MAX_BUFER) {
    this->size_max = size_data_in;

    this->tty_fd = open(port.c_str(), O_RDWR);

    if (tty_fd == -1) {
        throw open_exception;
    }
    cout << "File descriptor : " << tty_fd << endl;
    raw_mode(tty_fd, &this->old);

    this->my_thread = std::thread(&SerialLora::serial_exchange, this);
    cout << "Pass to raw mode" << endl;
}

SerialLora::~SerialLora() {
    this->my_thread.join();
    cout << "Close port" << endl;
    tcsetattr(tty_fd, TCSANOW, &old);
    close(tty_fd);
}

void SerialLora::raw_mode(int fd, struct termios *old_term) {
    struct termios term = {};

    tcgetattr(fd, &term);
    tcgetattr(fd, old_term);

    /* mode RAW */
    cfmakeraw(&term);

    if (cfsetspeed(&term, B57600) != 0) {
        cout << "Error set speed" << endl;
    }

    if (tcsetattr(fd, TCSANOW, &term) != 0) {
        cout << "Error set attr" << endl;
    }
}

SerialBuffer SerialLora::read_serial_Lora() {
    std::unique_lock<std::mutex> locker(mutex_serial_port_read);
    cv_serial_port.wait(locker, []() { return done_serial_port; });
    done_serial_port = false;
    return std::move(my_buffer_R);
}

int SerialLora::read_msg(int fd, int buffer_size) {
    auto c = 0;
    my_buffer_R = SerialBuffer(buffer_size);
    std::lock_guard<std::mutex> lk(mutex_serial_port_read);
    while (true) {
        while (read(fd, &c, 1) < 1)
            ; // read the message
        if (c == MSG_END) {
            break; // avoid copy MSG_END cara
        }
        my_buffer_R.msg[my_buffer_R.size] = c;
        my_buffer_R.size++;
        if (my_buffer_R.size >= buffer_size) {
            cout << "Error read msg overflow" << endl;
            break;
        }
    }
    done_serial_port = true;
    cv_serial_port.notify_one();
    return my_buffer_R.size;
}

int SerialLora::say_Y_N(int fd, bool new_msg) {
    auto r = ' ';
    if (new_msg) {
        r = MSG_YES;
    } else {
        r = MSG_NO;
    }
    if (write(fd, &r, 1) != 1) {
        cout << "Error write" << endl;
        return -1;
    }
    return 0;
}

int SerialLora::flush_with_space(int fd) {
    auto c = MSG_END;
    if (write(fd, &c, 1) > 0) {
        return 0;
    }
    return -1;
}

void SerialLora::write_serial_Lora(SerialBuffer &buff) {
    std::unique_lock<std::mutex> locker(mutex_serial_port_read_send);
    my_buffer_W = std::move(buff);
    new_msg = true;
    cv_serial_port_send.wait(locker, []() { return !new_msg; });
}

int SerialLora::write_msg(int fd) {
    std::unique_lock<std::mutex> locker(mutex_serial_port_read_send);
    auto olen = size_t{};
    int n = 0;
    int slen = my_buffer_W.size;
    n = slen / 3;
    if (slen % 3 != 0) {
        n += 1;
    }
    n *= 4;
    const int buffer_max_size = n + 1;
    SerialBuffer buffer_write_2(buffer_max_size);
    mbedtls_base64_encode(
        reinterpret_cast<unsigned char *>(buffer_write_2.msg.get()),
        buffer_max_size, &olen,
        reinterpret_cast<unsigned char *>(my_buffer_W.msg.get()),
        my_buffer_W.size);
    buffer_write_2.msg[olen++] = MSG_END;
    buffer_write_2.size = olen;

    if (write(fd, buffer_write_2.msg.get(), buffer_write_2.size) ==
        (ssize_t)olen) {
        new_msg = false;
        cv_serial_port_send.notify_one();
        return 0;
    }
    return -1;
}

void SerialLora::serial_exchange() {
    auto c = 'D';
    tcflush(tty_fd, TCIFLUSH);
    flush_with_space(tty_fd);
    while (true) {
        c = 0;
        if (read(tty_fd, &c, 1) > 0) {
            if (c == MSG_YES || c == MSG_NO) {
                usleep(10000);
                say_Y_N(tty_fd, new_msg);
                if (c == MSG_YES) {
                    read_msg(tty_fd, size_max);
                }
                if (new_msg) { // Write msg
                    if (write_msg(tty_fd) != 0) {
                        cout << "Sending failed" << endl;
                    }
                }
            } else {
                cout << "yes nor no" << endl;
            }
        }
    }
    /////////////////// return 0;
}