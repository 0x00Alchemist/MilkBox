#pragma once

typedef struct _MB_AREA_INFO {
	PVOID ImageBase;
	SIZE_T ImageSize;
} MB_AREA_INFO, *PMB_AREA_INFO;

typedef struct _MB_LIST {
	SIZE_T       Count;
	MB_AREA_INFO AreaInfo[1];
} MB_LIST, *PMB_LIST;

typedef struct _MB_DRIVER_INFO {
	PVOID Base;
} MB_DRIVER_INFO, *PMB_DRIVER_INFO;

typedef struct _MB_TABLE_OFFSET_INFO {
	PVOID Address;
} MB_TABLE_OFFSET_INFO, PMB_TABLE_OFFSET_INFO;


NTSTATUS
NTAPI
FindRuntimeImages(
	_Out_ PMB_LIST MbList
);
