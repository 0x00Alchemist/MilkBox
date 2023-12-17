#pragma once

// @note: @0x00Alchemist: Technically, the Windows loader and the HAL itself stores all runtime services, 
// however, it makes no sense to look at absolutely all pointers to functions, as some of them will never 
// be called or should not be called at all (SetVirtualAddressMap). 
typedef struct _HAL_EFI_RUNTIME_SERVICES_TABLE {
	PVOID GetTime;
	PVOID SetTime;

	PVOID GetVariable;
	PVOID GetNextVariableName;
	PVOID SetVariable;
	PVOID QueryVariableInfo;

	PVOID ResetSystem;
	PVOID ConvertPointer;

	PVOID UpdateCapsule;
	PVOID QueryCapsuleCapabilities;
} HAL_EFI_RUNTIME_SERVICES_TABLE, *PHAL_EFI_RUNTIME_SERVICES_TABLE;

typedef struct _HAL_EFI_RUNTIME_SERVICES_BLOCK {
	PHAL_EFI_RUNTIME_SERVICES_TABLE HalEfiRuntimeServices;
	SIZE_T                         ValueOfBlocks; // @note: @0x00Alchemist: need more researches, maybe, it's just a size of block
} HAL_EFI_RUNTIME_SERVICES_BLOCK, *PHAL_EFI_RUNTIME_SERVICES_BLOCK;

// @note: @0x00Alchemist: The HAL_IUM_EFI_WRAPPER_TABLE structure is located right after the HAL_EFI_RUNTIME_SERVICES_BLOCK structure 
// and is used only if VSL/DeviceGuard is enabled. It includes a structure with pointers to functions of HalpIum* type. 
// It's not used in the current form in the project, but it's planned to be used in the future
typedef struct _HAL_IUM_SERVICES {
	PVOID HalpIumGetTime;
	PVOID HalpIumSetTime;

	PVOID HalpIumResetSystem;

	PVOID HalpIumGetVariable;
	PVOID HalpIumGetNextVariableName;
	PVOID HalpIumSetVariable;

	PVOID HalpIumUpdateCapsule;

	PVOID HalpIumQueryVariableInfo;
} HAL_IUM_SERVICES, *PHAL_IUM_SERVICES;

typedef struct _HAL_IUM_EFI_WRAPPER_TABLE {
	PHAL_IUM_SERVICES HalIumServices;
	SIZE_T ValueOfBlocks;
} HAL_IUM_EFI_WRAPPER_TABLE, *PHAL_IUM_EFI_WRAPPER_TABLE;
