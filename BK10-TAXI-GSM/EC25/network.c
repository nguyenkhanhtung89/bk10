#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/poll.h>
#include "ql_sgmii.h"

#include "gps.h"
#include "network.h"
#include "ql_network.h"
#include "log.h"
#include "database.h"
#include "atc.h"
#include "fw.h"
#include "setting.h"
#include "devconfig.h"

#define MAX_QUEUE_TCP_SIZE	150
#define MAX_QUEUE_SERVER_SIZE	10

Queue_t queue_tcp;
Queue_t queue_server;
nw_data_def network_data;
sms_def SMSData;

E_QL_WWAN_NET_REG_STATE_T current_nw_mode = E_QL_WWAN_REG_NOT_REGISTERED;

char current_operator[128];
u16 MobileCountryCode = 0;

static char interface_name[16];

static int TCP_sockfd = -1;
u16 server_connect_loss_time = 0;
nw_client_handle_type    nw_handler_id = 0;
static int h_sim = 0;
static sms_client_handle_type h_sms = 0;

char *tech_domain[] = {"NONE", "3GPP", "3GPP2"};
char *radio_tech[] = {"unknown",
    "TD_SCDMA", "GSM",      "HSPAP",    "LTE",      "EHRPD",    "EVDO_B",
    "HSPA",     "HSUPA",    "HSDPA",    "EVDO_A",   "EVDO_0",   "1xRTT",
    "IS95B",    "IS95A",    "UMTS",     "EDGE",     "GPRS",     "NONE"};

typedef enum{
    STATE_NW_GET_SIMSTATE = 0,
	STATE_NW_CONFIG_HANDLER = 1,
    STATE_NW_QUERY_STATE = 2,
    STATE_INTERNET_QUERY_STATE = 3,
    STATE_WAN_INITIALIZE = 4,
    STATE_WAN_START = 5,
    STATE_WAN_DEACTIVATE = 6,
    STATE_INTERNET_READY = 7
}Enum_NWSTATE;

static Enum_NWSTATE m_nw_state = STATE_NW_GET_SIMSTATE;

typedef enum{
	STATE_WAIT_INTERNET = 0,
	STATE_SOC_CONNECT = 1,
	STATE_SOC_SEND = 2,
	STATE_TCP_RECONNECT = 3
}Enum_TCPSTATE;

static Enum_TCPSTATE m_tcp_state = STATE_WAIT_INTERNET;

void sms_init(void);

static void* Check_internet_timeout_Handler (void* arg);

static pthread_t thrd_svr_handler, thrd_check_internet_timeout_handler;

