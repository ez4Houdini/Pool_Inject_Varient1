#include "comAntiDebug.h"
int GetCpuCores()
{
    HRESULT hres;

    // 1. 初始化 COM
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
        return -1;

    // 2. 初始化安全层
    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );

    if (FAILED(hres))
    {
        CoUninitialize();
        return -1;
    }

    // 3. 创建 WMI locator
    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLoc
    );

    if (FAILED(hres))
    {
        CoUninitialize();
        return -1;
    }

    // 4. 连接 WMI namespace
    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL, NULL, 0,
        NULL, 0, 0,
        &pSvc
    );

    if (FAILED(hres))
    {
        pLoc->Release();
        CoUninitialize();
        return -1;
    }

    // 5. 设置安全级别
    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );

    // 6. 查询 CPU
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT NumberOfLogicalProcessors FROM Win32_Processor"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
    );

    int cores = 0;

    if (SUCCEEDED(hres))
    {
        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;

        if (pEnumerator)
        {
            pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

            if (uReturn)
            {
                VARIANT vtProp;
                pclsObj->Get(L"NumberOfLogicalProcessors", 0, &vtProp, 0, 0);

                cores = vtProp.intVal;

                VariantClear(&vtProp);
                pclsObj->Release();
            }
            pEnumerator->Release();
        }
    }

    // 7. 清理
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

    return cores;
}