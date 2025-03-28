#include "display.h"
#include "ST7565.h"
#include "osObjects.h" 
#include "ugui.h"
#include "lcd_utils.h"

/* GUI structure */
UG_GUI gui;
enum_DISP_INFO pDisplay;
char lcd_temp[128];

/* Set here your display resolution. */
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

void vLCDTask(const void* args);

/* MsgBox to store fnkey event */
osMessageQDef(FnKey, 10, uint8_t);
osMessageQId FnKey;

osThreadId LCD_Thread;
osThreadDef(vLCDTask, osPriorityNormal, 1, 1024);

void init_lcd_services(void){
  FnKey = osMessageCreate(osMessageQ(FnKey), NULL);
  if (FnKey == NULL) {
    SerialPushStr("Error! Creating FnKey envent", END_LF);
  }
	
	LCD_Thread = osThreadCreate(osThread(vLCDTask), NULL);
	if(!LCD_Thread) return;
}

void FnKey_RaiseEvent(void){
	if(FnKey != NULL)
		osMessagePut(FnKey, (uint32_t)1, 0);
}

void display_device_info(void){
	//display header and base infomation
	display_base_info();
	UG_FontSelect(&FONT_6X8);

	/*---------HEADER----------------------*/
	UG_FillFrame(2, 11, 126, 20, BLACK);
	UG_SetBackcolor(BLACK);
	UG_SetForecolor(WHITE);
	UG_PutString(10, 12, "THONG TIN THIET BI");
	UG_SetBackcolor(WHITE);
	UG_SetForecolor(BLACK);
	
	UG_DrawLine(0, 21, 128, 21, BLACK); // INPUT LINE
	/*-------------------------------------*/
	UG_FontSelect(&FONT_5X8);
	
	sprintf(lcd_temp, "ID: %llu", ec21_data.DeviceID);
	UG_PutString(2, 24, lcd_temp);
	
	sprintf(lcd_temp, "PIN: %.2fV", (float)((float)ADCData.battery_val / (float) 1000));
	UG_PutString(2, 33, lcd_temp);
	
	sprintf(lcd_temp, "ACQUY: %.2fV", (float)((float)ADCData.main_power_val / (float) 1000));
	UG_PutString(2, 42, lcd_temp);
	
	sprintf(lcd_temp, "VERSION: E-%d M-%d", ec21_data.EC21Ver, MCU_VERSION);
	UG_PutString(2, 51, lcd_temp);
}

void display_driver_info(void){
	//display header and base infomation
	display_base_info();
	UG_FontSelect(&FONT_6X8);
	
	/*---------HEADER----------------------*/
	UG_FillFrame(2, 11, 126, 20, BLACK);
	UG_SetBackcolor(BLACK);
	UG_SetForecolor(WHITE);
	UG_PutString(15, 12, "THONG TIN LAI XE");
	UG_SetBackcolor(WHITE);
	UG_SetForecolor(BLACK);
	
	UG_DrawLine(0, 21, 128, 21, BLACK); // INPUT LINE
	/*-------------------------------------*/
	UG_FontSelect(&FONT_5X8);
	
	sprintf(lcd_temp, "LX: %s", ec21_data.DriverInfo.Name);
	UG_PutString(2, 24, lcd_temp);
	
	sprintf(lcd_temp, "GPLX: %s", ec21_data.DriverInfo.License);
	UG_PutString(2, 33, lcd_temp);
	
	sprintf(lcd_temp, "NGAY CAP: %s", ec21_data.DriverInfo.IssDate);
	UG_PutString(2, 42, lcd_temp);
	
	sprintf(lcd_temp, "NGAY HET HAN: %s", ec21_data.DriverInfo.ExpDate);
	UG_PutString(2, 51, lcd_temp);
}

