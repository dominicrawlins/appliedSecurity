/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
*
* Use of this source code is restricted per the CC BY-NC-ND license, a copy of
* which can be found via http://creativecommons.org (and should be included as
* LICENSE.txt within the associated archive or repository).
*/

#include "helloworld.h"

uint8_t chartohex(uint8_t in){
  if(in > 9){
    return (in - 55);
  }
  else{
    return(in - 48);
  }
}

uint8_t octetstr_rd( uint8_t* r, int n_r ){
  uint8_t hexsize[2];
  hexsize[0] = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  hexsize[1] = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  scale_uart_rd(SCALE_UART_MODE_BLOCKING); //:
  hexsize[0] = chartohex(hexsize[0]);
  hexsize[1] = chartohex(hexsize[1]);
  uint8_t size = (hexsize[0] * 0x10) + hexsize[1];
  if(size > n_r){
    size = n_r;
  }
  for(int i = 0; i < size; i++){
    r[i] = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  }
  scale_uart_rd(SCALE_UART_MODE_BLOCKING); //CR
  return size;
}

void octetstr_wr( const uint8_t* x, int n_x ){
  if( scale_uart_wr_avail() == true){
    for(int i = 0; i < n_x; i++){
      scale_uart_wr(SCALE_UART_MODE_BLOCKING, x[i]);
    }
  }
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, '\x0D');//CR  for board & emulator
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, '\x0A');//LF  for emulator

}

int main( int argc, char* argv[] ) {
  // initialise the development board, using the default configuration
  if( !scale_init( &SCALE_CONF ) ) {
    return -1;
  }

  //  char x[] = "hello world";

  while( true ) {
    // read  the GPI     pin, and hence switch : t   <- GPI
    bool t = scale_gpio_rd( SCALE_GPIO_PIN_GPI        );
    // write the GPO     pin, and hence LED    : GPO <- t
    scale_gpio_wr( SCALE_GPIO_PIN_GPO, t     );

    // write the trigger pin, and hence LED    : TRG <- 1 (positive edge)
    scale_gpio_wr( SCALE_GPIO_PIN_TRG, true  );
    // delay for 500 ms = 1/2 s
    scale_delay_ms( 500 );
    // write the trigger pin, and hence LED    : TRG <- 0 (negative edge)
    scale_gpio_wr( SCALE_GPIO_PIN_TRG, false );
    // delay for 500 ms = 1/2 s
    scale_delay_ms( 500 );

    /*  int n = strlen( x );

    // write x = "hello world" to the UART
    for( int i = 0; i < n; i++ ) {
    scale_uart_wr( SCALE_UART_MODE_BLOCKING, x[ i ] );
  }
}
*/

    uint8_t* writeString = (uint8_t*)"cxauu";
    //char done = octetstr_rd(writeString, 5);
    octetstr_wr( writeString, 5);
    uint8_t* readString = (uint8_t*)"        ";
    uint8_t bytesRead = octetstr_rd(readString, 8);
    octetstr_wr(readString, bytesRead);
    //if(done == 1){
      //octetstr_wr(&string, 5);
    //}
  }
  return 0;
}
