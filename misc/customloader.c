// Run managed code from the Unmanaged Land
#include <stdio.h>
#define CINTERFACE
#define COBJMACROS

#include <metahost.h>
#include <Unknwnbase.h>
#pragma comment(lib, "mscoree.lib")
int main(){
    HRESULT result;
    
    // Interface for interacting with the CLR
    // https://learn.microsoft.com/en-us/dotnet/framework/unmanaged-api/hosting/iclrmetahost-interface
    ICLRMetaHost* pMetaHost = NULL;

    // Create instance of CLR
    // https://learn.microsoft.com/en-us/dotnet/framework/unmanaged-api/hosting/clrcreateinstance-function
    // Also note that the C and C++ versions of the same will be different
    // https://stackoverflow.com/questions/1351589/error-c2440-function-cannot-convert-from-const-iid-to-dword
    result = CLRCreateInstance(
                &CLSID_CLRMetaHost,          // The CLSID_CLRMetaHost is used to create an instance of the CLRMetaHost class 
                &IID_ICLRMetaHost,           // Corressponding RIID 
                (LPVOID*)&pMetaHost);        // Interface

    if (result != S_OK){
        fprintf(stderr, "[!] CLRCreateInstance() function failed (0x%x)\n", result);
        return -1;
    }
    printf("[i] Created CLR Instance\n");

    // Enumerate installed CLR runtimes
    IEnumUnknown * runtime = NULL;
    result = ICLRMetaHost_EnumerateInstalledRuntimes(pMetaHost, &runtime);
    if (result != S_OK){
        fprintf(stderr, "[!] EnumerateInstalledRuntimes() function failed (0x%x)\n", result);
        return -2;
    }
    printf("[i] Enumerated Runtimes\n"); 

    // Iterate over runtimes
    DWORD count;
    IUnknown *unk;
    WCHAR buf[MAX_PATH];
    ICLRRuntimeInfo *runtimeinfo;
    while((result = IEnumUnknown_Next(
                        runtime,            
                        1,                  // Number of elements to retrieve
                        &unk,               // Pointer to an array of IUnknown pointers that will receive the elements
                        0)                  // Pointer to a ULONG variable that will receive the number of elements actually retrieved.
                    ) == S_OK){
        
        // Obtain a pointer to another interface on an object. 
        result = IUnknown_QueryInterface(
                    unk,                     // Pointer to the IUnknown interface of the object being queried.
                    &IID_ICLRRuntimeInfo,    
                    (void**)&runtimeinfo);

        count = MAX_PATH;
        if (result == S_OK){
            // Get Version String
            result =  ICLRRuntimeInfo_GetVersionString(
                runtimeinfo,
                buf,
                &count                
            );
            if (result == S_OK && count != 0) {
                wprintf(L"[i] Found Runtime Version: %s\n", buf);
            }
            else {
                fprintf(stderr, "[!] ICLRRuntimeInfo_GetVersionString() Failed (0x%x)\n", result);
            }
        }   
        ICLRRuntimeInfo_Release(runtimeinfo);
        IUnknown_Release(unk);
        break;
    }

    // Check if a specific version of the CLR is loadable on the current system
    BOOL bLoadable;
    ICLRRuntimeInfo_IsLoadable(runtimeinfo, &bLoadable);
    if (bLoadable == FALSE){
        fprintf(stderr, "[!] Specified version of CLR is not loadable\n");
        return -7;
    }

    // Get last supported runtime
    ICLRRuntimeHost *runtimehost = NULL;
    result = ICLRRuntimeInfo_GetInterface(
        runtimeinfo,
        &CLSID_CLRRuntimeHost, 
        &IID_ICLRRuntimeHost, 
        (LPVOID*)&runtimehost);
    
    if (result != S_OK){
        fprintf(stderr, "[!] ICLRRuntimeInfo_GetInterface() function failed (0x%x)\n", result);
        return -3;
    }

    // Check runtime host
    if (count != 0 && NULL == runtimehost){
        fprintf(stderr, "[!] No Valid Runtime Found\n");
        return -4;
    }
    wprintf(L"[i] Using Runtime: %s\n", buf);

    // Start Runtime
    printf("[i] Starting Runtime\n");
    result =  ICLRRuntimeHost_Start(runtimehost);
    if (result != S_OK){
        fprintf(stderr, "[!] ICLRRuntimeHost_Start() function failed (0x%x)\n", result);
        return -5;
    }

    printf("\n[i] Entering Land of Managed Code\n");
    DWORD res = 0;
    const LPCWSTR dll_path = L"C:\\Users\\whokilleddb\\Codes\\etw-patching-for-dummies\\misc\\helloworld.dll";
    result = ICLRRuntimeHost_ExecuteInDefaultAppDomain(
        runtimehost,
        dll_path,
        L"HelloWorld.Program",
        L"EntryPoint",
        L"Hello There (General Kenobi)",
        &res
    );

    if (result != S_OK){
        fprintf(stderr, "[!] ICLRRuntimeHost_ExecuteInDefaultAppDomain() function failed (0x%x)\n", result);
        if (result==0x80070002){
            fprintf(stderr, "[!] The speicified .NET assembly could not be found\n");
        }
        return -6;
    }
    printf("[i] Exit Code: %d\n", res);
    printf("[i] Back to Unmanaged Land\n\n");

    // Stop Runtime
    printf("[i] Stopping Runtime\n");
    result =  ICLRRuntimeHost_Stop(runtimehost);
    if (result != S_OK){
        fprintf(stderr, "[!] ICLRRuntimeHost_Stop() function failed (0x%x)\n", result);
        return -6;
    }

    printf("[i] Performing Cleanup!\n");
    IEnumUnknown_Release(runtime);
    ICLRRuntimeInfo_Release(runtimeinfo);
    ICLRRuntimeHost_Release(runtimehost);
    ICLRMetaHost_Release(pMetaHost);
    printf("[i] Cleanup Done!\n");
    return 0;
}
