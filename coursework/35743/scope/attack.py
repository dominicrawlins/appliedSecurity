# Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
#
# Use of this source code is restricted per the CC BY-NC-ND license, a copy of
# which can be found via http://creativecommons.org (and should be included as
# LICENSE.txt within the associated archive or repository).

import numpy, struct, sys, binascii, time, random, serial, argparse
import matplotlib.pyplot as plt
import picoscope.ps2000a as ps2000a
from Crypto.Cipher import AES


sbox = [0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
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
0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16]

hammingWeightTable = []

PS2000A_RATIO_MODE_NONE      = 0
PS2000A_RATIO_MODE_AGGREGATE = 1
PS2000A_RATIO_MODE_DECIMATE  = 2
PS2000A_RATIO_MODE_AVERAGE   = 4


## Load  a trace data set from an on-disk file.
##
## \param[in] f the filename to load  trace data set from
## \return    t the number of traces
## \return    s the number of samples in each trace
## \return    M a t-by-16 matrix of AES-128  plaintexts
## \return    C a t-by-16 matrix of AES-128 ciphertexts
## \return    T a t-by-s  matrix of samples, i.e., the traces


def traces_ld( f ) :
  fd = open( f, "rb" )

  def rd( x ) :
    ( r, ) = struct.unpack( x, fd.read( struct.calcsize( x ) ) ) ; return r

  t = rd( '<I' )
  s = rd( '<I' )

  M = numpy.zeros( ( t, 16 ), dtype = numpy.uint8 )
  C = numpy.zeros( ( t, 16 ), dtype = numpy.uint8 )
  T = numpy.zeros( ( t,  s ), dtype = numpy.int16 )

  for i in range( t ) :
    for j in range( 16 ) :
      M[ i, j ] = rd( '<B' )

  for i in range( t ) :
    for j in range( 16 ) :
      C[ i, j ] = rd( '<B' )

  for i in range( t ) :
    for j in range( s  ) :
      T[ i, j ] = rd( '<h' )

  fd.close()

  return t, s, M, C, T

## Store a trace data set into an on-disk file.
##
## \param[in] f the filename to store trace data set into
## \param[in] t the number of traces
## \param[in] s the number of samples in each trace
## \param[in] M a t-by-16 matrix of AES-128  plaintexts
## \param[in] C a t-by-16 matrix of AES-128 ciphertexts
## \param[in] T a t-by-s  matrix of samples, i.e., the traces

def traces_st( f, t, s, M, C, T ) :
  fd = open( f, "wb" )

  def wr( x, y ) :
    fd.write( struct.pack( x, y ) )

  wr( '<I', t,  )
  wr( '<I', s,  )

  for i in range( t ) :
    for j in range( 16 ) :
      wr( '<B', M[ i, j ] )

  for i in range( t ) :
    for j in range( 16 ) :
      wr( '<B', C[ i, j ] )

  for i in range( t ) :
    for j in range( s  ) :
      wr( '<h', T[ i, j ] )

  fd.close()


def octetstr2str( x ) :
  t = x.split( ':' ) ; n = int( t[ 0 ], 16 ) ; x = binascii.a2b_hex( t[ 1 ] )

  if( n != len( x ) ) :
    raise ValueError
  else :
    return x

def str2octetstr( x ) :
  return ( '%02X' % ( len( x ) ) ) + ':' + ( binascii.b2a_hex( x ) )

def scope_adc2volts( range, x , scope_adc_max) :
  return ( float( x ) / float( scope_adc_max ) ) * range;


def board_rdln( fd    ) :
  r = ''

  while( True ):
    t = fd.read( 1 )

    if( t == '\x0D' ) :
      break
    else:
      r += t

  return r

def board_wrln( fd, x ) :
  fd.write( x + '\x0D' ) ; fd.flush()


def calculateHammingWeights():
    for byte in range(256):
        hammingWeight = 0
        while byte:
            hammingWeight += (byte & 1)
            byte = byte >> 1
        hammingWeightTable.append(hammingWeight)


def lsbHypotheticalConsumption(plaintextByte, keyHypothesis):
    xorValue = plaintextByte ^ keyHypothesis
    hypInterValue = sbox[xorValue]
    hypotheticalPowerConsumption = hypInterValue & 0x01
    return hypotheticalPowerConsumption


def hammingWeightConsumption(plaintextByte, keyHypothesis):
    xorValue = plaintextByte ^ keyHypothesis
    hypInterValue = sbox[xorValue]
    hypotheticalPowerConsumption = hammingWeightTable[hypInterValue]
    return hypotheticalPowerConsumption

def checkKey(finalKey, M, C):
    k = struct.pack( 16 * 'B', *finalKey )
    m = struct.pack( 16 * 'B', *M )
    c = struct.pack( 16 * 'B', *C )
    t = AES.new( k ).encrypt( m )
    if(t == c):
        print("correct")
    else:
        print("incorrect")