static char* errorcode[] =
{
    "MCM_SUCCESS",                              //  0, /**<  Success. */
    "MCM_SUCCESS_CONDITIONAL_SUCCESS",          //  1, /**<  Conditional success. */
    "MCM_ERROR_MCM_SERVICES_NOT_AVAILABLE",     //  2, /**<  "MCM services not available. */
    "MCM_ERROR_GENERIC",                        //  3, /**<  Generic error. */
    "MCM_ERROR_BADPARM",                        //  4, /**<  Bad parameter. */
    "MCM_ERROR_MEMORY",                         //  5, /**<  Memory error. */
    "MCM_ERROR_INVALID_STATE",                  //  6, /**<  Invalid state. */
    "MCM_ERROR_MALFORMED_MSG",                  //  7, /**<  Malformed message. */
    "MCM_ERROR_NO_MEMORY",                      //  8, /**<  No memory. */
    "MCM_ERROR_INTERNAL",                       //  9, /**<  Internal error. */
    "MCM_ERROR_ABORTED",                        //  10, /**<  Action was aborted. */
    "MCM_ERROR_CLIENT_IDS_EXHAUSTED",           //  11, /**<  Client IDs have been exhausted. */
    "MCM_ERROR_UNABORTABLE_TRANSACTION",        //  12, /**<  Unabortable transaction. */
    "MCM_ERROR_INVALID_CLIENT_ID",              //  13, /**<  Invalid client ID. */
    "MCM_ERROR_NO_THRESHOLDS",                  //  14, /**<  No thresholds. */
    "MCM_ERROR_INVALID_HANDLE",                 //  15, /**<  Invalid handle. */
    "MCM_ERROR_INVALID_PROFILE",                //  16, /**<  Invalid profile. */
    "MCM_ERROR_INVALID_PINID",                  //  17, /**<  Invalid PIN ID. */
    "MCM_ERROR_INCORRECT_PIN",                  //  18, /**<  Incorrect PIN. */
    "MCM_ERROR_NO_NETWORK_FOUND",               //  19, /**<  No network found. */
    "MCM_ERROR_CALL_FAILED",                    //  20, /**<  Call failed. */
    "MCM_ERROR_OUT_OF_CALL",                    //  21, /**<  Out of call. */
    "MCM_ERROR_NOT_PROVISIONED",                //  22, /**<  Not provisioned. */
    "MCM_ERROR_MISSING_ARG",                    //  23, /**<  Missing argument. */
    "MCM_ERROR_ARG_TOO_LONG",                   //  24, /**<  Argument is too long. */
    "MCM_ERROR_INVALID_TX_ID",                  //  25, /**<  Invalid Tx ID. */
    "MCM_ERROR_DEVICE_IN_USE",                  //  26, /**<  Device is in use. */
    "MCM_ERROR_OP_NETWORK_UNSUPPORTED",         //  27, /**<  OP network is not supported. */
    "MCM_ERROR_OP_DEVICE_UNSUPPORTED",          //  28, /**<  OP device is not supported. */
    "MCM_ERROR_NO_EFFECT",                      //  29, /**<  No effect. */
    "MCM_ERROR_NO_FREE_PROFILE",                //  30, /**<  No free profile. */
    "MCM_ERROR_INVALID_PDP_TYPE",               //  31, /**<  Invalid PDP type. */
    "MCM_ERROR_INVALID_TECH_PREF",              //  32, /**<  Invalid technical preference. */
    "MCM_ERROR_INVALID_PROFILE_TYPE",           //  33, /**<  Invalid profile type. */
    "MCM_ERROR_INVALID_SERVICE_TYPE",           //  34, /**<  Invalid service type. */
    "MCM_ERROR_INVALID_REGISTER_ACTION",        //  35, /**<  Invalid register action. */
    "MCM_ERROR_INVALID_PS_ATTACH_ACTION",       //  36, /**<  Invalid PS attach action. */
    "MCM_ERROR_AUTHENTICATION_FAILED",          //  37, /**<  Authentication failed. */
    "MCM_ERROR_PIN_BLOCKED",                    //  38, /**<  PIN is blocked. */
    "MCM_ERROR_PIN_PERM_BLOCKED",               //  39, /**<  PIN is permanently blocked. */
    "MCM_ERROR_SIM_NOT_INITIALIZED",            //  40, /**<  SIM is not initialized. */
    "MCM_ERROR_MAX_QOS_REQUESTS_IN_USE",        //  41, /**<  Maximum QoS requests are in use. */
    "MCM_ERROR_INCORRECT_FLOW_FILTER",          //  42, /**<  Incorrect flow filter. */
    "MCM_ERROR_NETWORK_QOS_UNAWARE",            //  43, /**<  Network QoS is unaware. */
    "MCM_ERROR_INVALID_ID",                     //  44, /**<  Invalid ID. */
    "MCM_ERROR_INVALID_QOS_ID",                 //  45, /**<  Invalid QoS ID. */
    "MCM_ERROR_REQUESTED_NUM_UNSUPPORTED",      //  46, /**<  Requested number is not supported. */
    "MCM_ERROR_INTERFACE_NOT_FOUND",            //  47, /**<  Interface was not found. */
    "MCM_ERROR_FLOW_SUSPENDED",                 //  48, /**<  Flow is suspended. */
    "MCM_ERROR_INVALID_DATA_FORMAT",            //  49, /**<  Invalid data format. */
    "MCM_ERROR_GENERAL",                        //  50, /**<  General error. */
    "MCM_ERROR_UNKNOWN",                        //  51, /**<  Unknown error. */
    "MCM_ERROR_INVALID_ARG",                    //  52, /**<  Invalid argument. */
    "MCM_ERROR_INVALID_INDEX",                  //  53, /**<  Invalid index. */
    "MCM_ERROR_NO_ENTRY",                       //  54, /**<  No entry. */
    "MCM_ERROR_DEVICE_STORAGE_FULL",            //  55, /**<  Device storage is full. */
    "MCM_ERROR_DEVICE_NOT_READY",               //  56, /**<  Device is not ready. */
    "MCM_ERROR_NETWORK_NOT_READY",              //  57, /**<  Network is not ready. */
    "MCM_ERROR_CAUSE_CODE",                     //  58, /**<  Cause code error. */
    "MCM_ERROR_MESSAGE_NOT_SENT",               //  59, /**<  Message was not sent. */
    "MCM_ERROR_MESSAGE_DELIVERY_FAILURE",       //  60, /**<  Message delivery failure. */
    "MCM_ERROR_INVALID_MESSAGE_ID",             //  61, /**<  Invalid message ID. */
    "MCM_ERROR_ENCODING",                       //  62, /**<  Encoding error. */
    "MCM_ERROR_AUTHENTICATION_LOCK",            //  63, /**<  Authentication lock error. */
    "MCM_ERROR_INVALID_TRANSITION",             //  64, /**<  Invalid transition. */
    "MCM_ERROR_NOT_A_MCAST_IFACE",              //  65, /**<  Not an MCast interface. */
    "MCM_ERROR_MAX_MCAST_REQUESTS_IN_USE",      //  66, /**<  Maximum MCast requests are in use. */
    "MCM_ERROR_INVALID_MCAST_HANDLE",           //  67, /**<  Invalid MCast handle. */
    "MCM_ERROR_INVALID_IP_FAMILY_PREF",         //  68, /**<  Invalid IP family preference. */
    "MCM_ERROR_SESSION_INACTIVE",               //  69, /**<  Session is inactive. */
    "MCM_ERROR_SESSION_INVALID",                //  70, /**<  Session is invalid. */
    "MCM_ERROR_SESSION_OWNERSHIP",              //  71, /**<  Session ownership error. */
    "MCM_ERROR_INSUFFICIENT_RESOURCES",         //  72, /**<  Insufficient resources. */
    "MCM_ERROR_DISABLED",                       //  73, /**<  Disabled. */
    "MCM_ERROR_INVALID_OPERATION",              //  74, /**<  Invalid operation. */
    "MCM_ERROR_INVALID_CMD",                    //  75, /**<  Invalid command. */
    "MCM_ERROR_TPDU_TYPE",                      //  76, /**<  Transfer Protocol data unit type error. */
    "MCM_ERROR_SMSC_ADDR",                      //  77, /**<  Short message service center address error. */
    "MCM_ERROR_INFO_UNAVAILABLE",               //  78, /**<  Information is not available. */
    "MCM_ERROR_SEGMENT_TOO_LONG",               //  79, /**<  Segment is too long. */
    "MCM_ERROR_SEGMENT_ORDER",                  //  80, /**<  Segment order error. */
    "MCM_ERROR_BUNDLING_NOT_SUPPORTED",         //  81, /**<  Bundling is not supported. */
    "MCM_ERROR_OP_PARTIAL_FAILURE",             //  82, /**<  OP partial failure. */
    "MCM_ERROR_POLICY_MISMATCH",                //  83, /**<  Policy mismatch. */
    "MCM_ERROR_SIM_FILE_NOT_FOUND",             //  84, /**<  SIM file was not found. */
    "MCM_ERROR_EXTENDED_INTERNAL",              //  85, /**<  Extended internal error. */
    "MCM_ERROR_ACCESS_DENIED",                  //  86, /**<  Access is denied. */
    "MCM_ERROR_HARDWARE_RESTRICTED",            //  87, /**<  Hardware is restricted. */
    "MCM_ERROR_ACK_NOT_SENT",                   //  88, /**<  Acknowledgement was not sent. */
    "MCM_ERROR_INJECT_TIMEOUT",                 //  89, /**<  Inject timeout error. */
    "MCM_ERROR_INCOMPATIBLE_STATE",             //  90, /**<  Incompatible state. */
    "MCM_ERROR_FDN_RESTRICT",                   //  91, /**<  Fixed dialing number restrict error. */
    "MCM_ERROR_SUPS_FAILURE_CAUSE",             //  92, /**<  SUPS failure cause. */
    "MCM_ERROR_NO_RADIO",                       //  93, /**<  No radio. */
    "MCM_ERROR_NOT_SUPPORTED",                  //  94, /**<  Not supported. */
    "MCM_ERROR_NO_SUBSCRIPTION",                //  95, /**<  No subscription. */
    "MCM_ERROR_CARD_CALL_CONTROL_FAILED",       //  96, /**<  Card call control failed. */
    "MCM_ERROR_NETWORK_ABORTED",                //  97, /**<  Network was aborted. */
    "MCM_ERROR_MSG_BLOCKED",                    //  98, /**<  Message was blocked. */
    "MCM_ERROR_INVALID_SESSION_TYPE",           //  99, /**<  Invalid session type. */
    "MCM_ERROR_INVALID_PB_TYPE",                //  100, /**<  Invalid phonebook type. */
    "MCM_ERROR_NO_SIM",                         //  101, /**<  No SIM was found. */
    "MCM_ERROR_PB_NOT_READY",                   //  102, /**<  Phonebook not ready. */
    "MCM_ERROR_PIN_RESTRICTION",                //  103, /**<  PIN restriction. */
    "MCM_ERROR_PIN2_RESTRICTION",               //  104, /**<  PIN2 restriction. */
    "MCM_ERROR_PUK_RESTRICTION",                //  105, /**<  PIN unlocking key restriction. */
    "MCM_ERROR_PUK2_RESTRICTION",               //  106, /**<  PIN unlocking key2 restriction. */
    "MCM_ERROR_PB_ACCESS_RESTRICTED",           //  107, /**<  Phonebook access is restricted. */
    "MCM_ERROR_PB_DELETE_IN_PROG",              //  108, /**<  Phonebook delete is in progress. */
    "MCM_ERROR_PB_TEXT_TOO_LONG",               //  109, /**<  Phonebook text is too long. */
    "MCM_ERROR_PB_NUMBER_TOO_LONG",             //  110, /**<  Phonebook number is too long. */
    "MCM_ERROR_PB_HIDDEN_KEY_RESTRICTION",      //  111, /**<  Phonebook hidden key restriction. */
    "MCM_ERROR_PB_NOT_AVAILABLE",               //  112, /**<  Phonebook is not available. */
    "MCM_ERROR_DEVICE_MEMORY_ERROR",            //  113, /**<  Device memory error. */
    "MCM_ERROR_SIM_PIN_BLOCKED",                //  114, /**<  SIM PIN is blocked. */
    "MCM_ERROR_SIM_PIN_NOT_INITIALIZED",        //  115, /**<  SIM PIN is not initialized. */
    "MCM_ERROR_SIM_INVALID_PIN",                //  116, /**<  SIM PIN is invalid. */
    "MCM_ERROR_SIM_INVALID_PERSO_CK",           //  117, /**<  SIM invalid personalization CK. */
    "MCM_ERROR_SIM_PERSO_BLOCKED",              //  118, /**<  SIM personalization blocked. */
    "MCM_ERROR_SIM_PERSO_INVALID_DATA",         //  119, /**<  SIM personalization contains invalid data. */
    "MCM_ERROR_SIM_ACCESS_DENIED",              //  120, /**<  SIM access is denied. */
    "MCM_ERROR_SIM_INVALID_FILE_PATH",          //  121, /**<  SIM file path is invalid. */
    "MCM_ERROR_SIM_SERVICE_NOT_SUPPORTED",      //  122, /**<  SIM service is not supported. */
    "MCM_ERROR_SIM_AUTH_FAIL",                  //  123, /**<  SIM authorization failure. */
    "MCM_ERROR_SIM_PIN_PERM_BLOCKED"            //  124, /**<  SIM PIN is permanently blocked. */
};

void init_queue_tcp(void){
	initialize(&queue_tcp, MAX_QUEUE_TCP_SIZE);
	initialize(&queue_server, MAX_QUEUE_SERVER_SIZE);
}

static void ql_nw_regstate_cb_func( E_QL_WWAN_NET_REG_STATE_T   state, void *contextPtr){
	//log_nw("######### Current network regstate changed to %d  ######\n", state);
    if((state != E_QL_WWAN_REG_REGISTERED_HOME_NETWORK) && (state != E_QL_WWAN_REG_ROAMING)){
    	m_nw_state = STATE_WAN_DEACTIVATE;
    	m_tcp_state = STATE_TCP_RECONNECT;

    	network_data.net_work_info.radio_tech = NW_MODE_GSM;
    	network_data.net_work_info.signal_strength = 0;
    }
    current_nw_mode = state;
}

bool Vehicle_in_China(void){
	if(SystemSetting.data.disable_camera_roaming_mode == 0){
		return false;
	}
	if((MobileCountryCode == CHINA_MCC_1) || (MobileCountryCode == CHINA_MCC_2) || (MobileCountryCode == CHINA_MCC_3) || (MobileCountryCode == CHINA_MCC_4)){
		return true;
	}
	return false;
}

