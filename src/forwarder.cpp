/*******************************************************************************
 *
 * Copyright (c) 2015 Thomas Telkamp
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 *******************************************************************************/

#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <map>
#include <net/if.h>
#include <netdb.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

using namespace std;

// extern "C" {
//    #include "base64.h"
//}

#include "static_addr.h"

#include <arpa/inet.h>
#include <netdb.h> //hostent

auto currentMode = 0x81;

char message[256];
char b64[256];

bool sx1272 = true;
int maxMessages = 10;

auto modemstat = 0x00;

struct sockaddr_in si_other;
int s, slen = sizeof(si_other);
struct ifreq ifr;

uint32_t cp_nb_rx_rcv;
uint32_t cp_nb_rx_ok;
uint32_t cp_nb_rx_bad;
uint32_t cp_nb_rx_nocrc;
uint32_t cp_up_pkt_fwd;

enum sf_t { SF7 = 7, SF8, SF9, SF10, SF11, SF12 };

enum {
    EU868_F1 = 868100000, // g1 SF7-12
    EU868_F2 = 868300000, // g1 SF7-12 FSK SF7/250
    EU868_F3 = 868500000, // g1 SF7-12
    EU868_F4 = 868850000, // g2 SF7-12
    EU868_F5 = 869050000, // g2 SF7-12
    EU868_F6 = 869525000, // g3 SF7-12
    EU868_J4 = 864100000, // g2 SF7-12 used during join
    EU868_J5 = 864300000, // g2 SF7-12 ditto
    EU868_J6 = 864500000, // g2 SF7-12 ditto
};
/*******************************************************************************
 *
 * Configure these values!
 *
 *******************************************************************************/

// Set spreading factor (SF7 - SF12)
sf_t sf = SF7;

// Set center frequency
uint32_t freq = 868100000; // in Mhz! (868.1)

// Set location
float lat = 43.61150;
float lon = 1.43590;
int alt = 162;

/* Informal status fields */
static char platform[24] = "Single Channel Gateway"; // platform definition
static char email[40] = "";                          // used for contact email
static char description[64] =
    "Singe Channel Gateway"; // used for free form description

// define servers
// TODO: use host names and dns

//#define DEFAULTSERVER "127.0.0.1"
#define DEFAULTSERVER "52.169.76.203" // router.eu.thethings.network

#define DEFAULTPORT 1700 // The port on which to send data

// #############################################
// #############################################
std::map<std::string, std::pair<std::string, int>> serverList;

#define BUFLEN 2048 // Max length of buffer

#define PROTOCOL_VERSION 1
#define PKT_PUSH_DATA 0
#define PKT_PUSH_ACK 1
#define PKT_PULL_DATA 2
#define PKT_PULL_RESP 3
#define PKT_PULL_ACK 4

#define TX_BUFF_SIZE 2048
#define STATUS_SIZE 1024

void die(const char *s) {
    perror(s);
    exit(1);
}

int hostToIp(const char *host, char *ip, const int bufflen) {
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in *h;
    int ret = -1;

    if (bufflen < INET6_ADDRSTRLEN)
        die("hostToIP, supplied buffer too short");

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((ret = getaddrinfo(host, NULL, NULL, &servinfo)) != 0) {
        std::stringstream error;
        error << "Failed to look up server: ";
        error << host;
        error << ", ";
        error << gai_strerror(ret);
        die(error.str().c_str());
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        h = (struct sockaddr_in *)p->ai_addr;
        strncpy(ip, inet_ntoa(h->sin_addr), bufflen);
    }

    freeaddrinfo(servinfo);
    return 0;
}

bool receivePkt(char *payload) { return true; }

int resolve_ip_address(const char *hostname, char *ip) {
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ((he = gethostbyname(hostname)) == NULL) {
        // get the host info
        herror("gethostbyname");
        return 1;
    }

    addr_list = (struct in_addr **)he->h_addr_list;

    for (i = 0; addr_list[i] != NULL; i++) {
        // Return the first one;
        strcpy(ip, inet_ntoa(*addr_list[i]));
        return 0;
    }

    return 1;
}

