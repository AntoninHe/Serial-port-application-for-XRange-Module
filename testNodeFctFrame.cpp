#include <stdint.h> //#include "board.h"
#include <iostream>             // sdt::cout, sdt::cin, sdt::endl
#include "forwarder.hpp"

extern "C"{
    #include "base64.h"
    #include "base64/mbedtls/base64.h"
    #include "node_like_fct.h"
}

using std::cout;
using std::endl;

/*!
 * Buffer containing the data to be sent or received.
 */
//static uint8_t LoRaMacBuffer[LORAMAC_PHY_MAXPAYLOAD];
extern uint8_t LoRaBridgeToGatewayBuffer[];

/*!
 * Length of packet in LoRaMacBuffer
 */
//static uint16_t LoRaMacBufferPktLen = 0;
extern uint16_t LoRaBridgeToGatewayBufferPktLen;

int main(){
    //uint8_t MyBuffer[255]={'d','a','t','A','B','C'};
    unsigned char MyBuffer[255]={'d','a','t','a'};
    unsigned char size_in=4;

    PrepareFrameTx(MyBuffer,size_in);

    const unsigned char size_data_out_max = 255;
    size_t size_data_out = 0;
    unsigned char data_out[size_data_out_max];

    data_out[size_data_out]=0;

    mbedtls_base64_encode(data_out, sizeof(data_out), &size_data_out , LoRaBridgeToGatewayBuffer, LoRaBridgeToGatewayBufferPktLen);
    cout << data_out << endl;

    testForwarder( (char *)data_out, size_data_out);


//int mbedtls_base64_encode( unsigned char *dst, size_t dlen, size_t *olen,



//  {
//      const uint8_t size_data_out_max = 255;
//      size_t size_data_out = 0;
//      uint8_t data_out[size_data_out_max];


//      size_data_out = bin_to_b64(LoRaBridgeToGatewayBuffer, LoRaBridgeToGatewayBufferPktLen, (char *)data_out, (int)sizeof(data_out));
//      data_out[size_data_out]=0;
//      cout << data_out << endl;
//  }

    //cout << size_data_out << endl;

    //LoRaMacBuffer[LoRaMacBufferPktLen + 1] = '\0';

    //for(int i=0; i < LoRaMacBufferPktLen; i++){
    // for(int i=0; i < 5l0; i++){
    //     cout << i <<< ": " << std::to_string(LoRaMacBuffer[i]) << endl;

    // }
    //cout << LoRaMacBufferPktLen << endl;

    //cout << LoRaMacBuffer << endl;
}



