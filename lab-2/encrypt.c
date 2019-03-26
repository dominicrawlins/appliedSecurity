/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
*
* Use of this source code is restricted per the CC BY-NC-ND license, a copy of
* which can be found via http://creativecommons.org (and should be included as
* LICENSE.txt within the associated archive or repository).
*/

#include "encrypt.h"

#define matrixsize 16

#define TEST 1

#define TRUE 1
#define FALSE 0

aes_gf28_t xtime( aes_gf28_t a );
aes_gf28_t sbox( aes_gf28_t a );
aes_gf28_t inverse( aes_gf28_t a );
aes_gf28_t aes_gf28_mul ( aes_gf28_t a, aes_gf28_t b );
void aes_init(uint8_t* k, uint8_t* r );
void aes_enc( uint8_t* c, uint8_t* m, uint8_t* k );
void aes_enc_key_update(aes_gf28_t* k, aes_gf28_t rc);
void aes_enc_add_rnd_key( aes_gf28_t* s, aes_gf28_t* rk );
void aes_enc_rnd_sub( aes_gf28_t* s );
void aes_enc_rnd_row( aes_gf28_t* s );
void aes_enc_rnd_mix( aes_gf28_t* s );
aes_gf28_t roundkey(aes_gf28_t previous);
void test();
void testinverse();
void testsbox();
void testroundkeyupdate();
void testaddroundkey();
void testsubbytes();
void testshiftrows();
void testmixcolumns();
int testmatrices(aes_gf28_t* a, aes_gf28_t* b);


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

  //run with argument 1 to test
  int main( int argc, char* argv[] ) {
    aes_gf28_t k[ 16 ] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };
    aes_gf28_t m[ 16 ] = { 0x32, 0x43, 0xF6, 0xA8, 0x88, 0x5A, 0x30, 0x8D, 0x31, 0x31, 0x98, 0xA2, 0xE0, 0x37, 0x07, 0x34 };
    aes_gf28_t c[ 16 ] = { 0x39, 0x25, 0x84, 0x1D, 0x02, 0xDC, 0x09, 0xFB, 0xDC, 0x11, 0x85, 0x97, 0x19, 0x6A, 0x0B, 0x32 };
    aes_gf28_t t[ 16 ];

    int isTest = 0;
    if(argc == 2){
      isTest = atoi(argv[1]);
    }

    if(isTest == TEST){
      printf("\n\n------------------\nrunning testing\n------------------\n\n\n\n");
      test();
    }
    else{
      aes_enc(t, m, k);
      if( !memcmp( t, c, 16 * sizeof( uint8_t ) ) ) {
        printf( "encryption correct\n" );
      }
      else {
        printf( "encryption incorrect\n" );
        for(int i = 0; i < 16; i++){
          printf("expected value: %x, actual value: %x\n", c[i], t[i]);
        }
      }


    }


    /*
    AES_KEY rk;

    AES_set_encrypt_key( k, 128, &rk );
    AES_encrypt( m, t, &rk );

    if( !memcmp( t, c, 16 * sizeof( uint8_t ) ) ) {
    printf( "AES.Enc( k, m ) == c\n" );
  }
  else {
  printf( "AES.Enc( k, m ) != c\n" );
}
aes_gf28_t a = 0x58;
aes_gf28_t an = sbox(a);
printf("%x\n", an);
*/




}

void aes_init(uint8_t* k, uint8_t* r ){



}


void aes_enc( aes_gf28_t* c, aes_gf28_t* m, aes_gf28_t* k ){
  aes_gf28_t rc[10] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};
  uint8_t roundKeyCounter = 0;
  for(int i = 0; i< matrixsize; i++){
    c[i] = m[i];
  }
  aes_enc_add_rnd_key(c, k);


  for(int i = 0; i < 9; i++){
    aes_enc_rnd_sub(c);
    aes_enc_rnd_row(c);
    aes_enc_rnd_mix(c);
    uint rconst = rc[roundKeyCounter];
    aes_enc_key_update(k, rconst);
    aes_enc_add_rnd_key(c, k);
    roundKeyCounter++;
  }
  aes_enc_rnd_sub(c);
  aes_enc_rnd_row(c);
  uint rconst = rc[roundKeyCounter];
  aes_enc_key_update(k, rconst);
  aes_enc_add_rnd_key(c, k);
}


