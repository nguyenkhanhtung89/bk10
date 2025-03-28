#include "kalman.h"
#include <ql_oe.h>
#include "stdint.h"
#include "math.h"

float _err_measure_ad1;
float _err_estimate_ad1;
float _kalman_gain_ad1;
float _q_ad1;

float _err_measure_ad2;
float _err_estimate_ad2;
float _kalman_gain_ad2;
float _q_ad2;

float _current_estimate_ad1;
float _last_estimate_ad1;

float _current_estimate_ad2;
float _last_estimate_ad2;

void SimpleKalmanFilter_ad1(float mea_e, float est_e, float q){
	_err_measure_ad1=mea_e;
	_err_estimate_ad1=est_e;
	_q_ad1 = q;
}

void SimpleKalmanFilter_ad2(float mea_e, float est_e, float q){
	_err_measure_ad2=mea_e;
	_err_estimate_ad2=est_e;
	_q_ad2 = q;
}

float updateEstimate_ad1(float mea){
	_kalman_gain_ad1 = _err_estimate_ad1/(_err_estimate_ad1 + _err_measure_ad1);
	_current_estimate_ad1 = _last_estimate_ad1 + _kalman_gain_ad1 * (mea - _last_estimate_ad1);
	_err_estimate_ad1 =  (1.0 - _kalman_gain_ad1)*_err_estimate_ad1 + fabs(_last_estimate_ad1 -_current_estimate_ad1)*_q_ad1;
	_last_estimate_ad1 =_current_estimate_ad1;
	return _current_estimate_ad1;
}

float updateEstimate_ad2(float mea){
	_kalman_gain_ad2 = _err_estimate_ad2/(_err_estimate_ad2 + _err_measure_ad2);
	_current_estimate_ad2 = _last_estimate_ad2 + _kalman_gain_ad2 * (mea - _last_estimate_ad2);
	_err_estimate_ad2 =  (1.0 - _kalman_gain_ad2)*_err_estimate_ad2 + fabs(_last_estimate_ad2 -_current_estimate_ad2)*_q_ad2;
	_last_estimate_ad2 =_current_estimate_ad2;
	return _current_estimate_ad2;
}

void setMeasurementError_ad1(float mea_e){
	_err_measure_ad1=mea_e;
}

void setMeasurementError_ad2(float mea_e){
	_err_measure_ad1=mea_e;
}

void setEstimateError_ad1(float est_e){
	_err_estimate_ad1=est_e;
}

void setEstimateError_ad2(float est_e){
	_err_estimate_ad2=est_e;
}

void setProcessNoise_ad1(float q){
	_q_ad1=q;
}

void setProcessNoise_ad2(float q){
	_q_ad2=q;
}

float getKalmanGain_ad1(void){
	return _kalman_gain_ad1;
}

float getKalmanGain_ad2(void){
	return _kalman_gain_ad2;
}

float getEstimateError_ad1(void){
	return _err_estimate_ad1;
}

float getEstimateError_ad2(void){
	return _err_estimate_ad2;
}
