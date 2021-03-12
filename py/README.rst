README for rubberband Python3 Module
====================================

Introduction
------------

**rubberband** is a simple Python3 wrapper around the well-known librubberband_ sound stretching / pitch-shifting library.  Unlike existing Python wrappers (e.g. pyrubberband_) this is a true native extension.

The initial release provides a single function that will stretch a mono audio stream by multiplying its duration by a provided factor.  Future versions may include pitch shifting, and more complex processing based on data maps.

Installation
------------

The module is available only for MacOS and Linux.  The code may compile on Windows, but it has not been tested. Dependencies are:

 - Python 3 (preferably 3.6 or greater)
 - librubberband_ (> 1.8)
 - libsndfile_ (> 1.0)

Assuming these dependencies are met, then installation is simplicity itself::

    pip3 install rubberband


The install script does check for the required libraries, and will complain vociferously if they cannot be located.  Information on obtaining them is available from the links above.

API
---

Audio stream formatting
~~~~~~~~~~~~~~~~~~~~~~~

The module exposes a single function **rubberband.stretch** which applies the librubberband_ algorithm to a mono audio stream, encoded in one of the following formats, each of which has a corresponding constant, as set out in the third column:

.. table::

   =======  ===========================  ================
   Format   Description                  Constant
   =======  ===========================  ================
   PCM_U8   unsigned 8-bit               rubberband.uint8
   PCM_S8   signed 8-bit                 rubberband.int8
   PCM_16   signed 16-bit                rubberband.int16
   PCM_32   signed 32-bit                rubberband.int32
   FLOAT    *normalised* 32-bit float    rubberband.float32
   =======  ===========================  ================

Note that floating point data is assumed to be normalised, so all samples lie in the range [-1,1).

Audio data can be passed to **rubberband.stretch** in any of three ways:

  **Typed array**
    A 1-dimensional NUMPY_ typed **array** object, whose **dtype** is one of ``numpy.uint8``,
    ``numpy.int8``, ``numpy.int16``, ``numpy.int32`` or ``numpy.float32``.  The type of the audio
    data is deduced from this, using the strangely convenient fact that if **T** is one of ``uint8``, 
    ``int8``, ``int16``, ``int32``, ``float32`` then

    .. code:: python

      numpy.dtype(numpy.T).num == rubberband.T 

  **List**
    A simple Python **list**, all of whose elements are of a type implicitly convertible to **float**.  
    In this case, the audio format cannot be deduced, so it must be specified using the *format* argument
    to **rubberband.stretch** (see below).

  **Raw bytestream**
    A Python **bytes** object, whose content is the raw PCM byte stream (note: audio file metadata, 
    e.g. WAV file headers, must be stripped, so only PCM data remains).  Again, in this case, the audio
    format cannot be deduced, so it must be specified using the *format* argument
    to **rubberband.stretch** (see below).

In all cases, the output from **rubberband.stretch** has the same PCM format, and is stored in the same
kind of object, as the input.  So, for example, given a **bytes** object representing a **PCM_16** 
audio channel, **rubberband.stretch** returns **bytes** object representing a **PCM_16** 
stretched audio channel.

Method signature
~~~~~~~~~~~~~~~~


**rubberband.stretch** (*input*, *format* = **rubberband.float32**, *rate* = **48000** , *ratio* = **1** , *crispness* = **5** , *formants* = **False**, *precise* = **True** )

Arguments   

      *input*
            The input is assumed to represent a single channel of PCM audio data, encoded with one 
            of the schemes listed above.  It can be any of the types set out above. 

      *format*
            The PCM format of the data, specified using one of the constants set out above.  This 
            value is *ignored* if *input* is a NUMPY typed array, in which case the format is deduced 
            from its **dtype**.

      *rate*
            The frame rate of the input audio stream (so bit rate divided by sample size).

      *ratio*
            The ratio of output length to input length (in seconds / number of samples).

      *crispness*
            Integer 0 - 6, default 5: measure of performance - see the `rubberband-cli documentation`_ 
            for more details.

      *formants*
            Boolean, default **False** : whether or not to preserve formants - see the 
            `rubberband-cli documentation`_ for more details.
            
      *precise*
            Boolean, default **True** : whether or not to use the precise stretching algorithm - 
            see the `rubberband-cli documentation`_ for more details.

Return value
      An object containing the stretched audio data, represented using the same PCM encoding as the
      *input*. Samples are normalised to lie in the expected range for the format. 


Example
-------

  .. code:: python

   import rubberband 
   import soundfile

   data,rate=soundfile.read('infile.wav',dtype='int16')
   bitrate=rate*16
   nFrames=len(data)
   print(f'Raw input type is : {type(data)}')

   oldDuration=nFrames/rate
   newDuration=6
   ratio=newDuration/oldDuration
  
   out=rubberband.stretch(data,rate=rate,ratio=ratio,crispness=5,formants=False,precise=True)
   soundfile.write('outfile.wav',out,rate,'PCM_16')




.. _librubberband: https://breakfastquay.com/rubberband/
.. _pyrubberband: https://pypi.org/project/pyrubberband/
.. _libsndfile: http://www.mega-nerd.com/libsndfile/
.. _`rubberband-cli documentation`: https://breakfastquay.com/rubberband/usage.txt
.. _NUMPY: https://numpy.org