void display_drive_result(void){
	//display header and base infomation
	display_base_info();
	UG_FontSelect(&FONT_6X8);
	
	/*---------HEADER----------------------*/
	UG_FillFrame(2, 11, 126, 20, BLACK);
	UG_SetBackcolor(BLACK);
	UG_SetForecolor(WHITE);
	UG_PutString(45, 12, "LAI XE");
	UG_SetBackcolor(WHITE);
	UG_SetForecolor(BLACK);
	
	UG_DrawLine(0, 21, 128, 21, BLACK); // INPUT LINE
	/*-------------------------------------*/
	UG_FontSelect(&FONT_5X8);
	sprintf(lcd_temp, "THOI GIAN LX: %d P", ec21_data.drv_continue);
	UG_PutString(2, 24, lcd_temp);
	
	sprintf(lcd_temp, "THOI GIAN LXTN: %d P", ec21_data.m_driver_inday);
	UG_PutString(2, 33, lcd_temp);
	
	sprintf(lcd_temp, "THOI GIAN DUNG: %d P", ec21_data.drv_stop);
	UG_PutString(2, 42, lcd_temp);
	
	sprintf(lcd_temp, "KM TRONG NGAY: %.2f P", ec21_data.distance);
	UG_PutString(2, 51, lcd_temp);
}

void display_drive_report(void){
	//display header and base infomation
	display_base_info();
	UG_FontSelect(&FONT_6X8);
	
	/*---------HEADER----------------------*/
	UG_FillFrame(2, 11, 126, 20, BLACK);
	UG_SetBackcolor(BLACK);
	UG_SetForecolor(WHITE);
	UG_PutString(42, 12, "BAO CAO");
	UG_SetBackcolor(WHITE);
	UG_SetForecolor(BLACK);
	
	UG_DrawLine(0, 21, 128, 21, BLACK); // INPUT LINE
	/*-------------------------------------*/
	UG_FontSelect(&FONT_5X8);
	
	sprintf(lcd_temp, "LXLT: %d LAN", ec21_data.drv_continue_count);
	UG_PutString(2, 24, lcd_temp);
	
	sprintf(lcd_temp, "DUNG DO: %d LAN", ec21_data.park_count);
	UG_PutString(2, 33, lcd_temp);
	
	sprintf(lcd_temp, "VI PHAM TOC DO: %d LAN", ec21_data.ov_speed_count);
	UG_PutString(2, 42, lcd_temp);
	
	sprintf(lcd_temp, "VI PHAM LXLT: %d LAN", ec21_data.over_drv_continue_count);
	UG_PutString(2, 51, lcd_temp);
}

void display_device_setting(void){
	//display header and base infomation
	display_base_info();
	UG_FontSelect(&FONT_6X8);

	/*---------HEADER----------------------*/
	UG_FillFrame(2, 11, 126, 20, BLACK);
	UG_SetBackcolor(BLACK);
	UG_SetForecolor(WHITE);
	UG_PutString(12, 12, "THONG TIN CAI DAT");
	UG_SetBackcolor(WHITE);
	UG_SetForecolor(BLACK);
	
	UG_DrawLine(0, 21, 128, 21, BLACK); // INPUT LINE
	/*-------------------------------------*/
	UG_FontSelect(&FONT_5X8);
	sprintf(lcd_temp, "BIEN SO: %s", ec21_data.NumberPlate);
	UG_PutString(2, 24, lcd_temp);
	
	sprintf(lcd_temp, "GUI KHI CHAY: %dS", ec21_data.FrqRun);
	UG_PutString(2, 33, lcd_temp);
	
	sprintf(lcd_temp, "GUI KHI DUNG: %dS", ec21_data.FrqStop);
	UG_PutString(2, 42, lcd_temp);
	
	sprintf(lcd_temp, "TOC DO GIOI HAN: %dKM/H", ec21_data.speed_limit);
	UG_PutString(2, 51, lcd_temp);
}

