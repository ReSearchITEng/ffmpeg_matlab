#!/bin/bash 
# -x
#AMRWB 16KHz
#./ffmpeg.exe -loglevel debug -y -vn -f s16le -ac 1 -ar 16000 -acodec pcm_s16le -sample_fmt s16 -i a1_16khz.raw -ac 1 -ab 23850 -ar 16000 -f amr -acodec libvo_amrwbenc -sample_fmt s16 -aframes 150 a1_aframes150.awb

function InitLinux(){

	if [[ `uname` == "Linux" ]]; then
		export FFHOME="/cygdrive/c/_me/PhD2014_ffmpeg/code/"
		export FFSUBDIR="/Code/ffmpeg.old/"
		export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${FFSUBDIR}
	else
		export FFHOME="/cygdrive/c/_me/PhD2014_ffmpeg/code/"  #Windows
		export FFSUBDIR="./"  #Windows
	fi

	cd ${FFHOME}

}

# * * * * * * * * * * * * * * * * * * * * * * * *
function InitFFMPEGvars(){

	FFMPEG_AWB_BITRATE=23850 #23850 #AWB:(16KHz) 6.60k, 8.85k, 12.65k, 14.25k, 15.85k, #>18.25k, 19850, 23.05k, 23.85k <#
	#FFMPEG_AWB_BITRATE=15850 #23850 #AWB:(16KHz) 6.60k, 8.85k, 12.65k, 14.25k, 15.85k, #>18.25k, 19850, 23.05k, 23.85k <#
	FFDEBUG="-debug_ts -loglevel debug"
	FFDEBUG=" "
	cd ${FFHOME}
	FFMPEG_COMMON=" -y -vn ${FFDEBUG} "
	FFMPEG_COMMON_OUT=" -ac 1 "
	WAV2AMR_IN="tcp://127.0.0.1:${PORT_SOCAT_PUSH_WAV_TO_FFMPEG}"
	FWAV2AMR_IN=" ${1} "
	#WAV2AMR_IN="./n1_16khz.wav"
	AMRWB_ENC_IN_OPTS=" -f s16le -ac 1 -ar 16000 -acodec pcm_s16le "
	FAMRWB_ENC_IN_OPTS=" "
	AMRWB_ENC_OUT_OPTS=" ${FFMPEG_COMMON_OUT} -ab ${FFMPEG_AWB_BITRATE} -ar 16000 -f amr -acodec libvo_amrwbenc "
	AMRWB_ENC_OUT="tcp://127.0.0.1:${PORT_GIVE_AMRWB}?listen"
	FAMRWB_ENC_OUT=" ${1}.awb "
	AMRWB_ENC_ALL_OPTS=" ${FFMPEG_COMMON} ${AMRWB_ENC_IN_OPTS} -i ${WAV2AMR_IN} ${AMRWB_ENC_OUT_OPTS} ${AMRWB_ENC_OUT} "
	FAMRWB_ENC_ALL_OPTS=" ${FFMPEG_COMMON} ${FAMRWB_ENC_IN_OPTS} -i ${FWAV2AMR_IN} ${AMRWB_ENC_OUT_OPTS} ${FAMRWB_ENC_OUT} "

	cd ${FFHOME}
	AMR2WAV_IN_wSOCAT="tcp://127.0.0.1:${PORT_GIVE_AMRWB_wSOCAT}" #Similar to AMRWB_OUT, without the listen in the end
	AMR2WAV_IN="tcp://127.0.0.1:${PORT_GIVE_AMRWB}" #Similar to AMRWB_OUT, without the listen in the end
	FAMR2WAV_IN=" ${1} " 
	AMRWB_DEC_IN_OPTS=" -ab ${FFMPEG_AWB_BITRATE} -f amr -acodec libopencore_amrwb" #-acodec amrwb face probleme!
	AMRWB_DEC_OUT_OPTS=" ${FFMPEG_COMMON_OUT} -f wav -ar 16000 -acodec pcm_s16le "
	AMRWB_DEC_OUT="tcp://127.0.0.1:${PORT_GIVE_WAV_toSOCAT}?listen"
	AMRWB_DEC_OUT="tcp://127.0.0.1:${PORT_GIVE_WAV}?listen"
	FAMRWB_DEC_OUT=" ${1}.wav "
	AMRWB_DEC_ALL_OPTS=" ${FFMPEG_COMMON} ${AMRWB_DEC_IN_OPTS} -i ${AMR2WAV_IN} ${AMRWB_DEC_OUT_OPTS} ${AMRWB_DEC_OUT} "
	FAMRWB_DEC_ALL_OPTS=" ${FFMPEG_COMMON} ${AMRWB_DEC_IN_OPTS} -i ${FAMR2WAV_IN} ${AMRWB_DEC_OUT_OPTS} ${FAMRWB_DEC_OUT} "
	AMRWB_DEC_ALL_OPTS_wSOCAT=" ${FFMPEG_COMMON} ${AMRWB_DEC_IN_OPTS} -i ${AMR2WAV_IN_wSOCAT} ${AMRWB_DEC_OUT_OPTS} ${AMRWB_DEC_OUT_wSOCAT} "

}

