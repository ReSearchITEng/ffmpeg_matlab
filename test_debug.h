/* log levels from log.h:
#define AV_LOG_PANIC     0
#define AV_LOG_FATAL     8 //yellow
#define AV_LOG_ERROR    16
#define AV_LOG_WARNING  24
#define AV_LOG_INFO     32 //while (normal color)
#define AV_LOG_VERBOSE  40 //green
#define AV_LOG_DEBUG    48
*/
#define AV_LOG_DUMP_TO_FILE 35

////////////////////
//static void get_audio_frame(int16_t *samples, int frame_size, int nb_channels)
int16_t *generate_audio_frame(int frame_size){
	int16_t *audio_bufin_samples;
	audio_bufin_samples = av_malloc(frame_size * 2 * 1);//samples = av_malloc(audio_input_frame_size * 2 (bytes/sample) * c->channels); //audio_input_frame_size = c->frame_size;
    int j, i, v;
    int16_t *q;
    float t=0.5;

    q = audio_bufin_samples;
    for(j=0;j<frame_size;j++) {
        v = (int)(sin(t) * 10000); //for(i = 0; i < nb_channels; i++)
            *q++ = v;
    }

    return audio_bufin_samples;
}


int16_t *generate_audio_frame_from_file(char *filename, int *buffer_size_in_bytes){
    int actually_read;
    //Buffer_in_int16_t ST_buffer_in;
    int16_t *audio_bufin_samples = av_malloc(*buffer_size_in_bytes * 1);//samples = av_malloc(audio_input_frame_size * 2 (bytes/sample) * c->channels); //audio_input_frame_size = c->frame_size;

    //filename="tmp.beforeamr";
    FILE *f = fopen(filename, "rb");if (!f) { av_log(NULL,AV_LOG_ERROR, "could not open input file %s\n", filename); exit_program(1);    }

    //Dropping WAV header which is 44 bytes long.
    fread(audio_bufin_samples, 44, 1, f);

    //size_t fread(void *ptr, size_t size_of_element, size_t nmemb_akanumberofelements, FILE *stream);
    actually_read =fread(audio_bufin_samples, 1, *buffer_size_in_bytes, f);
    av_log(NULL,AV_LOG_INFO,"\nI read %d bytes  from: %s\n", actually_read, filename);

    buffer_size_in_bytes=&actually_read;

    return audio_bufin_samples;
}

static void write_to_file2(uint8_t *audio_outbuf, enum AVCodecID codec_id, int times, int buffer_size_in_bytes){
	int i=0;
	FILE *f;
    char *filename;
	static const char AMRNB_header [] = "#!AMR\n";
	static const char AMRWB_header [] = "#!AMR-WB\n";

    switch (codec_id){
    case AV_CODEC_ID_AMR_NB:
	//if (strcmp(type,"amr_nb")==0){
    	filename="tmp.amr"; f = fopen(filename, "wb");if (!f) {         av_log(NULL,AV_LOG_ERROR, "NB2:could not open %s\n", filename);        exit_program(1);    }
        fwrite(AMRNB_header, sizeof(AMRNB_header) - 1, 1 ,f);
        for (i=0;i<times;i++)
        	fwrite(audio_outbuf, 1, buffer_size_in_bytes , f);//AMR_NB_FROM_ENC_FRAME_SIZE
    	//avio_write(pb, AMR_header,   sizeof(AMR_header)   - 1); // magic number
	//}
	break;
	case AV_CODEC_ID_AMR_WB:
	//else if (strcmp(type,"amr_wb")==0){
    	filename="tmp.awb"; f = fopen(filename, "wb");if (!f) {         av_log(NULL,AV_LOG_ERROR, "WB2:could not open %s\n", filename);        exit_program(1);    }
        fwrite(AMRWB_header, sizeof(AMRWB_header) - 1, 1 ,f);
        for (i=0;i<times;i++)
        	fwrite(audio_outbuf, 1, buffer_size_in_bytes , f);//AMR_WB_FROM_ENC_FRAME_SIZE
        //avio_write(pb, AMRWB_header, sizeof(AMRWB_header) - 1); // magic number
	//}
    break;
	default:
		av_log(NULL,AV_LOG_ERROR,"I don't know file header for requested AV_CODEC_ID ");//,codec_id);
		//printf("AV_CODEC_ID_AMR_WB;//http://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ggaadca229ad2c20e060a14fec08a5cc7cea3d4c051c8b18a3c18750676a308c10be
	}

    fclose(f);
}

