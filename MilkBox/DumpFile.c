#include <ntddk.h>
#include <wdm.h>
#include <ntimage.h>
#include <ntintsafe.h>

#include "Utils.h"
#include "Runtime.h"

static UINT64 PreviousImageBase = 0;


NTSTATUS
NTAPI
WriteDumpFile(
	_In_ PVOID          ImageBase,
	_In_ SIZE_T         ImageSize,
	_In_ UNICODE_STRING FileName
) {	
	PAGED_CODE();

	// @note: @0x00Alchemist: because I'm paranoidal
	if(!ValidateImage(ImageBase, ImageSize)) {
		KdPrint(("[ MilkBox ] Invalid image!\n"));
		return STATUS_INVALID_ADDRESS;
	}

	// @note: @0x00Alchemist: round image size to pages
	ULONG Size = 0;
	INT32 Pages = BYTES_TO_PAGES(ImageSize);
	NTSTATUS Status = RtlULongMult(Pages, PAGE_SIZE, &Size);
	if(!NT_SUCCESS(Status)) {
		KdPrint(("[ MilkBox ] RtlULongMult failed\n"));
		return Status;
	}

	// @note: @0x00Alchemist: check if virtual addresses of theoretical RT driver corresponds with physical
	Status = TestRelatedPhysAddr(ImageBase, Pages);
	if(!NT_SUCCESS(Status)) {
		KdPrint(("[ MilkBox ] Found unrelated page!\n"));
		return Status;
	}

	// @note: @0x00Alchemist: create and write dump
	OBJECT_ATTRIBUTES ObjAttr;
	InitializeObjectAttributes(&ObjAttr, &FileName, (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), NULL, NULL);

	HANDLE hFile;
	IO_STATUS_BLOCK IoStatusBlock;
	Status = ZwCreateFile(&hFile, (FILE_READ_DATA | FILE_WRITE_DATA), &ObjAttr, &IoStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL, (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE), FILE_OVERWRITE_IF, (FILE_SYNCHRONOUS_IO_ALERT | FILE_NON_DIRECTORY_FILE), NULL, 0);
	if(!NT_SUCCESS(Status)) {
		KdPrint(("[ MilkBox ] Cannot create dump file! Status: 0x%X\n", Status));
		return Status;
	}

	LARGE_INTEGER ByteOffset = { 0 };
	Status = ZwWriteFile(hFile, NULL, NULL, NULL, &IoStatusBlock, ImageBase, Size, &ByteOffset, NULL);
	if(!NT_SUCCESS(Status))
		KdPrint(("[ MilkBox ] Cannot write data to file! Status: 0x%X\n", Status));

	ZwClose(hFile);

	return Status;
}

NTSTATUS
NTAPI
ProcessDump(
	_In_ PMB_LIST MbList
) {
	PAGED_CODE();

	NTSTATUS Status = STATUS_SUCCESS;

	for(INT i = 0; i < MbList->Count; i++) {
		// @todo: @0x00Alchemist: remove hardcoded path
		if(MbList->AreaInfo[i].ImageBase != PreviousImageBase) {
			WCHAR Name[255] = { 0 };
			TIME_FIELDS CurrentTime = GetCurrentTime();
			
			CreateFileName(Name, 255, L"\\DosDevices\\C:\\MilkBox\\[0x%llX]-RT_Driver-%d-%d-%d.bin", MbList->AreaInfo[i].ImageBase, CurrentTime.Day, CurrentTime.Month, CurrentTime.Year);
			
			UNICODE_STRING FileName;
			RtlInitUnicodeString(&FileName, Name);

			Status = WriteDumpFile(MbList->AreaInfo[i].ImageBase, MbList->AreaInfo[i].ImageSize, FileName);
			if(!NT_SUCCESS(Status))
				break;
		
			PreviousImageBase = MbList->AreaInfo[i].ImageBase;
		}
	}

	return Status;
}
