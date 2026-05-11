//pch.h
#pragma once
#include <Windows.h>
#include <objbase.h>
#include <stdio.h>
#include <intrin.h>
#include <stdlib.h>
#include <Wbemidl.h>
#include <comutil.h>
#include <iostream>
#define WIN32_NO_STATUS
#include <Windows.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>
#include <winternl.h>
#include <TlHelp32.h>

#pragma comment(lib, "ntdll.lib")

#define ProcessBasicInformation 0 //ProcessInformationClass 枚举值，表示查询基本信息
#define ProcessHandleSnapshotInformation 51 //ProcessInformationClass 枚举值，表示查询句柄快照信息

typedef CLIENT_ID* PCLIENT_ID;

// 单个 HANDLE 条目信息
typedef struct _PROCESS_HANDLE_TABLE_ENTRY_INFO
{
    HANDLE HandleValue;
    // HANDLE 值
    // 例如：0x54

    ULONG_PTR HandleCount;
    // 当前对象被多少 HANDLE 引用
    // 类似引用计数

    ULONG_PTR PointerCount;
    // 内核对象指针引用计数
    // 通常 >= HandleCount

    ACCESS_MASK GrantedAccess;
    // 该 HANDLE 拥有的权限
    // 例如：
    // PROCESS_VM_READ
    // PROCESS_DUP_HANDLE
    // PROCESS_ALL_ACCESS

    ULONG ObjectTypeIndex;
    // 对象类型索引
    // 对应：
    // Process
    // Thread
    // File
    // Token
    // Event
    // 等等
    //
    // 不同 Windows 版本编号不同

    ULONG HandleAttributes;
    // HANDLE 属性
    // 常见：
    // OBJ_INHERIT
    // OBJ_PROTECT_CLOSE

    ULONG Reserved;
    // 保留字段

} PROCESS_HANDLE_TABLE_ENTRY_INFO,
* PPROCESS_HANDLE_TABLE_ENTRY_INFO;



// 整个 HANDLE 快照表
typedef struct _PROCESS_HANDLE_SNAPSHOT_INFORMATION
{
    ULONG_PTR NumberOfHandles;
    // 当前进程拥有的 HANDLE 数量

    ULONG_PTR Reserved;
    // 保留字段

    PROCESS_HANDLE_TABLE_ENTRY_INFO Handles[1];
    // HANDLE 数组（变长数组）
    //
    // 实际不是只有1个
    // 真正大小：
    //
    // [结构头]
    // [Handle1]
    // [Handle2]
    // [Handle3]
    // ...

} PROCESS_HANDLE_SNAPSHOT_INFORMATION, * PPROCESS_HANDLE_SNAPSHOT_INFORMATION;


// Native API 使用的 PID/TID 结构
//typedef struct _CLIENT_ID
//{
//    HANDLE UniqueProcess; // 目标进程PID
//    HANDLE UniqueThread;  // 目标线程TID
//
//} CLIENT_ID, * PCLIENT_ID;

//typedef struct __PUBLIC_OBJECT_TYPE_INFORMATION
//{
//    UNICODE_STRING TypeName;
//
//    ULONG Reserved[22];
//
//} PUBLIC_OBJECT_TYPE_INFORMATION,* PPUBLIC_OBJECT_TYPE_INFORMATION;


// NtOpenProcess //这里打开目标进程
typedef NTSTATUS(NTAPI* PFN_NTOpenProcess)
(
    PHANDLE            ProcessHandle,     // 输出：返回打开后的进程句柄
    ACCESS_MASK        DesiredAccess,     // 请求的访问权限
    POBJECT_ATTRIBUTES ObjectAttributes,  // 对象属性（通常为 NULL 或 InitializeObjectAttributes 初始化）
    PCLIENT_ID         ClientId           // 目标 PID/TID
    );


// NtQueryInformationProcess //这里用来找句柄表
typedef NTSTATUS(NTAPI* PFN_NTQueryInformationProcess)
(
    HANDLE ProcessHandle,                 // 目标进程句柄
    ULONG  ProcessInformationClass,       // 查询类型（如 ProcessBasicInformation）
    PVOID  ProcessInformation,            // 输出缓冲区
    ULONG  ProcessInformationLength,      // 缓冲区大小
    PULONG ReturnLength                   // 返回实际写入大小
    );


// NtDuplicateObject 这里偷句柄用的
typedef NTSTATUS(NTAPI* PFN_NTDuplicateObject)
(
    HANDLE SourceProcessHandle,           // 源进程句柄
    HANDLE SourceHandle,                  // 要复制的句柄
    HANDLE TargetProcessHandle,           // 目标进程句柄（通常 GetCurrentProcess()）
    PHANDLE TargetHandle,                 // 输出：复制后的句柄
    ACCESS_MASK DesiredAccess,            // 新句柄权限
    ULONG HandleAttributes,               // 句柄属性
    ULONG Options                         // 复制选项（如 DUPLICATE_SAME_ACCESS）
    );

//查询句柄对应的对象类型名称
typedef NTSTATUS(NTAPI* PFN_NTQueryObject)
(
    HANDLE Handle,                        // 要查询的句柄

    OBJECT_INFORMATION_CLASS ObjectInformationClass,
    // 查询类型
    // 如：
    // ObjectTypeInformation
    // ObjectNameInformation

    PVOID ObjectInformation,              // 输出缓冲区

    ULONG ObjectInformationLength,        // 缓冲区大小

    PULONG ReturnLength                   // 返回实际写入大小
    );
typedef NTSTATUS(NTAPI* PFN_NTClose)
(
    HANDLE Handle    // 要关闭的句柄
    );