# transcriber
Simple Wav Audio Transcriber

This program is a simple autocorrelation based audio transcriber that takes a wav file as an input and produces a lilypond file as an output. The output file can be engraved by the Lilypond program to produce sheet music. It is intended for monophonic low noise audio. It does not produce polyphonic output. Polyphonic inputs will produce outputs corresponding to the loudest note present. This program was tested against dry line-in recorded guitar tracks.  

This program was developed on ubuntu for use from the terminal and uses libsndfile to process the wav file. Libsndfile is required to compile this program. 

To Compile using gcc, type 

gcc transcriptor.c -lm -lsndfile 

The program takes two arguments, wav file name and tempo. Tempo is optional, if not specified 100bpm is assumed. 

To run the program, type

./a.out wavfilesname.wav 110

When run, the program will print to the terminal sample rate information from the wav file and then will print off every detected frequency as it processes the wav file. Upon succesfull completetion, it will create the file <wavfilesname.wav>.ly using a filtered list of detected frequencies. The output file can be converted into sheet music by running the program lilypond from the terminal.

lilypond [filename]

