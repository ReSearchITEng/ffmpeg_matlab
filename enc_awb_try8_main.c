#include "utils.h"

//uint8_t *main_encode(int16_t *audio_bufin_samples, enum CodecID codec_id);

//uint8_t *encode_amr_nb(int16_t *audio_bufin_samples);

//uint8_t *encode_amr_wb(int16_t *audio_bufin_samples);

int main(int argc, char **argv){

    /* parameters expected from matlab as arguments, ideally as pointers: */
//	Buffer_in_int16_t *ST_buffer_in;

  	enum AVCodecID nFromMatlab_codec_id;//as defined in avcodec.h: http://ffmpeg.org/doxygen/trunk/libavcodec_2avcodec_8h_source.html
	nFromMatlab_codec_id=AV_CODEC_ID_AMR_WB;//http://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ggaadca229ad2c20e060a14fec08a5cc7cea3d4c051c8b18a3c18750676a308c10be

    int nFromMatlab_Codec_bit_rate=23850;//WB: 23850,23050,19850,....,8850,6600  //Output bit_rate
                                         //NB: 12200,10200,7950,....5150,4750,1800(SID) http://en.wikipedia.org/wiki/Adaptive_Multi-Rate_audio_codec
    enum AVSampleFormat nFromMatlab_Codec_AV_SAMPLE_FMT;
    nFromMatlab_Codec_AV_SAMPLE_FMT=AV_SAMPLE_FMT_S16;//for amr is ignored, we hardcode to AV_SAMPLE_FMT_S16.
                                                      //as defined in samplefmt.h: http://ffmpeg.org/doxygen/trunk/samplefmt_8h_source.html
    int nFromMatlab_Codec_channels=1;           //for amr is ignored, we hardcode to 1 channel.


    int16_t *pnFromMatlab_audio_bufin_samples=NULL;//for demo, this will be init later, from a wav file...
    int nFromMatlab_audio_bufin_byte_size=-1;//for demo, this will be init later, based on frames to be read from file.

    int log_level=AV_LOG_VERBOSE;

    /* END Parameters expected from Matlab */

    av_log_set_level(log_level);
    av_log(NULL,AV_LOG_WARNING,"\n Starting Our code... \n");

	uint8_t *audio_outbuf;
	AVCodecContext *c;

    c = prepare_codec_context(nFromMatlab_codec_id,ENCODE,&nFromMatlab_Codec_bit_rate);

	///DEMO (FOR AMRWB):
	int number_of_frames=1;//Number of frames to be read from file.; 150*320=48000; 8kHz=> 6 sec.

	char *filename_wav=argv[1];
	filename_wav="c:\\_me\\PhD2014_ffmpeg\\mex\\amrwb2_retry\\a1_16khz.wav"; number_of_frames=150;
	//filename_wav="c:\\_me\\PhD2014_ffmpeg\\mex\\amrwb2_retry\\684.wav";number_of_frames=1;

	nFromMatlab_audio_bufin_byte_size=number_of_frames*c->frame_size*av_get_bytes_per_sample(c->sample_fmt);
	pnFromMatlab_audio_bufin_samples=generate_audio_frame_from_file(filename_wav,&nFromMatlab_audio_bufin_byte_size);
	//bytes_to_be_read_from_file=(*ST_buffer_in).size_of_buff;
	//mergeOK: write_raw_to_file((*ST_buffer_in).audio_bufin_samples, "before_amr_wb", 1, bytes_to_be_read_from_file);

	//audio_bufin_samples=malloc(bytes_to_be_read_from_file);
	//audio_bufin_samples=memcpy(audio_bufin_samples,(*ST_buffer_in).audio_bufin_samples, bytes_to_be_read_from_file);

	///END DEMO (FOR AMRWB)


	/// Performing magic...
	audio_outbuf=main_encode2(c,pnFromMatlab_audio_bufin_samples,nFromMatlab_audio_bufin_byte_size);

    if (av_log_get_level()>=AV_LOG_DEBUG)
        dump_codec_context(c); //for debug

    //If matlab pointers, don't free them! :)
    close_free(c, pnFromMatlab_audio_bufin_samples, audio_outbuf);

    exit_program(0);

return  0;

} 
