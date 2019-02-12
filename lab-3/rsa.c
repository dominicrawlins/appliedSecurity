/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "rsa.h"


void rsa_keygen( mpz_t N, mpz_t e, mpz_t d, int lambda );
void l2r_1exp( mpz_t r, mpz_t x, mpz_t y, mpz_t N );
void rsa_enc( mpz_t c, mpz_t m, mpz_t e, mpz_t N );
void rsa_dec( mpz_t m, mpz_t c, mpz_t d, mpz_t N );

int main( int argc, char* argv[] ) {

return 0;
}
