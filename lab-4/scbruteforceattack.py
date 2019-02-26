# Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
#
# Use of this source code is restricted per the CC BY-NC-ND license, a copy of
# which can be found via http://creativecommons.org (and should be included as
# LICENSE.txt within the associated archive or repository).



#side channel attack impleneted using returned t, combined with brute force

import sys, subprocess, time


def interact( G ) :
  # send      G      to   attack target

  target_in.write( '%s\n' % ( G ) ) ; target_in.flush()

  # receive ( t, r ) from attack target

  t = int( target_out.readline().strip() )
  r = int( target_out.readline().strip() )

  return ( t, r )

def attack() :

    length = 0
    for trylength in range(1,9):
        G = 'a' * trylength

        ( t, r ) = interact( G )

        print 'G = %s' % ( G )
        print 't = %d' % ( t )
        print 'r = %d' % ( r )
        print ''

        if(r == 1):
            return

        if(t > 0):
            length = trylength
            break

    G = 'a' * length

    for characternumber in range(length):
        for charactertry in range(26):
            G = G[:characternumber] + chr(97 + charactertry) + G[(characternumber+1):]
            ( t, r ) = interact( G )

            print 'G = %s' % ( G )
            print 't = %d' % ( t )
            print 'r = %d' % ( r )
            print ''

            if(r == 1):
                return


            if(t > characternumber+1):
                break

            time.sleep(0.1)



if ( __name__ == '__main__' ) :
  # produce a sub-process representing the attack target

  target = subprocess.Popen( args   = sys.argv[ 1 ],
                             stdout = subprocess.PIPE,
                             stdin  = subprocess.PIPE )

  # construct handles to attack target standard input and output

  target_out = target.stdout
  target_in  = target.stdin

  # execute a function representing the attacker

  attack()
