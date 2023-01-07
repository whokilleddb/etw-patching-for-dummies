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

// The CLR runtime provider GUID is e13c0d23-ccbc-4e12-931b-d9cc2eee27e4.
static GUID ClrRuntimeProviderGuid = { 0xe13c0d23, 0xccbc, 0x4e12, { 0x93, 0x1b, 0xd9, 0xcc, 0x2e, 0xee, 0x27, 0xe4 } };

// Print process name
void __print_proc_name();
// Check if pid exists
int check_pid();
// Callback function that is called when an event is received
void __stdcall EventRecordCallback(EVENT_RECORD* EventRecord);
// Start logging session
void start_logger();