void sendudp(char *msg, int length) {

    // send the update
    std::map<std::string, std::pair<std::string, int>>::iterator iter;
    for (iter = serverList.begin(); iter != serverList.end(); iter++) {
        cout << iter->second.second << endl;
        cout << iter->second.first << endl;
        si_other.sin_port = htons(iter->second.second);
        inet_aton(iter->second.first.c_str(), &si_other.sin_addr);
        if (sendto(s, (char *)msg, length, 0, (struct sockaddr *)&si_other,
                   slen) == -1) {
            die("sendto()");
        }
    }
}
void sendstat() {

    static char status_report[STATUS_SIZE]; /* status report as a JSON object */
    char stat_timestamp[24];
    time_t t;

    int stat_index = 0;

    /* pre-fill the data buffer with fixed fields */
    status_report[0] = PROTOCOL_VERSION;
    status_report[3] = PKT_PUSH_DATA;

    // status_report[4] = (unsigned char)ifr.ifr_hwaddr.sa_data[0];
    // status_report[5] = (unsigned char)ifr.ifr_hwaddr.sa_data[1];
    // status_report[6] = (unsigned char)ifr.ifr_hwaddr.sa_data[2];
    // status_report[7] = 0xFF;
    // status_report[8] = 0xFF;
    // status_report[9] = (unsigned char)ifr.ifr_hwaddr.sa_data[3];
    // status_report[10] = (unsigned char)ifr.ifr_hwaddr.sa_data[4];
    // status_report[11] = (unsigned char)ifr.ifr_hwaddr.sa_data[5];

    status_report[4] = (unsigned char)STATIC_MAC_ADDR_0;
    status_report[5] = (unsigned char)STATIC_MAC_ADDR_1;
    status_report[6] = (unsigned char)STATIC_MAC_ADDR_2;
    status_report[9] = (unsigned char)STATIC_MAC_ADDR_3;
    status_report[10] = (unsigned char)STATIC_MAC_ADDR_4;
    status_report[11] = (unsigned char)STATIC_MAC_ADDR_5;

    status_report[7] = 0xFF;
    status_report[8] = 0xFF;

    /* start composing datagram with the header */
    uint8_t token_h = (uint8_t)rand(); /* random token */
    uint8_t token_l = (uint8_t)rand(); /* random token */
    status_report[1] = token_h;
    status_report[2] = token_l;
    stat_index = 12; /* 12-byte header */

    /* get timestamp for statistics */
    t = time(NULL);
    strftime(stat_timestamp, sizeof stat_timestamp, "%F %T %Z", gmtime(&t));

    int j = snprintf(
        (char *)(status_report + stat_index), STATUS_SIZE - stat_index,
        "{\"stat\":{\"time\":\"%s\",\"lati\":%.5f,\"long\":%.5f,\"alti\":%i,"
        "\"rxnb\":%u,\"rxok\":%u,\"rxfw\":%u,\"ackr\":%.1f,\"dwnb\":%u,"
        "\"txnb\":%u,\"pfrm\":\"%s\",\"mail\":\"%s\",\"desc\":\"%s\"}}",
        stat_timestamp, lat, lon, (int)alt, cp_nb_rx_rcv, cp_nb_rx_ok,
        cp_up_pkt_fwd, (float)0, 0, 0, platform, email, description);
    stat_index += j;
    status_report[stat_index] = 0; /* add string terminator, for safety */

    printf("stat update: %s\n",
           (char *)(status_report + 12)); /* DEBUG: display JSON stat */

    // send the update
    sendudp(status_report, stat_index);
}

