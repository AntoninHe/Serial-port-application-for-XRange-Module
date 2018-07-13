/* 
 * author: Antonin Héréson
 * Serial port courses from :
 * http://pficheux.free.fr/articles/lmf/serial/
 * https://en.wikibooks.org/wiki/Serial_Programming/termios
 */

#ifndef SERIAL_LORA_HPP
#define SERIAL_LORA_HPP

#include <unistd.h>
#include <termios.h>
#include <stdbool.h>

/*
 * Name : exchange
 *
 * port_name : serial port mame 
 * data_in_size : data buffer size 
 * data_out_size : data buffer size
 * size_data_in : buffer size
 * size_data_out : buffer size
 *
 * Return :     0 if 0k
 *              -1 if the response from the device is unexpected
 *              -2 if tranmission error
 *
 * */
int serial_exchange(const char *port, size_t size_data_in);
//int serial_exchange(const char *port, char *p_data_in, size_t size_data_in, char *p_data_out, size_t size_data_out);

#endif
