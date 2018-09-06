/*
 * author: Antonin Héréson
 * Serial port courses from :
 * http://pficheux.free.fr/articles/lmf/serial/
 * https://en.wikibooks.org/wiki/Serial_Programming/termios
 */

#include <condition_variable> // std::condition_variable
#include <exception>          // std::exception
#include <iostream>           // sdt::cout, sdt::cin, sdt::endl
#include <memory>             // std::unique_ptr
#include <mutex>              // std::mutex, std::unique_lock
#include <thread>             // std::thread

#include <cppcodec/base64_rfc4648.hpp>

extern "C" {
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
}

#include "serial_lora.hpp"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using base64 = cppcodec::base64_rfc4648;

constexpr auto MSG_YES = char{'!'};
constexpr auto MSG_NO = char{'?'};
constexpr auto MSG_END = char{'*'};

/////////////////////////////////////////////////
std::mutex mutex_serial_port_read;
std::condition_variable cv_serial_port;
auto my_buffer_R = string();
auto done_serial_port = false;
/////////////////////////////////////////////////

/////////////////////////////////////////////////
std::mutex mutex_serial_port_read_send;
std::condition_variable cv_serial_port_send;
auto my_buffer_W = string();
/////////////////////////////////////////////////
auto new_msg = false;

SerialLora::SerialLora(const std::string& port) {

    this->tty_fd = open(port.c_str(), O_RDWR);

    if (tty_fd == -1) {
        throw openException();
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
    auto term = termios{};

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

string &&SerialLora::read_serial_Lora() {
    std::unique_lock<std::mutex> locker(mutex_serial_port_read);
    cv_serial_port.wait(locker, []() { return done_serial_port; });
    done_serial_port = false;
    return std::move(my_buffer_R);
}

int SerialLora::read_msg(int fd) {
    auto c = char{0};
    my_buffer_R = string();
    std::lock_guard<std::mutex> lk(mutex_serial_port_read);
    while (true) {
        while (read(fd, &c, 1) < 1)
            ; // read the message
        if (c == MSG_END) {
            break; // avoid copy MSG_END cara
        }
        my_buffer_R += static_cast<char>(c);
    }
    done_serial_port = true;
    cv_serial_port.notify_one();
    return my_buffer_R.size();
}

int SerialLora::say_Y_N(int fd, bool new_msg) {
    auto r = char{};
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
    // auto c = MSG_END;
    if (write(fd, &MSG_END, 1) > 0) {
        return 0;
    }
    return -1;
}

void SerialLora::write_serial_Lora(string &buff) {
    std::unique_lock<std::mutex> locker(mutex_serial_port_read_send);
    my_buffer_W = std::move(buff);
    new_msg = true;
    cv_serial_port_send.wait(locker, []() { return !new_msg; });
}

int SerialLora::write_msg(int fd) {
    std::unique_lock<std::mutex> locker(mutex_serial_port_read_send);

    my_buffer_W = base64::encode(my_buffer_W) + MSG_END;

    if (write(fd, my_buffer_W.data(), my_buffer_W.size()) ==
        static_cast<ssize_t>(my_buffer_W.size())) {
        new_msg = false;
        cv_serial_port_send.notify_one();
        return 0;
    }
    return -1;
}

void SerialLora::serial_exchange() {
    auto c = char{'D'};
    tcflush(tty_fd, TCIFLUSH);
    flush_with_space(tty_fd);
    while (true) {
        c = 0;
        if (read(tty_fd, &c, 1) > 0) {
            if (c == MSG_YES || c == MSG_NO) {
                usleep(10000);
                say_Y_N(tty_fd, new_msg);
                if (c == MSG_YES) {
                    read_msg(tty_fd);
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