void receivepacket(const char my_msg[], int receivedbytes) {

    long int SNR;
    // int rssicorr;

    ///////////////////////////////////////////////////////////
    // TODO
    ///////////////////////////////////////////////////////////

    maxMessages--;
    // Divide by 4
    SNR = 0; //( value & 0xFF ) >> 2;

    // rssicorr = 139;// sx1272
    // // rssicorr = 157; // else sx1272

    ///////////////////////////////////////////////////////////

    // printf("Packet RSSI: %d, ",readRegister(0x1A)-rssicorr);
    // printf("RSSI: %d, ",readRegister(0x1B)-rssicorr);
    printf("SNR: %li, ", SNR);
    printf("Length: %i", (int)receivedbytes);
    printf("\n");

    int j;
    // j = bin_to_b64((uint8_t *)message, receivedbytes, (char *)(b64), 341);
    // fwrite(b64, sizeof(char), j, stdout);

    char buff_up[TX_BUFF_SIZE]; /* buffer to compose the upstream packet */
    int buff_index = 0;

    /* gateway <-> MAC protocol variables */
    // static uint32_t net_mac_h; /* Most Significant Nibble, network order */
    // static uint32_t net_mac_l; /* Least Significant Nibble, network order */

    /* pre-fill the data buffer with fixed fields */
    buff_up[0] = PROTOCOL_VERSION;
    buff_up[3] = PKT_PUSH_DATA;

    /* process some of the configuration variables */
    // net_mac_h = htonl((uint32_t)(0xFFFFFFFF & (lgwm>>32)));
    // net_mac_l = htonl((uint32_t)(0xFFFFFFFF &  lgwm  ));
    //*(uint32_t *)(buff_up + 4) = net_mac_h;
    //*(uint32_t *)(buff_up + 8) = net_mac_l;

    buff_up[4] = (unsigned char)STATIC_MAC_ADDR_0;
    buff_up[5] = (unsigned char)STATIC_MAC_ADDR_1;
    buff_up[6] = (unsigned char)STATIC_MAC_ADDR_2;
    buff_up[9] = (unsigned char)STATIC_MAC_ADDR_3;
    buff_up[10] = (unsigned char)STATIC_MAC_ADDR_4;
    buff_up[11] = (unsigned char)STATIC_MAC_ADDR_5;

    buff_up[7] = 0xFF;
    buff_up[8] = 0xFF;

    // buff_up[4] = (unsigned char)ifr.ifr_hwaddr.sa_data[0];
    // buff_up[5] = (unsigned char)ifr.ifr_hwaddr.sa_data[1];
    // buff_up[6] = (unsigned char)ifr.ifr_hwaddr.sa_data[2];
    // buff_up[7] = 0xFF;
    // buff_up[8] = 0xFF;
    // buff_up[9] = (unsigned char)ifr.ifr_hwaddr.sa_data[3];
    // buff_up[10] = (unsigned char)ifr.ifr_hwaddr.sa_data[4];
    // buff_up[11] = (unsigned char)ifr.ifr_hwaddr.sa_data[5];

    /* start composing datagram with the header */
    uint8_t token_h = (uint8_t)rand(); /* random token */
    uint8_t token_l = (uint8_t)rand(); /* random token */
    buff_up[1] = token_h;
    buff_up[2] = token_l;
    buff_index = 12; /* 12-byte header */

    // TODO: tmst can jump is time is (re)set, not good.
    struct timeval now;
    gettimeofday(&now, NULL);
    uint32_t tmst = (uint32_t)(now.tv_sec * 1000000 + now.tv_usec);

    /* start of JSON structure */
    memcpy((void *)(buff_up + buff_index), (void *)"{\"rxpk\":[", 9);
    buff_index += 9;
    buff_up[buff_index] = '{';
    ++buff_index;
    j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE - buff_index,
                 "\"tmst\":%u", tmst);
    buff_index += j;
    j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE - buff_index,
                 ",\"chan\":%1u,\"rfch\":%1u,\"freq\":%.6lf", 0, 0,
                 (double)freq / 1000000);
    buff_index += j;
    memcpy((void *)(buff_up + buff_index), (void *)",\"stat\":1", 9);
    buff_index += 9;
    memcpy((void *)(buff_up + buff_index), (void *)",\"modu\":\"LORA\"", 14);
    buff_index += 14;
    /* Lora datarate & bandwidth, 16-19 useful chars */
    switch (sf) {
    case SF7:
        memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF7", 12);
        buff_index += 12;
        break;
    case SF8:
        memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF8", 12);
        buff_index += 12;
        break;
    case SF9:
        memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF9", 12);
        buff_index += 12;
        break;
    case SF10:
        memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF10", 13);
        buff_index += 13;
        break;
    case SF11:
        memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF11", 13);
        buff_index += 13;
        break;
    case SF12:
        memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF12", 13);
        buff_index += 13;
        break;
    default:
        memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF?", 12);
        buff_index += 12;
    }
    memcpy((void *)(buff_up + buff_index), (void *)"BW125\"", 6);
    buff_index += 6;

    //"codr":"4/5"
    memcpy((void *)(buff_up + buff_index), (void *)",\"codr\":\"4/5\"", 13);
    buff_index += 13;
    //          switch((int)((modemstat & 0xE0) >> 5)) {
    //          case 1:
    //              memcpy((void *)(buff_up + buff_index), (void
    //              *)",\"codr\":\"4/5\"", 13); buff_index += 13; break;
    //          case 2:
    //              memcpy((void *)(buff_up + buff_index), (void
    //              *)",\"codr\":\"4/6\"", 13); buff_index += 13; break;
    //          case 3:
    //              memcpy((void *)(buff_up + buff_index), (void
    //              *)",\"codr\":\"4/7\"", 13); buff_index += 13; break;
    //          case 4:
    //              memcpy((void *)(buff_up + buff_index), (void
    //              *)",\"codr\":\"4/8\"", 13); buff_index += 13; break;
    //          default:
    //              memcpy((void *)(buff_up + buff_index), (void
    //              *)",\"codr\":\"?\"", 13); buff_index += 11;
    //          }

    SNR = 9; // "lsnr":9
    j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE - buff_index,
                 ",\"lsnr\":%li", SNR);
    buff_index += j;
    /////////////////////////////////////////////////////////////////
    // TODO
    /////////////////////////////////////////////////////////////////

    // j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE-buff_index,
    // ",\"rssi\":%d,\"size\":%u", readRegister(0x1A)-rssicorr, receivedbytes);

    //"rssi":-69
    j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE - buff_index,
                 ",\"rssi\":%d,\"size\":%u", -69, receivedbytes);
    buff_index += j;

    /////////////////////////////////////////////////////////////////

    memcpy((void *)(buff_up + buff_index), (void *)",\"data\":\"", 9);
    buff_index += 9;

    ////////////////////////////////////////////////////////////////////////
    ///                  Message received                               ////
    ////////////////////////////////////////////////////////////////////////

    memcpy((void *)(buff_up + buff_index), (void *)(my_msg), receivedbytes);

    buff_index += receivedbytes;

    // j = bin_to_b64((uint8_t *)message, receivedbytes, (char *)(buff_up +
    // buff_index), 341);

    ////////////////////////////////////////////////////////////////////////
    ///                END  Message received                            ////
    ////////////////////////////////////////////////////////////////////////
    // buff_index += j;
    buff_up[buff_index] = '"';
    ++buff_index;

    /* End of packet serialization */
    buff_up[buff_index] = '}';
    ++buff_index;
    buff_up[buff_index] = ']';
    ++buff_index;
    /* end of JSON datagram payload */
    buff_up[buff_index] = '}';
    ++buff_index;
    buff_up[buff_index] = 0; /* add string terminator, for safety */

    printf("rxpk update: %s\n",
           (char *)(buff_up + 12)); /* DEBUG: display JSON payload */

    // send the messages
    sendudp(buff_up, buff_index);

    fflush(stdout);
}

