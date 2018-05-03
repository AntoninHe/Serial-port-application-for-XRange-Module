/* 
 * author: Antonin Héréson
 * Serial port courses from :
 * http://pficheux.free.fr/articles/lmf/serial/
 * https://en.wikibooks.org/wiki/Serial_Programming/termios
 */

int serialExchange(char *port, char *dataIN, size_t sizeDataIN, char *dataOUT, size_t sizeDataOUT){

#include <unistd.h>

