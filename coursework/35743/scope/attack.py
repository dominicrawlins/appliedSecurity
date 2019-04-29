# Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
#
# Use of this source code is restricted per the CC BY-NC-ND license, a copy of
# which can be found via http://creativecommons.org (and should be included as
# LICENSE.txt within the associated archive or repository).

import numpy, struct, sys, time
import matplotlib.pyplot as plt
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
    k = bytearray.fromhex(hex(finalKey)[2:])
    message = 0x0
    cipher = 0x0
    for i in range(16):
        mish = (M[i])
        message = (message << 8) | int(mish)
        cipher = (cipher << 8) | int(C[i])
    m = bytearray.fromhex(hex(message)[2:])
    c = AES.new( bytes(k) ).encrypt( bytes(m) )
    calculatedcipher = c.hex()
    realcipher = hex(cipher)[2:]
    if(calculatedcipher == realcipher):
        print("correct")
    else:
        print("incorrect")


## Attack implementation, as invoked from main after checking command line
## arguments.
##
## \param[in] argc number of command line arguments
## \param[in] argv           command line arguments

def attack( argc, argv, attackMethod):
    print("attacking")
    print("calculating hamming weights")
    calculateHammingWeights()
    print("hamming weights calculated \n\ngetting trace data")
    numberOfTraces, noOfSamplesInTrace, M, C, T = traces_ld("dantraces.dat")
    noOfSamplesUsed = 100
    finalKey = 0x0
    startingTraceValue = 3000
    windowSize = 3000
    for keyByte in range (16):
        print("keybyte: ", hex(keyByte))
        correlationTable = numpy.zeros((windowSize, 256))
        plotKeyTries = []
        maxCorrelation = 0
        bestKey = -1
        timeFound = -1
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
            plotKeyTries.append(thismaxcorrelation)
        startingTraceValue = timeFound
        windowSize = 200
        finalKey = bestKey |  (finalKey << 8)
        print("final key:", hex(finalKey))
        print("best key", hex(bestKey))
        print("time found: ", timeFound)
    print(hex(finalKey))

    checkKey(finalKey, M[0,:], C[0,:])



if ( __name__ == '__main__' ) :
  print("in main")
  #attackMethod: 0 for LSB, 1 for Hamming Weight
  attack( len( sys.argv ), sys.argv, 0)