voice_client_handle_type    h_voice     = 0;
int flg_rep_call_check_dev_info = 0;
static void ql_voice_call_ind_func(unsigned int ind_id,   void* ind_data,   uint32_t ind_data_len){
	int    ret     = 0;

	if(NULL == ind_data){
		return;
	}

	switch(ind_id){
		case E_QL_MCM_VOICE_CALL_IND:{
			if(ind_data_len != sizeof(ql_mcm_voice_call_ind)){
				break;
			}

			ql_mcm_voice_call_ind *pVoiceCallInd = (ql_mcm_voice_call_ind*)ind_data;

			char *call_state[] = {"INCOMING", "DIALING", "ALERTING", "ACTIVE", "HOLDING", "END", "WAITING"};

			int i = 0;
			for(i = 0; i < pVoiceCallInd->calls_len; i++){//######### Call id=1, PhoneNum:0979795791, event=INCOMING!  ######
				log_system("######### Call id=%d, PhoneNum:%s, event=%s!  ######\n",
							pVoiceCallInd->calls[i].call_id, pVoiceCallInd->calls[i].number, call_state[pVoiceCallInd->calls[i].state]);
				if((str_find(pVoiceCallInd->calls[i].number, "979795791") >= 0) && (pVoiceCallInd->calls[i].state == 0)){
					flg_rep_call_check_dev_info = 1;
				}
				ret = QL_Voice_Call_Answer(h_voice, pVoiceCallInd->calls[i].call_id);
				log_system("QL_Voice_Call_Answer ret = %d\n", ret);
			}

			break;
		}

		case E_QL_MCM_VOICE_ECALL_STATUE_IND:
			break;
		case E_QL_MCM_VOICE_ECALL_EVENT_IND:
			break;
		case E_QL_MCM_VOICE_UNKOWN_IND:
		default:
			break;
	}
}

u8 flg_dowload_done = 0;
char download_link_[160];
u8 count_download_failed = 0;
static void check_file_missing(void){
	if((flg_dowload_done == 1) || (count_download_failed > 5)){
		return;
	}

	if(access("/usrdata/xtra2.bin", F_OK) != 0){
		memset(download_link_, 0x0, 160);
		sprintf(download_link_,"%s/bk10/ec21/xtra2.bin\0", SystemSetting.data.update_fw_url);

		int res = file_download(download_link_, "/usrdata/", "xtra2.bin");
		if(res == 0){
			__off_t size;
			struct stat st;

			stat("/usrdata/xtra2.bin", &st);
			size = st.st_size;

			if(size == 59763 ){
				log_system("Download file xtra2.bin SUCCESS -> size = %d \n", size);
				sleep(3);
				//reboot device
				ec21_reboot();

			}else{
				system("rm -rf /usrdata/xtra2.bin");
			}
		}else{
			count_download_failed++;
			log_system("Download file xtra2.bin ERROR \n");
		}
	}else{
		flg_dowload_done = 1;
	}
}

static u8 is_voice_call_init = 0;
void voice_call_init(void){
	int ret = 0;

	if(is_voice_call_init==0){
		/* check sim status */
		if(network_data.sim_status != E_QL_SIM_STAT_READY) return;

		is_voice_call_init=1;
		ret = QL_Voice_Call_Client_Init(&h_voice);
		if(ret < 0)
		{
			log_nw("QL_Voice_Call_Client_Init FAIL.	ret:%d\n",ret);
			is_voice_call_init = 0;
		}
		log_nw("QL_Voice_Call_Client_Init ret = %d, with h_voice=%d\n", ret, h_voice);

		ret = QL_Voice_Call_AddCommonStateHandler(h_voice, (QL_VoiceCall_CommonStateHandlerFunc_t)ql_voice_call_ind_func);
		if(ret < 0)
		{
			log_nw("QL_Voice_Call_AddCommonStateHandler FAIL.		ret:%d\n",ret);
			is_voice_call_init = 0;
		}
		log_nw("QL_Voice_Call_AddCommonStateHandler ret = %d\n", ret);
	}
}

void nw_event_ind_handler (nw_client_handle_type nw_handler_id, uint32_t ind_flag, void *ind_msg_buf, uint32_t ind_msg_len, void *contextPtr){
    switch(ind_flag){
        case NW_IND_VOICE_REG_EVENT_IND_FLAG:{
                QL_MCM_NW_VOICE_REG_EVENT_IND_T *ind = (void*)ind_msg_buf;
               // log_nw("Recv event indication : VOICE REG EVENT\n");

                if(ind->registration_valid){
                    log_nw("voice_registration: \ntech_domain=%s, radio_tech=%s, roaming=%d, registration_state=%d, deny_reason=%d\n",
                            tech_domain[ind->registration.tech_domain],
                            radio_tech[ind->registration.radio_tech],
                            ind->registration.roaming,
                            ind->registration.registration_state,
                            ind->registration.deny_reason);
                }
                if(ind->registration_details_3gpp_valid){
                    log_nw("voice_registration_details_3gpp: \ntech_domain=%s, radio_tech=%s, mcc=%s, mnc=%s, roaming=%d, forbidden=%d, cid=0x%X, lac=%d, psc=%d, tac=%d\n",
                            tech_domain[ind->registration_details_3gpp.tech_domain],
                            radio_tech[ind->registration_details_3gpp.radio_tech],
                            ind->registration_details_3gpp.mcc,
                            ind->registration_details_3gpp.mnc,
                            ind->registration_details_3gpp.roaming,
                            ind->registration_details_3gpp.forbidden,
                            ind->registration_details_3gpp.cid,
                            ind->registration_details_3gpp.lac,
                            ind->registration_details_3gpp.psc,
                            ind->registration_details_3gpp.tac);
                }
                if(ind->registration_details_3gpp2_valid){
                    log_nw("voice_registration_details_3gpp2: \ntech_domain=%s, radio_tech=%s, mcc=%s, mnc=%s, roaming=%d, forbidden=%d, sid=%d, nid=%d, bsid=%d\n",
                            tech_domain[ind->registration_details_3gpp2.tech_domain],
                            radio_tech[ind->registration_details_3gpp2.radio_tech],
                            ind->registration_details_3gpp2.mcc,
                            ind->registration_details_3gpp2.mnc,
                            ind->registration_details_3gpp2.roaming,
                            ind->registration_details_3gpp2.forbidden,
                            ind->registration_details_3gpp2.sid,
                            ind->registration_details_3gpp2.nid,
                            ind->registration_details_3gpp2.bsid);
                }
                break;
            }
        case NW_IND_DATA_REG_EVENT_IND_FLAG:{
                QL_MCM_NW_DATA_REG_EVENT_IND_T *ind = (void*)ind_msg_buf;

                log_nw("Recv event indication : DATA REG EVENT\n");

                if(ind->registration_valid){
                    log_nw("data_registration: \ntech_domain=%s, radio_tech=%s, roaming=%d, registration_state=%d, deny_reason=%d\n",
                            tech_domain[ind->registration.tech_domain],
                            radio_tech[ind->registration.radio_tech],
                            ind->registration.roaming,
                            ind->registration.registration_state,
                            ind->registration.deny_reason);
                }
                if(ind->registration_details_3gpp_valid){
                    log_nw("data_registration_details_3gpp: \ntech_domain=%s, radio_tech=%s, mcc=%s, mnc=%s, roaming=%d, forbidden=%d, cid=0x%X, lac=%d, psc=%d, tac=%d\n",
                            tech_domain[ind->registration_details_3gpp.tech_domain],
                            radio_tech[ind->registration_details_3gpp.radio_tech],
                            ind->registration_details_3gpp.mcc,
                            ind->registration_details_3gpp.mnc,
                            ind->registration_details_3gpp.roaming,
                            ind->registration_details_3gpp.forbidden,
                            ind->registration_details_3gpp.cid,
                            ind->registration_details_3gpp.lac,
                            ind->registration_details_3gpp.psc,
                            ind->registration_details_3gpp.tac);
                }
                if(ind->registration_details_3gpp2_valid){
                    log_nw("data_registration_details_3gpp2: \ntech_domain=%s, radio_tech=%s, mcc=%s, mnc=%s, roaming=%d, forbidden=%d, sid=%d, nid=%d, bsid=%d\n",
                            tech_domain[ind->registration_details_3gpp2.tech_domain],
                            radio_tech[ind->registration_details_3gpp2.radio_tech],
                            ind->registration_details_3gpp2.roaming,
                            ind->registration_details_3gpp2.forbidden,
                            ind->registration_details_3gpp2.sid,
                            ind->registration_details_3gpp2.nid,
                            ind->registration_details_3gpp2.bsid);
                }
                break;
            }
        case NW_IND_SIGNAL_STRENGTH_EVENT_IND_FLAG:{
                QL_MCM_NW_SINGNAL_EVENT_IND_T *ind = (void*)ind_msg_buf;

               // log_nw("Recv event indication : SIGNAL STRENGTH EVENT\n");

                if(ind->gsm_sig_info_valid)
                {
                	network_data.net_work_info.radio_tech = NW_MODE_GSM;
                	network_data.net_work_info.signal_strength = ind->gsm_sig_info.rssi;

                    log_nw("gsm_sig_info: rssi=%d\n",
                            ind->gsm_sig_info.rssi);
                }
                if(ind->wcdma_sig_info_valid)
                {
                	network_data.net_work_info.radio_tech = NW_MODE_WCDMA;
                	network_data.net_work_info.signal_strength = ind->wcdma_sig_info.rssi;

                    log_nw("wcdma_sig_info: rssi=%d, ecio=%d\n",
                            ind->wcdma_sig_info.rssi,
                            ind->wcdma_sig_info.ecio);
                }
                if(ind->tdscdma_sig_info_valid)
                {
                	network_data.net_work_info.radio_tech = NW_MODE_TDSCDMA;
                	network_data.net_work_info.signal_strength = ind->tdscdma_sig_info.rssi;

                    log_nw("tdscdma_sig_info: rssi=%d, rscp=%d, ecio=%d, sinr=%d\n",
                            ind->tdscdma_sig_info.rssi,
                            ind->tdscdma_sig_info.rscp,
                            ind->tdscdma_sig_info.ecio,
                            ind->tdscdma_sig_info.sinr);
                }
                if(ind->lte_sig_info_valid)
                {
                	network_data.net_work_info.radio_tech = NW_MODE_LTE;
                	network_data.net_work_info.signal_strength = ind->lte_sig_info.rssi;

                    log_nw("lte_sig_info: rssi=%d, rsrq=%d, rsrp=%d, snr=%d\n",
                            ind->lte_sig_info.rssi,
                            ind->lte_sig_info.rsrq,
                            ind->lte_sig_info.rsrp,
                            ind->lte_sig_info.snr);
                }
                if(ind->cdma_sig_info_valid)
                {
                	network_data.net_work_info.radio_tech = NW_MODE_CDMA;
                	network_data.net_work_info.signal_strength = ind->cdma_sig_info.rssi;

                    log_nw("cdma_sig_info: rssi=%d, ecio=%d\n",
                            ind->cdma_sig_info.rssi,
                            ind->cdma_sig_info.ecio);
                }
                if(ind->hdr_sig_info_valid)
                {
                	network_data.net_work_info.radio_tech = NW_MODE_HDR;
                	network_data.net_work_info.signal_strength = ind->hdr_sig_info.rssi;

                    log_nw("hdr_sig_info: rssi=%d, ecio=%d, sinr=%d, io=%d\n",
                            ind->hdr_sig_info.rssi,
                            ind->hdr_sig_info.ecio,
                            ind->hdr_sig_info.sinr,
                            ind->hdr_sig_info.io);
                }
                break;
            }
        case NW_IND_CELL_ACCESS_STATE_CHG_EVENT_IND_FLAG:{
                QL_MCM_NW_CELL_ACCESS_STATE_EVENT_IND_T *ind = (void*)ind_msg_buf;
                log_nw("Recv event indication : CELL ACCESS STATE EVENT\n");
                log_nw("state = %d\n", ind->state);
                break;
            }
        case NW_IND_NITZ_TIME_UPDATE_EVENT_IND_FLAG:{
                QL_MCM_NW_NITZ_TIME_EVENT_IND_T *ind = (void*)ind_msg_buf;
                log_nw("Recv event indication : NITZ TIME EVENT\n");
                log_nw("nitz_time=%s, abs_time=%lld, leap_sec=%d, \n",
                        ind->info.nitz_time, ind->info.abs_time, ind->info.leap_sec);
                break;
            }
        default:
            break;
    }
}

