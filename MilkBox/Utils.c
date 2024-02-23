#include <ntddk.h>
#include <ntimage.h>
#include <ntstrsafe.h>


/**
 * \brief Calculate offset
 * 
 * \param Base Base
 * \param Offset Offset
 * 
 * \return Calculated offset
 */
PVOID
NTAPI
CalcOffset(
	_In_ PVOID Base,
	_In_ INT32 Offset
) {
	return (PVOID)((UINT_PTR)Base + Offset);
}

/**
 * \brief Creates name for dump file
 * 
 * \param Str Template of file name
 * \param Size Length of template
 * \param Format Format 
 */
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

/**
 * \brief Gets current date
 * 
 * \return Currrent date, month and year
 */
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

/**
 * \brief Searches for specified section
 * 
 * \param ImageBase Image base
 * \param Query Name of section
 * 
 * \return Base address of section (if found)
 */
PVOID
NTAPI
FindSection(
	_In_ const PVOID  ImageBase,
	_In_ const PUCHAR Query
) {
	// \note @0x00Alchemist: headers controlled at this moment, no reason to recheck it
	PIMAGE_DOS_HEADER Dos = (PIMAGE_DOS_HEADER)ImageBase;
	PIMAGE_NT_HEADERS64 Nt64 = (PIMAGE_NT_HEADERS64)CalcOffset(ImageBase, Dos->e_lfanew);

	PIMAGE_SECTION_HEADER Section = IMAGE_FIRST_SECTION(Nt64);
	for(INT i = 0; i < Nt64->FileHeader.NumberOfSections; i++) {
		if(memcmp(Section[i].Name, Query, IMAGE_SIZEOF_SHORT_NAME) == 0)
			return CalcOffset(ImageBase, Section[i].VirtualAddress);
	}

	return NULL;
}

/**
 * \brief Validates PE headers
 * 
 * \param ImageBase Image base
 * 
 * \return TRUE - Image has been validated
 * \return FALSE - Invalid image
 */
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

/**
 * \brief Checks if virtual addresses of RT driver corresponds with physical address
 * 
 * \param ImageBase Image base
 * \param Pages Size of driver in pages
 * 
 * \return STATUS_SUCCESS - Test passed successfully
 * \return STATUS_ADDRESS_NOT_ASSOCIATED - VA of driver not corresponds with phys address
 */
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

/**
 * \brief Gets base address of windows kernel
 * 
 * \return Base address of kernel (if found)
 */
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

	// \note @0x00Alchemist: validate image headers (bc i'm paranoidal)
	PIMAGE_DOS_HEADER Dos = (PIMAGE_DOS_HEADER)ImageBase;
	if(Dos->e_magic != IMAGE_DOS_SIGNATURE)
		return NULL;

	PIMAGE_NT_HEADERS64 Nt64 = (PIMAGE_NT_HEADERS64)CalcOffset(ImageBase, Dos->e_lfanew);
	if(Nt64->Signature != IMAGE_NT_SIGNATURE)
		return NULL;

	return ImageBase;
}

/**
 * \brief Gets image size
 * 
 * \param ImageBase Image base
 * 
 * \return Size of driver (if found)
 */
UINT32
NTAPI
GetImageSize(
	_In_  PVOID  ImageBase
) {
	PAGED_CODE();

	UINT32 ImageSize = 0;

	PIMAGE_DOS_HEADER Dos = (PIMAGE_DOS_HEADER)ImageBase;
	if (Dos->e_magic != IMAGE_DOS_SIGNATURE)
		return 0;

	PIMAGE_NT_HEADERS64 Nt64 = (PIMAGE_NT_HEADERS64)((UINT_PTR)ImageBase + Dos->e_lfanew);
	if (Nt64->Signature != IMAGE_NT_SIGNATURE)
		return 0;

	ImageSize = Nt64->OptionalHeader.SizeOfImage;

	return ImageSize;
}