void aes_enc_key_update(aes_gf28_t* k, aes_gf28_t rc){
  aes_gf28_t newkey[matrixsize];
  newkey[0] = rc ^ sbox(k[13]) ^ k[0];
  for(int row = 1; row < matrixsize/4; row++){
    newkey[row] = sbox(k[12+((row+1)%4)]) ^ k[row];
  }
  for(int column = 1; column < matrixsize/4; column++){
    for(int row = 0; row < matrixsize/4; row++){
      newkey[row + column*4] = k[row + column*4] ^ newkey[row+ (column-1)*4];
    }
  }
  for(int i = 0; i < matrixsize; i++){
    k[i] = newkey[i];
  }
}



aes_gf28_t xtime( aes_gf28_t a ){
  if( ( a & 0x80 ) == 0x80 ) {
    return 0x1B ^ ( a << 1 );
  }
  else{
    return ( a << 1 );
  }
}

aes_gf28_t roundkey(aes_gf28_t previous){
  return xtime(previous);
}





aes_gf28_t sbox( aes_gf28_t a ){
  a = sboxTable[a];
  return a;
}

aes_gf28_t inverse( aes_gf28_t a ){
  aes_gf28_t t_0 = aes_gf28_mul ( a, a ); // a^2
  aes_gf28_t t_1 = aes_gf28_mul ( t_0 , a ); // a^3
  t_0 = aes_gf28_mul ( t_0 , t_0 ); // a^4
  t_1 = aes_gf28_mul ( t_1 , t_0 ); // a^7
  t_0 = aes_gf28_mul ( t_0 , t_0 ); // a^8
  t_0 = aes_gf28_mul ( t_1 , t_0 ); // a^15
  t_0 = aes_gf28_mul ( t_0 , t_0 ); // a^30
  t_0 = aes_gf28_mul ( t_0 , t_0 ); // a^60
  t_1 = aes_gf28_mul ( t_1 , t_0 ); // a^67
  t_0 = aes_gf28_mul ( t_0 , t_1 ); // a^127
  t_0 = aes_gf28_mul ( t_0 , t_0 ); // a^254

  return t_0;
}


aes_gf28_t aes_gf28_mul ( aes_gf28_t a, aes_gf28_t b ) {
  aes_gf28_t t = 0;

  for( int i = 7; i >= 0; i-- ) {
    t = xtime ( t );

    if( ( b >> i ) & 1 ) {
      t ^= a;
    }
  }

  return t;
}

void aes_enc_add_rnd_key( aes_gf28_t* s, aes_gf28_t* rk ){
  for(int i = 0; i < matrixsize; i++){
    s[i] = s[i] ^ rk[i];
  }
}
void aes_enc_rnd_sub( aes_gf28_t* s ){
  for(int i = 0; i < matrixsize; i++){
    s[i] = sbox(s[i]);
  }
}