def acquireTraces():
    numberOfTraces = 100

    parser = argparse.ArgumentParser()

    parser.add_argument( '--mode', dest = 'mode',             action = 'store', choices = [ 'uart', 'socket' ], default = 'uart'             )

    parser.add_argument( '--uart', dest = 'uart', type = str, action = 'store',                                 default = '/dev/scale-board' )
    parser.add_argument( '--host', dest = 'host', type = str, action = 'store' )
    parser.add_argument( '--port', dest = 'port', type = int, action = 'store' )

    parser.add_argument( '--data', dest = 'data', type = str, action = 'store' )

    args = parser.parse_args()


    fd = serial.Serial( port = args.uart, baudrate = 9600, bytesize = serial.EIGHTBITS, parity = serial.PARITY_NONE, stopbits = serial.STOPBITS_ONE, timeout = None )

    scope = ps2000a.PS2000a()
    scope_adc_min = scope.getMinValue()
    scope_adc_max = scope.getMaxValue()

    scope.setChannel( channel = 'A', enabled = True, coupling = 'DC', VRange =   5.0E-0 )
    scope_range_chan_a =   5.0e-0
    scope.setChannel( channel = 'B', enabled = True, coupling = 'DC', VRange = 500.0E-3 )
    scope_range_chan_b = 500.0e-3

    ( _, samples, samples_max ) = scope.setSamplingInterval( 4.0E-9, 0.2E-3 )

    scope.setSimpleTrigger( 'A', threshold_V = 2.0E-0, direction = 'Rising', timeout_ms = 0 )

    M = numpy.zeros( ( numberOfTraces, 16 ), dtype = numpy.uint8 )
    C = numpy.zeros( ( numberOfTraces, 16 ), dtype = numpy.uint8 )
    T = numpy.zeros( ( numberOfTraces,  samples ), dtype = numpy.int16 )

    time.sleep( 1 )

    print("starting to get traces")
    for traceNumber in range(numberOfTraces):
        print("tracenumber: "+ str(traceNumber))
        message = ""
        for keyByteIndex in range(16):
            keyByte = random.randint(0,255)
            M[traceNumber, keyByteIndex] = keyByte
            byteString = str(hex(keyByte)[2:])
            if(len(byteString) == 1):
                byteString = "0" + byteString
            message = message + byteString
        scope.runBlock()
        uartMessage = "10:" + message
        print("writing message")
        print(uartMessage)
        board_wrln(fd, "01:01")
        board_wrln(fd, uartMessage)
        board_wrln(fd, "00:")
        print("reading message")
        cipher = board_rdln(fd)
        cipherString = str(cipher[3:])

        print("getting trace data")
        while ( not scope.isReady() ) : time.sleep( 1 )
        ( A, _, _ ) = scope.getDataRaw( channel = 'A', numSamples = samples, downSampleMode = PS2000A_RATIO_MODE_NONE )
        ( B, _, _ ) = scope.getDataRaw( channel = 'B', numSamples = samples, downSampleMode = PS2000A_RATIO_MODE_NONE )
        scope.stop()

        if(len(cipherString) != 32):
            print("error: cipher string not correct length")
        for i in range(16):
            C[traceNumber, i] = int(cipherString[i*2:(2*i)+2], 16)

        plotB = []
        for i in range(samples):
            if(traceNumber == 1):
                plotB.append(B[i])
            T[traceNumber, i] = B[i]

        if(traceNumber == 1):
            xaxis = numpy.linspace(0, samples, samples)
            plt.plot(xaxis, plotB, )
            plt.savefig('B.png')

    traces_st("saved.dat", numberOfTraces, samples, M, C, T)
    fd.close()
    scope.close()
    return numberOfTraces, samples, M, C, T



## Attack implementation, as invoked from main after checking command line
## arguments.
##
## \param[in] argc number of command line arguments
## \param[in] argv           command line arguments

def attack( argc, argv):
    print("attacking")
    print("calculating hamming weights")
    calculateHammingWeights()
    numberOfTraces = -1
    noOfSamplesInTrace = -1
    M = []
    C = []
    T = []
    if(argc == 2):
        print("getting trace data")
        numberOfTraces, noOfSamplesInTrace, M, C, T = traces_ld(argv[1])
        print("got trace data")
    else:
        print("getting own trace data")
        numberOfTraces, noOfSamplesInTrace, M, C, T = acquireTraces()

    print(M.shape)
    print(C.shape)
    print(T.shape)
    print(numberOfTraces)
    print(noOfSamplesInTrace)
    noOfSamplesUsed = 100
    finalKey = []
    startingTraceValue = 0
    windowSize = 8000
    for keyByte in range (16):
        print("keybyte: ", hex(keyByte))
        correlationTable = numpy.zeros((windowSize, 256))
        plotKeyTries = []
        maxCorrelation = 0
        bestKey = -1
        timeFound = -1
        plotcorrelations = []
        for keyHypothesis in range(256):
            thismaxcorrelation = 0
            hypoConsumptions = []
            for sampleNumber in range(noOfSamplesUsed):
                plaintextByte = M[sampleNumber, keyByte]
                hypotheticalPowerConsumption = hammingWeightConsumption(plaintextByte, keyHypothesis)
                hypoConsumptions.append(hypotheticalPowerConsumption)
            for timeRecording in range(startingTraceValue, (windowSize+startingTraceValue)):
                correlation = abs(numpy.corrcoef(T[:noOfSamplesUsed, timeRecording], hypoConsumptions)[1,0])
                if (correlation > maxCorrelation):
                    maxCorrelation = correlation
                    bestKey = keyHypothesis
                    timeFound = timeRecording
                if(correlation > thismaxcorrelation):
                    thismaxcorrelation = correlation
                correlationTable[timeRecording-startingTraceValue, keyHypothesis] = correlation
            if(keyByte == 0):
                plotcorrelations.append(thismaxcorrelation)
            plotKeyTries.append(thismaxcorrelation)
        startingTraceValue = timeFound
        windowSize = 300
        finalKey.append(bestKey)
        #print("final key:", hex(finalKey))
        #print("best key", hex(bestKey))
        #print("time found: ", timeFound)
    xaxis = numpy.linspace(0, 256, 256)
    plt.plot(xaxis, plotcorrelations, )

    plt.savefig('correlationsS.png')
    print(finalKey)

    checkKey(finalKey, M[0,:], C[0,:])



if ( __name__ == '__main__' ) :
  attack( len( sys.argv ), sys.argv)
