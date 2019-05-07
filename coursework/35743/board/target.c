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

  uint8_t sboxTable[256] = {0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16};


    uint8_t masks[4];
    uint8_t masksPrime[4];
    uint8_t bigM;
    uint8_t bigMPrime;

    uint8_t sboxMasked[256];


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
  //scale_uart_wr(SCALE_UART_MODE_BLOCKING, '\x0D');//CR  for board & emulator
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
}

uint8_t xtime( uint8_t a ){
  if( ( a & 0x80 ) == 0x80 ) {
    return 0x1B ^ ( a << 1 );
  }
  else{
    return ( a << 1 );
  }
}

uint8_t aes_gf28_mul ( uint8_t a, uint8_t b ) {
  uint8_t t = 0;

  for( int i = 7; i >= 0; i-- ) {
    t = xtime ( t );

    if( ( b >> i ) & 1 ) {
      t ^= a;
    }
  }

  return t;
}

void calculateMixColMask(){
  uint8_t xorarray[4] = {0x02, 0x03, 0x01, 0x01};
  for(int maskNumber = 0; maskNumber < 4; maskNumber++){
    uint8_t newvalue = 0x00;
    for(int loopno = 0; loopno < 4; loopno++){
      uint8_t a = xorarray[(4-maskNumber + loopno)%4];
      uint8_t b = masks[loopno];
      newvalue ^= aes_gf28_mul(a,b);
    }
    masksPrime[maskNumber] = newvalue;
  }
}

void calculateSboxMasked(){
  for(int i = 0; i < 256; i++){
    sboxMasked[i ^ bigM] = sboxTable[i] ^ bigMPrime;
  }
}

void mask(uint8_t* c, uint8_t mask1, uint8_t mask2, uint8_t mask3, uint8_t mask4, uint8_t mask5, uint8_t mask6, uint8_t mask7, uint8_t mask8){
	for(int i = 0; i< 4; i++){
		c[(i*4)]	= c[(i*4)] ^ (mask1 ^ mask5);
		c[(i*4)+1]	= c[(i*4)+1] ^ (mask2 ^ mask6);
		c[(i*4)+2]	= c[(i*4)+2] ^ (mask3 ^ mask7);
		c[(i*4)+3]	= c[(i*4)+3] ^ (mask4 ^ mask8);
	}
}