void display_private_setting(void){
	//display header and base infomation
	display_base_info();
	UG_FontSelect(&FONT_6X8);

	/*---------HEADER----------------------*/
	UG_FillFrame(2, 11, 126, 20, BLACK);
	UG_SetBackcolor(BLACK);
	UG_SetForecolor(WHITE);
	UG_PutString(15, 12, "THONG TIN CAI DAT");
	UG_SetBackcolor(WHITE);
	UG_SetForecolor(BLACK);
	
	UG_DrawLine(0, 21, 128, 21, BLACK); // INPUT LINE
	/*-------------------------------------*/
	UG_FontSelect(&FONT_5X8);
	
	sprintf(lcd_temp, "IP: %s ", ec21_data.Domain);
	UG_PutString(2, 24, lcd_temp);
	
	sprintf(lcd_temp, "PORT: %d ", ec21_data.Port);
	UG_PutString(2, 33, lcd_temp);
	UG_PutString(2, 42, "-------------------------");
	UG_PutString(2, 51, "-------------------------");
}

void display_home(void){
	//display header and base infomation
	display_base_info();
	
	UG_DrawLine(0, 52, 128, 52, BLACK); // INPUT LINE
	UG_DrawLine(21, 52, 21, 64, BLACK); // ACC LINE
	UG_DrawLine(48, 52, 48, 64, BLACK); // DOOR LINE
	UG_DrawLine(69, 52, 69, 64, BLACK); // AIR LINE
	
	//display ACC-DOOR-AIR
	display_inout(ec21_data.acc_status, ec21_data.door_status, ec21_data.air_status);
	
	//display date time
	UG_FontSelect(&FONT_12X20);
	sprintf(lcd_temp, "%02d:%02d", ec21_data.date_time.hour, ec21_data.date_time.minute);
	UG_PutString(2, 10, lcd_temp);
	
	UG_FontSelect(&FONT_6X8);
	sprintf(lcd_temp, "%02d/%02d/%d", ec21_data.date_time.day, ec21_data.date_time.month, (ec21_data.date_time.year + 1900));
	UG_PutString(63, 19, lcd_temp);
	
	//dis play driver name and driver continue time
	UG_DrawLine(0, 28, 128, 28, BLACK); // INPUT LINE
	display_drive_continue_time_load(ec21_data.DriverInfo.Name, (ec21_data.drv_continue * 60));
	
	//display speed
	sprintf(lcd_temp, "%d KM/h", ec21_data.speed);
	UG_PutString(75, 54, lcd_temp); 
}

void LCD_display(void){
	switch(pDisplay){
		case DISP_HOME:
			display_home();
			break;
		case DISP_DEVICE_INFO:
			display_device_info();
			break;
		case DISP_DRIVER_INFO:
			display_driver_info();
			break;
		case DISP_DRIVER_RESULT:
			display_drive_result();
			break;
		case DISP_DRIVER_REPORT:
			display_drive_report();
			break;
		case DISP_DEVICE_SETTING:
			display_device_setting();
			break;
		case DISP_PRIVATE_SETTING:
			display_private_setting();
			break;
		case DISP_FUEL_VALUE:
			break;
	}
}

void vLCDTask(const void* args){
	/* init glcd driver */
	LCD_Init(3);
	
	/* init glcd ugui control */
	UG_Init(&gui, (void( * )(UG_S16, UG_S16, UG_COLOR)) ST7565_setpixel, 128, 64);
	UG_SelectGUI(&gui);
	
	/* Register hardware acceleration */
  UG_DriverRegister(DRIVER_DRAW_LINE, (void * ) ST7565_drawline);
  UG_DriverRegister(DRIVER_FILL_FRAME, (void * ) ST7565_fillrect);
	
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	
	for(;;){
		/* wait fnkey */
		osMessageGet(FnKey, 1000);
		
		ST7565_clear();
		
		LCD_display();
		
		ST7565_display();

		osThreadYield(); 
	}
}
