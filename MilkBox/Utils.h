#pragma once

PVOID
NTAPI
CalcOffset(
	_In_ PVOID Base,
	_In_ INT32 Offset
);

PVOID
NTAPI
FindSection(
	_In_ const PVOID  ImageBase,
	_In_ const PUCHAR Query
);

VOID
NTAPI
CreateFileName(
	_Out_       PWCHAR  Str,
	_In_  const UINT32  Size,
	_In_  const PWCHAR  Format,
	...
);

PVOID
NTAPI
FindKernelBase(
	VOID
);

BOOLEAN
NTAPI
ValidateImage(
	_In_ PVOID   ImageBase
);

NTSTATUS
NTAPI
TestRelatedPhysAddr(
	_In_ PVOID ImageBase,
	_In_ INT32 Pages
);

TIME_FIELDS
NTAPI
GetCurrentTime(
	VOID
);

UINT32
NTAPI
GetImageSize(
	_In_  PVOID  ImageBase
);
