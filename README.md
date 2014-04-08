ffmpeg_matlab
=============

ffmpeg interface so one can call efficiently encoding/decoding/recoding methods from matlab. It is mainly for mex files, but a non-mex script is also provided(calling ffmpeg commad line (cli)/script))

Status: works audio encoding for AMRWB!
I did not merge the matlab section yet, for now tested with only as a c app.
I work with mingw64 (win764), gnumex, ffmpeg (headers and libs) from http://ffmpeg.zeranoe.com/builds/.
Target is to have it compatible with matlab 2010+ 64bit.

For now, only encoding is done. Tested with AMRWB.

TODO:
-> make it mex file (simply add the header and other similar details from a demo mex file). Compile from matlab with gnumex+mingw.
-> create decoding methods
-> create recoding/transcoding methods
-> use pointers to/from matlab for increased performance
-> find solution for VBR codecs to allocate mem properly.

HOW TO TEST Results:
Run ffmpeg with RAW file as input (WAV without first 44 bytes of header). The result should be identical to what this app returns.
./ffmpeg.exe -loglevel debug -y -vn -f s16le -ac 1 -ar 16000 -acodec pcm_s16le -sample_fmt s16 -i a1_16khz.raw -ac 1 -ab 23850 -ar 16000 -f amr -acodec libvo_amrwbenc -sample_fmt s16 -aframes 150 a1_aframes150.awb
