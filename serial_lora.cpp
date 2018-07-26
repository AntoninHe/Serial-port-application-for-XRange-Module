/*
 * author: Antonin Héréson
 * Serial port courses from :
 * http://pficheux.free.fr/articles/lmf/serial/
 * https://en.wikibooks.org/wiki/Serial_Programming/termios
 */

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <condition_variable> // std::condition_variable
#include <iostream>           // sdt::cout, sdt::cin, sdt::endl
#include <memory>             // unique_ptr
#include <mutex>              // std::mutex, std::unique_lock

#include "mbedtls/base64.h"
#include "serial_lora.hpp"

const char MSG_YES = '!';
const char MSG_NO = '?';
const char MSG_END = '*';

#define BASE64BUFFERSIZE 500

/////////////////////////////////////////////////
std::mutex mutex_serial_port_read;
std::condition_variable cv_serial_port;
SerialBuffer my_buffer_R = SerialBuffer(0);
auto done_serial_port = false;

/////////////////////////////////////////////////

/////////////////////////////////////////////////
std::mutex mutex_serial_port_read_send;
std::condition_variable cv_serial_port_send;
SerialBuffer my_buffer_W = SerialBuffer(0);
auto new_msg = false;

/////////////////////////////////////////////////

using std::cin;
using std::cout;
using std::endl;

const int SIZE_MAX_BUFER = 200;

SerialBuffer::SerialBuffer(int size_msg) {
    this->msg = std::unique_ptr<char[]>(new char[size_msg]);
}

void raw_mode(int fd, struct termios *old_term) {
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

SerialBuffer read_serial_Lora() {
    std::unique_lock<std::mutex> locker(mutex_serial_port_read);
    cv_serial_port.wait(locker, []() { return done_serial_port; });
    done_serial_port = false;
    return std::move(my_buffer_R);
}

int read_msg(int fd, int buffer_size) {
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

int say_Y_N(int fd, bool new_msg) {
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

int flush_with_space(int fd) {
    auto c = MSG_END;
    if (write(fd, &c, 1) > 0) {
        return 0;
    }
    return -1;
}

void write_serial_Lora(SerialBuffer &buff) {
    std::unique_lock<std::mutex> locker(mutex_serial_port_read_send);
    my_buffer_W = std::move(buff);
    new_msg = true;
    cv_serial_port_send.wait(locker, []() { return !new_msg; });
}

int write_msg(int fd) {
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

// int write_msg(int fd) {
//     std::unique_lock<std::mutex> locker(mutex_serial_port_read_send);
//     // size = base 64 worst case + MSG_END
//     const auto buffer_max_size = 4 * my_buffer_W.size / 3 + 1 + 1;
//     SerialBuffer buffer_write(buffer_max_size);
//     // std::unique_ptr<unsigned char[]> buffer_write(
//     auto olen = size_t{};
// cout << "-----------------------" << endl;
//     mbedtls_base64_encode((unsigned char *)buffer_write.msg.get(),
//                           buffer_max_size, &olen,
//                           (unsigned char *)my_buffer_W.msg.get(),
//                           my_buffer_W.size);
// buffer_write.size = olen;
//     buffer_write.msg[buffer_write.size++] = MSG_END;
//     if (write(fd, buffer_write.msg.get(), buffer_write.size) ==
//         (ssize_t)buffer_write.size) {
//         new_msg = false;
//         cv_serial_port_send.notify_one();
//         return 0;
//     }
//     return -1;
// }

int serial_exchange(const char *port, int size_data_in = SIZE_MAX_BUFER) {

    auto tty_fd = 0;
    auto c = 'D';
    struct termios old = {};

    tty_fd = open(port, O_RDWR);

    if (tty_fd == -1) {
        cout << "opening failed" << endl;
        return -1;
    }
    cout << "File descriptor : " << tty_fd << endl;
    raw_mode(tty_fd, &old);
    cout << "Pass to raw mode" << endl;

    tcflush(tty_fd, TCIFLUSH);
    flush_with_space(tty_fd);
    while (true) {
        c = 0;
        if (read(tty_fd, &c, 1) > 0) {
            if (c == MSG_YES || c == MSG_NO) {
                usleep(10000);
                say_Y_N(tty_fd, new_msg);
                if (c == MSG_YES) {
                    read_msg(tty_fd, size_data_in);
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
    cout << "Close port" << endl;
    tcsetattr(tty_fd, TCSANOW, &old);
    close(tty_fd);
    return 0;
}
