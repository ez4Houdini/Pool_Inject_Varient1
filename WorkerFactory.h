//WorkerFactory.h
#pragma once
#include "pch.h"
#include <winternl.h>
#include <ntstatus.h>
#pragma comment(lib, "ntdll.lib")
#define WorkerFactoryBasicInformation 7

typedef struct _WORKER_FACTORY_BASIC_INFORMATION
{
    LARGE_INTEGER Timeout;              // 空闲超时（控制 worker 回收时间）
    LARGE_INTEGER RetryTimeout;        // 重试创建线程的延迟时间
    LARGE_INTEGER IdleTimeout;         // worker 空闲超时

    BOOLEAN Paused;                    // Worker Factory 是否暂停
    BOOLEAN TimerSet;                  // 是否设置了定时器
    BOOLEAN QueuedToExWorker;          // 是否已加入扩展 worker 队列
    BOOLEAN MayCreate;                 // 是否允许创建新线程
    BOOLEAN CreateInProgress;          // 是否正在创建线程
    BOOLEAN InsertedIntoQueue;         // 是否已插入队列
    BOOLEAN Shutdown;                  // 是否已关闭

    ULONG BindingCount;               // 绑定计数（通常与 IO/Work 绑定相关）
    ULONG ThreadMinimum;              // 最小线程数（线程池下限）
    ULONG ThreadMaximum;              // 最大线程数（线程池上限）

    ULONG PendingWorkerCount;         // 等待执行的 worker 数
    ULONG WaitingWorkerCount;         // 等待中的 worker 数
    ULONG TotalWorkerCount;           // 当前 worker 总数

    ULONG ReleaseCount;               // 释放计数（线程回收相关）

    LONGLONG InfiniteWaitGoal;        // 无限等待目标（内部调度用）

    PVOID StartRoutine;               // worker 启动函数入口（线程执行起点）

    // 🔥 关键字段：指向线程池池结构（FULL_TP_POOL）
    // 在 PoolParty / TP_WORK hijack / worker factory 利用中非常关键
    PVOID StartParameter;

    HANDLE ProcessId;                 // 关联进程 ID（或伪句柄/标识）

    SIZE_T StackReserve;              // 线程栈保留大小
    SIZE_T StackCommit;               // 线程栈提交大小

    NTSTATUS LastThreadCreationStatus;// 最近一次线程创建状态
} WORKER_FACTORY_BASIC_INFORMATION, * PWORKER_FACTORY_BASIC_INFORMATION;
// 定义 NtQueryInformationWorkerFactory 函数指针类型
// 用于查询 Worker Factory 对象的内部信息（属于 ntdll 的 Native API）






typedef NTSTATUS(NTAPI* PFN_NtQueryInformationWorkerFactory)(
    HANDLE WorkerFactoryHandle,                 // [in]  Worker Factory 对象句柄
    DWORD WorkerFactoryInformationClass,        // [in]  信息类别（决定查询什么结构）
    PVOID WorkerFactoryInformation,             // [out] 输出缓冲区，用于接收查询结果
    ULONG WorkerFactoryInformationLength,       // [in]  输出缓冲区大小（字节）
    PULONG ReturnLength                         // [out, optional] 实际返回数据长度
    );
NTSTATUS MyNtQueryInformationFactory(HANDLE hWorkerFactory, PWORKER_FACTORY_BASIC_INFORMATION pInfo);