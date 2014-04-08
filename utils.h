//http://stackoverflow.com/questions/2410459/encode-audio-to-aac-with-libavcodec
// http://www.ietf.org/rfc/rfc3267.txt?number=3267 - AMR standards

// TEST TARGET the result should be identical to:
//./ffmpeg.exe -loglevel debug -y -vn -sample_fmt s16 -i  a1_16khz.wav -ac 1 -ab 23850 -ar 16000 -f amr -acodec libvo_amrwbenc -sample_fmt s16 a1.wav.example.awb
//./ffmpeg.exe -loglevel debug -y -vn -f s16le -ac 1 -ar 16000 -acodec pcm_s16le -sample_fmt s16 -i 640.raw -ac 1 -ab 23850 -ar 16000 -f amr -acodec libvo_amrwbenc -sample_fmt s16 640bytes.awb
//./ffmpeg.exe -loglevel debug -y -vn -f s16le -ac 1 -ar 16000 -acodec pcm_s16le -sample_fmt s16 -i a1_16khz.raw -ac 1 -ab 23850 -ar 16000 -f amr -acodec libvo_amrwbenc -sample_fmt s16 -aframes 150 a1_aframes150.awb

#include "libavformat/avformat.h"
//#define AMR_WB_TO_ENC_FRAME_SIZE 320
//320*2bytes=640 bytes each input frame!
//#define AMR_NB_TO_ENC_FRAME_SIZE 160
// AMRWB out frame size=14,32,61
//#define AMR_WB_FROM_ENC_FRAME_SIZE_23850 61
//#define AMR_NB_FROM_ENC_FRAME_SIZE_ERROR 13
//#define AMR_NB_FROM_ENC_FRAME_SIZE 32???

#include "libavformat/avio.h"
#include "libavutil/opt.h"
#include "libavutil/log.h"

#include "test_debug.h"

#define DECODE 0
#define ENCODE 1

// NEW - 2014

// nice exit, with a getch() at the end when log_level>PANIC
exit_program(int exit_value){

    if (exit_value>0)
        av_log(NULL,AV_LOG_ERROR,"There was an error. Press enter to exit now.\n");
    else
        av_log(NULL,AV_LOG_INFO, "Press Any Key to EXIT. BYE!");

    if (av_log_get_level()>AV_LOG_INFO){
        av_log(NULL,AV_LOG_WARNING, "Log level is above %d, so it goes in interactive mode, asking for key input!", AV_LOG_INFO);
        getch();
    }

    exit(exit_value);
}

static void amr_encode_fix_avctx(AVCodecContext *avctx) //like amr_decode_fix_avctx from: http://sage.math.washington.edu/home/wstein/www/home/mhampton/ffmpeg/libavcodec/libamr.c
{
    const int is_amr_wb = 1 + (avctx->codec_id == AV_CODEC_ID_AMR_WB);

    avctx->sample_fmt = AV_SAMPLE_FMT_S16;

    if (!avctx->sample_rate)
        avctx->sample_rate = 8000 * is_amr_wb;

    if (!avctx->channels)
        avctx->channels = 1; // ONLY MONO is SUPPORTED (at least for AMRWB)

    //avctx->frame_size = 160 * is_amr_wb; // We could skip this, input frame size is set automatically by encoder and we use this as a verifigcation that it was open successfully

    //if bit_rate is completely wrong, we set it here to max values:
    if (avctx->bit_rate > is_amr_wb*12200) //this is output bitrate; for input there is bits_per_raw_sample
        avctx->bit_rate = (is_amr_wb>1) ? 23850:12200;
    // @decode the bit_rate is set by decoder
	//av_dict_set(&opts,) -> e.g. enable DTX (flag -dtx 1; default is 0)
}

