#include "filter.h"
#include <ql_oe.h>

#include "mcu.h"
#include "gps.h"
#include "network.h"
#include "queue.h"
#include "setting.h"
#include "log.h"
#include "utils.h"
#include "database.h"
#include "devconfig.h"
#include "kalman.h"

#define FILTER_TIME      15
#define MAX_FILTER_TIME  120

u8 fuel_index = 0;

static u8  condition = 0;

//static void printf_arr(int arr[], u8 len){
//	u8 i;
//	for(i = 0; i < len; i++){
//		log_system("%d - ", arr[i]);
//	}
//	log_system("\n........................................\n");
//}

int fuel_1_datas[MAX_FILTER_TIME + 1];
int fuel_1_datas_sort[MAX_FILTER_TIME + 1];

int fuel_2_datas[MAX_FILTER_TIME + 10];
int fuel_2_datas_sort[MAX_FILTER_TIME + 1];

bool flg_full_load = FALSE;

static int kalman_filter_value_ad1 = 0;
static int kalman_filter_value_ad2 = 0;

static int this_value_ad1 = 0;
static int this_value_ad2 = 0;

static int prv_value_ad1 = 0;
static int prv_value_ad2 = 0;

static int raw_value_ad1 = 0;
static int raw_value_ad2 = 0;

//static u8 time_for_fast_filter = 0;
u8 count_ad1_change = 0;
u8 count_ad2_change = 0;
bool kalman_startup = false;
void add_fuel_data(int ad1, int ad2){
	raw_value_ad1 = ad1;
	raw_value_ad2 = ad2;

	if(!kalman_startup){
		kalman_startup = true;
		//log_system("KALMAN START\n");

		kalman_filter_value_ad1 = (int)updateEstimate_ad1((float)ad1);
		kalman_filter_value_ad1 = (int)updateEstimate_ad1((float)ad1);
		kalman_filter_value_ad1 = (int)updateEstimate_ad1((float)ad1);
		kalman_filter_value_ad1 = (int)updateEstimate_ad1((float)ad1);
		kalman_filter_value_ad1 = (int)updateEstimate_ad1((float)ad1);

		kalman_filter_value_ad2 = (int)updateEstimate_ad2((float)ad2);
		kalman_filter_value_ad2 = (int)updateEstimate_ad2((float)ad2);
		kalman_filter_value_ad2 = (int)updateEstimate_ad2((float)ad2);
		kalman_filter_value_ad2 = (int)updateEstimate_ad2((float)ad2);
		kalman_filter_value_ad2 = (int)updateEstimate_ad2((float)ad2);
	}else{
		kalman_filter_value_ad1 = (int)updateEstimate_ad1((float)ad1);
		kalman_filter_value_ad2 = (int)updateEstimate_ad2((float)ad2);
	}

	this_value_ad1 = fuel_1_datas[fuel_index] = kalman_filter_value_ad1;
	this_value_ad2 = fuel_2_datas[fuel_index] = kalman_filter_value_ad2;

	//log_system("-> INDEX %d,  AD1 => %d : %d, AD2 : %d : %d   ERR = %f : %f \n", fuel_index, ad1, this_value_ad1, ad2, this_value_ad2, getEstimateError_ad1(), getEstimateError_ad2());

	if(GPS_Data.gps_speed <= SystemSetting.data.RunLevel){//change ad when stop
		if(abs(this_value_ad1 - prv_value_ad1) >= 200){
			GPS_Data.stop_time = 0;
			//log_system("CHANGED AD 1 VALUE\n");
			count_ad1_change++;
			if(count_ad1_change > 5){
				count_ad1_change = 0;
//				if(SystemSetting.data.filter_level > 4){
//					time_for_fast_filter = (FILTER_TIME * SystemSetting.data.filter_level);
//				}
				prv_value_ad1 = this_value_ad1;
			}
		}else{
			count_ad1_change = 0;
		}

		if(abs(this_value_ad2 - prv_value_ad2) >= 200){
			GPS_Data.stop_time = 0;
			//log_system("CHANGED AD 2 VALUE  \n");
			count_ad2_change++;
			if(count_ad2_change > 5){
				count_ad2_change = 0;
				prv_value_ad2 = this_value_ad2;

//				if(SystemSetting.data.filter_level > 4){
//					time_for_fast_filter = (FILTER_TIME * SystemSetting.data.filter_level);
//				}
			}
		}else{
			count_ad2_change = 0;
		}
	}

	fuel_index++;
	if(fuel_index >= (FILTER_TIME * SystemSetting.data.filter_level)){
		fuel_index = 0;
		flg_full_load = true;
	}
}

