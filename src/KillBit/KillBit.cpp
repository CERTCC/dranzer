#include "stdafx.h"
#include <stdio.h>
#include "KillBit.h"

#define COM_KILL_BIT TEXT("SOFTWARE\\Microsoft\\Internet Explorer\\ActiveX Compatibility")
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// int GetKillBit(WCHAR *CLSID_Str_Wide)                                             //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
int GetKillBit(WCHAR *CLSID_Str_Wide)
{
 TCHAR KeyString[1024];
 HKEY  hKey;
 DWORD CompatibilityFlags,Type,CompatibilityFlagsSize=sizeof(CompatibilityFlags);
 int  RetVal=-1;
 LONG RegRtnVal;

 sprintf(KeyString,"%s\\%ws",COM_KILL_BIT,CLSID_Str_Wide);
 if ((RegRtnVal=RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
	          KeyString,
			  0, KEY_QUERY_VALUE, &hKey))!=ERROR_SUCCESS)
 {
  if (RegRtnVal!=ERROR_FILE_NOT_FOUND)
  printf("RegOpenKeyEx %d failed\n",RegRtnVal);
  return(RetVal);
 }
 if ((RegRtnVal=RegQueryValueEx(hKey,TEXT("Compatibility Flags"), NULL, &Type, (BYTE *)&CompatibilityFlags, 
	                &CompatibilityFlagsSize))==ERROR_SUCCESS) 
 {
  if ((CompatibilityFlags & 0x00000400)==0x00000400) RetVal=1;
  else RetVal=0;
 }
 else 
 {
  if (RegRtnVal!=ERROR_FILE_NOT_FOUND)
  printf("RegQueryValueEx %d failed\n",RegRtnVal);
 }

RegCloseKey(hKey);

return(RetVal);
}
