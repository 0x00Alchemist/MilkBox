#include <ntddk.h>
#include <ntimage.h>
#include <ntstrsafe.h>


PVOID
NTAPI
CalcOffset(
	_In_ PVOID Base,
	_In_ INT32 Offset
) {
	return (PVOID)((UINT_PTR)Base + Offset);
}

VOID
NTAPI
CreateFileName(
	_Out_       PWCHAR Str,
	_In_  const UINT32 Size,
	_In_  const PWCHAR Format,
	...
) {
	va_list Ptr;
	va_start(Ptr, Format);

	RtlStringCbVPrintfW(Str, Size, Format, Ptr);

	va_end(Ptr);
}

TIME_FIELDS
NTAPI
GetCurrentTime(
	VOID
) {
	LARGE_INTEGER SystemTime;
	LARGE_INTEGER Localized;
	TIME_FIELDS Time;

	KeQuerySystemTime(&SystemTime);
	ExSystemTimeToLocalTime(&SystemTime, &Localized);
	RtlTimeToTimeFields(&Localized, &Time);

	return Time;
}

PVOID
NTAPI
FindSection(
	_In_ const PVOID  ImageBase,
	_In_ const PUCHAR Query
) {
	// @note: @0x00Alchemist: headers controlled at this moment, no reason to recheck it
	PIMAGE_DOS_HEADER Dos = (PIMAGE_DOS_HEADER)ImageBase;
	PIMAGE_NT_HEADERS64 Nt64 = (PIMAGE_NT_HEADERS64)CalcOffset(ImageBase, Dos->e_lfanew);

	PIMAGE_SECTION_HEADER Section = IMAGE_FIRST_SECTION(Nt64);
	for(INT i = 0; i < Nt64->FileHeader.NumberOfSections; i++) {
		if(memcmp(Section[i].Name, Query, IMAGE_SIZEOF_SHORT_NAME) == 0)
			return CalcOffset(ImageBase, Section[i].VirtualAddress);
	}

	return NULL;
}

BOOLEAN
NTAPI
ValidateImage(
	_In_ PVOID   ImageBase
) {
	PIMAGE_DOS_HEADER Dos = (PIMAGE_DOS_HEADER)ImageBase;
	if (Dos->e_magic != IMAGE_DOS_SIGNATURE)
		return FALSE;

	PIMAGE_NT_HEADERS64 Nt64 = (PIMAGE_NT_HEADERS64)CalcOffset(ImageBase, Dos->e_lfanew);
	if (Nt64->Signature != IMAGE_NT_SIGNATURE)
		return FALSE;

	return TRUE;
}

NTSTATUS
NTAPI
TestRelatedPhysAddr(
	_In_ PVOID ImageBase,
	_In_ INT32 Pages
) {
	for(INT32 i = 0; i < Pages; ++i) {
		INT32 Offset = (PAGE_SIZE * i);
		PHYSICAL_ADDRESS Phys = MmGetPhysicalAddress(CalcOffset(ImageBase, Offset));
		if(Phys.QuadPart == 0)
			return STATUS_ADDRESS_NOT_ASSOCIATED;
	}

	return STATUS_SUCCESS;
}


typedef PVOID(NTAPI *__T_RtlPcToFileHeader)(PVOID PcValue, PVOID *ImageBase);
__T_RtlPcToFileHeader RtlPcToFileHeader;

PVOID
NTAPI
FindKernelBase(
	VOID
) {
	UNICODE_STRING Function = RTL_CONSTANT_STRING(L"RtlPcToFileHeader");
	RtlPcToFileHeader = (__T_RtlPcToFileHeader)MmGetSystemRoutineAddress(&Function);
	if(RtlPcToFileHeader == NULL) {
		KdPrint(("[ MilkBox ] Cannot find \"RtlPcToFileHeader\" function!\n"));
		return NULL;
	}

	PVOID ImageBase = NULL;
	RtlPcToFileHeader((PVOID)RtlPcToFileHeader, &ImageBase);

	// @note: @0x00Alchemist: validate image headers (bc i'm paranoidal)
	PIMAGE_DOS_HEADER Dos = (PIMAGE_DOS_HEADER)ImageBase;
	if(Dos->e_magic != IMAGE_DOS_SIGNATURE)
		return NULL;

	PIMAGE_NT_HEADERS64 Nt64 = (PIMAGE_NT_HEADERS64)CalcOffset(ImageBase, Dos->e_lfanew);
	if(Nt64->Signature != IMAGE_NT_SIGNATURE)
		return NULL;

	return ImageBase;
}