void parseCommandline(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (0 == strncasecmp(argv[i], "-u", 2)) {
            char ip[INET6_ADDRSTRLEN];
            std::string server = argv[i] + 2;
            int port = DEFAULTPORT;
            if (server.find(':') != std::string::npos) {
                std::stringstream parser(
                    server.substr(server.find(':') + 1, std::string::npos));
                parser >> port;
                server = server.substr(0, server.find(':'));
                if (port == 0) {
                    std::stringstream error;
                    error << "Invalid server string given, cannot parse given "
                             "port number: ";
                    error << parser.str();
                    die(error.str().c_str());
                } else if (port < 1024) {
                    die("Don't use priviledged ports < 1024 for sending");
                }
            }
            hostToIp(server.c_str(), ip, INET6_ADDRSTRLEN);
            std::string address = ip;
            serverList.insert(
                std::make_pair(server, std::make_pair(address, port)));
        } else if (0 == strncasecmp(argv[i], "-sf", 3)) {
            int sFactor = 0;
            std::string sfString = argv[i] + 3;
            std::stringstream parser(sfString);
            parser >> sFactor;

            switch (sFactor) {
            case 7:
                sf = SF7;
                break;
            case 8:
                sf = SF8;
                break;
            case 9:
                sf = SF9;
                break;
            case 10:
                sf = SF10;
                break;
            case 11:
                sf = SF11;
                break;
            case 12:
                sf = SF12;
                break;
            default:
                die("Invalid spreading factor specified, valid range is 7-12");
            }
        } else if (0 == strncasecmp(argv[i], "-f", 2)) {
            uint32_t frequency = 0;
            std::string fString = argv[i] + 2;
            std::stringstream parser(fString);
            parser >> frequency;

            if (frequency >= EU868_J4 && frequency <= EU868_F6) {
                // EUR Channels
                if (frequency == EU868_F1 || frequency == EU868_F2 ||
                    frequency == EU868_F3 || frequency == EU868_F4 ||
                    frequency == EU868_F5 || frequency == EU868_F6 ||
                    frequency == EU868_J4 || frequency == EU868_J5 ||
                    frequency == EU868_J6) {
                    freq = frequency;
                } else {
                    std::stringstream error;
                    error << "Invalid EU channel specified: ";
                    error << frequency;
                    error << "Hz";
                    die(error.str().c_str());
                }
            } else if (frequency >= 902000000 && frequency <= 928000000) {
                // Todo valid channels in the US
                freq = frequency;
            } else {
                std::stringstream error;
                error << "The specified frequency ";
                error << frequency;
                error << "Hz is outside of any valid ISM band.";
                die(error.str().c_str());
            }
        } else {
            std::cout
                << "Usage: " << argv[0]
                << " [-uSERVERNAMEORIP[:PORT]] [-sf(7-12)] [-fFREQUENCYHZ]"
                << std::endl;
            std::cout << "   Example: " << argv[0]
                      << " -ucroft.thethings.girovito.nl" << std::endl;
            std::cout << "   Example: " << argv[0]
                      << " -ucroft.thethings.girovito.nl:1700" << std::endl;
            std::cout << "   Example: " << argv[0]
                      << " -u192.168.0.111 -sf8 -f868100000" << std::endl;
            std::cout << "   Multiple servers can be supplied with multiple -u "
                         "parameters."
                      << std::endl;
            std::cout
                << "   When no server is supplied, the default server is used ("
                << DEFAULTSERVER << ")." << std::endl;
            std::cout
                << "   When no port is supplied, the default port is used ("
                << DEFAULTPORT << ")." << std::endl;
            std::cout << "   Spreading factor can be specified using -sf7 to "
                         "-sf12 (default value is SF7)"
                      << std::endl;
            std::cout << "   Listening frequency can be specified using "
                         "-fFREQUENCY in Hz. (default 868100000)"
                      << std::endl;
            std::cout << "   European frequencies are checked for a valid Lora "
                         "channel, valid Frequencies are: "
                      << std::endl;
            std::cout << "     868100000Hz, 868300000Hz, 868500000Hz,"
                      << std::endl;
            std::cout << "     868850000Hz, 869050000Hz, 869525000Hz,"
                      << std::endl;
            std::cout << "     864100000Hz, 864300000Hz, 864500000Hz"
                      << std::endl;
            std::cout << "   The 900MHz Range (902-928MHz) is currently not "
                         "validated."
                      << std::endl
                      << std::endl;

            std::stringstream error;
            error << "Unknown command line parameter: ";
            error << argv[i];
            die(error.str().c_str());
        }
    }

    if (0 == serverList.size()) {
        // Add the default server, if no servers are given on the command line,
        // to make it work as before without any parameters supplied.
        char ip[INET6_ADDRSTRLEN];
        hostToIp(DEFAULTSERVER, ip, INET6_ADDRSTRLEN);
        std::string server = DEFAULTSERVER;
        std::string address = ip;
        serverList.insert(
            std::make_pair(server, std::make_pair(address, DEFAULTPORT)));
    }
}

