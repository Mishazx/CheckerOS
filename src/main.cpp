#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <sysinfoapi.h>
#include <wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#elif __linux__
#include <sys/utsname.h>
#include <unistd.h>
#include <fstream>
#elif __APPLE__
#include <sys/utsname.h>
#include <unistd.h>
#include <sys/sysctl.h>
#endif

void getOSInfo() {
#ifdef _WIN32
    OSVERSIONINFO osvi;
    SYSTEM_INFO si;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (GetVersionEx(&osvi)) {
        std::cout << "Operating System: Windows\n";
        std::cout << "Version: " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion << "\n";
        std::cout << "Build Number: " << osvi.dwBuildNumber << "\n";
    }

    GetSystemInfo(&si);
    std::cout << "Architecture: ";
    switch (si.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64:
        std::cout << "64-bit\n";
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        std::cout << "32-bit\n";
        break;
    default:
        std::cout << "Unknown architecture\n";
        break;
    }
#elif __linux__ || __APPLE__
    struct utsname buffer;
    if (uname(&buffer) == 0) {
        std::cout << "Operating System: " << buffer.sysname << "\n";
        std::cout << "Version: " << buffer.version << "\n";
        std::cout << "Release: " << buffer.release << "\n";
        std::cout << "Architecture: " << buffer.machine << "\n";
    } else {
        std::cerr << "Unable to retrieve OS information.\n";
    }
#endif
}

void getCPUInfo() {
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    std::cout << "Number of Processors: " << si.dwNumberOfProcessors << "\n";

    HRESULT hres;
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        std::cout << "Failed to initialize COM library. Error code = " << hres << "\n";
        return;
    }

    hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    if (FAILED(hres)) {
        std::cout << "Failed to initialize security. Error code = " << hres << "\n";
        CoUninitialize();
        return;
    }

    IWbemLocator *pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);
    if (FAILED(hres)) {
        std::cout << "Failed to create IWbemLocator object. Error code = " << hres << "\n";
        CoUninitialize();
        return;
    }

    IWbemServices *pSvc = NULL;
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
    if (FAILED(hres)) {
        std::cout << "Could not connect. Error code = " << hres << "\n";
        pLoc->Release();
        CoUninitialize();
        return;
    }

    hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
    if (FAILED(hres)) {
        std::cout << "Could not set proxy blanket. Error code = " << hres << "\n";
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT Name FROM Win32_Processor"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
    if (FAILED(hres)) {
        std::cout << "Query for processor name failed. Error code = " << hres << "\n";
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn) {
            break;
        }

        VARIANT vtProp;
        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        std::cout << "Processor Name: " << vtProp.bstrVal << "\n";
        VariantClear(&vtProp);
        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

#elif __linux__
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("model name") != std::string::npos) {
            std::cout << "Processor Name: " << line.substr(line.find(":") + 2) << "\n";
            break;
        }
    }
    cpuinfo.close();
#elif __APPLE__
    char buffer[128];
    size_t bufferlen = 128;
    sysctlbyname("machdep.cpu.brand_string", &buffer, &bufferlen, NULL, 0);
    std::cout << "Processor Name: " << buffer << "\n";
#endif
}

int main() {
    std::cout << "System Information:\n";
    getOSInfo();
    getCPUInfo();
    return 0;
}
