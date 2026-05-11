//HijackHandle.h
#pragma once
#pragma once

#include <Windows.h>

HANDLE Hijack_TP_WORK_HANDLE(PHANDLE phTargetProcess, DWORD pid);
DWORD getPIDByName(const wchar_t* TargetName);