static void write_raw_to_file(int16_t *audio_inbuf, const char *type, int times, int buffer_size_in_bytes){
	int i=0;
	FILE *f;
    char *filename;

    if (strcmp(type,"before_amr_nb")==0){
    	filename="tmp.beforeamr"; f = fopen(filename, "wb");if (!f) {         fprintf(stderr, "could not open %s\n", filename);        exit_program(1);    }
        //fwrite(AMRNB_header, sizeof(AMRNB_header) - 1, 1 ,f);
        for (i=0;i<times;i++)
        	fwrite(audio_inbuf, 1, buffer_size_in_bytes, f);//AMR_NB_TO_ENC_FRAME_SIZE, f);
	}
	else if (strcmp(type,"before_amr_wb")==0){
    	filename="tmp.beforeawb"; f = fopen(filename, "wb");if (!f) {         fprintf(stderr, "could not open %s\n", filename);        exit_program(1);    }
        //fwrite(AMRNB_header, sizeof(AMRNB_header) - 1, 1 ,f);
        for (i=0;i<times;i++)
        	fwrite(audio_inbuf, 1, buffer_size_in_bytes, f);//AMR_WB_TO_ENC_FRAME_SIZE, f);
	}
	else {
		printf("WHAT???");
	}

    fclose(f);
}

/////////////////////////////////////////////////////////
//////////////////////// HEX DUMPS PART: ////////////////
/////////////////////////////////////////////////////////
static void hex_dump_internal(void *avcl, FILE *f, int level, uint8_t *buf, int size)
{
    int len, i, j, c;
#undef fprintf
#define PRINT(...) do { if (!f) av_log(avcl, level, __VA_ARGS__); else fprintf(f, __VA_ARGS__); } while(0)

    for(i=0;i<size;i+=16) {
        len = size - i;
        if (len > 16)
            len = 16;
        PRINT("%08x ", i);
        for(j=0;j<16;j++) {
            if (j < len)
                PRINT(" %02x", buf[i+j]);
            else
                PRINT("   ");
        }
        PRINT(" ");
        for(j=0;j<len;j++) {
            c = buf[i+j];
            if (c < ' ' || c > '~')
                c = '.';
            PRINT("%c", c);
        }
        PRINT("\n");
    }
#undef PRINT
}
//void av_hex_dump(FILE *f, uint8_t *buf, int size)
//{
//    hex_dump_internal(NULL, f, 0, buf, size);
//}

//void av_hex_dump_log(void *avcl, int level, uint8_t *buf, int size)
//{
//    hex_dump_internal(avcl, NULL, level, buf, size);
//}

static void pkt_dump_internal(void *avcl, FILE *f, int level, AVPacket *pkt, int dump_payload, AVRational time_base)
{
#undef fprintf
#define PRINT(...) do { if (!f) av_log(avcl, level, __VA_ARGS__); else fprintf(f, __VA_ARGS__); } while(0)
    PRINT("stream #%d:\n", pkt->stream_index);
    PRINT("  keyframe=%d\n", ((pkt->flags & AV_PKT_FLAG_KEY) != 0));
    PRINT("  duration=%0.3f\n", pkt->duration * av_q2d(time_base));
    /* DTS is _always_ valid after av_read_frame() */
    PRINT("  dts=");
    if (pkt->dts == AV_NOPTS_VALUE)
        PRINT("N/A");
    else
        PRINT("%0.3f", pkt->dts * av_q2d(time_base));
    /* PTS may not be known if B-frames are present. */
    PRINT("  pts=");
    if (pkt->pts == AV_NOPTS_VALUE)
        PRINT("N/A");
    else
        PRINT("%0.3f", pkt->pts * av_q2d(time_base));
    PRINT("\n");
    PRINT("  size=%d\n", pkt->size);
#undef PRINT
    if (dump_payload)
        av_hex_dump(f, pkt->data, pkt->size);
}

