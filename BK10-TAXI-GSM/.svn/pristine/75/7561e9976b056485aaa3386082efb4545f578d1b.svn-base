#ifndef KALMAN_H
#define KALMAN_H

#include <stdio.h>
#include <stdarg.h>
#include <ql_oe.h>

	// class SimpleKalmanFilter
void SimpleKalmanFilter_ad1(float mea_e, float est_e, float q);
void SimpleKalmanFilter_ad2(float mea_e, float est_e, float q);

float updateEstimate_ad1(float mea);
float updateEstimate_ad2(float mea);

void setMeasurementError_ad1(float mea_e);
void setMeasurementError_ad2(float mea_e);

void setEstimateError_ad1(float est_e);
void setEstimateError_ad2(float est_e);

void setProcessNoise_ad1(float q);
void setProcessNoise_ad2(float q);

float getKalmanGain_ad1(void);
float getKalmanGain_ad2(void);

float getEstimateError_ad1(void);
float getEstimateError_ad2(void);

#endif /* KALMAN */