int hostname_to_ip(char * hostname , char* ip){
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ( (he = gethostbyname( hostname ) ) == NULL){
        return -1;
    }

    addr_list = (struct in_addr **) he->h_addr_list;

    for(i = 0; addr_list[i] != NULL; i++){
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
    return -1;
}

bool validateIpAddress(char* ipAddress){
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

void Push_TCP_Data(tcp_mail_queue_typedef obj){
	log_nw("PUSH QUEUE TRACE = %d, DATA = %d, LEN = %d \n", obj.trace, obj.Data_Type, obj.size);

	if(obj.size > TCP_MAX_LENGTH) return;

	if(!enqueue(&queue_tcp, &obj, sizeof(tcp_mail_queue_typedef))){
		log_nw("Queue tcp full\n");
	}
}

void Push_SERVER_Data(sv_mail_queue_typedef obj){
	if(obj.size > SERVER_MAX_LENGTH) return;
	if(!enqueue(&queue_server, &obj, sizeof(sv_mail_queue_typedef))){
		log_nw("Queue server full\n");
	}
}

int Send_SMS(char* phone_number, char* content){
	int ret = 0;
	int                 i       = 0;
	int                 len     = 0;
	E_QL_SMS_FORMAT_T   e_format = 0;
	ql_sms_info_t       *pt_sms_info = NULL;

	if(h_sms == 0){
		return -1;
	}

	if(strlen(phone_number) < 8) return -1;

	pt_sms_info = (ql_sms_info_t*)malloc(sizeof(ql_sms_info_t));
	if(pt_sms_info == NULL){
		log_nw("SMS Malloc fail!\n");
		return -1;
	}
	memset(pt_sms_info, 0, sizeof(ql_sms_info_t));
	memcpy(pt_sms_info->src_addr, phone_number, strlen(phone_number));
	memcpy(pt_sms_info->sms_data, content, strlen(content));
	pt_sms_info->sms_data_len = strlen(content);
	pt_sms_info->format = E_QL_SMS_FORMAT_IRA;

	ret = QL_SMS_Send_Sms(h_sms, pt_sms_info);
	log_info("Send sms to phone number: %s", pt_sms_info->src_addr);
	if(ret < 0){
		return -1;
	}

	free(pt_sms_info);
	return 0;
}

static void server_config(u8 *msg){
	u8 *server_message=NULL;
	eNumCMDCode cmdx = 0;
	server_message = strstr(msg,"_");
	server_message ++;
	cmdx = device_setting(server_message, SERVER_SET);
}

static u8 check_phone_number(char* phone){
	if(((strlen(SystemSetting.data.Phone1) <= 5) && (strlen(SystemSetting.data.Phone2) <= 5)) || ((str_find(SystemSetting.data.Phone1, phone + 3) >= 0) ||	(str_find(SystemSetting.data.Phone2, phone + 3) >= 0))){
		return 1;
	}else{
		return 0;
	}
}

static void send_log_sms(char * Phone, char * content){
	tcp_mail_queue_typedef buf;
	u16 tcp_len;
	u8 crc = 0, tmp = 0;

	/* Frame header */
	buf.data[0] = 0xF0;
	buf.data[1] = 0xF0;
	/* Device ID */
	buf.data[2] =  SystemSetting.data.DeviceID & 0xFF;
	buf.data[3] = (SystemSetting.data.DeviceID >> 8) & 0xFF;
	buf.data[4] = (SystemSetting.data.DeviceID >> 16) & 0xFF;
	buf.data[5] = (SystemSetting.data.DeviceID >> 24) & 0xFF;
	buf.data[6] = (SystemSetting.data.DeviceID >> 32) & 0xFF;
	buf.data[7] = (SystemSetting.data.DeviceID >> 40) & 0xFF;
	buf.data[8] = (SystemSetting.data.DeviceID >> 48) & 0xFF;
	buf.data[9] = (SystemSetting.data.DeviceID >> 56) & 0xFF;
	/* Version */
	buf.data[10] = SMS_CONFIG_DEV;
	//11-12 -> data len
	/* Date Time*/
	*(int32_t*)(buf.data + 13) = GPS_Data.timestamp;

	tcp_len = 17;

	/* phone number */
	tmp = strlen(Phone);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, Phone, tmp);
	tcp_len += tmp;

	/* sms content */
	tmp = strlen(content);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, content, tmp);
	tcp_len += tmp;

	/* Length data */
	tcp_len += 1;//crc len
	buf.data[11] = tcp_len & 0xFF;
	buf.data[12] = tcp_len >> 8;

	/* CRC */
	crc = CRC_8BitsCompute(buf.data, tcp_len - 1);
	buf.data[tcp_len - 1] = crc;

	buf.size = tcp_len;
	buf.Data_Type = SMS_CONFIG_DEV;
	buf.trace = 0;

	Push_TCP_Data(buf);
}

static int SMS_parser_content(char* content){
	int idx = 0;
	int jdx = 0;
	int kdx = 0;

	if(str_find(content, "BK") >= 0){
		idx = char_index_of(content, '_', 1);
		if(idx < 0) return -1;
		jdx= char_index_of(content, '_', 2);
		if(idx < 0) return -1;

		/* check password lenght */
		if(jdx - (idx + 1) >= MAX_SMS_PASSWORD_LEN) return -1;

		sprintf(DeviceSetting.recover_pass, "bk10%d\0", ((u8)GPS_Data.gps_time->tm_mday ^ (u8)GPS_Data.gps_time->tm_mon));

		if(!strncmp(content + idx + 1, DeviceSetting.recover_pass, strlen(DeviceSetting.recover_pass))){
			DeviceSetting.recover_active = true;
		}

		//log_nw("Gia tri PASS: %s\n", DeviceSetting.recover_pass);

		if(!strncmp(content + idx + 1, SystemSetting.data.PassWord, strlen(SystemSetting.data.PassWord)) || (DeviceSetting.recover_active == true)){
			if(str_find(content, "BKSET") >= 0){
				kdx = char_index_of(content, '_', 3);
				if(kdx < 0) return -1;
				SMSData.CommandCode = device_setting((uint8_t*)(content + jdx + 1), SMS_SET);
				return 0;
			}else if(str_find(content, "BKGET") >= 0){//BKGET_XXXXXX_A_B
				SMSData.CommandCode = atoi(content + jdx + 1);
				return 0;
			}
		}else{
			SMSData.CommandCode = PASSWORD_FAILED;
			return 0;
		}
	}else{
		return -1;
	}
}

