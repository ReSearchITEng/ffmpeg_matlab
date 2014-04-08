ffmpeg_matlab
=============

ffmpeg interface so one can call efficiently encoding/decoding/recoding methods from matlab. It is mainly for mex files, but a non-mex script is also provided(calling ffmpeg commad line (cli)/script))

Works audio encoding for AMRWB!
I did not merge the matlab section yet, for now tested with only as a c app.
I work with mingw64 (win764), gnumex, ffmpeg (headers and libs) from http://ffmpeg.zeranoe.com/builds/.
Target is to have it compatible with matlab 2010+ 64bit.

For now, only encoding is done. Tested with AMRWB.
