#include "stdafx.h"   
#include "KillApplication.h"
#include <vdmdbg.h>
#include <malloc.h>
#include <stdio.h>
#include <tlhelp32.h>

   typedef struct
   {
      DWORD   dwID ;
      DWORD   dwThread ;
   } TERMINFO ;

   // Declare Callback Enum Functions.
   BOOL CALLBACK TerminateAppEnum( HWND hwnd, LPARAM lParam ) ;

   BOOL CALLBACK Terminate16AppEnum( HWND hwnd, LPARAM lParam ) ;

   /*----------------------------------------------------------------
   DWORD WINAPI TerminateApp( DWORD dwPID, DWORD dwTimeout )

   Purpose:
      Shut down a 32-Bit Process (or 16-bit process under Windows 95)

   Parameters:
      dwPID
         Process ID of the process to shut down.

      dwTimeout
         Wait time in milliseconds before shutting down the process.

   Return Value:
      TA_FAILED - If the shutdown failed.
      TA_SUCCESS_CLEAN - If the process was shutdown using WM_CLOSE.
      TA_SUCCESS_KILL - if the process was shut down with
         TerminateProcess().
      NOTE:  See header for these defines.
   ----------------------------------------------------------------*/
   DWORD WINAPI TerminateApp( DWORD dwPID, DWORD dwTimeout )
   {
      HANDLE   hProc ;
      DWORD   dwRet ;

      // If we can't open the process with PROCESS_TERMINATE rights,
      // then we give up immediately.
      hProc = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, FALSE,
         dwPID);

      if(hProc == NULL)
      {
         return TA_FAILED ;
      }

      // TerminateAppEnum() posts WM_CLOSE to all windows whose PID
      // matches your process's.
      EnumWindows((WNDENUMPROC)TerminateAppEnum, (LPARAM) dwPID) ;

      // Wait on the handle. If it signals, great. If it times out,
      // then you kill it.
      if(WaitForSingleObject(hProc, dwTimeout)!=WAIT_OBJECT_0)
         dwRet=(TerminateProcess(hProc,0)?TA_SUCCESS_KILL:TA_FAILED);
      else
         dwRet = TA_SUCCESS_CLEAN ;

      CloseHandle(hProc) ;

      return dwRet ;
   }

   /*----------------------------------------------------------------
   DWORD WINAPI Terminate16App( DWORD dwPID, DWORD dwThread,
                        WORD w16Task, DWORD dwTimeout )

   Purpose:
      Shut down a Win16 APP.

   Parameters:
      dwPID
         Process ID of the NTVDM in which the 16-bit application is
         running.

      dwThread
         Thread ID of the thread of execution for the 16-bit
         application.

      w16Task
         16-bit task handle for the application.

      dwTimeout
         Wait time in milliseconds before shutting down the task.

   Return Value:
      If successful, returns TA_SUCCESS_16
      If unsuccessful, returns TA_FAILED.
      NOTE:  These values are defined in the header for this
      function.

   NOTE:
      You can get the Win16 task and thread ID through the
      VDMEnumTaskWOW() or the VDMEnumTaskWOWEx() functions.
   ----------------------------------------------------------------*/
   DWORD WINAPI Terminate16App( DWORD dwPID, DWORD dwThread,
                        WORD w16Task, DWORD dwTimeout )
   {
      HINSTANCE      hInstLib ;
      TERMINFO      info ;

      // You will be calling the functions through explicit linking
      // so that this code will be binary compatible across
      // Win32 platforms.
      BOOL (WINAPI *lpfVDMTerminateTaskWOW)(DWORD dwProcessId,
         WORD htask) ;

      hInstLib = LoadLibraryA( "VDMDBG.DLL" ) ;
      if( hInstLib == NULL )
         return TA_FAILED ;

      // Get procedure addresses.
      lpfVDMTerminateTaskWOW = (BOOL (WINAPI *)(DWORD, WORD ))
         GetProcAddress( hInstLib, "VDMTerminateTaskWOW" ) ;

      if( lpfVDMTerminateTaskWOW == NULL )
      {
         FreeLibrary( hInstLib ) ;
         return TA_FAILED ;
      }

      // Post a WM_CLOSE to all windows that match the ID and the
      // thread.
      info.dwID = dwPID ;
      info.dwThread = dwThread ;
      EnumWindows((WNDENUMPROC)Terminate16AppEnum, (LPARAM) &info) ;

      // Wait.
      Sleep( dwTimeout ) ;

      // Then terminate.
      lpfVDMTerminateTaskWOW(dwPID, w16Task) ;

      FreeLibrary( hInstLib ) ;
      return TA_SUCCESS_16 ;
   }

   BOOL CALLBACK TerminateAppEnum( HWND hwnd, LPARAM lParam )
   {
      DWORD dwID ;

      GetWindowThreadProcessId(hwnd, &dwID) ;

      if(dwID == (DWORD)lParam)
      {
         PostMessage(hwnd, WM_CLOSE, 0, 0) ;
      }

      return TRUE ;
   }

   BOOL CALLBACK Terminate16AppEnum( HWND hwnd, LPARAM lParam )
   {
      DWORD      dwID ;
      DWORD      dwThread ;
      TERMINFO   *termInfo ;

      termInfo = (TERMINFO *)lParam ;

      dwThread = GetWindowThreadProcessId(hwnd, &dwID) ;

      if(dwID == termInfo->dwID && termInfo->dwThread == dwThread )
      {
         PostMessage(hwnd, WM_CLOSE, 0, 0) ;
      }

      return TRUE ;
   }


