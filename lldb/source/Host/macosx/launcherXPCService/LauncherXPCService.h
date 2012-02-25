#ifndef LLDB_LauncherXPCService_h
#define LLDB_LauncherXPCService_h

// These XPC messaging keys are used for communication between Host.mm and the XPC service.
#define LauncherXPCServiceArgPrefxKey           "arg"
#define LauncherXPCServiceEnvPrefxKey           "env"
#define LauncherXPCServiceCPUTypeKey            "cpuType"
#define LauncherXPCServicePosixspawnFlagsKey    "posixspawnFlags"
#define LauncherXPCServiceChildPIDKey           "childPID"
#define LauncherXPCServiceErrorTypeKey          "errorType"
#define LauncherXPCServiceCodeTypeKey           "errorCode"

#endif
