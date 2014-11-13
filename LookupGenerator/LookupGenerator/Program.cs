// LookupGenerator implements a generator for a morse decoder lookup string.
//  Copyright (C) 2014 Nicola Cimmino
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see http://www.gnu.org/licenses/.
//
// I referred to https://github.com/jacobrosenthal/Goertzel for the Goertzel 
//  implementation.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LookupGenerator
{
    class Program
    {

        const int LOOKUP_SIZE = 64;

        static char[] lookupString = new char[LOOKUP_SIZE];

        static int currentDecoderIndex = 0;
        static int currentDashJump = LOOKUP_SIZE;
        static int morseTableASCIIIndex = 0;

        static void Main(string[] args)
        {

            // Initialize the lookup table with ".". This is not strictly
            // necessary but gives more readabilty to the final string as
            // the empty spaces become visible.
            for (int ix = 0; ix < LOOKUP_SIZE;ix++)
            {
                lookupString[ix] = '.';
            }

            // Go through each morse sequence, simulate the algorithm and see where we land.
            // If the place is empty store there the relative ASCII to build the lookup string.
            // If the place is alrady taken we have collision and user will need to expand
            // the the lookup table.
            foreach (String morseString in morseTable)
            {
                foreach (char mark in morseString.ToCharArray())
                {
                    movePointer(mark);
                }

                if (lookupString[currentDecoderIndex] == '.')
                {
                    lookupString[currentDecoderIndex] = morseTableASCII[morseTableASCIIIndex++];
                }
                else
                {
                    Console.WriteLine("Collision! Increase LOOKUP_SIZE.");
                    return;
                }
                movePointer('\0');
            }

            Console.WriteLine(lookupString);

        }

        // Applyes one step of the alogorithm
        static void movePointer(char currentMark)
        {
            currentDashJump = (int)Math.Floor(currentDashJump / 2.0f);
            if (currentMark == '.')
            {
                currentDecoderIndex++;
            }
            else if (currentMark == '-')
            {
                currentDecoderIndex += currentDashJump;
            }
            else if (currentMark == '\0')
            {
                currentDecoderIndex = 0;
                currentDashJump = 65;
                return;
            }
        }

        // Equivalent ASCII of each of the entries in morseTable
        static string morseTableASCII = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

        // Morse symbols represented with . and -
        static String[] morseTable =
        {
            ".-",
            "-...",
            "-.-.",
            "-..",
            ".",
            "..-.",
            "--.",
            "....",
            "..",
            ".---",
            "-.-",
            ".-..",
            "--",
            "-.",
            "---",
            ".--.",
            "--.-",
            ".-.",
            "...",
            "-",
            "..-",
            "...-",
            ".--",
            "-..-",
            "-.--",
            "--..",
            "-----",
            ".----",
            "..---",
            "...--",
            "....-",
            ".....",
            "-....",
            "--...",
            "---..",
            "----."
        };
    }
}
