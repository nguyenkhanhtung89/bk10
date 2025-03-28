#include <ql_oe.h>
#include <math.h>

#include "lis3dsh.h"
#include "lis3dh_reg.h"
#include "vibrate.h"
#include "log.h"
#include "devconfig.h"

#define M_PI 3.14159265358979323846

#define RAD_TO_DEG (double)(180.0 / M_PI)
u8 countCheckInitSensor = 0;
double roll, pitch, yaw;
u8 SensorVibrate1 = LIS3DH;
enum_LSM_State LSM_Status;

int16_t data_raw_acceleration[3];
tri_axisSensor_typedef measureTriAxisSensor;

static IMU_6AXES_StatusTypeDef IIC_init(void){
	static IMU_6AXES_StatusTypeDef status;

	IMU_6AXES_InitTypeDef InitStructure;

	/* Configure sensor */
	InitStructure.ACC_FullScale = 2.0f; /* 2G */
	InitStructure.ACC_OutputDataRate = 100.0f; /* 100HZ */
	InitStructure.ACC_X_Axis = 1; /* Enable */
	InitStructure.ACC_Y_Axis = 1; /* Enable */
	InitStructure.ACC_Z_Axis = 1; /* Enable */

	status = LIS3DSH_init(&InitStructure);

	return status;
}

static int32_t ACC_axes[3];

#define MAX_LENGTH_VIBRATE 10
typedef struct{
	double roll;
	double pich;
}motion_tydef;

static u8 del_idx = 0;
static u8 hold_state = 0;
static u8 lock_state = 0;

motion_tydef motion_values[MAX_LENGTH_VIBRATE];

void EstimateVibrateValue(double del_roll, double del_pitch){
	u8 idx = 0;
	u8 vibrate = 0, move = 0;
	double diff_roll = 0, diff_pitch = 0;

	motion_values[del_idx].roll = del_roll;
	motion_values[del_idx].pich = del_pitch;
	del_idx++;


	if(del_idx >= MAX_LENGTH_VIBRATE){
		del_idx = 0;
	}

	for(idx = 0; idx < MAX_LENGTH_VIBRATE; idx++){
		 diff_roll = fabs(motion_values[idx].roll);
		 diff_pitch = fabs(motion_values[idx].pich);
		if ((diff_roll > (double)(SystemSetting.data.MotionValue + (double)3.0)) && (diff_pitch > (double)(SystemSetting.data.MotionValue + (double)3.0))){
			move++;
		}else if ((diff_roll > (double)SystemSetting.data.MotionValue) && (diff_pitch > (double)SystemSetting.data.MotionValue)){
			vibrate++;
		}
	}

	if(move >= 3){
		LSM_Status = LSM_VEHICLE_RUN;
		log_debug("\nLSM VEHICLE MOVE\n");
		hold_state ++;
		if(hold_state > 20){
			hold_state = 120;
			lock_state = 1;
		}
	}else if(vibrate >= 3){
		LSM_Status = LSM_ENGINE_ON;
		log_debug("\nLSM VEHICLE ENGINE ON\n");
		hold_state ++;
		if(hold_state > 20){
			hold_state = 120;
			lock_state = 1;
		}
	}else{
		if(hold_state > 0){
			hold_state --;
		}else{
			lock_state = 0;
		}

		if(!lock_state){
			LSM_Status = LSM_STOPPING;
		}
	}
}

signed int accelerationX, accelerationY, accelerationZ;
u8 time_rpt_acc = 0;