function fcleanup(){ 
	rm -f ./wav_*.awb*
	ls -lart ./wav_*.awb*
	
}

function famrwbenc(){ 
	echo "${FAMRWB_ENC_ALL_OPTS} "
	${FFSUBDIR}ffmpeg ${FAMRWB_ENC_ALL_OPTS}
	echo "${FAMRWB_ENC_ALL_OPTS} "
}

function c2full(){ 
	if [ $# -lt 2 ] ; then
		echo "Usage: c2full <BitRate> <InputAudioFile> E.g.: c2full 1400 Loituma.mp3"
		return
	fi
	BitRate=${1}
	InputAudio=${2}
	${FFSUBDIR}ffmpeg ${FFMPEG_COMMON} -i ${InputAudio} -ar 8000 ${FFMPEG_COMMON_OUT} ${InputAudio}_8Khz.wav
	c2w2w ${BitRate} ${InputAudio}_8Khz.wav

	
}

function c22wav(){ 
	if [ $# -lt 2 ] ; then
		echo "Usage: c22wav <BitRate> <file.c2>  E.g.: c22wav 1200 file.c2"
		return
	fi
	set -vx
	BitRate=${1}
	InputC2=${2}
	${FFSUBDIR}c2dec.exe ${BitRate} ${InputC2} ${InputC2}_tmpdecoded.raw
	${FFSUBDIR}raw2wav.sh ${InputC2}_tmpdecoded.raw ${InputC2}.wav
	set +vx
	
}


function c2w2w(){ 
	if [ $# -lt 2 ] ; then
		echo "Usage: c2w2w <BitRate> <8KhzInputWav> E.g.: c2w2w 1400 a1_8khz.wav"
		return
	fi
	BitRate=${1}
	InputWav=${2}
	${FFSUBDIR}wav2raw.sh ${InputWav} ${InputWav}.raw
	${FFSUBDIR}c2enc.exe ${BitRate} ${InputWav}.raw ${InputWav}_${BitRate}.c2
	${FFSUBDIR}c2dec.exe ${BitRate} ${InputWav}_${BitRate}.c2 ${InputWav}_${BitRate}.raw
	${FFSUBDIR}raw2wav.sh ${InputWav}_${BitRate}.raw ${InputWav}_${BitRate}.wav
	
}

function famrwbdec(){ 
	echo "${FAMRWB_DEC_ALL_OPTS} "
	${FFSUBDIR}ffmpeg -y -vn ${FAMRWB_DEC_ALL_OPTS}
	echo "${FAMRWB_DEC_ALL_OPTS} "
}

function famrwbplayamr(){ echo "${FFSUBDIR}ffplay ${FAMRWB_ENC_OUT}"; ${FFSUBDIR}ffplay ${FAMRWB_ENC_OUT}
}

function famrwbplaywav(){ echo "${FFSUBDIR}ffplay ${FAMRWB_DEC_OUT}"; ${FFSUBDIR}ffplay ${FAMRWB_DEC_OUT}
}

function catwav(){
export PREF=${1}
#http://www.boutell.com/scripts/catwav.html
for X in `ls ${PREF}*f[1-8].wav ${PREF}*m[1-8].wav`; do
sox ${X} -r 8000 -c 1 -s  ${X}.raw
done
echo
cat `ls *f[1-8].wav.raw *m[1-8].wav.raw` >FULL_8kHz_${PREF}.raw
set -vx
sox -r 8000 -c 1 -s -2 FULL_8kHz_${PREF}.raw FULL_8kHz_${PREF}.wav
set +vx
rm *f[1-8].wav.raw *m[1-8].wav.raw
ls -lart FULL_8kHz_${PREF}.wav
}

function catwavFor(){ #NOT USED
	for X in A_eng B_eng Ch Fr Ger Ru Sp; do
catwav ${X}
	done
	ls -lart FULL_8kHz_*.wav
}

function pesqAll(){
export FFSUBDIR=./
set -vx
echo `date` >> PESQ_Results.txt
for Y in A_eng B_eng Ch Fr Ger Ru Sp;do
catwav ${Y}
for BitRateC2 in 1200 1400 1500 2500;do
c2w2w ${BitRateC2} FULL_8kHz_${Y}.wav
echo "./pesq.exe +8000 FULL_8kHz_${Y}.wav FULL_8kHz_${Y}.wav_${BitRateC2}.wav" >> PESQ_Results.txt
./pesq.exe +8000 FULL_8kHz_${Y}.wav FULL_8kHz_${Y}.wav_${BitRateC2}.wav >> PESQ_Results.txt
done
done
echo `date` >> PESQ_Results.txt
}


# * * * * * * * * * * * * * * * * * * * * * * * * 
##################  		MAIN 	    #####
# * * * * * * * * * * * * * * * * * * * * * * * * 

if [ $# -eq 0 ] ; then
	echo "Usage: $0 <stepName>,  stepName"
	return 1;
fi

FunMain="${1}"; 
shift
InitLinux

while [ $# -gt 0 ] ; do

Fun=${FunMain}
	echo "Input file is: $1 "
	InitFFMPEGvars "${1}"

	case ${Fun} in
		fcleanup)  ${Fun} | tee ${Fun}.log  ;;
		
		famrwbenc) ${Fun} | tee ${Fun}.log ; FOUT=${FAMRWB_ENC_OUT};; # &
		enc) Fun=famrwbenc; ${Fun} | tee ${Fun}.log ; FOUT=${FAMRWB_ENC_OUT};; # &
		
		famrwbdec) ${Fun} |tee ${Fun}.log ; FOUT=${FAMRWB_DEC_OUT};; # &
		dec) Fun=famrwbdec; ${Fun} |tee ${Fun}.log ; FOUT=${FAMRWB_DEC_OUT};; # &
		
		ed) FOUT=${1}.awb.wav; rm -f $FOUT ${1}.awb; famrwbenc | tee ${Fun}.log; InitFFMPEGvars "${1}.awb"; famrwbdec | tee -a ${Fun}.log;; # &
		
		famrwbplaywav) ${Fun} >${Fun}.log ;; # &
		
		c2w2w) ${Fun} ${*} ; shift;; #should have 2 params, for 2nd one we shift later
		
		c2full) ${Fun} ${*} ; shift;; #should have 2 params, for 2nd one we shift later
		
		c22wav) ${Fun} ${*} ; shift;; #should have 2 params, for 2nd one we shift later
		
		pesqAll) ${Fun} ${*}; shift;; #Computes PESQ while in the folder of samples, along with all required bins: pesq, sox, some shs;
		
		#dd) dd bs=1 count=684c if=686.wav of=684.wav
		#    dd bs=1 count=684c skip=44 if=686.wav of=640.raw
		#//./ffmpeg.exe -loglevel debug -y -vn -f s16le -ac 1 -ar 16000 -acodec pcm_s16le -sample_fmt s16 -i 640.raw -ac 1 -ab 23850 -ar 16000 -f amr -acodec libvo_amrwbenc -sample_fmt s16 640bytes.awb
		#//./ffmpeg.exe -loglevel debug -y -vn -f s16le -ac 1 -ar 16000 -acodec pcm_s16le -sample_fmt s16 -i a1_16khz.raw -ac 1 -ab 23850 -ar 16000 -f amr -acodec libvo_amrwbenc -sample_fmt s16 -aframes 150 a1_aframes150.awb

		
		*) echo "Unknown param; exiting" ;; #exit 1 ;;
	esac
	
	ls -lart ${1}*
	
	echo "${FFSUBDIR}ffplay ${FOUT}"
	echo "mp ${FOUT}"
	
	shift

done


