#include <stdint.h> //#include "board.h"
#include <iostream> // sdt::cout, sdt::cin, sdt::endl
#include "forwarder.hpp"

extern "C" {
#include "mbedtls/base64.h"
#include "node_like_fct.h"
}

using std::cout;
using std::endl;

/*!
 * Buffer containing the data to be sent or received.
 */
// static uint8_t LoRaMacBuffer[LORAMAC_PHY_MAXPAYLOAD];
extern uint8_t LoRaBridgeToGatewayBuffer[];

/*!
 * Length of packet in LoRaMacBuffer
 */
// static uint16_t LoRaMacBufferPktLen = 0;
extern uint16_t LoRaBridgeToGatewayBufferPktLen;

int main() {
    // uint8_t MyBuffer[255]={'d','a','t','A','B','C'};
    unsigned char MyBuffer[255] = {'d', 'a', 't', 'a'};
    unsigned char size_in = 4;

    PrepareFrameTx(MyBuffer, size_in);

    const unsigned char size_data_out_max = 255;
    size_t size_data_out = 0;
    unsigned char data_out[size_data_out_max];

    data_out[size_data_out] = 0;

    mbedtls_base64_encode(data_out, sizeof(data_out), &size_data_out,
                          LoRaBridgeToGatewayBuffer,
                          LoRaBridgeToGatewayBufferPktLen);
    cout << data_out << endl;

    forwarder((char *)data_out, size_data_out);
}