/** Initialise an AES-128 encryption, e.g., expand the cipher key k into round
  * keys, or perform randomised pre-computation in support of a countermeasure;
  * this can be left blank if no such initialisation is required, because the
  * same k and r will be passed as input to the encryption itself.
  *
  * \param[in]  k   an   AES-128 cipher key
  * \param[in]  r   some         randomness
  */
  uint8_t sbox( uint8_t a ){
    a = sboxTable[a];
    return a;
  }



  void aes_enc_rnd_mix( uint8_t* s ){
    uint8_t newmatrix[16];
    for(int i = 0; i < 4; i++){
      int columnoffset = i*4;
      uint8_t xorarray[4] = {0x02, 0x03, 0x01, 0x01};
      for(int row = 0; row < 4; row++){
        uint8_t newvalue = 0x00;
        for(int loopno = 0; loopno < 4; loopno++){
          uint8_t a = xorarray[(4-row + loopno)%4];
          uint8_t b = s[columnoffset+loopno];
          newvalue ^= aes_gf28_mul(a,b);
        }
        newmatrix[columnoffset + row] = newvalue;
      }
    }
    for(int i = 0; i < 16; i++){
      s[i] = newmatrix[i];
    }
  }

  void aes_enc_rnd_row( uint8_t* s ){
    for(int i = 0; i < 4; i++){
      uint8_t row[4];
      for(int j = 0; j < 4; j++){
        row[j] = s[j*4 + i];
      }
      int pushforward = 4-i;
      for(int j = 0; j < 4; j++){
        int newmatrixposition = (j+pushforward)%4;
        s[newmatrixposition*4 + i] = row[j];
      }
    }
  }

  void aes_enc_rnd_sub( uint8_t* s ){
    for(int i = 0; i < 16; i++){
      s[i] = sboxMasked[(s[i])];
    }
  }


  void aes_enc_add_rnd_key( uint8_t* s, uint8_t* rk , uint8_t remaskingType){
    uint8_t remaskKey[16];
    for(int i = 0; i < 16; i++){
      remaskKey[i] = rk[i];
    }
    if(remaskingType == 0){
      mask(remaskKey, masksPrime[0], masksPrime[1], masksPrime[2], masksPrime[3], bigM, bigM, bigM, bigM);
    }
    else if(remaskingType == 1){
      mask(remaskKey, 0, 0, 0, 0, bigMPrime, bigMPrime, bigMPrime, bigMPrime);
    }
    for(int i = 0; i < 16; i++){
      s[i] = s[i] ^ remaskKey[i];
    }
  }


  void aes_enc_key_update(uint8_t* k, uint8_t rc){
    uint8_t newkey[16];
    newkey[0] = rc ^ sbox(k[13]) ^ k[0];
    for(int row = 1; row < 4; row++){
      newkey[row] = sbox(k[12+((row+1)%4)]) ^ k[row];
    }
    for(int column = 1; column < 4; column++){
      for(int row = 0; row < 4; row++){
        newkey[row + column*4] = k[row + column*4] ^ newkey[row+ (column-1)*4];
      }
    }
    for(int i = 0; i < 16; i++){
      k[i] = newkey[i];
    }
  }


void aes_init(                                uint8_t* k, const uint8_t* r ) {
  for(int i = 0; i < 4; i++){
    masks[i] = r[i];
  }
  bigM = r[4];
  bigMPrime = r[5];
  calculateMixColMask();
  calculateSboxMasked();
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

void aes     ( uint8_t* c, const uint8_t* m,  uint8_t* k, const uint8_t* r ) {
  aes_init(k, 0);
  uint8_t rc[10] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};
  uint8_t roundKeyCounter = 0;
  for(int i = 0; i< 16; i++){
    c[i] = m[i];
  }
  mask(c, masksPrime[0], masksPrime[1], masksPrime[2], masksPrime[3], 0, 0, 0, 0);
  aes_enc_add_rnd_key(c, k, 0);


  for(int i = 0; i < 9; i++){
    aes_enc_rnd_sub(c);
    aes_enc_rnd_row(c);
    mask(c, masks[0], masks[1], masks[2], masks[3], bigMPrime, bigMPrime, bigMPrime, bigMPrime);
    aes_enc_rnd_mix(c);
    uint8_t rconst = rc[roundKeyCounter];
    aes_enc_key_update(k, rconst);
    aes_enc_add_rnd_key(c, k, 0);
    roundKeyCounter++;
  }
  aes_enc_rnd_sub(c);
  aes_enc_rnd_row(c);
  uint8_t rconst = rc[roundKeyCounter];
  aes_enc_key_update(k, rconst);
  aes_enc_add_rnd_key(c, k, 1);
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

  uint8_t cmd[ 1 ], c[ SIZEOF_BLK ], m[ SIZEOF_BLK ] = {0x32, 0x43, 0xF6, 0xA8, 0x88, 0x5A, 0x30, 0x8D, 0x31, 0x31, 0x98, 0xA2, 0xE0, 0x37, 0x07, 0x34}, k[ SIZEOF_KEY ] = { 0xCD, 0x97, 0x16, 0xE9, 0x5B, 0x42, 0xDD, 0x48, 0x69, 0x77, 0x2A, 0x34, 0x6A, 0x7F, 0x58, 0x13 }, r[ SIZEOF_RND ] = {0x01,0x02,0x03,0x04,0x05,0x06};

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
      case 2: {
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
