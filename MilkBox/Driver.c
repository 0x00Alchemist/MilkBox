#include <ntddk.h>
#include <wdm.h>

#include "Runtime.h"
#include "DumpFile.h"

#pragma code_seg("PAGE")

#define IOCTL_LOCATE_RT_DRIVERS	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1100, METHOD_OUT_DIRECT, FILE_SPECIAL_ACCESS)
#define IOCTL_DUMP_RT_DRIVERS	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1105, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

static MB_LIST MilkBoxList = { 0 };


NTSTATUS
NTAPI
DriverDispatch(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PIRP Irp
) {
	PAGED_CODE();

	UNREFERENCED_PARAMETER(DriverObject);

	NTSTATUS Status = STATUS_SUCCESS;
	PIO_STACK_LOCATION IoStackLocation = IoGetCurrentIrpStackLocation(Irp);
	
	ULONG ControlCode = IoStackLocation->Parameters.DeviceIoControl.IoControlCode;
	switch(ControlCode) {
		case IOCTL_LOCATE_RT_DRIVERS: {
			Status = FindRuntimeImages(&MilkBoxList);
			if(!NT_SUCCESS(Status))
				KdPrint(("[ MilkBox ] Cannot find RT drivers!\n"));
		}
		break;
		case IOCTL_DUMP_RT_DRIVERS:
			if(MilkBoxList.Count == 0) {
				KdPrint(("[ MilkBox ] Unable to get list of runtime images\n"));
				break;
			}

			Status = ProcessDump(&MilkBoxList);
			if(!NT_SUCCESS(Status))
				KdPrint(("[ MilkBox ] Unable to create dumps! (However, something may have been dumped)\n"));
		break;
		default:
			Status = STATUS_INVALID_PARAMETER_2;
		break;
	}

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = Status;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Status;
}

NTSTATUS
NTAPI
DriverCreateClose(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PIRP Irp
) {
	UNREFERENCED_PARAMETER(DriverObject);

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = STATUS_SUCCESS;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
DriverUnimplemented(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PIRP Irp
) {
	UNREFERENCED_PARAMETER(DriverObject);

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = STATUS_ILLEGAL_FUNCTION;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS 
NTAPI
DriverUnload(
	_In_ PDRIVER_OBJECT DriverObject
) {
	UNICODE_STRING SymLink = RTL_CONSTANT_STRING(L"\\DosDevices\\MilkBox");
	
	IoDeleteSymbolicLink(&SymLink);
	IoDeleteDevice(DriverObject->DeviceObject);

	return STATUS_SUCCESS;
}

NTSTATUS 
NTAPI
DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject, 
	_In_ PUNICODE_STRING RegistryPath
) {
	UNREFERENCED_PARAMETER(RegistryPath);

	// @note: @0x00Alchemist: reassign IRP hanlers
	for(INT32 i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
		DriverObject->MajorFunction[i] = DriverUnimplemented;

	DriverObject->DriverUnload = DriverUnload;

	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDispatch;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCreateClose;

	DEVICE_OBJECT DeviceObject;
	RtlSecureZeroMemory(&DeviceObject, sizeof(DEVICE_OBJECT));

	// @note: @0x00Alchemist: register device object
	UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\MilkBox");
	NTSTATUS Status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, TRUE, &DeviceObject);
	if(!NT_SUCCESS(Status)) {
		KdPrint(("[ MilkBox ] Cannot create device!\n"));
		return Status;
	}

	DeviceObject.Flags |= DO_BUFFERED_IO;
	DeviceObject.Flags &= DO_DEVICE_INITIALIZING;

	UNICODE_STRING SymLink = RTL_CONSTANT_STRING(L"\\DosDevices\\MilkBox");
	IoCreateSymbolicLink(&SymLink, &DeviceName);

	return STATUS_SUCCESS;
}
