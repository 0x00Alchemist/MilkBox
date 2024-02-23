#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>

#include "Service.h"
#include "Utils.h"

#define IOCTL_LOCATE_RT_DRIVERS	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1100, METHOD_OUT_DIRECT, FILE_SPECIAL_ACCESS)
#define IOCTL_DUMP_RT_DRIVERS	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1105, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define CMD_RTD		193505071ULL
#define CMD_WD		5863936ULL
#define CMD_UD		5863870ULL
#define CMD_EX		5863362ULL

static HANDLE hMilkBoxDevice = NULL;


/**
 * \brief Handles user input
 * 
 * \param Cmd Command
 * 
 * \return TRUE - User decieded to exit
 * \return FALSE - User wants to continue
 */
BOOLEAN 
WINAPI
CommandParser(
	_In_ WCHAR *Cmd
) {
	BOOLEAN Exit = FALSE;
	DWORD Ret = 0;

	/// \note @0x00Alchemist: there's can be hash collisions, be aware. Our hashes precomputed though
	switch(HashString(Cmd)) {
		case CMD_RTD:
			if(!DeviceIoControl(hMilkBoxDevice, IOCTL_LOCATE_RT_DRIVERS, NULL, 0, NULL, 0, &Ret, NULL)) {
				wprintf(L"[ MilkBox ] Cannot locate runtime drivers!\n");
			} else {
				wprintf(L"[ MilkBox ] Located succesfully! You can now enter \"wd\" to dump files.\n");
			}
		break;
		case CMD_WD:
			if(!DeviceIoControl(hMilkBoxDevice, IOCTL_DUMP_RT_DRIVERS, NULL, 0, NULL, 0, &Ret, NULL)) {
				wprintf(L"[ MilkBox ] Cannot write dump file!\n");
			} else {
				wprintf(L"[ MilkBox ] Dumped!\n");
			}
		break;
		case CMD_UD:
			if(DeleteMilkBoxService())
				Exit = TRUE;
		break;
		case CMD_EX:
			Exit = TRUE;
		break;
		default:
			PrintUsageInfo();
	}

	return Exit;
}

/**
 * \brief Entry point of the program
 * 
 * \param Argc Value of args
 * \param Argv Array of args
 * 
 * \return 0 - Terminated succesfully
 * \return 1 - An error occurred during driver or service registration
 */
INT
WINAPI
wmain(
	_In_ INT Argc, 
	_In_ WCHAR *Argv[]
) {
	WCHAR Path[MAX_PATH] = { 0 };
	wprintf(L"[ MilkBox ] Provide path to driver: ");
	wscanf(L"%s", Path);

	/// \note @0x00Alchemist: check path validity
	if((wcslen(Path) > MAX_PATH) || Path == NULL) {
		wprintf(L"[ MilkBox ] Invalid path!\n");
		return 1;
	}

	/// \note @0x00Alchemist: create directory for dumps
	if(!CreateWorkingDirectory())
		return 1;

	/// \note @0x00Alchemist: register our service
	if(!CreateMilkBoxService(Path))
		return 1;

	/// \note @0x00Alchemist: start our service
	if(!StartMilkBoxService())
		return 1;

	/// \note @0x00Alchemist: open session
	hMilkBoxDevice = OpenMilkBoxDeviceSession();
	if(hMilkBoxDevice == NULL)
		return 1;

	/// \note @0x00Alchemist: parse user input
	BOOLEAN Continue = FALSE;
	do {
		WCHAR Cmd[64] = { 0 };
		
		wprintf(L"[ MilkBox ] Enter command: ");
		wscanf(L"%s", Cmd);
		wprintf(L"\n");

		Continue = CommandParser(Cmd);
	} while(!Continue);

	/// \note @0x00Alchemist: stop our service, it's not critical if we cannot stop it programmatically at exit
	StopMilkBoxService();

	/// \note @0x00Alchemist: close session
	CloseMilkBoxDeviceSession(hMilkBoxDevice);

	wprintf(L"[ MilkBox ] Bye bye!\n");

	return 0;
}
