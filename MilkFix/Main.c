#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>


/**
 * \brief Fixes dumped driver
 * 
 * \param hFile Handle of opened file
 * 
 * \return TRUE - Fixed succesfully
 * \return FALSE - Some error occured (unable to allocate heap, read file or write to file)
 */
BOOLEAN
WINAPI
FixDump(
	_In_ HANDLE hFile
) {
	/// \note @0x00Alchemist: get file size
	DWORD dwSize = GetFileSize(hFile, NULL);

	/// \note @0x00Alchemist: allocate heap to store our dump
	PVOID Buffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
	if(Buffer == NULL) {
		wprintf(L"[ MilkFix ] Cannot allocate memory to store dump!\n");
		return FALSE;
	}

	/// \note @0x00Alchemist: read file
	DWORD dwRead = 0;
	if(!ReadFile(hFile, Buffer, dwSize, &dwRead, NULL)) {
		wprintf(L"[ MilkFix ] Unable to read file!\n");
		return FALSE;
	}

	/// \note @0x00Alchemist: get PE headers (no reason to control them)
	PIMAGE_DOS_HEADER Dos = (PIMAGE_DOS_HEADER)Buffer;
	PIMAGE_NT_HEADERS64 Nt64 = (PIMAGE_NT_HEADERS64)((UINT8 *)Buffer + Dos->e_lfanew);

	/// \note @0x00Alchemist: get sections and fix 'em
	PIMAGE_SECTION_HEADER SectionHeader = (PIMAGE_SECTION_HEADER)((UINT64)&Nt64->OptionalHeader + Nt64->FileHeader.SizeOfOptionalHeader);

	INT NumberOfSections = Nt64->FileHeader.NumberOfSections;
	for(INT i = 0; i < NumberOfSections; ++i) {
		SectionHeader[i].SizeOfRawData = SectionHeader[i].Misc.VirtualSize;
		SectionHeader[i].PointerToRawData = SectionHeader[i].VirtualAddress;
	}

	Nt64->OptionalHeader.SizeOfHeaders = SectionHeader[0].PointerToRawData;
	Nt64->OptionalHeader.SizeOfImage = SectionHeader[NumberOfSections - 1].VirtualAddress + SectionHeader[NumberOfSections - 1].Misc.VirtualSize;
	
	/// \note @0x00Alchemist: fix data dirs, we need only relocs and debug dirs 
	/// (since the rest of the data directories are not used (due to the specification))
	Nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = 0;
	Nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = 0;

	Nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress = 0;
	Nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size = 0;

	/// \note @0x00Alchemist: write back to file
	DWORD dwWritten = 0;
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	if(!WriteFile(hFile, Buffer, dwSize, &dwWritten, NULL)) {
		wprintf(L"[ MilkFix ] Unable to write back dump!\n");
		return FALSE;
	}

	HeapFree(GetProcessHeap(), 0, Buffer);

	return TRUE;
}

/**
 * \brief Entry point of the program
 * 
 * \param Argc Value of args
 * \param Argv Array of args
 * 
 * \return 0 - Dumped succesfully
 * \return 1 - Cannot fix dump
 */
INT
WINAPI
wmain(
	_In_ INT      Argc,
	_In_ WCHAR  **Argv
) {
	WCHAR Path[MAX_PATH] = { 0 };

	wprintf(L"[ MilkFix ] Provide path to dump: ");

	wscanf(L"%s", Path);
	if((wcslen(Path) > MAX_PATH) || (Path == NULL)) {
		wprintf(L"[ MilkFix ] Invalid path!\r\n");
	}

	HANDLE hFile = CreateFileW(Path, (GENERIC_READ | GENERIC_WRITE), (FILE_SHARE_READ | FILE_SHARE_WRITE), NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE) {
		DWORD dwErr = GetLastError();
		wprintf(L"[ MilkFix ] Unable to open file! (0x%lX)\n", dwErr);
		return 1;
	}

	/// \note @0x00Alchemist: fix dump of driver
	if(!FixDump(hFile)) {
		wprintf(L"[ MilkFix ] Cannot fix dump!\n");
		return 1;
	}

	wprintf(L"[ MilkFix ] Fixed\r\n");

	CloseHandle(hFile);

	return 0;
}
