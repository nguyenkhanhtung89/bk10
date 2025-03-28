#include "atc.h"
#include "log.h"
#include "devconfig.h"

#define ATC_RESP_CMD_MAX_LEN    1024
static atc_client_handle_type    h_atc     = 0;

#define BUF_SIZE 32

int atc_send_cmd(u8* atc_cmd_req, u8* atc_cmd_resp){
    int    cmdIdx  = 0;
    int    ret     = E_QL_OK;
    char    buf[BUF_SIZE] = {0};

    ret = QL_ATC_Client_Init(&h_atc);
    //log_debug("QL_ATC_Client_Init ret=%d with h_atc=0x%x\n", ret, h_atc);
    if(ret != 0){
    	return -1;
    }

    ret = QL_ATC_Send_Cmd(h_atc, atc_cmd_req, atc_cmd_resp, ATC_RESP_CMD_MAX_LEN);
    log_debug("QL_ATC_Send_Cmd \"%s\" ret=%d with response: %s\n", atc_cmd_req, ret, atc_cmd_resp);
    if(ret != 0){
    	return -1;
    }

    ret = QL_ATC_Client_Deinit(h_atc);
    //log_debug("QL_ATC_Client_Deinit ret=%d\n", ret);

    return 0;
}

#define CHK_SIMDET		"AT+QSIMDET?"
#define RSP_SIMDET		"+QSIMDET: 1,0"
#define CFG_SIMDET		"AT+QSIMDET=1,0"
u8 flg_simdet_cfg = 0;
static char atc_cmd_response[ATC_RESP_CMD_MAX_LEN]  = {0};
void config_sim_detect(void){
	if(flg_simdet_cfg)return;

	memset(atc_cmd_response, 0, sizeof(atc_cmd_response));
	if(atc_send_cmd(CHK_SIMDET, atc_cmd_response) < 0) return;

	if(str_find(atc_cmd_response, RSP_SIMDET) >= 0){
		flg_simdet_cfg = 1;
		return;
	}

	memset(atc_cmd_response, 0, sizeof(atc_cmd_response));
	atc_send_cmd(CFG_SIMDET, atc_cmd_response);

	memset(atc_cmd_response, 0, sizeof(atc_cmd_response));
	atc_send_cmd("AT&W", atc_cmd_response);
}


u8 FlgTLV320Config = 0;
#define CHK_QDAI    "AT+QDAI?"
#define RSP_QDAI_ALC "+QDAI: 3,0,0,4,0,0,1,1"
#define RSP_QDAI_TLV "+QDAI: 5,0,0,4,0,0,1,1"
#define CFG_QDAI_TLV "AT+QDAI=5,0,0,4,0,0,1,1"
static char atc_audio_response[ATC_RESP_CMD_MAX_LEN]  = {0};
int config_tlv_codec(void){
	if(!FlgTLV320Config){
		memset(atc_audio_response, 0, sizeof(atc_audio_response));
		if(atc_send_cmd(CHK_QDAI, atc_audio_response) < 0) return -1;

		if(str_find(atc_audio_response, RSP_QDAI_TLV) >= 0){
			FlgTLV320Config = 1;
			return 1;
		}else if(str_find(atc_audio_response, RSP_QDAI_ALC) >= 0){
			if(!atc_send_cmd(CFG_QDAI_TLV, atc_audio_response)){
				system("echo \"tlv320aic3x-codec.2-0018\" > /sys/devices/soc:sound/codec_name");
				system("echo \"tlv320aic3x-hifi\" > /sys/devices/soc:sound/rx_dai_name");
				system("echo \"tlv320aic3x-hifi\" > /sys/devices/soc:sound/tx_dai_name");
				system("insmod /usr/lib/modules/3.18.20/kernel/sound/soc/codecs/snd-soc-tlv320aic3x.ko");
				log_debug("~~~~~~~~~~~~~~~~~~~~~CONFIG TLV302AI CODEC~~~~~~~~~~~~~~~~~~~ \r\n");
				sleep(3);
				ec21_reboot();
				return -1;
			}
			return -1;
		}
	}
	return 1;
}

u8 audio_ready(void){
	return FlgTLV320Config;
}


