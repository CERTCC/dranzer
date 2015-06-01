#ifndef __GetCOM_Objects_H
#define __GetCOM_Objects_H
#include <wchar.h>
typedef struct
{
 WCHAR CLSID_Str_Wide[MAX_PATH];
 TCHAR CLSID_Description[1024];
} COM_ObjectInfoType;

HKEY OpenCOM_ObjectList(DWORD *NumObjects);
BOOL GetCOM_ObjectInfo(HKEY  hKey,DWORD Index,COM_ObjectInfoType *COM_Info);
BOOL GetCOM_ObjectInfo(wchar_t  *CLSID_String,COM_ObjectInfoType *COM_Info);
BOOL CloseCOM_ObjectList(HKEY  hKey);
#endif