#include "display.h"
#include "ST7565.h"
#include "osObjects.h" 
#include "ugui.h"
#include "lcd_utils.h"
#include "power.h"

#define BATTERY_LOW  3600
#define BATTERY_FULL 4000
char lcd_tmp[128];

void display_battery(u16 milivol){
	UG_DrawFrame(2, 2, 14, 8, BLACK);
	UG_DrawLine(14, 3, 14, 6, BLACK);
	
	if(milivol >= BATTERY_FULL){
		UG_FillFrame(3, 3, 13, 7, BLACK);
	}else if(milivol >= BATTERY_LOW){
		u8 level = 0;	
		level = (((milivol - BATTERY_LOW) * 100) / (BATTERY_FULL - BATTERY_LOW)) / 10;
		//level = abs(level - 10);
		UG_FillFrame(3, 3, (3 + level), 7, BLACK);
	}
}

void display_server_connect(u8 connected){
	if(connected == 1){
		uint8_t cnn[] = {0x04, 0x06, 0x7F, 0x06, 0x24, 0x60, 0xFE, 0x60, 0x20};
		ST7565_drawbitmap(115, 1, cnn, 9,8,1);
	}
}

void display_charge(uint16_t main_pw){
	if(main_pw > 10000){
		uint8_t chg[] = {0x00, 0x00, 0x1C, 0x27, 0xE4, 0x27, 0x1C, 0x00, 0x00};
		ST7565_drawbitmap(20, 1, chg, 8,8,1);
	}
}

void display_drive_continue_time_load(char * name, u16 time){
	UG_FontSelect(&FONT_6X8);
	UG_PutString(2, 30, "LX  :");
	UG_FontSelect(&FONT_4X6);
	UG_PutString(32, 32, name);
	
	UG_FontSelect(&FONT_6X8);
	UG_PutString(2, 42, "LXLT: ");
	UG_DrawFrame(33, 40, 125, 50, BLACK);  // MAIN FRAME
	
	if(time > 14400){//88
		UG_FillFrame(35, 42, 123, 48, BLACK);   //driver continue load time
	}else{
		uint16_t k, ax;
		k =  14400/88;
		ax = time/k;
		UG_FillFrame(35, 42, (35 + ax), 48, BLACK);   //driver continue load time
	}
}

void display_signal_strength(enum_STR_DISP_type type, uint8_t x, uint8_t y, enum_STR_LEVEL lv){
  uint8_t i, j;
  u8 str[7] = {0xC0, 0x00, 0xE0, 0x00, 0xF0, 0x00, 0xF8};
	UG_FontSelect(&FONT_4X6);
	switch(type){
		case DISP_GPS:
			UG_PutString(x, y+3, "GPS");
			break;
		case DISP_GSM:
			UG_PutString(x, y+3, "GSM");
			break;
		case DISP_3G:
			UG_PutString(x, y+3, " 3G");
			break;
		case DISP_LTE:
			UG_PutString(x, y+3, "LTE");
			break;
		case DISP_NONE:
			UG_PutString(x, y+3, " ");
			break;
	}
	
	switch(lv){
		case STR_ERROR:
			UG_PutString(x + 14, y+2, " ");
			return;
		case STR_LV0:
			return;
		case STR_LV1:
			memset(str + 2, 0x0, 5);
			break;
		case STR_LV2:
			memset(str + 4, 0x0, 3);
			break;
		case STR_LV3:
			memset(str + 6, 0x0, 1);
			break;
		case STR_LV4:
			break;
	}
	
	for (j = 0; j < 8; j++) {
    for (i = 0; i < 8; i++ ) {
      if (str[i + (j / 8) * 8] & (1 << (j % 8))) {
        UG_DrawPixel(x + i + 15, y + j, BLACK);
      }
    }
  }
}

void display_inout(u8 acc, u8 door, u8 air){
	UG_FontSelect(&FONT_6X8);
	//show ACC status
	if(acc == 1){
		UG_FillFrame(1, 53, 21, 63, BLACK);
		UG_SetBackcolor(BLACK);
		UG_SetForecolor(WHITE);
		UG_PutString(2, 54, "ACC");
	}
	UG_PutString(2, 54, "ACC");
	UG_SetBackcolor(WHITE);
	UG_SetForecolor(BLACK);
	
	//show door status
	if(door == 1){
		UG_FillFrame(22, 53, 48, 63, BLACK);
		UG_SetBackcolor(BLACK);
		UG_SetForecolor(WHITE);
		UG_PutString(23, 54, "DOOR");
	}
	UG_PutString(23, 54, "DOOR");
	UG_SetBackcolor(WHITE);
	UG_SetForecolor(BLACK);
	
	//show air status
	if(air == 1){
		UG_FillFrame(49, 53, 69, 63, BLACK);
		UG_SetBackcolor(BLACK);
		UG_SetForecolor(WHITE);
		UG_PutString(50, 54, "AIR");
	}
	UG_PutString(50, 54, "AIR");
	UG_SetBackcolor(WHITE);
	UG_SetForecolor(BLACK);
}

enum_STR_LEVEL get_lte_str_level(u8 signal_strength){
	if(signal_strength == 0) return STR_ERROR;
	if(signal_strength < 65){
		return STR_LV4;
	}else if(signal_strength < 75){
		return STR_LV3;
	}else if(signal_strength < 85){
		return STR_LV2;
	}else if(signal_strength < 95){
		return STR_LV1;
	}else if(signal_strength < 105){
		return STR_LV0;
	}
	return STR_ERROR;
}

enum_STR_DISP_type get_radio_type(enum_radio_tech type){
	switch(type){
		case NW_MODE_GSM:
			return DISP_GSM;
		case NW_MODE_WCDMA:
			return DISP_3G;
		case NW_MODE_TDSCDMA:
			return DISP_NONE;
		case NW_MODE_LTE:
			return DISP_LTE;
		case NW_MODE_CDMA:
			return DISP_NONE;
		case NW_MODE_HDR:
			return DISP_NONE;
	}
	return DISP_NONE;
}

void display_base_info(void){
	enum_STR_LEVEL sig_level;
	enum_STR_DISP_type disp_type;
	
	//draw frame and lines
	UG_DrawFrame(0, 0, 128, 64,BLACK);  // MAIN FRAME
	UG_DrawLine(0, 9, 128, 9, BLACK); // INPUT LINE
	//display batery level
	display_battery(ADCData.battery_val);
	
	//display charge or not charge
	display_charge(ADCData.main_power_val);
	
	//display server connected or not connect
	display_server_connect(ec21_data.network_Data.tcp_status);
	
	//display GPS signal strength
	sig_level = (ec21_data.gps_status == 1) ?  STR_LV4 : STR_LV0;
	display_signal_strength(DISP_GPS ,60, 0, sig_level);
	
	disp_type = get_radio_type(ec21_data.network_Data.radio_tech);
	sig_level = get_lte_str_level(ec21_data.network_Data.signal_strength);
	
	//display LTE signal strength
	display_signal_strength(disp_type ,87, 0, sig_level);
}