char phone_fwd_number[QL_SMS_MAX_ADDR_LENGTH];
u8 time_fwd_sms = 0;
static void sms_handler( QL_SMS_MsgRef   msgRef,
                            void*               contextPtr){
	int i = 0;
	u8 len_phonenum = strlen(msgRef->src_addr);

    for(i = 0; i < msgRef->sms_data_len; i++){
        if(msgRef->sms_data[i] == 0x11)
        	msgRef->sms_data[i] = 0x5F;
    }
    log_info("Rcv Sms: %s length: %d\nContent: %s\n", msgRef->src_addr ,msgRef->sms_data_len, msgRef->sms_data);

	//send log to server
	send_log_sms(msgRef->src_addr, msgRef->sms_data);

	if(time_fwd_sms != 0){
		Send_SMS(phone_fwd_number, msgRef->sms_data);
		return;
	}

    DeviceSetting.sms_enset = check_phone_number(msgRef->src_addr);

	if(SMS_parser_content(msgRef->sms_data) == 0){
		if(SMSData.CommandCode == CMD_FWD_SMS){//forward tin nhan trong vong 2 phut
			memset(phone_fwd_number, 0x0, QL_SMS_MAX_ADDR_LENGTH);
			strcpy(phone_fwd_number, msgRef->src_addr);
			time_fwd_sms = 120;
		}

		if(SMSData.CommandCode != NOT_RESPONSE_CMD){
			memset(SMSData.rep, 0 , QL_SMS_MAX_MT_MSG_LENGTH);
			strcpy(SMSData.rep, get_device_setting(SMSData.CommandCode, SMS_SET));
			Send_SMS(msgRef->src_addr, SMSData.rep);
		}
		DeviceSetting.recover_active = false;
		DeviceSetting.sms_enset = false;
	}
}

int flg_device_warning = -1;
bool check_internet(void){
	char text[320];
    struct hostent *he;

	if ((he = gethostbyname( "dvbk.vn" ) ) == NULL){
    	log_nw("There is no internet connection\n");
        return false;
    }

	if((SystemSetting.data.DeviceID == 0) && (GPS_Data.tm_up > 600)){
		if(flg_device_warning < 0){

			get_location(text);

			flg_device_warning = Send_SMS(WARNING_PHONE_NUMBER, text);

			log_system("WARNING!!! DEVICEID = 0\n");
		}
	}

	if(flg_rep_call_check_dev_info != 0){
		get_device_info(text);
		flg_rep_call_check_dev_info = Send_SMS(WARNING_PHONE_NUMBER, text);
		log_system("SEND DEVICE INFO\n");
	}

    log_nw("There is internet connection\n");
    return true;
}

int lan_port_init(void){
	char sptr[120];

	ql_lan_dhcp_config_s lan_config;
	memset(&lan_config, 0, sizeof(lan_config));

	ql_sgmii_disable();

	usleep(100000);

	if(ql_sgmii_enable() == -1) {
		log_system("\nEnable SGMII Module failure\n");
		return -1;
	}else{
		log_system("\nEnable SGMII Module Success\n");
	}
	ql_sgmii_autoneg_set(0);

	if(ql_sgmii_speed_set(QL_SGMII_SPEED_100MHZ) == -1) {
		log_system("\nSET SGMII 100MHZ failure\n");
	}else{
		log_system("\nSET SGMII 100MHZ Success\n");
	}

	if(ql_sgmii_duplex_set(QL_SGMII_DUPLEX_FULL) == -1) {
		log_system("\nSET DUPLEX_FULL failure\n");
	}else{
		log_system("\nSET DUPLEX_FULL Success\n");
	}

	strcpy(sptr, "192.168.1.1\0");
	if(-1 != quectel_ipv4_check(sptr)) {
		strcpy(lan_config.gw_ip, sptr);
	}else{
		log_system("%s : error format  \n\n", sptr );
	}

	strcpy(sptr, "255.255.255.0\0");
	if(-1 != quectel_ipv4_check(sptr)) {
		strcpy(lan_config.netmask, sptr);
	}else{
		log_system("%s : error format  \n\n", sptr );
	}

	strcpy(sptr, "192.168.1.20\0");
	if(-1 != quectel_ipv4_check(sptr)) {
		strcpy(lan_config.dhcp_start_ip, sptr);
	}else{
		log_system("%s : error format  \n\n", sptr );
	}

	strcpy(sptr, "192.168.1.30\0");
	if(-1 != quectel_ipv4_check(sptr)) {
		strcpy(lan_config.dhcp_end_ip, sptr);
	}else{
		log_system("%s : error format  \n\n", sptr );
	}

	lan_config.enable_dhcp = 1;
	lan_config.lease_time = 86400;

	if(QL_LAN_DHCP_Config_Set(lan_config) != -1) {
		log_system("LAN Config Success!\n");
	}else{
		log_system("LAN Config Failure!\n");
	}

	memset(&lan_config, 0, sizeof(lan_config));
	if(QL_LAN_DHCP_Config_Get(&lan_config) != -1) {
		log_system("Gateway IP :%s\n", lan_config.gw_ip);
		log_system("Subnet Mask :%s\n", lan_config.netmask);
		log_system("DHCP Start IP :%s\n", lan_config.dhcp_start_ip);
		log_system("DHCP End IP :%s\n", lan_config.dhcp_end_ip);
		log_system("LAN Enable :%d\n", lan_config.enable_dhcp);
		log_system("DHCP lease time :%d\n", lan_config.lease_time);
	}else{
		log_system("-----------------------Get LAN Failure!------------------------------\n");
	}

	return 0;
}

static ql_data_call_s g_call = {
	.profile_idx = 1,
	.reconnect = true,
	.ip_family = QL_DATA_CALL_TYPE_IPV4,
};

static bool g_cb_recv = false;
static void data_call_state_callback(ql_data_call_state_s *state){
	//interface_name
	if(g_call.profile_idx == state->profile_idx){
		if(state->state == QL_DATA_CALL_CONNECTED){
			strncpy(interface_name, state->name, sizeof(interface_name));
			if(state->ip_family == QL_DATA_CALL_TYPE_IPV4){
				log_nw("\tIP address:          %s\n", inet_ntoa(state->v4.ip));
				log_nw("\tGateway address:     %s\n", inet_ntoa(state->v4.gateway));
				log_nw("\tPrimary DNS address: %s\n", inet_ntoa(state->v4.pri_dns));
				log_nw("\tSecond DNS address:  %s\n", inet_ntoa(state->v4.sec_dns));
			}
			g_cb_recv = true;
		}
	}
	return;
}

static int get_addr_by_ifname(const char *ifname, char *ipaddr_str, int ipaddr_len){
	int ret = -1;
	struct sockaddr_in *addr = NULL;
	struct ifreq ifr;

	if(ifname == NULL) return -1;

	memset(&ifr, 0, sizeof(struct ifreq));

	int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if(fd == -1){
		log_nw("get_addr_by_ifname socket error.\n");
		return -1;
	}

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
	if ((ret = ioctl(fd, SIOCGIFADDR, &ifr)) != 0){
		goto OUT;
	}

	addr = (struct sockaddr_in *)&ifr.ifr_addr;
	strncpy(ipaddr_str, inet_ntoa(addr->sin_addr), ipaddr_len);

	ret = 0;
OUT:
	close(fd);
	return ret;
}

static int QL_Start_Data_Call(void){
	int ret, count = 20, flag = 0;
	ql_data_call_error_e err = QL_DATA_CALL_ERROR_NONE;

	ret = QL_Data_Call_Start(&g_call, &err);

	if(ret != MCM_SUCCESS_V01){
		QL_Data_Call_Destroy();
		log_nw("NEW ql_data_call_start failed. ret=%d, err=%d\n", ret, err);
		return -1;
	}

	log_nw("NEW ql_data_call_start success. ret=%d\n", ret);

	while(count--){
		sleep(1);
		if(g_cb_recv){
			flag = 1;
			break;
		}
	}
	if(flag != 1){
		log_nw("NEW  ql_data_call_start callback return failed or timeout\n");
		return -1;
	}

	log_nw("NEW  ql_data_call_start callback success \n");
	return 0;
}

