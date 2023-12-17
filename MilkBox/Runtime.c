#include <ntddk.h>
#include <ntimage.h>
#include <wdm.h>

#include "HAL.h"
#include "Runtime.h"
#include "Utils.h"

#define RT_SERVICES_COUNT	(sizeof(HAL_EFI_RUNTIME_SERVICES_TABLE) / sizeof(PVOID))


UINT32
NTAPI
GetImageSize(
	_In_  PVOID  ImageBase
) {
	PAGED_CODE();

	UINT32 ImageSize = 0;

	PIMAGE_DOS_HEADER Dos = (PIMAGE_DOS_HEADER)ImageBase;
	if(Dos->e_magic != IMAGE_DOS_SIGNATURE)
		return 0;

	PIMAGE_NT_HEADERS64 Nt64 = (PIMAGE_NT_HEADERS64)((UINT_PTR)ImageBase + Dos->e_lfanew);
	if(Nt64->Signature != IMAGE_NT_SIGNATURE)
		return 0;

	ImageSize = Nt64->OptionalHeader.SizeOfImage;

	return ImageSize;
}

PVOID
NTAPI
FindBase(
	_In_ PVOID RuntimeFunction
) {
	PAGED_CODE();

	PVOID SearchBase = NULL;

	// @note: @0x00Alchemist: value of pages can be changed (minimum 8 pages needed). 
	// Raising pages under scanning can cause slowdowns
	INT PagesToScan = 16;

	// @note: @0x00Alchemist: search in range of function for DOS headers
	SearchBase = PAGE_ALIGN(RuntimeFunction);
	while(PagesToScan--) {
		// @note: @0x00Alchemist: because there's should be associated physical address of runtime driver.
		// If it's 0, driver not associated with anything
		PHYSICAL_ADDRESS Phys = MmGetPhysicalAddress(SearchBase);
		if(Phys.QuadPart == 0)
			return NULL;

		PIMAGE_DOS_HEADER Dos = (PIMAGE_DOS_HEADER)SearchBase;
		if(Dos->e_magic == IMAGE_DOS_SIGNATURE) {
			PIMAGE_NT_HEADERS64 Nt64 = (PIMAGE_NT_HEADERS64)CalcOffset(SearchBase, Dos->e_lfanew);
			if(Nt64->Signature == IMAGE_NT_SIGNATURE)
				return SearchBase;
		}
			

		SearchBase = ((UINT_PTR)SearchBase - PAGE_SIZE);
	}

	return SearchBase;
}

VOID
NTAPI
FindRuntimeDriverByFunction(
	_In_  PHAL_EFI_RUNTIME_SERVICES_TABLE HalEfiRuntimeServicesTable,
	_Out_ PMB_DRIVER_INFO                 DriverInfo,
	_Out_ SIZE_T                          *Entries
) {
	PAGED_CODE();

	// @note: @0x00Alchemist: search driver for each function 
	SIZE_T Count = 0;
	for(INT i = 0; i < RT_SERVICES_COUNT; i++) {
		// @note: @0x00Alchemist: some of RT services can be NULL, skip them
		if(((PVOID *)HalEfiRuntimeServicesTable)[i] != NULL) {
			PVOID Base = FindBase(((PVOID *)HalEfiRuntimeServicesTable)[i]);
			if(Base != NULL) {
				// @note: @0x00Alchemist: save base address of image
				DriverInfo[i].Base = Base;
				*Entries = Count;

				Count++;
			}
		}
	}
}

PVOID
NTAPI
LocateRuntimeBlock(
	VOID
) {
	PAGED_CODE();

	// @note: @0x00Alchemist: locate kernel base address
	PVOID ImageBase = FindKernelBase();
	if(ImageBase == NULL) {
		KdPrint(("[ MilkBox ] Cannot find kernel base address!\n"));
		return NULL;
	}

	// @note: @0x00Alchemist: locate CFGRO section
	PVOID RawBlock = FindSection(ImageBase, "CFGRO\0\0");
	if(RawBlock == NULL) {
		KdPrint(("[ MilkBox ] Cannot find \"CFGRO\" section!\n"));
		return NULL;
	}

	// @note: @0x00Alchemist: should be after RtlpInvertedFunctionTable (in 8 bytes)
	RawBlock = CalcOffset(RawBlock, 8);

	// @note: @0x00Alchemist: I wrote it under a bottle of whiskey
	MB_TABLE_OFFSET_INFO Offset = { 0 };

	SIZE_T Copied = 0;
	MM_COPY_ADDRESS Address;

	Address.VirtualAddress = RawBlock;
	NTSTATUS Status = MmCopyMemory(&Offset, Address, sizeof(Offset), MM_COPY_MEMORY_VIRTUAL, &Copied);
	if(!NT_SUCCESS(Status)) {
		KdPrint(("[ MilkBox ] Cannot copy memory! Status: 0x%X\n", Status)); 
		return NULL;
	}

	return Offset.Address;
}


NTSTATUS 
NTAPI
FindRuntimeImages(
	_Out_ PMB_LIST MbList
) {
	PAGED_CODE();

	// @note: @0x00Alchemist: locate HAL_EFI_RUNTIME_SERVICES_TABLE
	PVOID Address = LocateRuntimeBlock();
	if(Address == NULL)
		return STATUS_NOT_FOUND;

	// @note: @0x00Alchemist: copy block
	SIZE_T Copied = 0;
	MM_COPY_ADDRESS TableAddress;
	HAL_EFI_RUNTIME_SERVICES_TABLE HalEfiRuntimeServicesTable = { 0 };

	TableAddress.VirtualAddress = Address;
	NTSTATUS Status = MmCopyMemory(&HalEfiRuntimeServicesTable, TableAddress, sizeof(HalEfiRuntimeServicesTable), MM_COPY_MEMORY_VIRTUAL, &Copied);
	if(!NT_SUCCESS(Status)) {
		KdPrint(("[ MilkBox ] Cannot copy memory! Status: 0x%X\n", Status));
		return Status;
	}

	// @note: @0x00Alchemist: find drivers to related functions
	SIZE_T Count;
	MB_DRIVER_INFO DriverInfo[RT_SERVICES_COUNT] = { 0 };
	FindRuntimeDriverByFunction(&HalEfiRuntimeServicesTable, &DriverInfo, &Count);

	// @note: @0x00Alchemist: collect information about drivers (do not worry about address collisions, we exclude it later)
	for(INT i = 0; i < Count; i++) {
		if(DriverInfo[i].Base != NULL) {
			UINT32 Size = GetImageSize(DriverInfo[i].Base);
			if(Size > 0) {
				MbList->AreaInfo[i].ImageBase = DriverInfo[i].Base;
				MbList->AreaInfo[i].ImageSize = Size;
			}
		}
	}

	MbList->Count = Count;

	return STATUS_SUCCESS;
}
