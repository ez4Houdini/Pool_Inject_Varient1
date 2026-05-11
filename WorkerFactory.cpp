//WorkerFactory.cpp
#include "WorkerFactory.h"
NTSTATUS MyNtQueryInformationFactory(HANDLE hWorkerFactory, PWORKER_FACTORY_BASIC_INFORMATION pInfo)
{
	PFN_NtQueryInformationWorkerFactory NtQueryInformationWorkerFactory =(PFN_NtQueryInformationWorkerFactory)GetProcAddress( GetModuleHandleW(L"ntdll.dll"),"NtQueryInformationWorkerFactory");
	if (!NtQueryInformationWorkerFactory)
	{
		printf("未找到NtQueryInformationWorkerFactory函数\n");
		return STATUS_NOT_FOUND;
	}

	ULONG returnLength = 0;
	NTSTATUS status = NtQueryInformationWorkerFactory(hWorkerFactory, WorkerFactoryBasicInformation, pInfo, sizeof(WORKER_FACTORY_BASIC_INFORMATION), &returnLength);
	if(status!= STATUS_SUCCESS)
	{
		printf("查询Worker Factory信息失败，状态码: 0x%X\n", status);
		return status;
	}
	printf("Worker Factory信息查询成功，返回长度: %lu 字节\n", returnLength);
	return status;
}