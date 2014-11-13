The goal of this project is to develop a device able to listen for audio containing morse code audio and decode it to text to serial port.

The hardware is a very simple board I used in many other my DSP projects, just an Arduino Nano with a microphone and a pre-amp.

![Proto](documentation/proto.png)

The current code expects audio at 700Hz and a morse speed around 13 WPM. You can see a video of the device in action at: https://www.youtube.com/watch?v=iYScOh34aIk

Morse detection
=============

After sampling the first thing the software does is to apply a Goetzel filter to detect the level of the 700Hz component of the captured audio. This is much more efficient than an FFT and, hence, allows to get estimates faster, so more often.  Once the level is detected a simple state machine is employed to keep track of the current status at each iteration. Depending on the level being above or below a preset threshold the application estomates dots, dashes, inter-element, inter-letter and inter-word stages of the signal and proceeds to decode those to ASCII chars. The alghorithm employed to translate the sequence of dots and dashes to ASCII is decribed in the next section.

Things that could be enhanched:

* Automatically adjust threshold
* Detect dot length to adjust to any speed

Morse to ASCII
============

For the conversion from morse (sequences of dots and dashes) to ASCII I make use of an alghotithm that I have not seen published anywhere so far. The alghorithm performs fundamentally a binary search inside a precalculated string taking either branch every time according to the current symbol being a dot or a dash. The pre calculated string has been prepared by running through the algorithm the morse code of each letters and seeing where is caused it to land in the string.

The algorithm can be decribe as follows. Have an index inside the lookup string inizialied to zero. Have an initial dash jump size of 64. At every received element (dot or dash) halve the initial dash jump and then increase by 1 the index inside the lookup string if a dot was received and by dash jump size if a dash was received. Repeat until a letter separator is reached, at that point the index inside the lookup string will point to the ASCII corresponding to the decoded morse.

  index=0
  dash_jump=64
  for each received element e
    dash_jump=dash_jump/2
    index = index + (e=='.')?1:dash_jump
  endfor
  ascii = lookupstring[index]
  
The current string contains 64 symbols that guarantees no clashing if all A-Z and 0-9 digits are used. If more symbols are to be added extra space should be made in the lookup string to avoid clashing. The size can be empirically determined by ensuring, when creating the string, that no char is assigned to a position that already contains a char, if that happens the string needs to be lengthened until no collisions are found.

Let's see an example step by step. Let's assume we receive a ".-", that is an "A". At the beginning the status is as in the following picture:

![Proto](documentation/step1.png)

When the "." comes in we reduce the dash jump to half and we then proceed to increase the pointer in the lookup string by one, as we receied the dot.

![Proto](documentation/step2.png)

At this point the index is on the "E", which makes sense as this would be an "E" if the symbol ended here. We instead receive a "-" so we go diving the dash jump to half again, that is to 16 and, since we have received a dash, we apply that offset to the index and end on postion 17 that contains an "A", as expected.

![Proto](documentation/step3.png)

Note that with this method invalid strings of elements cannot be detected as invalid because you will end up landing anyway on a char in the string and, in some cases, this might be a valid char even if represented by a different sequence, since most of the lookup contains some symbols.
