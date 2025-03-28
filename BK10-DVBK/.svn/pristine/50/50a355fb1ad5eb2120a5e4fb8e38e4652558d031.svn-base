#include "audio.h"
#include "log.h"
#include "atc.h"
#include "queue.h"
#include "devconfig.h"

Queue_t queue_audio;

void add_play_list(char *filepath, char *filename, u8 volume){
	audio_file_inf obj;
	u16 fpathlen, fnamelen;
	if(SystemSetting.data.disable_audio == 1) return;
	if(volume == 0) return;

	fpathlen = strlen(filepath);
	if((fpathlen == 0) || (fpathlen > 160)) return;

	fnamelen = strlen(filename);
	if((fnamelen == 0) || (fnamelen > 64)) return;

	memset(&obj, 0x0, sizeof(audio_file_inf));

	memcpy(obj.filepath, filepath, fpathlen);
	memcpy(obj.filename, filename, fnamelen);
	obj.volume = volume;

	set_volume(volume);

	if(!enqueue(&queue_audio, &obj, sizeof(audio_file_inf))){
		log_debug("Queue audio full\n");
	}
}

static int __ql_wav_pcm16Le_check(int fd){
    struct wav_header hdr;
    int offset;

    if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
        log_debug("%s: cannot read header\n", __FUNCTION__);
        return -1;
    }

    log_debug("read wav hdr\n");
    if ((hdr.riff_id != ID_RIFF)
        || (hdr.riff_fmt != ID_WAVE)
        || (hdr.fmt_id != ID_FMT)) {
        log_debug("%s:  is not a riff/wave file\n", __FUNCTION__);
        return -1;
    }

    if ((hdr.audio_format != FORMAT_PCM) || (hdr.fmt_sz != 16)){
        log_debug("%s: is not pcm format\n", __FUNCTION__);
        return -1;
    }

    if (hdr.bits_per_sample != 16) {
        log_debug("%s: is not 16bit per sample\n", __FUNCTION__);
        return -1;
    }

    offset = lseek(fd, 0, SEEK_CUR);
    log_debug("get wav hdr offset\n");
    return offset;
}

static int __ql_wav_amr_check(int fd){
    return 0;
}

static int __ql_wav_pcm16Le_set(int fd){
    struct wav_header hdr;

    memset(&hdr, 0, sizeof(struct wav_header));

    hdr.riff_id = ID_RIFF;
    hdr.riff_fmt = ID_WAVE;
    hdr.fmt_id = ID_FMT;
    hdr.fmt_sz = 16;
    hdr.audio_format = FORMAT_PCM;
    hdr.num_channels = 1;
    hdr.sample_rate = 8000;
    hdr.bits_per_sample = 16;
    hdr.byte_rate = (8000 * 1 * hdr.bits_per_sample) / 8;
    hdr.block_align = (hdr.bits_per_sample * 1) / 8;
    hdr.data_id = ID_DATA;
    hdr.data_sz = 0;

    hdr.riff_sz = hdr.data_sz + 44 - 8;
    if (write(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
        return -1;
    }

    return 0;
}

static int __ql_wav_amr_set(int fd){
    return 0;
}

static int ql_open_and_convert_file(char *filename){
    int fd, retval;
    struct wav_header hdr;

    if(!filename){
        return -EINVAL;
    }

    fd = open(filename, O_RDONLY);
    if (fd < 0){
        fprintf(stderr, "%s: cannot open '%s'\n", __FUNCTION__, filename);
        return fd;
    }

    if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)){
        fprintf(stderr, "%s: cannot read header\n", __FUNCTION__);
        return -errno;
    }
    lseek(fd, 0, SEEK_SET);

    return fd;
}

int Ql_AudFileOpen(char *file, struct ST_MediaParams *mediaParams, int *offset){
    int fd;
    int ret;

    if ((file == NULL) && (mediaParams == NULL)){
        log_debug("%s: args invalid\n", __FUNCTION__);
        goto openfile_failed;
    }

    fd = ql_open_and_convert_file(file);
    if (fd < 0) {
        log_debug("%s: open file failed\n", __FUNCTION__);
        goto openfile_failed;
    }

    /* check media params */
    switch (mediaParams->format) {
		case AUD_STREAM_FORMAT_AMR:
			__ql_wav_amr_check(fd);
			break;
		case AUD_STREAM_FORMAT_PCM:
			ret = __ql_wav_pcm16Le_check(fd);
			if (ret < 0) {
				log_debug("parseMediafilePcm16Le failed\n");
				goto openfile_failed;
			}
			if (offset != NULL) {
				*offset = ret;
			}
			break;
		default:
			log_debug("parse Unknown File\n");
			goto openfile_failed;
    }

    return fd;
    openfile_failed:
    return -1;
}


int Ql_PcmFileCreat(char *file, struct ST_MediaParams *mediaParams, int *offset){
    int fd = -1;
    int ret = -1;

    if(access(file, 0) == 0){
        log_debug("%s: file already exist.\n", __FUNCTION__);
        return -1;
    }

    fd = open(file, O_RDWR | O_CREAT, 0x0666);
    if (fd < 0){
        log_debug("%s: open file failed\n", __FUNCTION__);
        return -1;
    }

    switch (mediaParams->format){
    case AUD_STREAM_FORMAT_AMR:
        ret = __ql_wav_amr_set(fd);
        break;

    case AUD_STREAM_FORMAT_PCM:
        ret = __ql_wav_pcm16Le_set(fd);
        break;

    default:
        ret = -1;
        break;
    }

    if (ret < 0){
        close(fd);
        return -1;
    }

    return fd;
}

