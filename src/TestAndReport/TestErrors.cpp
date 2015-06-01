#include "stdafx.h"
#include "TestErrors.h"
#include <windows.h>
/*
============================================================
|                     GLOBAL VARIABLES                     |
|----------------------------------------------------------|
*/
char *ErrorList[]=
{
  "Buffer Overrun Fault",                            /* -13  */
  "Failed to create IE Test File",                   /* -12  */
  "Failed to attach debugger",                       /* -11  */
  "Failed to get Process ID",                        /* -10  */
  "Internet Explorer Crashed",                       /*  -9  */
  "Failed to Start Browser",                         /*  -8  */
  "COM Object Exception Occurred",                   /*  -7  */
  "Can't Create Test Results File",                  /*  -6  */  
  "User Abort",                                      /*  -5  */  
  "CreateProcess Failed",                            /*  -4  */     
  "COM IObjectSafety Set Interface Opt Fault",       /*  -3  */ 
  "COM CoCreateInstance using IObjectSafety Fault",  /*  -2  */
  "COM Object Operation Hung",                       /*  -1  */
  "SUCCESS",                                         /*   0  */
  "Bag Argument Count",                              /*   1  */
  "Null Pointer Error",                              /*   2  */
  "Buffer Overrun Fault crt Generated",              /*   3  */
  "MultibyteToWideChar Op Failed",                   /*   4  */
  "CLSID From String Failed",                        /*   5  */
  "RegOpenKey - CLSID String Failed",                /*   6  */
  "StringFromCLSID Failed"                           /*   7  */
  "COM Object Not Script Safe",                      /*   8  */
  "COM Object Not Safe for Untrusted Data",          /*   9  */
  "QueryInterface for IDispatchEx Failed",           /*  10  */
  "QueryInterface for IDispatch Failed",             /*  11  */
  "QueryInterface for IDispatch IPersistPropertyBag Failed", /*   12  */
  "GetTypeInfo Failed",                              /*  13  */
  "Max Recursive Level Reached",                     /*  14  */
  "GetDocumentation Failed",                         /*  15  */
  "Append File Failed"                               /*  16  */
};

static int NumErrors=sizeof(ErrorList)/
                    sizeof(ErrorList[0]);

/*                    
============================================================
|                     Error_String()                       |
|----------------------------------------------------------|
| Params : Error Number                                    |
| Desc   : Returns a pointer to a character string         |
|          describing the the error input.                 |
|                                                          |
| Returns:  NULL on failure.                               |
|==========================================================|
*/
char * ErrorString(int Error)
{
 int TmpError=Error-(ERROR_MIN);
 if ((TmpError<0) || (TmpError>=NumErrors)) 
 {                 
  if ((DWORD)Error==0xc0000005) return("Access violation");
  else return("[Unknown Error]");
 }
 else return(ErrorList[TmpError]);    
}
/*
============================================================
|                      END ERROR LIST                      |
|----------------------------------------------------------|
*/