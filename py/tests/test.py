#!/usr/bin/env python3
'''
Created on 16 Nov 2019

@author: julianporter
'''
import rubberband 
import soundfile
from sys import argv
import numpy

nFrames = None
rate = None
bitrate = None
stream = None

mode="list" if len(argv)<2 else argv[1]



data, rate = soundfile.read('slugs.wav',dtype='int16')
bitrate=rate*16
if mode=='list':
    stream=[float(d) for d in data]
elif mode=='buffer':
    stream=data.tobytes('C')
else:
    stream=data
        
ma = max(data)
mi = min(data)
nFrames=len(data)


oldDuration=nFrames/rate

newDuration=6
ratio=newDuration/oldDuration
print(f'Ratio is {ratio}, len = {nFrames}, max = {ma}, min = {mi}')
print(f'Raw input type is : {type(stream)}')
print(stream[:6])

out=rubberband.stretch(stream,format=rubberband.int16,rate=rate,ratio=ratio,crispness=5,formants=False,precise=True)
print(f'Raw output type is : {type(out)}')

if mode=='buffer':
    out=numpy.frombuffer(out,dtype=numpy.int16)
elif mode=='list':
    out=[numpy.int16(x) for x in out]
print(out[:6])
soundfile.write("slugs_st.wav",out,rate,'PCM_16')


  



    
    
    
    