void forwarder(const char my_msg[], const int receivedbytes) {

    std::stringstream desc;
    desc << "Single channel, ";
    desc << (double)freq / 1000000 << "MHz, ";
    desc << "SF" << sf;
    strncpy(description, desc.str().c_str(), 64);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        die("socket");
    }
    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(DEFAULTPORT);

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "enp0s31f6:", IFNAMSIZ - 1); // can we rely on eth0?

    printf("Listening at SF%i on %.6lf Mhz.\n", sf, (double)freq / 1000000);

    // while(1) {

    receivepacket(my_msg, receivedbytes);

    //      gettimeofday(&nowtime, NULL);
    //      uint32_t nowseconds = (uint32_t)(nowtime.tv_sec);
    //      if (nowseconds - lasttime >= 30) {
    //          lasttime = nowseconds;
    //          sendstat();
    //          cp_nb_rx_rcv = 0;
    //          cp_nb_rx_ok = 0;
    //          cp_up_pkt_fwd = 0;
    //      }
    // }
    // return (0);
}

// int main(int argc, char *argv[] ) {
//
//     parseCommandline(argc, argv);
//     std::stringstream desc;
//     desc << "Single channel, ";
//     desc << (double)freq/1000000 << "MHz, ";
//     desc << "SF" << sf;
//     strncpy(description, desc.str().c_str(), 64);
//     struct timeval nowtime;
//     uint32_t lasttime;
//
//     if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
//     {
//         die("socket");
//     }
//     memset((char *) &si_other, 0, sizeof(si_other));
//     si_other.sin_family = AF_INET;
//     si_other.sin_port = htons(DEFAULTPORT);
//
//     ifr.ifr_addr.sa_family = AF_INET;
//     strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);  // can we rely on eth0?
//     ioctl(s, SIOCGIFHWADDR, &ifr);
//
//     /* display result */
//     printf("Gateway ID: %.2x:%.2x:%.2x:ff:ff:%.2x:%.2x:%.2x\n",
//            (unsigned char)ifr.ifr_hwaddr.sa_data[0],
//            (unsigned char)ifr.ifr_hwaddr.sa_data[1],
//            (unsigned char)ifr.ifr_hwaddr.sa_data[2],
//            (unsigned char)ifr.ifr_hwaddr.sa_data[3],
//            (unsigned char)ifr.ifr_hwaddr.sa_data[4],
//            (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
//
//     printf("Listening at SF%i on %.6lf Mhz.\n", sf,(double)freq/1000000);
//
//     std::map<std::string, std::pair<std::string, int> >::iterator iter;
//     for(iter = serverList.begin(); iter != serverList.end(); iter++)
//     {
//         printf("Forwarding packets to: %s (%s), port: %d\n",
//         iter->first.c_str(), iter->second.first.c_str(),
//         iter->second.second);
//     }
//
//     printf("------------------\n");
//
//     while(1) {
//
//         receivepacket();
//
//         gettimeofday(&nowtime, NULL);
//         uint32_t nowseconds = (uint32_t)(nowtime.tv_sec);
//         if (nowseconds - lasttime >= 30) {
//             lasttime = nowseconds;
//             sendstat();
//             cp_nb_rx_rcv = 0;
//             cp_nb_rx_ok = 0;
//             cp_up_pkt_fwd = 0;
//         }
//     }
//     return (0);
// }
