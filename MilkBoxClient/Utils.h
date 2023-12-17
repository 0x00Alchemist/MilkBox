#pragma once


BOOLEAN
WINAPI
CreateWorkingDirectory(
	VOID
);

HANDLE
WINAPI
OpenMilkBoxDeviceSession(
	VOID
);

BOOLEAN
WINAPI
CloseMilkBoxDeviceSession(
	IN HANDLE hDevice
);

UINT64
WINAPI
HashString(
	_In_ const WCHAR *Buf
);

VOID
WINAPI
PrintUsageInfo(
	VOID
);
