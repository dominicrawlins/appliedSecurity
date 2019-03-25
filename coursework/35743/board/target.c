/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "target.h"

/** Read  an octet string (or sequence of bytes) from the UART, using a simple
  * length-prefixed, little-endian hexadecimal format.
  *
  * \param[out] r the destination octet string read
  * \return       the number of octets read
  */

  uint8_t charToHex(char in){
    if(in > 64 && in < 71){
      return (in - 55);
    }
    else if(in > 96 && in < 103){
      return(in - 87);
    }
    else if(in > 47 && in < 58){
      return(in - 48);
    }
    else return -1;
  }

  char hexToChar(uint8_t in){
    if(in > 9){
      return (in + 55);
    }
    else{
      return(in + 48);
    }
  }


int  octetstr_rd(       uint8_t* r, int n_r ) {
  uint8_t hexsize[2];
  hexsize[0] = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  hexsize[1] = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  scale_uart_rd(SCALE_UART_MODE_BLOCKING); //:
  hexsize[0] = charToHex(hexsize[0]);
  hexsize[1] = charToHex(hexsize[1]);
  uint8_t size = (hexsize[0] * 0x10) + hexsize[1];
  if(size > n_r){
    size = n_r;
  }
  for(int i = 0; i < size; i++){
    uint8_t firstCharacter = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
    uint8_t secondCharacter = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
    r[i] = (charToHex(firstCharacter) * 0x10) + charToHex(secondCharacter);
  }
  scale_uart_rd(SCALE_UART_MODE_BLOCKING); //CR
//  scale_uart_rd(SCALE_UART_MODE_BLOCKING); //LF
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, '\x0D');//CR  for board & emulator
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, '\x0A');//LF  for emulator
  return size;
}

/** Write an octet string (or sequence of bytes) to   the UART, using a simple
  * length-prefixed, little-endian hexadecimal format.
  *
  * \param[in]  r the source      octet string written
  * \param[in]  n the number of octets written
  */

void octetstr_wr( const uint8_t* x, int n_x ) {
  scale_uart_wr( SCALE_UART_MODE_BLOCKING, hexToChar(n_x / 0x10));
  scale_uart_wr( SCALE_UART_MODE_BLOCKING, hexToChar(n_x % 0x10));
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, ':');
  for(int i = 0; i < n_x; i++){
    scale_uart_wr(SCALE_UART_MODE_BLOCKING, hexToChar(x[i]/0x10));
    scale_uart_wr(SCALE_UART_MODE_BLOCKING, hexToChar(x[i] % 0x10));
  }
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, '\x0D');//CR  for board & emulator
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, '\x0A');//LF  for emulator
}

/** Initialise an AES-128 encryption, e.g., expand the cipher key k into round
  * keys, or perform randomised pre-computation in support of a countermeasure;
  * this can be left blank if no such initialisation is required, because the
  * same k and r will be passed as input to the encryption itself.
  *
  * \param[in]  k   an   AES-128 cipher key
  * \param[in]  r   some         randomness
  */

void aes_init(                               const uint8_t* k, const uint8_t* r ) {
  return;
}

/** Perform    an AES-128 encryption of a plaintext m under a cipher key k, to
  * yield the corresponding ciphertext c.
  *
  * \param[out] c   an   AES-128 ciphertext
  * \param[in]  m   an   AES-128 plaintext
  * \param[in]  k   an   AES-128 cipher key
  * \param[in]  r   some         randomness
  */

void aes     ( uint8_t* c, const uint8_t* m, const uint8_t* k, const uint8_t* r ) {
  return;
}

/** Initialise the SCALE development board, then loop indefinitely, reading a
  * command then processing it:
  *
  * 1. If command is inspect, then
  *
  *    - write the SIZEOF_BLK parameter,
  *      i.e., number of bytes in an  AES-128 plaintext  m, or ciphertext c,
  *      to the UART,
  *    - write the SIZEOF_KEY parameter,
  *      i.e., number of bytes in an  AES-128 cipher key k,
  *      to the UART,
  *    - write the SIZEOF_RND parameter,
  *      i.e., number of bytes in the         randomness r.
  *      to the UART.
  *
  * 2. If command is encrypt, then
  *
  *    - read  an   AES-128 plaintext  m from the UART,
  *    - read  some         randomness r from the UART,
  *    - initalise the encryption,
  *    - set the trigger signal to 1,
  *    - execute   the encryption, producing the ciphertext
  *
  *      c = AES-128.Enc( m, k )
  *
  *      using the hard-coded cipher key k plus randomness r if/when need be,
  *    - set the trigger signal to 0,
  *    - write an   AES-128 ciphertext c to   the UART.
  */

int main( int argc, char* argv[] ) {
  if( !scale_init( &SCALE_CONF ) ) {
    return -1;
  }

  uint8_t cmd[ 1 ], c[ SIZEOF_BLK ], m[ SIZEOF_BLK ], k[ SIZEOF_KEY ] = { 0x99, 0x79, 0x48, 0x3F, 0xDC, 0x2B, 0xE0, 0xB5, 0x7E, 0x90, 0x64, 0x50, 0xFC, 0x58, 0xB1, 0xB6 }, r[ SIZEOF_RND ];

  while( true ) {
    if( 1 != octetstr_rd( cmd, 1 ) ) {
      break;
    }

    switch( cmd[ 0 ] ) {
      case COMMAND_INSPECT : {
        uint8_t t = SIZEOF_BLK;
                    octetstr_wr( &t, 1 );
                t = SIZEOF_KEY;
                    octetstr_wr( &t, 1 );
                t = SIZEOF_RND;
                    octetstr_wr( &t, 1 );

        break;
      }
      case COMMAND_ENCRYPT : {
        if( SIZEOF_BLK != octetstr_rd( m, SIZEOF_BLK ) ) {
          break;
        }
        if( SIZEOF_RND != octetstr_rd( r, SIZEOF_RND ) ) {
          break;
        }

        aes_init(       k, r );

        scale_gpio_wr( SCALE_GPIO_PIN_TRG,  true );
        aes     ( c, m, k, r );
        scale_gpio_wr( SCALE_GPIO_PIN_TRG, false );

                          octetstr_wr( c, SIZEOF_BLK );

        break;
      }
      case 3 : {
        uint8_t returnthatstring[10];
        int sizeofstring = octetstr_rd(returnthatstring, 10);
        octetstr_wr(returnthatstring, sizeofstring);
        break;
      }

      default : {
        break;
      }
    }
  }

  return 0;
}
