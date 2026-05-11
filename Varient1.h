//Varient1.h
#pragma once
#include "pch.h"
#include "HijackHandle.h"
#include "WorkerFactory.h"
BOOL inject(unsigned char* pShellcode,SIZE_T shellcodeSize);

typedef NTSTATUS(NTAPI* PFN_NtSetInformationWorkerFactory)
(
    HANDLE WorkerFactoryHandle,
    ULONG WorkerFactoryInformationClass,
    PVOID WorkerFactoryInformation,
    ULONG WorkerFactoryInformationLength
);

NTSTATUS MyNtSetInformationWorkerFactory
(
    HANDLE WorkerFactoryHandle,
    ULONG WorkerFactoryInformationClass,
    PVOID WorkerFactoryInformation,
    ULONG WorkerFactoryInformationLength
);