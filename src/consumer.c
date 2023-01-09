#include <math.h>
#include <stdio.h>
#include <windows.h>
#include <TlHelp32.h>
#include <evntcons.h>
#include <evntrace.h>
#include <Evntcons.h>
#include "consumer.h"
#include "providers.h"
#pragma comment(lib, "advapi32.lib")

// PID to monitor
unsigned int PID = 0;

// Print process name
void __print_proc_name(){
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        // Return without printing
        fprintf(stderr, "[!] Failed to capture Snapshot!\n");
        return ;
    }

    PROCESSENTRY32 process;
    process.dwSize = sizeof(process);

    if (Process32First(snapshot, &process)) {
        do {
            if (process.th32ProcessID == PID) {
                printf("[i] Process name\t%s\n", process.szExeFile);
                break;
            }
        } while (Process32Next(snapshot, &process));
    }

    CloseHandle(snapshot);
}


// Check if process with a given PID exists/is accessible
// if yes, the function prints it's name and returns 0
// else, it returns -1
int check_pid(){
    int ret;
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, (DWORD)PID);
    if (process != NULL) {
        __print_proc_name();
        ret = 0;
    } else {
        fprintf(stderr, "[!] Process does not exist (0x%x)\n", PID, GetLastError());
        ret = -1;
    }
    CloseHandle(process);
    return ret;
}


// Callback function that is called when an event is received
void __stdcall EventRecordCallback(EVENT_RECORD* EventRecord) {
    PEVENT_HEADER eventHeader = &EventRecord->EventHeader;
    PEVENT_DESCRIPTOR eventDescriptor = &eventHeader->EventDescriptor;
    AssemblyLoadUnloadRundown_V1* assemblyUserData;
    unsigned int pid =  eventHeader->ProcessId;

    if (pid == PID){
        switch (eventDescriptor->Id) {
            case AssemblyDCStart_V1:
                assemblyUserData = (AssemblyLoadUnloadRundown_V1*)EventRecord->UserData;
                wprintf(L"[%d] - Assembly: %s\n", eventHeader->ProcessId, assemblyUserData->FullyQualifiedAssemblyName);
            break;
        }
    }
}


// Start logging session
void start_logger(){
    ULONG result;
    // Structure used to specify the properties of an event trace log file
    // https://learn.microsoft.com/en-us/windows/win32/api/evntrace/ns-evntrace-event_trace_logfilea
    EVENT_TRACE_LOGFILEA trace;
    // Structure is used to specify the properties of an event trace session 
    EVENT_TRACE_PROPERTIES *traceProp;
    memset(&trace, 0, sizeof(EVENT_TRACE_LOGFILEA));
    // PROCESS_TRACE_MODE_REAL_TIME 
    // This flag indicates that the trace should be performed in real-time,
    // as events are generated, rather than being performed in a deferred manner.
    // PROCESS_TRACE_MODE_EVENT_RECORD
    // This flag indicates that the trace should be performed using event records,
    // rather than using event trace buffers.
    trace.ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD;
    trace.LoggerName = (LPSTR)(LOGGER_NAME);
    // Callback function
    trace.EventRecordCallback = (PEVENT_RECORD_CALLBACK)EventRecordCallback;
    ULONG bufferSize = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(LOGGER_NAME) + sizeof(WCHAR);
    traceProp = (EVENT_TRACE_PROPERTIES*)LocalAlloc(LPTR, bufferSize);
    // Total size of memory allocated, in bytes, for the event tracing session properties.
    traceProp->Wnode.BufferSize    = bufferSize;
    // Clock resolution to use when logging the time stamp for each event.
    traceProp->Wnode.ClientContext = 2;
    // Indicates that the Wnode contains a traced GUID
    traceProp->Wnode.Flags         = WNODE_FLAG_TRACED_GUID;
    // EVENT_TRACE_REAL_TIME_MODE
    // View data in real time
    // specify that the trace session should use paged memory for event storage.
    // EVENT_TRACE_USE_PAGED_MEMORY
    // In ETW, events are stored in a trace buffer in memory, and the trace buffer can either be allocated in 
    // paged memory or non-paged memory. Paged memory is the regular memory used by the operating system and
    // is subject to paging, which means that it can be moved to and from disk as needed. 
    // Non-paged memory is memory that is always resident in physical memory and cannot be paged out to disk.
    traceProp->LogFileMode         = EVENT_TRACE_REAL_TIME_MODE | EVENT_TRACE_USE_PAGED_MEMORY;
    // Offset (in bytes) from the start of this structure's allocated memory to beginning of 
    // the nul-terminated string that contains the log file name.
    traceProp->LogFileNameOffset   = 0;
    // Offset (in bytes) from the start of the structure's allocated memory to the beginning of 
    // the nul-terminated string that contains the session name.
    traceProp->LoggerNameOffset    = sizeof(EVENT_TRACE_PROPERTIES);

    TRACEHANDLE hTrace = 0;

    // Registers and starts an event tracing session.
    printf("[i] Trace Session\t%s\n", LOGGER_NAME);
    result = StartTraceA(&hTrace, (LPCSTR)LOGGER_NAME, traceProp) ;
    if (result != ERROR_SUCCESS) {
        fprintf(stderr, "[!] Error starting trace: %d\n", result);
        exit(-4);
    }

    // Configure how an ETW event provider logs events to a trace session.
    result = EnableTraceEx(
        &MicrosoftWindowsDotNETRuntime,    // CLR Runtime Provider
        NULL,                              // No source identity is needed 
        hTrace,                            // Handle of the event tracing session for which you are configuring the provider.
        1,                                 // Set to 1 to enable receiving events from the provider
        TRACE_LEVEL_VERBOSE,               // 	Detailed diagnostic events
        0x8,                               // LoaderKeyword
        0,
        0,                                 // No Special Behaviour
        NULL
    );

    if (result != ERROR_SUCCESS) {
        fprintf(stderr, "[!] Error starting trace: %d (0x%x)\n", result, GetLastError());
        exit(-5);
    }

    // The OpenTrace function opens an ETW trace processing handle for 
    // consuming events from an ETW real-time trace session or an ETW log file.
    hTrace = OpenTrace(&trace);
    if (hTrace == INVALID_PROCESSTRACE_HANDLE) {
        fprintf(stderr, "[!] OpenTrace() failed (0x%x)\n", GetLastError());
        exit(-6);
    }

    // Listen for events
    printf("[i] Listening for events!\n");
    while (!_kbhit()){
        // Delivers events from one or more ETW trace processing sessions to the consumer.
        result = ProcessTrace(&hTrace, 1, NULL, NULL);
        if (result != ERROR_SUCCESS) {
            fprintf(stderr, "[!] ProcessTrace() failed (0x%x)\n", GetLastError());
            exit(-7);
        }
    }

    // Close the session
    CloseTrace(hTrace);
}


void main(int argc, char **argv){
    printf("[i] Microsoft-Windows-DotNETRuntime Event Consumer - @whokilleddb\n");

    unsigned int pid;
    if (argc != 2){
        fprintf(stderr, "[!] Incorrect Usage\n");
        fprintf(stderr, "[!] Usage: %s <PID>\n", argv[0]);
        exit(-1);
    }

    PID = (unsigned int)atoi(argv[1]);

    if (PID == 0){
        fprintf(stderr, "[!] Enter valid PID!\n");
        exit(-2);
    }

    printf("[i] Process PID\t\t%u\n", PID);

    // Check if process with given PID exists
    if (check_pid()<0){
        exit(-3);
    };

    // Start logger session
    start_logger();
}