static int play1Flag = 0;
int Ql_cb_Player1(int hdl, int result){
    log_debug("%s: hdl=%d, result=%d\n\r", __func__, hdl, result);
    if ((result == AUD_PLAYER_FINISHED) || (result == AUD_PLAYER_NODATA)){
        log_debug("%s: play finished\n\r", __func__);
        play1Flag = 1;
    }
    return 0;
}

void volume_up(u8 value){
	int idx;
	Ql_GPIO_SetLevel(AUDIO_CS, PINLEVEL_LOW);
	Ql_GPIO_SetLevel(AUDIO_VOL_CRTL, PINLEVEL_LOW);
	usleep(10000);//delay 10ms

	Ql_GPIO_SetLevel(AUDIO_CS, PINLEVEL_HIGH);

	for(idx = 0; idx < value; idx++){
		usleep(1);
		Ql_GPIO_SetLevel(AUDIO_VOL_CRTL, PINLEVEL_HIGH);
		usleep(1);
		Ql_GPIO_SetLevel(AUDIO_VOL_CRTL, PINLEVEL_LOW);
	}
	usleep(10000);//delay 10ms
	Ql_GPIO_SetLevel(AUDIO_CS, PINLEVEL_LOW);
}

void volume_down(u8 value){
	int idx;
	Ql_GPIO_SetLevel(AUDIO_CS, PINLEVEL_LOW);
	Ql_GPIO_SetLevel(AUDIO_VOL_CRTL, PINLEVEL_HIGH);
	usleep(10000);//delay 10ms

	Ql_GPIO_SetLevel(AUDIO_CS, PINLEVEL_HIGH);

	for(idx = 0; idx < value; idx++){
		usleep(1);
		Ql_GPIO_SetLevel(AUDIO_VOL_CRTL, PINLEVEL_LOW);
		usleep(1);
		Ql_GPIO_SetLevel(AUDIO_VOL_CRTL, PINLEVEL_HIGH);
	}
	usleep(10000);//delay 10ms
	Ql_GPIO_SetLevel(AUDIO_CS, PINLEVEL_LOW);
}

static u8 _Volume = 0;
int set_volume(u8 vol){
	int idx;
	if((_Volume != vol) && (vol <= 64)){
		_Volume = vol;

		/* volume down -> 0 */
		volume_down(64);
		/* setting volume */
		volume_up(_Volume);

		return 0;
	}
	return -1;
}

static char fname[192];
static u8 link_download_audio[320];
void* thread_audio_handler(void* arg){
    int iRet;
    int devphdl1;
    int devphdl2;
    int filefd1;
    int filefd2;
    int recdTime;

    unsigned int tmp32;
    struct ST_MediaParams mediaParas;

    Ql_GPIO_Init(AUDIO_PW, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_DISABLE);
    Ql_GPIO_SetLevel(AUDIO_PW, PINLEVEL_LOW);

    Ql_GPIO_Init(AUDIO_CS, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_DISABLE);
    Ql_GPIO_SetLevel(AUDIO_CS, PINLEVEL_LOW);

    Ql_GPIO_Init(AUDIO_VOL_CRTL, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_DISABLE);
    Ql_GPIO_SetLevel(AUDIO_VOL_CRTL, PINLEVEL_LOW);



	initialize(&queue_audio, MAX_QUEUE_SIZE);
	//add_play_list("/usrdata/sound/", "startup.wav", 5);//64

    for(;;){
    	if(!audio_ready()){
    		log_debug("AUDIO NOT READY\n");
    		sleep(5);
    		continue;
    	}

		if(!isEmpty(&queue_audio)){
			audio_file_inf buf;

			memset(fname, 0x0, 192);
			memset(&buf, 0x0, sizeof(audio_file_inf));

			dequeue(&queue_audio, &buf, sizeof(audio_file_inf));

			sprintf(fname, "%s%s", buf.filepath, buf.filename);

			if(access(fname, F_OK) == 0){
				log_debug("FILE %s EXIST \r\n", fname);

				//pw on amplifier
		    	Ql_GPIO_SetLevel(AUDIO_PW, PINLEVEL_HIGH);

		        mediaParas.format = AUD_STREAM_FORMAT_PCM;

		        filefd1 = Ql_AudFileOpen(fname, &mediaParas, NULL);

		        Ql_clt_set_mixer_value("SEC_AUX_PCM_RX Audio Mixer MultiMedia1", 1, "1");

		        devphdl1 = Ql_AudPlayer_Open(NULL, Ql_cb_Player1);
		        if (devphdl1 < 0){
		            log_debug("open audio play%d failed\n\r", devphdl1);
		            return 0;
		        }

		        play1Flag = 0;
		        iRet = Ql_AudPlayer_PlayFrmFile(devphdl1, filefd1, 0);

		        if (iRet != 0) {
		            log_debug("start a_Volumeudio play%d failed\n\r", devphdl1);
		        }

		        while (play1Flag != 1) {
		            sleep(1);
		        }

		        Ql_AudPlayer_Stop(devphdl1);
		        Ql_AudPlayer_Close(devphdl1);

		        //pw off amplifier
		        Ql_GPIO_SetLevel(AUDIO_PW, PINLEVEL_LOW);
			}else{
				log_debug("FILE %s NOT EXIST \r\n", buf.filename);
				//download wav file here

				memset(link_download_audio, 0x0, 320);
				sprintf(link_download_audio, "%s/audio/%s", SystemSetting.data.update_fw_url, buf.filename);

				int res = file_download(link_download_audio, buf.filepath, buf.filename);
				if(res == 0){
					add_play_list(buf.filepath, buf.filename, SystemSetting.data.SpeakerVolume);
					log_debug("DOWNLOAD %s SUCCESS!!!\r\n", buf.filename);
				}

				sleep(1);
			}
		}else{
			 sleep(3);
		}
    }
}

