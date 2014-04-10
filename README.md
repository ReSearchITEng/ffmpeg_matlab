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
Run ffmpeg with RAW (.sw) file as input (WAV without first 44 bytes of header). The result should be identical to what this app returns.
./ffmpeg.exe -loglevel debug -y -vn -f s16le -ac 1 -ar 16000 -acodec pcm_s16le -sample_fmt s16 -i a1_16khz.sw -ac 1 -ab 23850 -ar 16000 -f amr -acodec libvo_amrwbenc -sample_fmt s16 -aframes 150 a1_aframes150.awb

To view/check the wav signals, use the: http://audacity.sourceforge.net/
As ffmpeg's resampling is not the best, you may want to use: http://sox.sourceforge.net/  or http://shibatch.sourceforge.net/  (as suggested by http://transcoding.wordpress.com/2011/11/16/careful-with-audio-resampling-using-ffmpeg/)
Sox is great audio tool by itself, with lots of other functionalities.

HOW TO USE:
set input&output data from matlab and run the ffmpeg_matlab(parameters).

HOW TO Get ffmpeg info:
->Codec name can be found by ./ffmpeg.exe -codecs | grep -i amr  (for example)
->Codec's accepted sample format can be found by ./ffmpeg.exe -h encoder=amr_wb  (for example), or -h decoder=amr_wb, etc.
->all possibe sample formats: ./ffmpeg.exe -sample_fmts
->./ffmpeg.exe  -formats            show available formats
->./ffmpeg.exe -h full | grep -i <keyword for what you want to know>

HOW TO COMPILE:
1. Download: mingw-builds-install.exe from: http://sourceforge.net/projects/mingwbuilds/files/mingw-builds-install/
   Using it, install: x64-4.8.1-win32-seh-rev5
  (It will take automatically from: http://qt-project.org/wiki/MinGW-64-bit ; http://mingw-w64.sourceforge.net/download.php)

OR
1'. Use TDM (different packaging for mingw): http://sourceforge.net/projects/tdm-gcc/files/TDM-GCC%20Installer/tdm-gcc-webdl.exe

2. Download gnumex: http://sourceforge.net/projects/gnumex/files/ and set it up by pointing it to the mingw location.
3. Download latest ffmpeg 64 bit hearder files, libs, dlls and optionally executables files from: ffmpeg site.  For windows download them from: http://ffmpeg.zeranoe.com/builds/.
   You will need to download 2 archives: 
   4.1 dev archive for the header files and libs. E.g. ffmpeg-<...>-win64-dev.7z . This is req. in order to compile.
   4.2 binaries for dlls and exe. E.g.  ffmpeg-<...>-win64-shared.7z . These are req. in order to run the application (Unless you want to compile your mex file static, when you will not need them).
4. Optionally: install cygwin for a great number of Linux tools compiled for windows, from: http://cygwin.com/install.html
5. Clear understanding of what you want to achieve and how things are connected to eachother.
6. Once everything is set up, compile from matlab the .c file(s) .
7. As an IDE, one may use Code::Blocks for developing the c file, and once done, add the mex.h and other matlab related sections and compile it in matlab afterwards.

Good luck!
