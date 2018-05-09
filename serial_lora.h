/* 
 * author: Antonin Héréson
 * Serial port courses from :
 * http://pficheux.free.fr/articles/lmf/serial/
 * https://en.wikibooks.org/wiki/Serial_Programming/termios
 */

#ifndef SERIAL_LORA_H
#define SERIAL_LORA_H

#include <unistd.h>
#include <termios.h>

/*
 * Name : exchange
 *
 * fd : file descriptor
 * data_in : transmit data buffer  
 * data_out :received data buffer  
 * new_msg : if the a buffer is needed to be transmit
 *
 * Return :     0 if 0k
 *              -1 if the response from the device is unexpected
 *              -2 if tranmission error
 *
 * */
int exchange(const int fd, const char *data_in, const size_t size_data_in, char *p_data_out, size_t size_data_out, const bool new_msg);

/*
 * Name : open_port
 *
 * port_name : serial port mame 
 * p_termios_old : a structure wich contain will contain original serial port 
 *                 configuration
 *
 * Return : 0 if sucess -1 otherwise
 *
 * */
int open_port(const char port_name[], struct termios *p_termios_old);

/*
 * Name : exchange
 *
 * fd : file descriptor
 * port_name : serial port mame 
 * p_old : a structure wich contain will contain original serial port
 *
 */
int close_port(const char port_name[], struct termios *p_termios_old, const int fd);
#endif