////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// int DWORD GetProcessIdByName(char* lpProcessName)                                 // 
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
int GetProcessIdByName(char* lpProcessName,DWORD *dwPID)
{
   PROCESSENTRY32 pe;
   HANDLE thSnapshot;
   BOOL retval, ProcFound = false;
  
   if (dwPID==NULL) return(-1);
   thSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

   if(thSnapshot == INVALID_HANDLE_VALUE)
   {
      return(-1);
   }

   pe.dwSize = sizeof(PROCESSENTRY32);

    retval = Process32First(thSnapshot, &pe);

   while(retval)
   {
      if(_stricmp(pe.szExeFile, lpProcessName)==0)
      {
         ProcFound = true;
         break;	 
      }
      pe.dwSize = sizeof(PROCESSENTRY32);
      retval    = Process32Next(thSnapshot,&pe);
      
   }

   CloseHandle( thSnapshot );
   
   if (ProcFound) 
   {
	  *dwPID=pe.th32ProcessID;
	  return(0);
   }
   else return(-1);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// int DWORD KillAllProcessesByName(char* lpProcessName)                             // 
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
int KillAllProcessesByName(char* lpProcessName) 
{
   PROCESSENTRY32 pe;
   HANDLE thSnapshot;
   BOOL retval;
  
   thSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

   if(thSnapshot == INVALID_HANDLE_VALUE)
   {
      return(-1);
   }

   pe.dwSize = sizeof(PROCESSENTRY32);

    retval = Process32First(thSnapshot, &pe);

   while(retval)
   {
      if(_stricmp(pe.szExeFile, lpProcessName)==0 )
      {
		  TerminateApp(pe.th32ProcessID, 2000);
      }
      pe.dwSize = sizeof(PROCESSENTRY32);
      retval    = Process32Next(thSnapshot,&pe);
      
   }
 CloseHandle( thSnapshot );
  return(0);
}

////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// TProcessSnapShot * GetSnapShotOfProcesses(void)                                   // 
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
TProcessSnapShot * GetSnapShotOfProcesses(void)
{
   PROCESSENTRY32 pe;
   HANDLE thSnapshot;
   BOOL retval;
   TProcessSnapShot *SnapShot=NULL;
   DWORD NumProcesses=0;

   thSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

   if(thSnapshot == INVALID_HANDLE_VALUE)
   {
      return(NULL);
   }
   pe.dwSize = sizeof(PROCESSENTRY32);

    retval = Process32First(thSnapshot, &pe);

   while(retval)
   {
	  NumProcesses++;
      pe.dwSize = sizeof(PROCESSENTRY32);
      retval    = Process32Next(thSnapshot,&pe);      
   }

   if (NumProcesses==0)
   {
      CloseHandle( thSnapshot );
	  return(NULL);
   }

   if ((SnapShot=(TProcessSnapShot *)malloc(sizeof(TProcessSnapShot)))==NULL) 
   {
     CloseHandle( thSnapshot );
	 return(NULL);
   }
   SnapShot->NumProcesses=NumProcesses;

   if ((SnapShot->ProcessInfo=(TProcessInfo *)malloc(sizeof(TProcessInfo)*NumProcesses))==NULL)
   {
     free(SnapShot);
     CloseHandle( thSnapshot );
	 return(NULL);
   }
   NumProcesses=0;
   pe.dwSize = sizeof(PROCESSENTRY32);
   retval = Process32First(thSnapshot, &pe);

   while(retval)
   {
      if (NumProcesses>=SnapShot->NumProcesses)
	  {
	   printf("Process Snap Shot Collection Error 1\n");
       free(SnapShot->ProcessInfo);
       free(SnapShot);
       CloseHandle( thSnapshot );
	   return(NULL);
	  }
	  else
	  {
		  SnapShot->ProcessInfo[NumProcesses].PID=pe.th32ProcessID;
		  strcpy(SnapShot->ProcessInfo[NumProcesses].Name,pe.szExeFile);
	  }
	  NumProcesses++;
      pe.dwSize = sizeof(PROCESSENTRY32);
      retval    = Process32Next(thSnapshot,&pe); 
	  
   }
  CloseHandle( thSnapshot );
  if (NumProcesses!=SnapShot->NumProcesses)
	  {
	   printf("Process Snap Shot Collection Error 2\n");
       free(SnapShot->ProcessInfo);
       free(SnapShot);
	   return(NULL);
	  }
    return(SnapShot);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// int FreeSnapShot(TProcessSnapShot *SnapShot)                                      // 
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
int FreeSnapShot(TProcessSnapShot *SnapShot)
{
 if (SnapShot==NULL) return(-1);
 if (SnapShot->NumProcesses==0) return(-1);
 if (SnapShot->ProcessInfo==NULL) return(-1);
 free(SnapShot->ProcessInfo);
 free(SnapShot);
 return(0);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// int KillProcessesNotInSnapShot(TProcessSnapShot *SnapShot)                        // 
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
int KillProcessesNotInSnapShot(TProcessSnapShot *SnapShot)
{
  PROCESSENTRY32 pe;
  HANDLE thSnapshot;
  BOOL retval, ProcFound = false;;

 if (SnapShot==NULL) return(-1);
 if (SnapShot->NumProcesses==0) return(-1);
 if (SnapShot->ProcessInfo==NULL) return(-1);

   thSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

   if(thSnapshot == INVALID_HANDLE_VALUE)
   {
      return(NULL);
   }
   pe.dwSize = sizeof(PROCESSENTRY32);

    retval = Process32First(thSnapshot, &pe);

   while(retval)
   {
	   ProcFound=false;
	   for (DWORD i=0;i< SnapShot->NumProcesses;i++)
	   {
		   if ((pe.th32ProcessID==SnapShot->ProcessInfo[i].PID) &&
			   (_stricmp(pe.szExeFile, SnapShot->ProcessInfo[i].Name)==0))
		   {
		    ProcFound=true;
			break;
		   }
	   }
      if (!ProcFound)
	  {
       TerminateApp(pe.th32ProcessID, 2000);
	  }
      pe.dwSize = sizeof(PROCESSENTRY32);
      retval    = Process32Next(thSnapshot,&pe);      
   }
 CloseHandle( thSnapshot );
 return(0);

}