// main context definition
AVCodecContext *prepare_codec_context(enum AVCodecID codec_id, int encode, int *codec_bit_rate){//AVFormatContext *oc,
	AVCodecContext *c=NULL;
	AVCodec *codec;

	av_register_all();// In future optimize: avcodec_register(AVCodec*codec);  //https://stackoverflow.com/questions/5016760/av-register-all-vs-avcodec-register

   	codec = avcodec_find_encoder(codec_id);	//codec = avcodec_find_encoder_by_name("amr_wb");//codec_id);//libvo_amrwbenc
	if (!codec)
	{
		av_log(NULL,AV_LOG_ERROR, "ERROR at codec = avcodec_find_encoder(codec_id); - codec not found");
		exit_program(1);
	}

	c = avcodec_alloc_context3(codec);
    if (!c) {
        av_log(NULL,AV_LOG_ERROR, "Could not allocate audio codec context\n");
        exit_program(1);
    }

    if (encode){
        c->bit_rate=*codec_bit_rate;
        switch (codec_id){
        //if ( (codec_id==AV_CODEC_ID_AMR_WB) || (codec_id==AV_CODEC_ID_AMR_WB) )
            case AV_CODEC_ID_AMR_WB:{
                amr_encode_fix_avctx(c); //for decode is done automatically from what I saw in their decode sources, via a func named: amr_dencode_fix_avctx
                break;
            }
            case AV_CODEC_ID_AMR_NB:{
                amr_encode_fix_avctx(c); //for decode is done automatically from what I saw in their decode sources, via a func named: amr_dencode_fix_avctx
                break;
            }
            default:{
                av_log(NULL,AV_LOG_WARNING,"Non AMR coded. You are on your own risk");
                break;
            }
        }
    }

	if (avcodec_open2(c, codec, NULL) < 0) {
		av_log(NULL,AV_LOG_ERROR, "could not open codec\n");
		exit_program(1);
	}

	if (c->frame_size <= 1) {
		av_log(NULL,AV_LOG_ERROR, "c->frame_size <= 1 .Please modify code for audio_input_frame_size (usually PCM) \n");
		exit_program(1);
	}

    if (av_log_get_level()>=AV_LOG_INFO)
        dump_codec_context(c); //for debug

    // only for decoder try: to set request_sample_fmt
	return c;
}

/* deallocating mem before exit */
static void close_free(AVCodecContext *c, int16_t *audio_bufin_samples, uint8_t *audio_outbuf){

    av_log(NULL,AV_LOG_VERBOSE, "Going to run avcodec_close(c) to close codec context\n");
    avcodec_close(c);
    av_log(NULL,AV_LOG_VERBOSE, "Going to run av_free(audio_bufin_samples) to deallocate memory of audio_bufin_samples:\n");
    av_free(audio_bufin_samples);
    av_log(NULL,AV_LOG_VERBOSE, "Going to run av_free(audio_outbuf) to deallocate memory of audio_outbuf:\n");
    av_free(audio_outbuf);
    av_log(NULL,AV_LOG_VERBOSE, "Done with deallocation");

}