void aes_enc_rnd_row( aes_gf28_t* s ){
  for(int i = 0; i < matrixsize/4; i++){
    aes_gf28_t row[4];
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

void aes_enc_rnd_mix( aes_gf28_t* s ){
  aes_gf28_t newmatrix[16];
  for(int i = 0; i < matrixsize/4; i++){
    int columnoffset = i*4;
    aes_gf28_t xorarray[4] = {0x02, 0x03, 0x01, 0x01};
    for(int row = 0; row < matrixsize/4; row++){
      aes_gf28_t newvalue = 0x00;
      for(int loopno = 0; loopno < 4; loopno++){
        aes_gf28_t a = xorarray[(4-row + loopno)%4];
        aes_gf28_t b = s[columnoffset+loopno];
        newvalue ^= aes_gf28_mul(a,b);
      }
      newmatrix[columnoffset + row] = newvalue;
    }
  }
  for(int i = 0; i < 16; i++){
    s[i] = newmatrix[i];
  }
}

void test(){
  testinverse();
  testsbox();
  testroundkeyupdate();
  testaddroundkey();
  testsubbytes();
  testshiftrows();
  testmixcolumns();
}

void testinverse(){
  aes_gf28_t test[2] = {0x95, 0x58};

  aes_gf28_t expectedanswer[2] = {0x8A, 0x18};

  int testspassed = 0;

  for(int i = 0; i < 2; i++){
    aes_gf28_t givenanswer = inverse(test[i]);
    if(givenanswer == expectedanswer[i]){
      testspassed++;
    }
  }
  if(testspassed == 2){
    printf("inverse: PASSED\n");
  }
  else{
    printf("\ninverse: FAILED\n\n");
  }

}

void testsbox(){
  aes_gf28_t test[2] = {0x58, 0x70};

  aes_gf28_t expectedanswer[2] = {0x6A, 0x51};

  int testspassed = 0;

  for(int i = 0; i < 2; i++){
    aes_gf28_t givenanswer = sbox(test[i]);
    if(givenanswer == expectedanswer[i]){
      testspassed++;
    }
  }
  if(testspassed == 2){
    printf("sbox: PASSED\n");
  }
  else{
    printf("\nsbox: FAILED\n\n");
  }

}

void testroundkeyupdate(){
  aes_gf28_t key[16] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };
  aes_gf28_t correctanswer[16] = {0xa0,0xfa,0xfe,0x17, 0x88, 0x54,0x2c, 0xb1,0x23, 0xa3,0x39, 0x39,  0x2a,    0x6c,    0x76,    0x05};
  aes_gf28_t rc = 0x01;

  aes_enc_key_update(key, rc);

  int isRight = testmatrices(key, correctanswer);

  if(isRight){
    printf("round key update: PASSED\n");
  }
  else{
    printf("\nround key update: FAILED\n\n");
  }
}

void testaddroundkey(){
  aes_gf28_t first[16] = { 0x32, 0x43, 0xF6, 0xA8, 0x88, 0x5A, 0x30, 0x8D, 0x31, 0x31, 0x98, 0xA2, 0xE0, 0x37, 0x07, 0x34 };
  aes_gf28_t key[16] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };
  aes_gf28_t after[16] = {  0x19, 0x3d, 0xe3, 0xbe, 0xa0, 0xf4, 0xe2, 0x2b, 0x9a, 0xc6, 0x8d, 0x2a, 0xe9, 0xf8, 0x48, 0x08};

  aes_enc_add_rnd_key(first, key);

  int isRight = testmatrices(first, after);

  if(isRight){
    printf("add round key: PASSED\n");
  }
  else{
    printf("\nadd round key: FAILED\n\n");
  }
}

void testsubbytes(){
  aes_gf28_t c[16] = {  0x19, 0x3d, 0xe3, 0xbe, 0xa0, 0xf4, 0xe2, 0x2b, 0x9a, 0xc6, 0x8d, 0x2a, 0xe9, 0xf8, 0x48, 0x08};
  aes_gf28_t answer[16] = {0xd4, 0x27, 0x11, 0xae, 0xe0, 0xbf, 0x98, 0xf1, 0xb8, 0xb4, 0x5d, 0xe5, 0x1e, 0x41, 0x52, 0x30};

  aes_enc_rnd_sub(c);

  int isRight = testmatrices(c, answer);

  if(isRight){
    printf("sub bytes: PASSED\n");
  }
  else{
    printf("\nsub bytes: FAILED\n\n");
  }
}

void testshiftrows(){
  aes_gf28_t c[16] = {0xd4, 0x27, 0x11, 0xae, 0xe0, 0xbf, 0x98, 0xf1, 0xb8, 0xb4, 0x5d, 0xe5, 0x1e, 0x41, 0x52, 0x30};
  aes_gf28_t answer[16] = {0xd4, 0xbf, 0x5d, 0x30, 0xe0, 0xb4, 0x52, 0xae, 0xb8, 0x41, 0x11, 0xf1, 0x1e, 0x27, 0x98, 0xe5};

  aes_enc_rnd_row(c);

  int isRight = testmatrices(c, answer);

  if(isRight){
    printf("shift rows: PASSED\n");
  }
  else{
    printf("\nshift rows: FAILED\n\n");
  }

}

void testmixcolumns(){
  aes_gf28_t c[16] = {0xd4, 0xbf, 0x5d, 0x30, 0xe0, 0xb4, 0x52, 0xae, 0xb8, 0x41, 0x11, 0xf1, 0x1e, 0x27, 0x98, 0xe5};
  aes_gf28_t answer[16] = {0x04, 0x66, 0x81, 0xe5, 0xe0, 0xcb, 0x19, 0x9a, 0x48, 0xf8, 0xd3, 0x7a, 0x28, 0x06, 0x26, 0x4c};

  aes_enc_rnd_mix(c);

  int isRight = testmatrices(c, answer);

  if(isRight){
    printf("mix columns: PASSED\n");
  }
  else{
    printf("\nmix columns: FAILED\n\n");
  }
}

int testmatrices(aes_gf28_t* a, aes_gf28_t* b){
  int isRight = TRUE;
  for(int i = 0; i < 16; i++){
    if(!(a[i] == b[i])){
      isRight = FALSE;
    }
  }
  return isRight;
}
