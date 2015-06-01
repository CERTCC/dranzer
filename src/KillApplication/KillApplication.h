#ifndef __KillApplications_H
#define __KillApplications_H
#include <windows.h>

#define TA_FAILED 0
#define TA_SUCCESS_CLEAN 1
#define TA_SUCCESS_KILL 2
#define TA_SUCCESS_16 3

typedef struct
{
  DWORD        PID;
  char         Name[MAX_PATH];
} TProcessInfo;

typedef struct
{
 DWORD         NumProcesses;
 TProcessInfo *ProcessInfo;
} TProcessSnapShot;

DWORD WINAPI TerminateApp( DWORD dwPID, DWORD dwTimeout ) ;
DWORD WINAPI Terminate16App( DWORD dwPID, DWORD dwThread,
                        WORD w16Task, DWORD dwTimeout );
int KillAllProcessesByName(char* lpProcessName);
int GetProcessIdByName(char* lpProcessName,DWORD *dwPID);
int FreeSnapShot(TProcessSnapShot *SnapShot);
TProcessSnapShot * GetSnapShotOfProcesses(void);
int KillProcessesNotInSnapShot(TProcessSnapShot *SnapShot);

#endif