void* thread_filter_handler(void* arg){
	u8 idx = 0;
	u8 num_rec = 0;
	u32 ad_1_sum = 0;
	u32 ad_2_sum = 0;

	/* init kalman */
	SimpleKalmanFilter_ad1(1, 1, 0.001);

	SimpleKalmanFilter_ad2(1, 1, 0.001);

	for(;;){
		sleep(5);
		/* check mcu connected */
		if(mcu_connected == FALSE){
			MCU_Data.fuel_232_1 = 0;
			MCU_Data.fuel_232_2 = 0;
			//log_system("MCU MESSAGE TIMEOUT!!!!! \n");
			continue;
		}

		/* check level filter */
		if(SystemSetting.data.filter_level == 0){//LEVEL 0
			MCU_Data.fuel_232_1 = raw_value_ad1;
			MCU_Data.fuel_232_2 = raw_value_ad2;

			//log_system("############ LV 0 -> NO FILTER AD1 = %d, AD2 = %d ############\n", MCU_Data.fuel_232_1, MCU_Data.fuel_232_2);
			continue;
		}

//		/* only use kalman filter */
//		if(time_for_fast_filter > 5){
//			time_for_fast_filter = time_for_fast_filter - 5;
//
//			//log_system("FAST FILTER MODE\n");
//
//			MCU_Data.fuel_232_1 = this_value_ad1;
//			MCU_Data.fuel_232_2 = this_value_ad2;
//
//			//log_system("##### %d ####### AD1 = %d, AD2 = %d ############\n", time_for_fast_filter, MCU_Data.fuel_232_1, MCU_Data.fuel_232_2);
//			continue;
//		}

		for(idx = 0; idx < (FILTER_TIME * SystemSetting.data.filter_level); idx ++){
			fuel_1_datas_sort[idx] = fuel_1_datas[idx];
			fuel_2_datas_sort[idx] = fuel_2_datas[idx];
		}

		if(!flg_full_load){
			if((fuel_index / 3) >= 3){
				condition = fuel_index;
				//log_system("NO FULL LOAD\n");
			}else{
				MCU_Data.fuel_232_1 = this_value_ad1;
				MCU_Data.fuel_232_2 = this_value_ad2;
				//log_system("NO FULL LOAD INDEX < 9, AD1 = %d, AD2 = %d \n", MCU_Data.fuel_232_1, MCU_Data.fuel_232_2);
				continue;
			}
		}else{
			condition = (FILTER_TIME * SystemSetting.data.filter_level);
			//log_system("FULL LOAD\n");
		}

		qsort ((int*)fuel_1_datas_sort, condition, sizeof(int), cmpfunc);
		qsort ((int*)fuel_2_datas_sort, condition, sizeof(int), cmpfunc);

//		printf_arr(fuel_1_datas_sort, condition);
//		printf_arr(fuel_2_datas_sort, condition);

		num_rec = 0;
		ad_1_sum = 0;
		ad_2_sum = 0;

		for((idx = (condition / 3)); (idx < (condition - (condition  / 3))); (idx ++)){
			ad_1_sum += fuel_1_datas_sort[idx];
			ad_2_sum += fuel_2_datas_sort[idx];
			num_rec++;
		}
		if(num_rec != 0){
			MCU_Data.fuel_232_1 = (ad_1_sum / num_rec);
			MCU_Data.fuel_232_2 = (ad_2_sum / num_rec);
		}
		//log_system("############ AD1 = %d, AD2 = %d ############\n", MCU_Data.fuel_232_1, MCU_Data.fuel_232_2);
	}
}
