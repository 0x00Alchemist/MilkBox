#pragma once


BOOLEAN
WINAPI
StartMilkBoxService(
	VOID
);

VOID
WINAPI
StopMilkBoxService(
	VOID
);

BOOLEAN
WINAPI
DeleteMilkBoxService(
	VOID
);

BOOLEAN
WINAPI
CreateMilkBoxService(
	_In_ WCHAR *DriverPath
);
