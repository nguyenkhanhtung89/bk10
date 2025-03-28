#ifndef AREA_H
#define AREA_H

#include <stdio.h>
#include <stdarg.h>
#include <ql_oe.h>
#include "devconfig.h"
#include "gps.h"

#define	VN_EEZ_AREA		99
#define	VN_LONG_AREA	100


typedef enum{
	AREA_NOT_ALLOW = 0,
	AREA_ALLOW = 1
}enumAreaLevel;

typedef enum
{
	GroupOutsideAreaResidential = 1,
	GroupInsideAreaResidential = 2
}enumTypeAreaTypedef;


/*1: Duong doi(co dai phan cach o giua)
  2: Duong 1 chieu co tu 2 lan xe co gioi tro len
  3: Duong 2 chieu khong co giai phan cach
  4: Duong 1 chieu co 1 lan xe co gioi
*/
typedef enum
{
	TypeRoad1 = 1,
	TypeRoad2 = 2,
	TypeRoad3 = 3,
	TypeRoad4 = 4
}enumTypeRoadTypedef;

typedef struct{
	int AreaID;
	char AreaName[64];
	int nVert;   // So dinh cua hinh bao quanh khu vuc dang xet
	enum_GroupVehicle_typedef TypeCar;  /* 1: Xe oto con cho nguoi den 30 cho(tru xe bus)
	                           	   	   	   	   Xe oto tai co trong tai den 3.5 tan
	                						2: Xe cho nguoi tren 30 cho(tru xe bus)
	                           	   	   	   	   Xe oto tai co trong tai tren 3.5 tan
	                						3: Oto buyt, Oto dau keo ro mooc
	                           	   	   	   	   Oto chuyen dung, xe mo to
	                						4: Oto keo ro mooc, oto keo xe khac
	             	 	 	 	 	 	 */
	enumTypeRoadTypedef TypeRoad;  /*1: Duong doi(co dai phan cach o giua)
	                				2: Duong 1 chieu co tu 2 lan xe co gioi tro len
	                				3: Duong 2 chieu khong co giai phan cach
	                				4: Duong 1 chieu co 1 lan xe co gioi
	 	 	 	 	 	 	 	 	*/
	enumTypeAreaTypedef TypeArea;  /* 1: xe ngoai khu vuc dan cu
	                 2: xe trong khu vuc dan cu
	              */

	int TypeDirection;
	              /* -1:     Loai duong co 2 chieu di
	               * value:  Gia tri tuong ung voi goc so voi truc latitude goc (tu 0 -> 360) tuong ung voi chieu xe di
	               *
	               */

	int TrafficSigns; //Loai bien bao giao thong
}area_typedef;

typedef enum
{
	speedLimit40km = 1,                  /* Khu vuc cam cac phuong tien di tren 40 km/h */
	speedLimit50km = 2,                  /* Khu vuc cam cac phuong tien di tren 50 km/h */
	speedLimit60km = 3,                  /* Khu vuc cam cac phuong tien di tren 60 km/h */
	speedLimit70km = 4,                  /* Khu vuc cam cac phuong tien di tren 70 km/h */
	speedLimit80km = 5,
	speedLimit100km = 6,
	speedLimit120km = 7
}trafficSignsEnumTypedef;

//"AreaID"	INTEGER NOT NULL,
//"Lat"	REAL NOT NULL,
//"Lon"	REAL NOT NULL

//"ID"		   0
//"Name"	   1
//"SpeedLimit" 2
//"MinLat"	   3
//"MaxLat"	   4
//"MinLon"	   5
//"MaxLon"	   6
//"nVert"	   7
//"Type"	   8


void* thread_area_handler(void* arg);

#endif
