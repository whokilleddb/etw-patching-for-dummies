#include <Windows.h>
#include <windef.h>
#define LOGGER_NAME "ETW_Session_Tracer"
#define AssemblyDCStart_V1 155
#pragma once

#pragma pack(1)
typedef struct _AssemblyLoadUnloadRundown_V1
{
    ULONG64 AssemblyID;
    ULONG64 AppDomainID;
    ULONG64 BindingID;
    ULONG AssemblyFlags;
    WCHAR FullyQualifiedAssemblyName[1];
} AssemblyLoadUnloadRundown_V1, *PAssemblyLoadUnloadRundown_V1;
#pragma pack()

// Print process name
void __print_proc_name();
// Check if pid exists
int check_pid();
// Callback function that is called when an event is received
void __stdcall EventRecordCallback(EVENT_RECORD* EventRecord);
// Start logging session
void start_logger();
