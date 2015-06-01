#include "stdafx.h"
#include "GetCOM_Objects.h" 

#include <Shlwapi.h>



#define COM_CLSID_KEY TEXT("Software\\Classes\\CLSID")

static LONG  GetNumberOfSubKeys(HKEY hKey,LPDWORD lpcMaxSubKeyLen);
static int GetCLSID(TCHAR *SubKeyName,CLSID *clsid,WCHAR *CLSID_Str_Wide,DWORD CLSID_Str_Wide_Length);
static int GetCLSIDInfo(WCHAR *CLSID_Str_Wide,TCHAR *CLSID_Description,DWORD CLSID_DescriptionLength);
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// HKEY OpenCOM_ObjectList(DWORD *NumObjects)                                        //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
HKEY OpenCOM_ObjectList(DWORD *NumObjects)
{
 LONG  RetVal;
 HKEY  hKey;
 DWORD NumSubKeys;
 
 RetVal=RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
	          COM_CLSID_KEY,
			  0, KEY_READ, &hKey );
 if (RetVal!=ERROR_SUCCESS)
 {
  if (NumObjects) *NumObjects=0;
  return(NULL);
 }

 RetVal=GetNumberOfSubKeys(hKey,&NumSubKeys);
 if (RetVal!=ERROR_SUCCESS)
 {
  if (NumObjects) *NumObjects=0;
  RegCloseKey(hKey);
  return(NULL);
 }
if (NumObjects) *NumObjects=NumSubKeys;
return(hKey);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// BOOL CloseCOM_ObjectList(HKEY  hKey)                                              //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////

BOOL CloseCOM_ObjectList(HKEY  hKey)
{
  if (RegCloseKey(hKey)==ERROR_SUCCESS) return(true);
  else return(false);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// BOOL GetCOM_ObjectInfo(HKEY  hKey,DWORD Index,COM_ObjectInfoType *COM_Info)       //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
BOOL GetCOM_ObjectInfo(HKEY  hKey,DWORD Index, COM_ObjectInfoType *COM_Info)
{
  DWORD NumSubKeys;
  LONG  RetVal;
  TCHAR SubKeyName[MAX_PATH];
  DWORD SubKeyName_Length;
  CLSID clsid;

  RetVal=GetNumberOfSubKeys(hKey,&NumSubKeys);
  if (RetVal!=ERROR_SUCCESS)
  {
   return(false);
  }
  if (COM_Info==NULL) return(false);
  if (Index>=NumSubKeys) return(false);

  SubKeyName_Length=MAX_PATH;
  RetVal=RegEnumKeyEx(hKey, Index,SubKeyName, 
			          &SubKeyName_Length,NULL, 
					  NULL, NULL, NULL); 

  if (RetVal!=ERROR_SUCCESS)
   {
    return(false);
   }
   if (!GetCLSID(SubKeyName,&clsid,COM_Info->CLSID_Str_Wide,MAX_PATH)) 
	   return(false);
   if (!GetCLSIDInfo(COM_Info->CLSID_Str_Wide,COM_Info->CLSID_Description,1024))
		    strcpy(COM_Info->CLSID_Description,"[NOT AVAILABLE]");
   return(true);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// BOOL GetCOM_ObjectInfo(wchar_t *CLSID_String,COM_ObjectInfoType *COM_Info)               //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
BOOL GetCOM_ObjectInfo(wchar_t  *CLSID_String,COM_ObjectInfoType *COM_Info)
{
  CLSID clsid;

  if (CLSIDFromString(CLSID_String, &clsid)!=NOERROR) 
  {
   return(false);
  }
  
  if (StringFromGUID2(clsid,COM_Info->CLSID_Str_Wide,MAX_PATH)==0)
  {
   return(false);
  }
  if (!GetCLSIDInfo(COM_Info->CLSID_Str_Wide,COM_Info->CLSID_Description,1024))
		    strcpy(COM_Info->CLSID_Description,"[NOT AVAILABLE]");
  return(true);
}

////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// GetNumberOfSubKeys(HKEY hKey,LPDWORD lpcMaxSubKeyLen)                             //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////

static LONG  GetNumberOfSubKeys(HKEY hKey,LPDWORD lpcMaxSubKeyLen)
{
 LONG     RetVal;

 if (lpcMaxSubKeyLen==NULL) return(ERROR_INVALID_PARAMETER);

 RetVal=RegQueryInfoKey(hKey,NULL,NULL,NULL,lpcMaxSubKeyLen,
		                NULL,NULL,NULL,NULL,NULL,NULL,NULL);
 return(RetVal);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                //
/// int GetCLSID(TCHAR *SubKeyName,CLSID *clsid,WCHAR *CLSID_Str_Wide,DWORD CLSID_Str_Wide_Length) //            
///                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static int GetCLSID(TCHAR *SubKeyName,CLSID *clsid,WCHAR *CLSID_Str_Wide,DWORD CLSID_Str_Wide_Length)
{

 
 if (lstrcmp(SubKeyName,"CLSID")==0) 
	 
 {
	 return(0);
 }

 if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
	                     SubKeyName, -1, 
						 CLSID_Str_Wide, CLSID_Str_Wide_Length)==0) 
 {
  return(0);
 }

 if (CLSIDFromString(CLSID_Str_Wide, clsid)!=NOERROR) 
 {
  return(0);
 }
 else return(1);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                //
/// int GetCLSIDInfo(WCHAR *CLSID_Str_Wide,TCHAR *CLSID_Description,DWORD CLSID_DescriptionLength) //            
///                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static int GetCLSIDInfo(WCHAR *CLSID_Str_Wide,TCHAR *CLSID_Description,DWORD CLSID_DescriptionLength)
{
 TCHAR KeyString[1024];
 HKEY  hKey;
 int  RetVal=1;
 DWORD Type;
 TCHAR trim[ ] = TEXT(" \0");

 sprintf(KeyString,"%s\\%ws",COM_CLSID_KEY,CLSID_Str_Wide);
 if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, KeyString, 0, KEY_QUERY_VALUE, &hKey )!=
	 ERROR_SUCCESS) return(0);
 if (RegQueryValueEx(hKey, NULL, NULL, &Type, (BYTE *)CLSID_Description, 
	                &CLSID_DescriptionLength )!=ERROR_SUCCESS) 
 {
  strcpy(CLSID_Description,"[DESCRIPTION NOT AVAILABLE]");
 }
 else
 {
  StrTrim(CLSID_Description, trim);
  if (_tcsclen(CLSID_Description)==0)
   strcpy(CLSID_Description,"[DESCRIPTION NOT AVAILABLE]");
 }

 RegCloseKey(hKey);
 return(RetVal);
}