uint8_t *main_encode2(AVCodecContext *c, int16_t *audio_bufin_samples, const int bytes_initially_in_bufin){ //enum CodecID codec_id
//const - it cannot be modified.
//static - it will keep the value across multiple function calls, as if it would be a global variable (only thing is it cannot be seen from outside or other functions

int ret, got_output, i=0;
int bytes_left_in_bufin=bytes_initially_in_bufin;

FILE *f;
if (av_log_get_level()>=AV_LOG_DUMP_TO_FILE){
    f = fopen("main_encode2.RAW.awb", "wb");//for debug
    if (!f) {
        av_log(NULL,AV_LOG_ERROR, "main_encode2:Could not open file\n");
        exit_program(1);
    }
}

if (av_log_get_level()>=AV_LOG_DUMP_TO_FILE)
    write_raw_to_file(audio_bufin_samples, "before_amr_wb", 1,  bytes_initially_in_bufin);
/// START by Preparing the Input data. We are splitting the input buffer in small frames to feed the encoder slowly...
int bytes_from_input_bufin_already_pushed_out=0;
/* frame containing input raw audio: https://ffmpeg.org/doxygen/trunk/structAVFrame.html#details */
AVFrame *frame = av_frame_alloc();if (!frame) {    av_log(NULL,AV_LOG_ERROR, "Could not allocate audio frame (raw audio input data)\n");    exit_program(1);}
frame->nb_samples     = c->frame_size;
frame->format         = c->sample_fmt;//AV_SAMPLE_FMT_S16; both input and output in our case
frame->channel_layout = c->channel_layout;

int audio_bufin_single_frame_buffer_size = av_samples_get_buffer_size(NULL, c->channels, c->frame_size, c->sample_fmt, 0);
if (audio_bufin_single_frame_buffer_size < 0) {
    av_log(NULL,AV_LOG_ERROR, "Could not get sample buffer size\n");
    exit_program(1);
}

int16_t *audio_bufin_single_frame = av_malloc(audio_bufin_single_frame_buffer_size); /*aka samples array*/
if (!audio_bufin_single_frame) {
    av_log(NULL,AV_LOG_ERROR, "Could not allocate %d bytes for samples buffer\n",
            audio_bufin_single_frame_buffer_size);
    exit_program(1);
}

/* setup the data pointers in the AVFrame */
ret = avcodec_fill_audio_frame(frame, c->channels, c->sample_fmt,
                               (uint8_t*)audio_bufin_single_frame, audio_bufin_single_frame_buffer_size, 0);
if (ret < 0) {
    av_log(NULL,AV_LOG_ERROR, "Could not setup audio frame\n");
    exit_program(1);
}

///Preparing output buffs:
uint8_t *audio_outbuf;
int audio_outbuf_actual_encoded_size=0;
int audio_outbuf_max_size=-35000;
int audio_out_packet_size=-35000;
AVPacket pkt;
av_init_packet(&pkt);

/// I do only the 1st packet initially, only to find data and allocate perfect output buff size.
if (bytes_left_in_bufin >= audio_bufin_single_frame_buffer_size){
    pkt.data = NULL;
    pkt.size = 0;

    /* copy input buffer data to buf single frame */// memcpy(*to, *from, size_t len)
    /* TODO (me#6#): Performance: modify in such a way so there will be no need for memcpy.  */
    memcpy(audio_bufin_single_frame,&audio_bufin_samples[bytes_from_input_bufin_already_pushed_out/sizeof(audio_bufin_samples[0])],audio_bufin_single_frame_buffer_size);
    bytes_from_input_bufin_already_pushed_out+=audio_bufin_single_frame_buffer_size;
    bytes_left_in_bufin-=audio_bufin_single_frame_buffer_size;

    /* encode the samples */
    ret = avcodec_encode_audio2(c, &pkt, frame, &got_output);
    if (ret < 0)
    {
        av_log(NULL,AV_LOG_ERROR, "before alloc: Error encoding audio frame\n");
        exit_program(1);
    }

    /* now we allocate the output buffer as we now know the size of the output packet */
    /* TODO (me#4#): This does not work for VBR! It works for codecs that always return the same packet size, but not for all. */
    audio_out_packet_size=pkt.size;
    av_log(NULL,AV_LOG_INFO,"Received size of 1st package from encoder is: %d bytes\n",audio_out_packet_size);

    av_log(NULL,AV_LOG_INFO,"I estimate we have %d input frames, computed like this: bytes_initially_in_bufin(%d)/audio_bufin_single_frame_buffer_size(%d)\n", (int) ceil(bytes_initially_in_bufin/audio_bufin_single_frame_buffer_size), \
           bytes_initially_in_bufin, audio_bufin_single_frame_buffer_size);

    av_log(NULL,AV_LOG_INFO,"I suppose it's not VBR codec and I will allocate mem computed based on the above equation");

    audio_outbuf_max_size=(int) ceil(audio_out_packet_size*(bytes_initially_in_bufin/audio_bufin_single_frame_buffer_size+12));//+12 more packet size, just to be safe

    av_log(NULL,AV_LOG_INFO,"Going to allocate %d bytes for audio_outbuf (we add extra space 12 packages more, to be safe)\n", audio_outbuf_max_size);

    audio_outbuf = av_malloc(audio_outbuf_max_size);//max size
    if (!audio_outbuf) {
        av_log(NULL,AV_LOG_ERROR, "Could not allocate %d bytes for audio_outbuf \n",
                audio_outbuf_max_size);
        exit_program(1);
    }

    if (got_output) {
        if (av_log_get_level()>=AV_LOG_DUMP_TO_FILE){
            fwrite(pkt.data, 1, pkt.size, f);} //fflush(f);}
        /* TODO (me#6#): Performance: modify in such a way so there will be no need for memcpy.  */
        memcpy(&audio_outbuf[audio_outbuf_actual_encoded_size], pkt.data, pkt.size);
        audio_outbuf_actual_encoded_size+=pkt.size;
        if (audio_outbuf_actual_encoded_size>=audio_outbuf_max_size-pkt.size){
            av_log(NULL,AV_LOG_ERROR, "Error: audio_outbuf_actual_encoded_size(%d)>audio_outbuf_max_size(%d)-pkt.size(%d)\n",audio_outbuf_actual_encoded_size,audio_outbuf_max_size,pkt.size);
            exit_program(1);
        }
        av_free_packet(&pkt); //-> DONNO IF recommended, as pkt.data is pointing to our output buffer...
    }
    // av_frame_unref()  //At least for decoder I might need:
    // https://ffmpeg.org/doxygen/trunk/structAVFrame.html#details

    /// DEBUG SCREEN
    if (av_log_get_level()>=AV_LOG_DEBUG) {
        hex_dump_internal(NULL, NULL, 0, (uint8_t *) audio_bufin_single_frame, audio_bufin_single_frame_buffer_size);
        av_log(NULL,AV_LOG_INFO,"\nME[INFO][Codec:%s][Frame:%d][InFrameSize:%d][OutTotal:%d][OutFrameSize:%d], encoded frame:\n",avcodec_get_name(c->codec_id), 0, c->frame_size, audio_outbuf_actual_encoded_size,audio_out_packet_size);
        hex_dump_internal(NULL, NULL, 0, (uint8_t *) &audio_outbuf[audio_outbuf_actual_encoded_size-audio_out_packet_size], audio_out_packet_size);
    }
}
//av_init_packet(&pkt);pkt.data=NULL;// just to be sure... Maybe this full line should be commented out.

/// HERE WE LOOP over the rest of input buffer as long as required
while (bytes_left_in_bufin >= audio_bufin_single_frame_buffer_size){
        //av_init_packet(&pkt);
        //pkt.data = NULL;pkt.size = 0;// packet out buffer can be allocated by the encoder if you want...
        pkt.data = &audio_outbuf[audio_outbuf_actual_encoded_size];
        pkt.size = audio_out_packet_size;

        /* copy input buffer data to buf single frame */// memcpy(*to, *from, size_t len)
        memcpy(audio_bufin_single_frame,&audio_bufin_samples[bytes_from_input_bufin_already_pushed_out/sizeof(audio_bufin_samples[0])],audio_bufin_single_frame_buffer_size);
        bytes_from_input_bufin_already_pushed_out+=audio_bufin_single_frame_buffer_size;
        bytes_left_in_bufin-=audio_bufin_single_frame_buffer_size;

         /* encode the samples */
        ret = avcodec_encode_audio2(c, &pkt, frame, &got_output);
        audio_out_packet_size=pkt.size;//redundant, it is not required as we are the ones setting the pkt.size above.
        if (ret < 0) {
            av_log(NULL,AV_LOG_ERROR, "Error encoding audio frame\n");
            exit_program(1);
        }
        if (got_output) {
            if (av_log_get_level()>=AV_LOG_DUMP_TO_FILE)
                fwrite(pkt.data, 1, pkt.size, f); //fflush(f);
            //memcpy(&audio_outbuf[audio_outbuf_actual_encoded_size], pkt.data, pkt.size);
            audio_outbuf_actual_encoded_size+=pkt.size;
            if (audio_outbuf_actual_encoded_size>=audio_outbuf_max_size-pkt.size){
                av_log(NULL,AV_LOG_ERROR, "Error: audio_outbuf_actual_encoded_size(%d)>audio_outbuf_max_size(%d)-pkt.size(%d)\n",audio_outbuf_actual_encoded_size,audio_outbuf_max_size,pkt.size);
                exit_program(1);
            }
            av_free_packet(&pkt);//-> DONNO IF recommended, as pkt.data is pointing to our output buffer...
        }

        // av_frame_unref()  //At least for decoder I might need:
        // https://ffmpeg.org/doxygen/trunk/structAVFrame.html#details

        /// DEBUG SCREEN
        if (av_log_get_level()>=AV_LOG_DEBUG) {
            hex_dump_internal(NULL, NULL, 0, (uint8_t *) audio_bufin_single_frame, audio_bufin_single_frame_buffer_size);
            av_log(NULL,AV_LOG_INFO,"\nME[INFO][Codec:%s][Frame:%d][InFrameSize:%d][OutTotal:%d][OutFrameSize:%d], encoded frame:\n",avcodec_get_name(c->codec_id), 0, c->frame_size, audio_outbuf_actual_encoded_size,audio_out_packet_size);
            hex_dump_internal(NULL, NULL, 0, (uint8_t *) &audio_outbuf[audio_outbuf_actual_encoded_size-audio_out_packet_size], audio_out_packet_size);
        }

}

/* get the delayed frames */
for (got_output = 1; got_output; i++) {
    ret = avcodec_encode_audio2(c, &pkt, NULL, &got_output);
    if (ret < 0) {
        av_log(NULL,AV_LOG_ERROR, "Error encoding frame\n");
        exit_program(1);
    }
    av_log(NULL,AV_LOG_INFO,"We got a delayed frames.\n");

    if (got_output) {
        if (av_log_get_level()>=AV_LOG_DUMP_TO_FILE)
            fwrite(pkt.data, 1, pkt.size, f); //fflush(f);
        memcpy(&audio_outbuf[audio_outbuf_actual_encoded_size], pkt.data, pkt.size);
        audio_outbuf_actual_encoded_size+=pkt.size;
        if (audio_outbuf_actual_encoded_size>=audio_outbuf_max_size-pkt.size){
            av_log(NULL,AV_LOG_ERROR, "Error: audio_outbuf_actual_encoded_size(%d)>audio_outbuf_max_size(%d)-pkt.size(%d)\n",audio_outbuf_actual_encoded_size,audio_outbuf_max_size,pkt.size);
            exit_program(1);
        }
        av_free_packet(&pkt);//-> DONNO IF recommended, as pkt.data is pointing to our output buffer...
    }

    if (av_log_get_level()>=AV_LOG_DEBUG){
        hex_dump_internal(NULL, NULL, 0, (uint8_t *) &audio_outbuf[audio_outbuf_actual_encoded_size-audio_out_packet_size], audio_outbuf_actual_encoded_size);
        av_log(NULL,AV_LOG_INFO,"[Codec:%s][Frame:%d][InFrameSize:%d][OutputBytesTillNow:%d], encoded frame (delayed):\n",avcodec_get_name(c->codec_id), 0, c->frame_size, audio_outbuf_actual_encoded_size);
    }

}

    av_log(NULL,AV_LOG_INFO,"[Codec:%s][Frame:%d][InFrameSize:%d][OutputBytesTillNow:%d], encoded frame (delayed):\n",avcodec_get_name(c->codec_id), 0, c->frame_size, audio_outbuf_actual_encoded_size);

if (av_log_get_level()>AV_LOG_DUMP_TO_FILE)
    fclose(f);

if (av_log_get_level()>AV_LOG_DUMP_TO_FILE)
    write_to_file2( (uint8_t*) audio_outbuf, c->codec_id, 1, audio_outbuf_actual_encoded_size);

av_frame_free(&frame);

return audio_outbuf;

}