//QL_Data_Call_Stop
static int QL_Stop_Data_Call(void){
	int ret;
	char addr[16] = {0};
	ql_data_call_error_e err = QL_DATA_CALL_ERROR_NONE;

	ret = ql_data_call_stop(g_call.profile_idx, g_call.ip_family, &err);

	if(ret != MCM_SUCCESS_V01){
		log_nw("ql_data_call_stop failed.ret=%d\n", ret);
		return -1;
	}

	sleep(3);

	if(strlen(interface_name) > 0){
		ret = get_addr_by_ifname(interface_name, addr, sizeof(addr));
		if(ret == 0){
			log_nw("ql_data_call_stop failed, data call already connected");
			return -1;
		}
	}else{
		log_nw("interface_name length is 0.\n");
	}

	return 0;
}

static void* TCP_Handler (void* arg);
static pthread_t thrd_tcp_handler;

static void* SVR_Handler (void* arg);
static pthread_t thrd_svr_handler;

void get_signal_strength(void){
	QL_MCM_NW_SIGNAL_STRENGTH_INFO_T    t_info = {0};
	int ret = 0;

	if(nw_handler_id == 0) return;

	memset(&t_info, 0, sizeof(QL_MCM_NW_SIGNAL_STRENGTH_INFO_T));
	ret = QL_MCM_NW_GetSignalStrength(nw_handler_id, &t_info);
	log_nw("QL_MCM_NW_GetSignalStrength => nw_handler_id = %d, ret = %d, detail info:\n", nw_handler_id, ret);

	if(t_info.gsm_sig_info_valid){
    	network_data.net_work_info.radio_tech = NW_MODE_GSM;
    	network_data.net_work_info.signal_strength = t_info.gsm_sig_info.rssi;

		log_nw("gsm_sig_info: rssi=%d\n",
				t_info.gsm_sig_info.rssi);
	}

	if(t_info.wcdma_sig_info_valid){
    	network_data.net_work_info.radio_tech = NW_MODE_WCDMA;
    	network_data.net_work_info.signal_strength = t_info.wcdma_sig_info.rssi;

		log_nw("wcdma_sig_info: rssi=%d, ecio=%d\n",
				t_info.wcdma_sig_info.rssi,
				t_info.wcdma_sig_info.ecio);
	}
	if(t_info.tdscdma_sig_info_valid){
    	network_data.net_work_info.radio_tech = NW_MODE_TDSCDMA;
    	network_data.net_work_info.signal_strength = t_info.tdscdma_sig_info.rssi;

		log_nw("tdscdma_sig_info: rssi=%d, rscp=%d, ecio=%d, sinr=%d\n",
				t_info.tdscdma_sig_info.rssi,
				t_info.tdscdma_sig_info.rscp,
				t_info.tdscdma_sig_info.ecio,
				t_info.tdscdma_sig_info.sinr);
	}
	if(t_info.lte_sig_info_valid){
    	network_data.net_work_info.radio_tech = NW_MODE_LTE;
    	network_data.net_work_info.signal_strength = t_info.lte_sig_info.rssi;

		log_nw("lte_sig_info: rssi=%d, rsrq=%d, rsrp=%d, snr=%d\n",
				t_info.lte_sig_info.rssi,
				t_info.lte_sig_info.rsrq,
				t_info.lte_sig_info.rsrp,
				t_info.lte_sig_info.snr);
	}
	if(t_info.cdma_sig_info_valid){
    	network_data.net_work_info.radio_tech = NW_MODE_CDMA;
    	network_data.net_work_info.signal_strength = t_info.cdma_sig_info.rssi;

		log_nw("cdma_sig_info: rssi=%d, ecio=%d\n",
				t_info.cdma_sig_info.rssi,
				t_info.cdma_sig_info.ecio);
	}
	if(t_info.hdr_sig_info_valid){
    	network_data.net_work_info.radio_tech = NW_MODE_HDR;
    	network_data.net_work_info.signal_strength = t_info.hdr_sig_info.rssi;
		log_nw("hdr_sig_info: rssi=%d, ecio=%d, sinr=%d, io=%d\n",
				t_info.hdr_sig_info.rssi,
				t_info.hdr_sig_info.ecio,
				t_info.hdr_sig_info.sinr,
				t_info.hdr_sig_info.io);
	}
}

int time_get_strength = 0;
int reconnect = 0;
int time_to_check_internet = 0;
int data_call_failed_cnt = 0;
int time_wait_internet = 0;
void* thread_network(void* arg){
	int iret = 0;

	memset(&network_data, 0x0, sizeof(network_data));

	if (pthread_create(&thrd_tcp_handler, NULL, TCP_Handler, NULL) != 0){
		log_system("Fail to create thread send tcp data !\n");
	}

	if (pthread_create(&thrd_svr_handler, NULL, SVR_Handler, NULL) != 0){
		log_system("Fail to create thread TCP server receiver !\n");
	}

	if (pthread_create(&thrd_check_internet_timeout_handler, NULL, Check_internet_timeout_Handler, NULL) != 0){
		log_system("Fail to create thread check internet timeout !\n");
	}
	for(;;){
	//	log_nw("m_nw_state = %d \n", m_nw_state);

		switch(m_nw_state){
			case STATE_NW_GET_SIMSTATE:{//0 check sim status
				iret = QL_MCM_SIM_Client_Init(&h_sim);
				//log_nw("QL_MCM_SIM_Client_Init ret = %d with h_sim=%d\n", iret, h_sim);

	            memset(DeviceSetting.ICCID, 0x0, MAX_SIZE_ICCID + 1);
	            iret = QL_MCM_SIM_GetICCID(h_sim, E_QL_MCM_SIM_SLOT_ID_1, DeviceSetting.ICCID, MAX_SIZE_ICCID);
	            if(strlen(DeviceSetting.ICCID) == 20) DeviceSetting.ICCID[19] = 0;
	            log_system("\nICCID: %s\n", DeviceSetting.ICCID);

				memset(current_operator, 0x0, 128);
				MobileCountryCode = 0;

				network_data.sim_status =  QL_SIM_GetState(E_QL_SIM_EXTERNAL_SLOT_1);

				log_nw("m_nw_state = STATE_NW_GET_SIMSTATE -> %d\n", network_data.sim_status);

				if(network_data.sim_status == E_QL_SIM_STAT_UNKNOWN){
					network_data.sim_status = E_QL_SIM_STAT_NOT_INSERTED;
				}

				if(E_QL_SIM_STAT_READY == network_data.sim_status){
		            QL_SIM_APP_ID_INFO_T    t_info;

		            memset(DeviceSetting.IMSI, 0x0, MAX_SIZE_IMSI + 1);
		            t_info.e_slot_id    = E_QL_MCM_SIM_SLOT_ID_1;
		            t_info.e_app        = E_QL_MCM_SIM_APP_TYPE_3GPP;
		            iret = QL_MCM_SIM_GetIMSI(h_sim, &t_info, DeviceSetting.IMSI, MAX_SIZE_IMSI);
		            log_nw("QL_MCM_SIM_GetIMSI ret = %s, IMSI: %s\n", errorcode[iret], DeviceSetting.IMSI);

					log_nw("-->SIM STAT READY<--\n");
					m_nw_state = STATE_NW_CONFIG_HANDLER;
				}else{
					network_data.internet_status = false;
					log_nw("-->SIM STAT NOT READY<--\n");
					sleep(2);
				}
				break;
			}
			case STATE_NW_CONFIG_HANDLER:{//1
				QL_MCM_NW_CONFIG_INFO_T t_info   = {0};

				log_nw("m_nw_state = STATE_NW_CONFIG_HANDLER \n");
				iret = QL_MCM_NW_Client_Init(&nw_handler_id);
				log_nw("QL_MCM_NW_Client_Init => nw_handler_id = %d, ret = %d\n", nw_handler_id, iret);

				if(!iret){
					//mask 16_PRL | TDSCDMA | LTE | EVDO | CDMA | WCDMA | GSM
					t_info.preferred_nw_mode = 0x7F;
					t_info.roaming_pref = E_QL_MCM_NW_ROAM_STATE_ON;
		            iret = QL_MCM_NW_SetConfig(nw_handler_id, &t_info);
		            log_nw("QL_MCM_NW_SetConfig ret = %d\n", iret);

		            //register event handler
		            iret = QL_MCM_NW_AddRxMsgHandler(nw_handler_id, nw_event_ind_handler, (void*)NULL);
		            log_nw("QL_MCM_NW_AddRxMsgHandler, ret=%d\n", iret);

		            //register event type
		            iret = QL_MCM_NW_EventRegister(nw_handler_id, 0x04);
		            log_nw("QL_MCM_NW_EventRegister ret = %d\n", iret);
				}else{
					log_system("QL_MCM_NW_Client_Init failed\n");
				}
				m_nw_state = STATE_NW_QUERY_STATE;
				break;
			}
			case STATE_NW_QUERY_STATE:{//2
				int value = 0, value1 = 0;
				E_QL_NW_RADIO_ACCESS_TYPE_T rat;
				E_QL_WWAN_NET_REG_STATE_T state;
				QL_MCM_NW_OPERATOR_NAME_INFO_T  t_info = {0};

				log_nw("m_nw_state = STATE_NW_QUERY_STATE \n");

				iret = QL_NW_GetRegState(&rat, &state, &value, &value1);
				if(iret != 0){
					log_nw("Failed to QL_NW_GetRegState, iret=%d", iret);
				}

				if((state == E_QL_WWAN_REG_REGISTERED_HOME_NETWORK) || (state == E_QL_WWAN_REG_ROAMING)){
					log_nw("Network resgistered, state = %d  -> Query internet\n", state);
					m_nw_state = STATE_INTERNET_QUERY_STATE;
					current_nw_mode = state;
				}else{
					sleep(3);
					//reboot after x minute
				}

				iret = QL_NW_AddRegStateEventHandler(ql_nw_regstate_cb_func, (void*)NULL);
				log_nw("QL_NW_AddRegStateEventHandler ret %d\n", iret);

				iret = QL_MCM_NW_GetOperatorName(nw_handler_id, &t_info);
				if(!iret){
					 strcpy(current_operator, t_info.long_eons);
					 MobileCountryCode = ec21_atoi(t_info.mcc);
					 log_system("OperatorName: %s,  Mobile Country Code: %d\n", current_operator, MobileCountryCode);
				}

				break;
			}
			case STATE_INTERNET_QUERY_STATE:{//3
				log_nw("m_nw_state = STATE_INTERNET_QUERY_STATE \n");
				network_data.internet_status = check_internet();
				if(network_data.internet_status){
					m_nw_state = STATE_INTERNET_READY;
					time_wait_internet = 0;
					break;
				}
				time_wait_internet ++;
				if(time_wait_internet > 10){
					time_wait_internet = 0;
				m_nw_state = STATE_WAN_INITIALIZE;
				}
				break;
			}
			case STATE_WAN_INITIALIZE:{//4
				//check data call service ready
				iret =  QL_Data_Call_Init_Precondition();
				log_nw("****QL_Data_Call_Init_Precontition RET = %d \n", iret);
				if(!iret){
					iret = QL_Data_Call_Init(data_call_state_callback);
					if(iret != MCM_SUCCESS_V01){
						QL_Data_Call_Destroy();

						log_nw("QL_Data_Call_Init failed ->ret=%d\n", iret);
						m_nw_state = STATE_NW_QUERY_STATE;
						break;
					}
					log_nw("NEW QL_Data_Call_Init -> OK.\n");
					m_nw_state = STATE_WAN_START;
				}else{
					QL_Stop_Data_Call();
					QL_Data_Call_Destroy();

					m_nw_state = STATE_NW_GET_SIMSTATE;
				}
				break;
			}
			case STATE_WAN_START:{//6
				g_cb_recv = false;
				if(!QL_Start_Data_Call()){
					sleep(3);
					m_nw_state = STATE_INTERNET_QUERY_STATE;
					data_call_failed_cnt = 0;
				}else{
					if(data_call_failed_cnt++ > 20){
						data_call_failed_cnt = 0;
						log_nw("???????????????????????????Data_Call_Init failed ???????????????????? \n");
						sleep(3);
					}
					sleep(3);
				}
				break;
			}
			case STATE_WAN_DEACTIVATE:{//7
				log_nw("m_nw_state = STATE_WAN_DEACTIVATE \n");

				QL_Stop_Data_Call();
				QL_Data_Call_Destroy();

				m_nw_state = STATE_NW_GET_SIMSTATE;
				break;
			}
			case STATE_INTERNET_READY:{//8
				time_to_check_internet++;
				ec21_update_firmware();
				mcu_update_firmware();
				check_file_missing();
				if(time_to_check_internet > 60){
					time_to_check_internet = 0;
					/* check new firmware version and update */
					network_data.internet_status = check_internet();
					if(!network_data.internet_status){
						m_nw_state = STATE_WAN_DEACTIVATE;
					}
				}
				break;
			}
		}
		//setting sim detect
		config_sim_detect();
		//config audio codec
		config_tlv_codec();
		//voice call init
		voice_call_init();
		//init sms
		sms_init();

		time_get_strength++;
		if(time_get_strength > 20){
			time_get_strength = 0;
			get_signal_strength();
		}
		sleep(1);
	}
}