void cal_Vibrate(void) {
	static double pre_roll, pre_pitch;
	double del_roll, del_pitch;

	time_rpt_acc++;
    if(SensorVibrate1 == LIS3DSH){
		static double accX, accY, accZ;

		if(LSM6DS0_ACC_GetAxes(ACC_axes) != IMU_6AXES_OK){
			log_debug("X_GetAxes Err...");
		}

		accX = ACC_axes[0];
		accY = ACC_axes[1];
		accZ = ACC_axes[2];

		accelerationX = (signed int)(((signed int)accX) * 3.9);
		accelerationY = (signed int)(((signed int)accY) * 3.9);
		accelerationZ = (signed int)(((signed int)accZ) * 3.9);

		pitch = 180 * atan (accelerationX/sqrt(accelerationY*accelerationY + accelerationZ*accelerationZ))/M_PI;
		roll = 180 * atan (accelerationY/sqrt(accelerationX*accelerationX + accelerationZ*accelerationZ))/M_PI;
		yaw = 180 * atan (accelerationZ/sqrt(accelerationX*accelerationX + accelerationZ*accelerationZ))/M_PI;

		if((time_rpt_acc % 6) == 0){
			log_debug("ROLL %.5f, PITCH %.5f, YAW %.5f\n", roll, pitch, yaw);
		}
		del_roll = roll - pre_roll;
		del_pitch = pitch - pre_pitch;

		/* Estimate vibrate value */
		EstimateVibrateValue(del_roll, del_pitch);

		pre_roll = roll;
		pre_pitch = pitch;
    }else{
		u8 flags;
		u8 num = 0;
		u8 n_avg = 0;

		float acceleration_mg[3];

		//static double del_acceleration;
		double sumx = 0, sumy = 0;
		/*Check if FIFO level over threshold */
		lis3dh_fifo_fth_flag_get(&dev_ctx,&flags);

		if(flags){
			/*Read number of sample in FIFO*/
			lis3dh_fifo_data_level_get(&dev_ctx, &num);

			while(num-- > 0){
				/*Read XL samples*/
				lis3dh_acceleration_raw_get(&dev_ctx, data_raw_acceleration);
				acceleration_mg[0] = setRangeValue(lis3dh_from_fs2_hr_to_mg(data_raw_acceleration[0]));
				acceleration_mg[1] = setRangeValue(lis3dh_from_fs2_hr_to_mg(data_raw_acceleration[1]));
				acceleration_mg[2] = setRangeValue(lis3dh_from_fs2_hr_to_mg(data_raw_acceleration[2]));

				measureTriAxisSensor = calculate_triAxisTiltSensor(acceleration_mg[0], acceleration_mg[1], acceleration_mg[2]);

				sumx = sumx + measureTriAxisSensor.anpha;
				sumy = sumy + measureTriAxisSensor.beta;
				n_avg++;
			}
			roll = sumx / n_avg;
			pitch = sumy / n_avg;

			if((time_rpt_acc % 6) == 0){
				log_debug("ROLL: %.5f , PITCH: %.5f\n\n", roll, pitch);
			}
			del_roll = roll - pre_roll;
			del_pitch = pitch - pre_pitch;

			/* Estimate vibrate value */
			EstimateVibrateValue(del_roll, del_pitch);

			pre_roll = roll;
			pre_pitch = pitch;
		}
    }
}

IMU_6AXES_StatusTypeDef lis3dsh_status = IMU_6AXES_ERROR;
IMU_LIS3DH_StatusTypeDef lis3dh_status = IMU_LIS3DH_ERROR;
void *thread_vibrate(void *arg){
	for(;;){
		if(SensorVibrate1 == LIS3DH){
			if(lis3dh_status != IMU_LIS3DH_OK){
				lis3dh_status = LIS3DH_Init();
				if(lis3dh_status != IMU_LIS3DH_OK){
					log_system("LIS3DH NOT READY\n");
					SensorVibrate1 = LIS3DSH;
				}else{
					log_system("LIS3DH READY\n");
				}
				sleep(10);
				continue;
			}else{
				cal_Vibrate();
				usleep(500000);
			}
		}else if(SensorVibrate1 == LIS3DSH){
			if(lis3dsh_status != IMU_6AXES_OK){
				lis3dsh_status = IIC_init();
				if(lis3dsh_status != IMU_6AXES_OK){
					log_system("LIS3DSH NOT READY\n");
					SensorVibrate1 = LIS3DH;
				}else{
					log_system("LIS3DSH READY\n");
				}
				sleep(10);
				continue;
			}else{
				cal_Vibrate();
				usleep(500000);
			}
		}
	}
}

