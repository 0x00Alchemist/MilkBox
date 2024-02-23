#include <Windows.h>
#include <stdio.h>


/**
 * \brief Creates directory for future dumps
 * 
 * \return TRUE - The directory has been successfully created
 * \return FALSE - Unable to create directory
 */
BOOLEAN
WINAPI
CreateWorkingDirectory(
	VOID
) {
	BOOLEAN IsCreated = TRUE;

	if(!CreateDirectoryW(L"C:\\MilkBox", NULL)) {
		DWORD dwErr = GetLastError();
		if(dwErr == ERROR_ALREADY_EXISTS)
			return IsCreated;
		
		wprintf(L"[ MilkBox ] Cannot create directory for dumps!\n");
		IsCreated = FALSE;
	}

	return IsCreated;
}

/**
 * \brief Opens MilkBox handle
 * 
 * \return MilkBox handle (if opened)
 */
HANDLE
WINAPI
OpenMilkBoxDeviceSession(
	VOID
) {
	HANDLE hDevice = CreateFileW(L"\\\\.\\MilkBox", (FILE_READ_ACCESS | FILE_WRITE_ACCESS), (FILE_SHARE_READ | FILE_SHARE_WRITE), NULL, OPEN_EXISTING, 0, NULL);
	if(hDevice == INVALID_HANDLE_VALUE) {
		wprintf(L"[ MilkBox ] Cannot get device handle!\n");
		return NULL;
	}

	return hDevice;
}

/**
 * \brief Closes MilkBox handle
 * 
 * \param hDevice MilkBox handle
 */
BOOLEAN
WINAPI
CloseMilkBoxDeviceSession(
	_In_ HANDLE hDevice
) {
	CloseHandle(hDevice);
}

/**
 * \brief Hashes specified string
 * 
 * \param Buf String
 * 
 * \return Hashed string
 */
UINT64
WINAPI
HashString(
	_In_ const WCHAR *Buf
) {
	UINT64 Hash = 0x1505UL;
	INT C = 0;

	while(C = *Buf++)
		Hash = ((Hash << 5) + Hash) + C;

	return Hash;
}

/**
 * \brief Shows command list
 */
VOID 
WINAPI
PrintUsageInfo(
	VOID
) {
	wprintf(L"[ MilkBox ] Usage: \n");
	wprintf(L"rtd - Locate runtime drivers (should be performed firstly)\n");
	wprintf(L"wd - Write dump to binary file, dump location - \"C:\\MilkBox\"\n");
	wprintf(L"ud - Uninstall driver\n");
	wprintf(L"ex - Exit from program\n\n");
}