u8 count_error__ = 0;
enumTCP_result send_tcp_data(tcp_mail_queue_typedef buf){
	u16 time_out = 0;
	int iret = 0;
	sv_mail_queue_typedef tmp;

	/* clear queue server before send data */
	while(!isEmpty(&queue_server)){
		dequeue(&queue_server, &tmp, sizeof(sv_mail_queue_typedef));
		if(str_find(tmp.data, "@@SET_") >= 0){ //SET_04_180
			u8 crc = CRC_8BitsCompute(tmp.data, tmp.size - 1);
			if(crc == tmp.data[tmp.size - 1]){
				tmp.data[tmp.size - 1] = 0;
				server_config(tmp.data);///333
			}else{
				log_nw("MESSAGE FROM SERVER ERROR CRC!!!! \n");
			}
		}
	}

	iret = send(TCP_sockfd, buf.data, buf.size, 0);
	if(iret != buf.size){
		return DATA_SEND_ERROR;
	}else{
		while(isEmpty(&queue_server) && (time_out < 100)){
			time_out += 1;
			usleep(100000);
		}

		if(time_out >= 100){
			return DATA_TIMEOUT;
		}

		memset(&tmp, 0x0, sizeof(sv_mail_queue_typedef));
		dequeue(&queue_server, &tmp, sizeof(sv_mail_queue_typedef));
		tmp.data[tmp.size] = 0;

		if(SystemSetting.data.certified == 0){
			log_nw("PASSWORD FROM SERVER -> %s \n", tmp.data);
			/* check password from server */
			if(str_find(tmp.data, SystemSetting.data.KeyActive) >= 0){
				SystemSetting.data.certified = 1;
				write_number(CERTIFIED_PATH, (u64)SystemSetting.data.certified);
				return CERT_ACK_OK;
			}
			return CERT_ACK_ERROR;
		}

		if(str_find(tmp.data, "OK") >= 0){
			return DATA_DELIVERED;
		}

		if(str_find(tmp.data, "ERROR") >= 0){
			return DATA_ERROR;
		}

		if(str_find(tmp.data, "@@SET_") >= 0) {
			u8 crc = CRC_8BitsCompute(tmp.data, tmp.size - 1);

			if(crc == tmp.data[tmp.size - 1]){
				tmp.data[tmp.size - 1] = 0;
				server_config(tmp.data);//111
			}else{
				log_nw("MESSAGE FROM SERVER ERROR CRC!!!! \n");
			}
		}

		return DATA_UNKNOWN;
	}
}

static enumTCP_result request_key_active(void){
	tcp_mail_queue_typedef buf;
	u16 tcp_len;
	u8 crc = 0;

	/* Frame header */
	buf.data[0] = 0xF0;
	buf.data[1] = 0xF0;
	/* Device ID */
	buf.data[2] =  SystemSetting.data.DeviceID & 0xFF;
	buf.data[3] = (SystemSetting.data.DeviceID >> 8) & 0xFF;
	buf.data[4] = (SystemSetting.data.DeviceID >> 16) & 0xFF;
	buf.data[5] = (SystemSetting.data.DeviceID >> 24) & 0xFF;
	buf.data[6] = (SystemSetting.data.DeviceID >> 32) & 0xFF;
	buf.data[7] = (SystemSetting.data.DeviceID >> 40) & 0xFF;
	buf.data[8] = (SystemSetting.data.DeviceID >> 48) & 0xFF;
	buf.data[9] = (SystemSetting.data.DeviceID >> 56) & 0xFF;
	/* Version */
	buf.data[10] = REQUEST_CERT_KEY;
	/* Version */

	/* Length data */
	tcp_len = 14;
	buf.data[11] = tcp_len & 0xFF;
	buf.data[12] = tcp_len >> 8;

	/* CRC */
	crc = CRC_8BitsCompute(buf.data, 13);
	buf.data[13] = crc;

	buf.size = tcp_len;
	buf.Data_Type = REQUEST_CERT_KEY;
	buf.trace = 0;

	log_nw(".....SEND REQUEST PASSWORD....................... \n");

	return send_tcp_data(buf);
}