#if FF_API_PKT_DUMP
void av_pkt_dump(FILE *f, AVPacket *pkt, int dump_payload)
{
    AVRational tb = { 1, AV_TIME_BASE };
    pkt_dump_internal(NULL, f, 0, pkt, dump_payload, tb);
}
#endif

void av_pkt_dump2(FILE *f, AVPacket *pkt, int dump_payload, AVStream *st)
{
    pkt_dump_internal(NULL, f, 0, pkt, dump_payload, st->time_base);
}

#if FF_API_PKT_DUMP
void av_pkt_dump_log(void *avcl, int level, AVPacket *pkt, int dump_payload)
{
    AVRational tb = { 1, AV_TIME_BASE };
    pkt_dump_internal(avcl, NULL, level, pkt, dump_payload, tb);
}
#endif

void av_pkt_dump_log2(void *avcl, int level, AVPacket *pkt, int dump_payload,
                      AVStream *st)
{
    pkt_dump_internal(avcl, NULL, level, pkt, dump_payload, st->time_base);
}

//////////////////////////////////////////////////////////
void show_help_children(const AVClass *class, int flags)
{
   const AVClass *child = NULL;
   if (class->option) {
       av_opt_show2(&class, NULL, flags, 0);
       //printf("\n");
   }

   while (child = av_opt_child_class_next(class, child))
       show_help_children(child, flags);
}


/////////////////////////////////////////////////////////
void dump_codec_context(AVCodecContext *c){
    //http://ffmpeg.org/doxygen/trunk/structAVCodecContext.html#a1bdba69ea111e2a9d03fdaa7a46a4c45

av_log(NULL,AV_LOG_INFO,"\nME[INFO] codec_id:%s",avcodec_get_name(c->codec_id));
av_log(NULL,AV_LOG_INFO,"\nME[INFO] frame_size:%d (Number of samples per channel in an audio frame. )",c->frame_size);//Number of samples per channel in an audio frame.
av_log(NULL,AV_LOG_INFO,"\nME[INFO] frame_size in bytes:%d ",c->frame_size*av_get_bytes_per_sample(c->sample_fmt));//Number of samples per channel in an audio frame.
av_log(NULL,AV_LOG_INFO,"\nME[INFO] bit_rate:%d",c->bit_rate);
av_log(NULL,AV_LOG_INFO,"\nME[INFO] sample_rate:%d",c->sample_rate);
av_log(NULL,AV_LOG_INFO,"\nME[INFO] channels:%d",c->channels);
av_log(NULL,AV_LOG_INFO,"\nME[INFO] channel_layout:%d",c->channel_layout);
av_log(NULL,AV_LOG_INFO,"\nME[INFO] sample_fmt:%s",av_get_sample_fmt_name(c->sample_fmt));//enum AVSampleFormat
//av_log(NULL,AV_LOG_INFO,"\nME[INFO] sample_fmt:%s",av_get_sample_fmt_string(fmt_str, sizeof(fmt_str),c->sample_fmt));//enum AVSampleFormat
av_log(NULL,AV_LOG_INFO,"\nME[INFO] av_get_bytes_per_sample:%d", av_get_bytes_per_sample(c->sample_fmt));
av_log(NULL,AV_LOG_INFO,"\nME[INFO] flags :%d",c->flags );
av_log(NULL,AV_LOG_INFO,"\n" );

if (av_log_get_level()>=AV_LOG_VERBOSE){
    if (c->codec->priv_class) {
      show_help_children(c->codec->priv_class,
                         AV_OPT_FLAG_ENCODING_PARAM |
                         AV_OPT_FLAG_DECODING_PARAM);
    }
}
//c->codec->supported_samplerates

}
