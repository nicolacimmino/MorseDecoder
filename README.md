The goal of this project is to develop a device able to listen for audio containing morse code audio and decode it to text to serial port.

The hardware is a very simple board I used in many other my DSP projects, just an Arduino Nano with a microphone and a pre-amp.

![Proto](documentation/proto.png)

The current code expects audio at 700Hz and a morse speed around 13 WPM.

Morse detection
=============

After sampling the first thing the software does is to apply a Goetzel filter to detect the level of the 700Hz component of the captured audio. This is much more efficient than an FFT and, hence, allows to get estimates faster, so more often.  Once the level is detected a simple state machine is employed to keep track of the current status at each iteration. Depending on the level being above or below a preset threshold the application estomates dots, dashes, inter-element, inter-letter and inter-word stages of the signal and proceeds to decode those to ASCII chars. The alghorithm employed to translate the sequence of dots and dashes to ASCII is decribed in the next section.

Things that could be enhanched:

* Automatically adjust threshold
* Detect dot length to adjust to any speed

Morse to ASCII
============

For the conversion from morse (sequences of dots and dashes) to ASCII I make use of an alghotithm that I have not seen published anywhere so far. The alghorithm performs fundamentally a binary search inside a precalculated string taking either branch every time according to the current symbol being a dot or a dash. The pre calculated string has been prepared by running through the algorithm the morse code of each letters and seeing where is caused it to land in the string.

There are two indexes that point, initially, to the begnning and to the end of the string respectively. At every new element the function is called and, if the element is a dot the starting pointer is incremented by one and the end ponter is moved just before the half. If the element is a dash the start pointer is moved just after the half of the current range. In this way every sequence of dashes and dots will land in a different position of the string that will contain the ASCII representation of that sequence.

The current string contains 65 symbols that guarantee no clashing if all A-Z and 0-9 digits are used. If more symbols are to be added extra space should be made in the lookup string to avoid clashing. The size can be empirically determined by ensuring, when creating the string, that no char is assigned to a position that already contains a char, if that happens the string needs to be lengthened until no collisions are found.

Note that with this method invalid strings of elements cannot be detected as you will end up landing anyway on a char in the string and, in some cases, this might be a valid char even if represented by a different sequence.