void* TCP_Handler(void* arg){
	int iret = 0;
	static useconds_t sleep__ = 1000000;
	for(;;){
		sleep__ = 1000000;
		switch(m_tcp_state){
			case STATE_WAIT_INTERNET:{//0
				log_nw("m_tcp_state = STATE_WAIT_INTERNET \n");
				if(check_internet()){
					m_tcp_state = STATE_SOC_CONNECT;
				}else if(m_nw_state == STATE_INTERNET_READY){
					m_nw_state = STATE_WAN_DEACTIVATE;
				}
				sleep__ = 5000000;
				break;
			}
			case STATE_SOC_CONNECT:{//1
				log_nw("m_tcp_state = STATE_SOC_CONNECT \n");
				char domain[MAX_SIZE_URL] = {0};
				char ip_addr[64];
				strcpy(domain, SystemSetting.data.domain);
				int server_port = SystemSetting.data.port;

				if(validateIpAddress(domain)){//ip = 1
					log_nw("This is ip address -> %s:%d\n", domain, server_port);
					strcpy(ip_addr, domain);
				}else{//host = 0
					log_nw("This is domain name -> %s:%d\n", domain, server_port);
					hostname_to_ip(domain, ip_addr);
				}

				struct sockaddr_in server_address;
				memset(&server_address, 0, sizeof(server_address));
				server_address.sin_family = AF_INET;

				// creates binary representation of server name
				// and stores it as sin_addr
				inet_pton(AF_INET, ip_addr, &server_address.sin_addr);

				// htons: port in network order format
				server_address.sin_port = htons(server_port);

				// open a stream socket
				if ((TCP_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
					log_nw("could not create socket\n");

					m_tcp_state = STATE_WAIT_INTERNET;

					break;
				}

				/* set tcp connect time out */
				struct timeval socket_timeout;
				socket_timeout.tv_sec  = 10;  // after 10 seconds connect() will timeout
				socket_timeout.tv_usec = 0;
			    setsockopt(TCP_sockfd, SOL_SOCKET, SO_SNDTIMEO, &socket_timeout, sizeof(socket_timeout));
			    socket_timeout.tv_sec  = 5;  // after 5 seconds receiver will timeout
			    setsockopt(TCP_sockfd, SOL_SOCKET, SO_RCVTIMEO, &socket_timeout, sizeof(socket_timeout));

				// TCP is connection oriented, a reliable connection
				// **must** be established before any data is exchanged
				if(connect(TCP_sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
					log_nw("could not connect to server\n");
					reconnect ++;
					if((reconnect % 5 == 0) && (reconnect > 0)){
						log_nw("reconnect to server -> %d\n", reconnect);
						m_tcp_state = STATE_WAIT_INTERNET;
					}else if(reconnect > 50){
						log_nw("reconnect to server -> %d\n", reconnect);
						reconnect = 0;
						//LOG
					}
					break;
				}

			    log_nw("Connect OK, sockfd=%d\n", TCP_sockfd);

				m_tcp_state = STATE_SOC_SEND;
			    log_nw("m_tcp_state = STATE_SOC_SEND \n");
			    break;
			}
			case STATE_SOC_SEND:{//2
				enumTCP_result send_result;

				if(SystemSetting.data.certified == 1){
					if(!isEmpty(&queue_tcp)){
						tcp_mail_queue_typedef buf;

						peekqueue(&queue_tcp, &buf, sizeof(tcp_mail_queue_typedef));

						log_nw("<-- SENDING DATA(LEN=%d) -->\n", buf.size);

						/* Send to server via socket */
						send_result = send_tcp_data(buf);

						freequeue(&queue_tcp);

						switch(send_result){
							case DATA_DELIVERED:
								if((buf.Data_Type == REAL_TIME_DATA) || (buf.Data_Type == BACKUP_GPS_DATA)){
									if(clear_flg_send(buf.trace) < 0){
										log_nw("<-- CLEAR FLG SEND ERROR !!! --> \n");
									}
								}
								log_nw("<-- SEND TCP DATA SUCCESS --> \n");
								sleep__ = 100000;
								break;
							case DATA_ERROR:
								log_nw("<-- RESPOSER FROM SERVER -> ERROR --> \n");
								//count error and reconnect
								break;
							case DATA_TIMEOUT:
								m_tcp_state = STATE_TCP_RECONNECT;
								log_nw("<-- SEND TCP DATA TIMEOUT --> \n");
								break;
							case DATA_SEND_ERROR:
								m_tcp_state = STATE_TCP_RECONNECT;
								log_nw("<-- SEND TCP DATA ERROR !!! --> \n");
								break;
							case DATA_UNKNOWN:
								log_nw("<-- RESPOSER FROM SERVER -> UNKNOWN --> \n");
								break;
						}
					}
				}else{
					if(strlen(SystemSetting.data.KeyActive) > 10){
						if(request_key_active() != CERT_ACK_OK){
							sleep(10);
						}
					}else{
						SystemSetting.data.certified = 1;
						write_number(CERTIFIED_PATH, (u64)SystemSetting.data.certified);
					}
				}
				break;
			}
			case STATE_TCP_RECONNECT:{//3
				log_nw("m_tcp_state = STATE_TCP_RECONNECT \n");
				close(TCP_sockfd);
				TCP_sockfd = -1;
				m_tcp_state = STATE_WAIT_INTERNET;
				break;
			}
		}
		if(m_tcp_state == STATE_SOC_SEND){
			network_data.tcp_status = true;
		}else{
			network_data.tcp_status = false;
		}
		//delay
		usleep(sleep__);
	}
}

static u8 sv_rcv_buf[1024] = {0};
static void* SVR_Handler (void* arg){
	int len = 0;
	for(;;){
		if((m_tcp_state != STATE_SOC_SEND) || (TCP_sockfd < 0)){
			sleep(2);
			continue;
		}
		len = recv(TCP_sockfd, sv_rcv_buf, sizeof(sv_rcv_buf), 0);
		if(len > 0){
			log_nw("###News from server: %s\n", sv_rcv_buf);

			g_cb_recv = true;
			server_connect_loss_time = 0;

			if(sv_rcv_buf[0] == '>'){
				sv_mail_queue_typedef obj;

				memcpy(obj.data, sv_rcv_buf + 1, len - 1);
				obj.size = len - 1;

				Push_SERVER_Data(obj);

				//log_nw("TCP SERVER RESPONSE -> %s \n", rcv_buf);
			}
			if(str_find(sv_rcv_buf, "@@SET_") >= 0){
				u8 crc = CRC_8BitsCompute(sv_rcv_buf, len - 1);

				if(crc == sv_rcv_buf[len - 1]){
					log_nw("MESSAGE FROM SERVER CRC SUCCESS!!!! \n");
					sv_rcv_buf[len - 1] = 0;
					server_config(sv_rcv_buf);
				}else{
					log_nw("MESSAGE FROM SERVER ERROR CRC!!!! \n");
				}
			}else if(str_find(sv_rcv_buf, "@@RESET##") >= 0){
				reboot_camera();
				set_device_reset_timeout(10);
			}
			memset(sv_rcv_buf, 0, 1024);
		}else if(len == 0){
			m_tcp_state = STATE_TCP_RECONNECT;
			log_system("###TCP DISCONNECTED\n");
			sleep(1);
		}else if(len < 0){
			sleep(1);
		}
	}
}

u8 device_reboot_time = 0;
void set_device_reset_timeout(u8 time){
	mcu_send_with_cs(">REBOOT\0", strlen(">REBOOT\0"));
	device_reboot_time = time;
}

u8 get_device_reset_timeout(void){
	return device_reboot_time;
}

static u8 time_reconnect = 0;
static void* Check_internet_timeout_Handler (void* arg){
	int i=0;
	for(;;){
		server_connect_loss_time++;
		if(server_connect_loss_time > SystemSetting.data.time_reset_when_disconnect){
			log_system("############## TCP DISSCONNECTED SYSTEM RESET ##############\n");
			server_connect_loss_time = 0;
			//REBOOT EC21
			set_device_reset_timeout(30);
		}

		if(device_reboot_time != 0){
			device_reboot_time--;
			if(device_reboot_time == 0){
				log_system("################# ACCEPT SYSTEM RESET ###############\n");
				sleep(2);
				ec21_reboot();
			}
		}

		if(time_reconnect != 0){
			time_reconnect--;
			if(time_reconnect == 1){
				m_tcp_state = STATE_TCP_RECONNECT;
			}
		}

		if(time_fwd_sms != 0){
			time_fwd_sms--;
			if(time_fwd_sms == 1){
				memset(phone_fwd_number, 0x0, QL_SMS_MAX_ADDR_LENGTH);
			}
		}

		sleep(1);
	}
}

int check_ipaddress(char* ip) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr = {AF_INET, htons(80), inet_addr(ip)};
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));
	if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) != 0){
        return -1;
	}
	close(sockfd);
	return 0;
}

void sms_init(void){
	if(h_sms != 0) return;
	int ret;
	ret = QL_SMS_Client_Init(&h_sms);
	log_nw("QL_SMS_Client_Init ret=%d with h_sms=0x%x\n", ret, h_sms);
	if(ret != 0){
		h_sms = 0;
	}

	ret = QL_SMS_AddRxMsgHandler(sms_handler, (void*)h_sms);
	log_nw("QL_SMS_AddRxMsgHandler ret=%d \n", ret);
}

void tcp_reconnect(u8 time){
	if(time != 0){
		time_reconnect = time;
		return;
	}
	m_tcp_state = STATE_TCP_RECONNECT;
}

