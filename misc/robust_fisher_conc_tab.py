#! /usr/bin/env python

import math


# Define the functions needed...
def c3(k): return math.log(math.cosh(k))/k
def dc3(k): return -math.log(math.cosh(k))/(k*k) + math.tanh(k)/k

def invC3(expC):
  # Returns the matching k
  # Newton iterations - designed for accuracy, not speed...
  ret = 1.0
  for i in xrange(10000):
    f = c3(ret) - expC
    df = dc3(ret)
    ret -= f/df
    
  return ret


# Calculate the table...
res = 1000
dom = map(lambda x: float(x)/float(res),xrange(res))
ran = map(lambda x: invC3(x),dom)


# Format and output it as a c style array, for lookup table purposes...
out = 'real32 medDotToK[' + str(res) + '] = {'
out += reduce(lambda x,y: str(x)+','+str(y),ran)
out = out[:-1] + '];'

print out