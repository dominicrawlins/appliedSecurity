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

//run with argument 1 to test
int main( int argc, char* argv[] ) {
  aes_gf28_t k[ 16 ] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };
  aes_gf28_t m[ 16 ] = { 0x32, 0x43, 0xF6, 0xA8, 0x88, 0x5A, 0x30, 0x8D, 0x31, 0x31, 0x98, 0xA2, 0xE0, 0x37, 0x07, 0x34 };
  aes_gf28_t c[ 16 ] = { 0x39, 0x25, 0x84, 0x1D, 0x02, 0xDC, 0x09, 0xFB, 0xDC, 0x11, 0x85, 0x97, 0x19, 0x6A, 0x0B, 0x32 };
  aes_gf28_t t[ 16 ];


  int isTest = atoi(argv[1]);
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


void aes_enc( uint8_t* c, uint8_t* m, uint8_t* k ){
  c = m;
  aes_enc_add_rnd_key(c, k);

  aes_gf28_t rc = 0x01;
  for(int i = 0; i < 9; i++){
    aes_enc_rnd_sub(c);
    aes_enc_rnd_row(c);
    aes_enc_rnd_mix(c);
    aes_enc_key_update(k, rc);
    aes_enc_add_rnd_key(c, k);
    roundkey(rc);
  }
  aes_enc_rnd_sub(c);
  aes_enc_rnd_row(c);
  aes_enc_key_update(k, rc);
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
  a = inverse ( a );

  a = ( 0x63 ) ^ // 0 1 1 0 0 0 1 1
  ( a ) ^ // a_7 a_6 a_5 a_4 a_3 a_2 a_1 a_0
  ( a << 1 ) ^ // a_6 a_5 a_4 a_3 a_2 a_1 a_0 0
  ( a << 2 ) ^ // a_5 a_4 a_3 a_2 a_1 a_0 0 0
  ( a << 3 ) ^ // a_4 a_3 a_2 a_1 a_0 0 0 0
  ( a << 4 ) ^ // a_3 a_2 a_1 a_0 0 0 0 0
  ( a >> 7 ) ^ // 0 0 0 0 0 0 0 a_7
  ( a >> 6 ) ^ // 0 0 0 0 0 0 a_7 a_6
  ( a >> 5 ) ^ // 0 0 0 0 0 a_7 a_6 a_5
  ( a >> 4 ) ; // 0 0 0 0 a_7 a_6 a_5 a_4

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
