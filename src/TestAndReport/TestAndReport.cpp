// TestAndReport.cpp : Defines the entry point for the console application.
//
#define _WIN32_WINNT 0x0400
#include "stdafx.h"
#include <windows.h>
#include <Winuser.h>
#include <comdef.h>
#include <objbase.h>
#include <mshtmhst.h>
#include <stdio.h>
#include <tchar.h>
#include <objsafe.h>
#include <io.h>    // For _access and _waccess
#include "win32_exception.h"
#include "TestErrors.h"
#include <Shlwapi.h>
#include <DispEx.h>
#include <exdisp.h>

#include "string_m.h"
#include "KillApplication.h"

#define COM_KILL_BIT TEXT("SOFTWARE\\Microsoft\\Internet Explorer\\ActiveX Compatibility")

#define PARAMS_LOAD_TIMEOUT_MIN              6000
#define PARAMS_LOAD_MBYTES_PER_SEC          20000

#define METHOD_AND_PROP_STRING_CHARACTER       'x'
#define METHOD_AND_PROP_STRING_LENGTH    (1024*10)

#define PARAM_STRING_CHARACTER                 'x'
#define PARAM_STRING_LENGTH              (1024*10)

#define PROPERTY_BAG_STRING_CHARACTER          'x'
#define PROPERTY_BAG_STRING_LENGTH       (1024*10)

#define MAXRECURSE 2
#define LOGINFO 1
#define MIN_STRING_LENGTH 4
#define LOG_CRASH_ON_FREE 1
#define TEST_ENGINE_VERSION	"$Rev: 101 $"


#define MAX_NUMBER_OF_SECTIONS         20

typedef struct _StringList 
 {
  char               * String;
  struct _StringList * pNext;
  struct _StringList * pPrev;
 } TStringList;

typedef struct
{
 DWORD Offset;
 DWORD Length;
} TSectionsToCheck;

typedef enum 
{
 NONE,
 TEST_CONTROL,
 GEN_INTERFACE,
 IE_LOAD,
 PARAMS_IN_IE_PROPBAG,
 PARAMS_IN_IE_BINARY_SCAN,
 PRINT_COM_OBJECT_INFORMATION
} TExecutionMode;

class MyIPropertyBag: public IPropertyBag
{
 public:
	// IPropertyBag interface
	STDMETHODIMP Read(LPCOLESTR pwszPropName, VARIANT *pVar, IErrorLog *pErrorLog);
	STDMETHODIMP Write(LPCOLESTR pwszPropName, VARIANT *pVar);
	STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }
	STDMETHODIMP QueryInterface(REFIID riid, void ** ppv){ return NOERROR;}
};
static int      GetKillBit(WCHAR *CLSID_Str_Wide);
static inline   BOOL FileExists(LPCTSTR lpszFile)  { return _taccess(lpszFile, 0) == 0; } 
static char   * FindFile(char *FileName);
static string_m CustomTypeToString(HREFTYPE refType, ITypeInfo* pTypeInfo);
static string_m TypeDescriptionToString(TYPEDESC* typeDesc, ITypeInfo* pTypeInfo);
static int      PrintComObjectInformation(CLSID *clsid);
static int      TestCOM_Object(CLSID *clsid);
static int      Test_Dispatch_Interface(IDispatch *pIDispatch,DWORD Level);
static int      Test_DispatchEx_Interface(IDispatchEx *pIDispatchEx,DWORD Level);
static int      TestMethods(IDispatch *pIDispatch,DWORD Level);
static int      TestMethod(IDispatch *pIDispatch,ITypeInfo *pTypeInfo, FUNCDESC* FunctionDescription,BSTR InterfaceName,DWORD Level);
static int      TestProperites(IDispatch *pIDispatch, DWORD Level);
static int      Get_CLSID_From_String(TCHAR *CLSID_String,CLSID *clsid);
static int      Get_CLSID_Description(TCHAR *CLSID_String,TCHAR *Description,DWORD DescriptionLength);
static int      SetArgument(VARIANTARG *arg,BSTR BigString);
static int      WriteText(HANDLE hFile,char *Text);
static void     ParseArguments(int argc, _TCHAR* argv[]);
static void     PrintUsage(_TCHAR* argv[]);
static int      PrintInterfaceInfo(IDispatch *pIDispatch);
static int      IELoadUrl(char * Url);
static int      Get_COM_FileName(TCHAR *CLSID_String,TCHAR *FileName,DWORD FileNameLength);
static int      PrintFileInfo(HANDLE handle,TCHAR *FileName);
static char *   GetArgumentStringEquivalent(USHORT *arg);
static int      Generate_HTML_PARAMS_Test_File(char *FileName,char *CLSID);
static int      IELoadUrlPARAMS(char * Url,DWORD Timeout);
static void     PrintVariant(VARIANT &var);
static HANDLE   Create_HTML_PARAMS_Test_File(char *FileName,char *CLSID);
static int      TestParamsViaBinaryScan(CLSID *clsid);
static int      GetIeVersion(DWORD * Major, DWORD * Minor,DWORD *BuildNo, DWORD *SubBuildNo);
static int      DeQuoteString(char * String);

static DWORD WINAPI    DebugProcessThread(LPVOID arg);
static int             Create_HTML_Test_File(char *FileName,char *CLSID);
static void            DeleteTempFile(char *FileName);  
static void            StringsASCII(BYTE	 *Memory,DWORD Size) ;
static void            StringsUnicode(BYTE	 *Memory,DWORD Size); 
static int             OpenFileName(char* FileName);
static TStringList *   InsertString(char *String);
static TStringList *   FindString(char *String);
static void            RemoveString(TStringList *pNode);
static void            DeleteStringList(void);
static int             TestParamsPropertyBag(CLSID *clsid);
static void            PrintStringList(void);
static BOOL            GenerateVariantText(VARIANT &var,LPCOLESTR pwszPropName,HANDLE hfile);
static BOOL            CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam);
static void            ActivateControlInIE(void);
static BOOL CALLBACK   EnumChildWindowCallback(HWND hwnd, LPARAM lParam);

static DWORD           ComputeInitialLoadTime(char *FileName);
static void            dumphex(unsigned char *dptr,int size);

static BYTE	             *ControlLoadedInMemory=NULL;
static DWORD              MemoryNumBytes=0;
static TSectionsToCheck   SectionsToCheck[MAX_NUMBER_OF_SECTIONS];
static DWORD              NumberOfSectionsToCheck=0;
static TStringList       *StringListHead=NULL;
static TStringList       *StringListTail=NULL;

static char          * GetExceptionName (DWORD ExceptionCode );
static TCHAR           COM_FileName[MAX_PATH]={0};
static TExecutionMode  ExecutionMode=NONE;
static char            TempTextBuffer[2048];
static HANDLE          hLogFile=NULL;
static bool            COM_Object_Exeception_Occurred=false;
static char          * CLSID_String=NULL;
static char          * OutputFileName=NULL;
static HANDLE          DebugProcessThreadHandle;
static volatile BOOL   KillDebugMonitor=false;
static volatile BOOL   MonitorDetectedIECrash=false;
static BOOL            HaveCOM_Filename=false;
static char            lpPathBuffer[MAX_PATH-14+1];
static char *          LargePropertyBagTestString=NULL;
static HANDLE          TestFileHandle=NULL;
static BOOL            HaveParamsToTest=false;
static TCHAR           CLSID_Description[1024]={0};
static DWORD           ZoneID;
static char            PropAndMethodDispString[256];
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// int _tmain()                                                                      //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
int _tmain(int argc, _TCHAR* argv[])
{
 CLSID clsid;
 int   RetVal=0;
 DWORD RetryCount=0;
 DWORD dwRetVal;
 win32_exception::install_handler();
 sprintf(PropAndMethodDispString,"<%c%c%c%c.....{%d}>",METHOD_AND_PROP_STRING_CHARACTER,
	                              METHOD_AND_PROP_STRING_CHARACTER,
							      METHOD_AND_PROP_STRING_CHARACTER,
							      METHOD_AND_PROP_STRING_CHARACTER,
							      METHOD_AND_PROP_STRING_LENGTH);
 // Get the temp path.
  dwRetVal = GetTempPath(sizeof(lpPathBuffer),     // length of the buffer
                                lpPathBuffer);      // buffer for path 
    if (dwRetVal==0)
    {
     printf ("GetTempPath failed with error %d.\n", GetLastError());
     return (-1);
    }
	else if (dwRetVal>sizeof(lpPathBuffer))
	{
     printf ("GetTempPath buffer too small\n");
	 return (-1);
	}
 ParseArguments(argc,argv);
 
 RetVal=Get_CLSID_From_String(CLSID_String,&clsid);
 if (RetVal!=SUCCESS) 
 {
  printf("Get_CLSID_From_String Failed\n");
  return(RetVal);
 }

 RetVal=Get_CLSID_Description(CLSID_String,CLSID_Description,sizeof(CLSID_Description));
 if (RetVal!=SUCCESS) 
 {
   printf("Get_CLSID_Description Failed\n");
   return(RetVal);
 }

 if (Get_COM_FileName(CLSID_String, COM_FileName,sizeof(COM_FileName))==SUCCESS) HaveCOM_Filename=true;
 if (OutputFileName==NULL) hLogFile=GetStdHandle(STD_OUTPUT_HANDLE);
 else
 {
  while(1)
  {
  hLogFile = CreateFile(OutputFileName,       // file to create
                    GENERIC_WRITE,          // open for writing
                    0,                      // do not share
                    NULL,                   // default security
                    CREATE_ALWAYS,          // 
                    FILE_ATTRIBUTE_NORMAL,  // normal file
                    NULL);                  // no attr. template

   if (hLogFile == INVALID_HANDLE_VALUE) 
    {  
     if (RetryCount<5)
 	  {
       RetryCount++;
       Sleep(2000);
  	   continue;
	  }
	 else
	 {
      printf("Could not open output file\n");
      return (CANT_CREATE_TEST_RESULTS_FILE);
	 }
    }
   else break;
  }
 }
  CoInitialize(NULL);
  sprintf(TempTextBuffer,"*******************************************************************************\r\n");
  WriteText(hLogFile,TempTextBuffer);

  if (ExecutionMode==TEST_CONTROL)
  {
   sprintf(TempTextBuffer,"Testing COM Object - %s %s\r\n",CLSID_String,CLSID_Description);
  }
  else if (ExecutionMode==IE_LOAD)
  {
   sprintf(TempTextBuffer,"Loading COM Object in to IE - %s %s\r\n",CLSID_String,CLSID_Description);
  }
  else if (ExecutionMode==PARAMS_IN_IE_PROPBAG) 
  {
   sprintf(TempTextBuffer,"Testing COM Object PARAMS (Property Bag) in IE - %s %s\r\n",CLSID_String,CLSID_Description);
  }
  else if (ExecutionMode==PARAMS_IN_IE_BINARY_SCAN) 
  {
   sprintf(TempTextBuffer,"Testing COM Object PARAMS (Binary Scan) in IE - %s %s\r\n",CLSID_String,CLSID_Description);
  }
  else if (ExecutionMode==GEN_INTERFACE)
   {
    sprintf(TempTextBuffer,"Interface for COM Object - %s %s\r\n",CLSID_String,CLSID_Description);
   }
  else if (ExecutionMode==PRINT_COM_OBJECT_INFORMATION)
   {
    sprintf(TempTextBuffer,"Details for COM Object - %s %s\r\n",CLSID_String,CLSID_Description);
   }
  WriteText(hLogFile,TempTextBuffer); 
  if (HaveCOM_Filename) PrintFileInfo(hLogFile,COM_FileName);
  else
  {
    sprintf(TempTextBuffer,"*******************************************************************************\r\n");
   WriteText(hLogFile,TempTextBuffer);

	sprintf(TempTextBuffer,"%-20s: %s\r\n","COM Object Filename","[UNKNOWN]");
	WriteText(hLogFile,TempTextBuffer);
    sprintf(TempTextBuffer,"*******************************************************************************\r\n");
    WriteText(hLogFile,TempTextBuffer);

  }
  sprintf(TempTextBuffer,"*******************************************************************************\r\n"); 
 WriteText(hLogFile,TempTextBuffer); 
 
  if (ExecutionMode==TEST_CONTROL)
  {
   printf("Testing COM Object - %s %s\n",CLSID_String,CLSID_Description);
  }
  else if (ExecutionMode==IE_LOAD) 
  {
   printf("Loading COM Object in to IE - %s %s\n",CLSID_String,CLSID_Description);
  }
  else if (ExecutionMode==PARAMS_IN_IE_PROPBAG) 
  {
   printf("Testing COM Object PARAMS in IE (Property Bag) - %s %s\n",CLSID_String,CLSID_Description);
  }
  else if (ExecutionMode==PARAMS_IN_IE_BINARY_SCAN) 
  {
   printf("Testing COM Object PARAMS in IE (Binary Scan) - %s %s\n",CLSID_String,CLSID_Description);
  }
  else if (ExecutionMode==GEN_INTERFACE)
  {
   printf("Generating Interface for COM Object - %s %s\n",CLSID_String,CLSID_Description);
  }
  else if (ExecutionMode==PRINT_COM_OBJECT_INFORMATION)
  {
   printf("Obtaining details for COM Object - %s %s\n",CLSID_String,CLSID_Description);
  }
  try {
	   if (ExecutionMode==IE_LOAD) 
	   {
	    strcat(lpPathBuffer,IELOAD_HTML);
	    DeleteTempFile(lpPathBuffer);
        if (Create_HTML_Test_File(lpPathBuffer,CLSID_String)!=0) 
		{
		 RetVal=FAILED_TO_CREATE_IE_TEST_FILE;
		}
		else
		{
		 RetVal=IELoadUrl(lpPathBuffer); 
		}
	   }
	   else if (ExecutionMode==PARAMS_IN_IE_PROPBAG) 
	   {
        RetVal= TestParamsPropertyBag(&clsid);
	   }
       else if (ExecutionMode==PARAMS_IN_IE_BINARY_SCAN)
	   {
	    RetVal= TestParamsViaBinaryScan(&clsid);       
 	   }
	   else if (ExecutionMode==PRINT_COM_OBJECT_INFORMATION)
	   {
	    RetVal= PrintComObjectInformation(&clsid);       
 	   }

       else RetVal=TestCOM_Object(&clsid);
 	  } 
  catch (const access_violation& e) 
       { 
		try
		{
		sprintf(TempTextBuffer,"*****************************\r\n");
		WriteText(hLogFile,TempTextBuffer);
		sprintf(TempTextBuffer,"***   Access Violation    ***\r\n");
		WriteText(hLogFile,TempTextBuffer);
	    sprintf(TempTextBuffer,"*****************************\r\n");
		WriteText(hLogFile,TempTextBuffer);
	    sprintf(TempTextBuffer,"%s at 0x%p :Bad %s on 0x%p\r\n",
			  	e.what(), e.where(),e.isWrite()?"write":"read",e.badAddress());
		WriteText(hLogFile,TempTextBuffer);
		sprintf(TempTextBuffer,"*****************************\r\n");
		WriteText(hLogFile,TempTextBuffer);
         CoUninitialize();
         CloseHandle(hLogFile);
         return(COM_OBJECT_EXECEPTION_OCCURRED);
		}
		catch(...)
		{
		}

       }
  catch (const win32_exception& e) 
	   {    
		try
		{
		 sprintf(TempTextBuffer,"*****************************\r\n");
		 WriteText(hLogFile,TempTextBuffer);
		 sprintf(TempTextBuffer,"***   Win32 Exception     ***\r\n");
		 WriteText(hLogFile,TempTextBuffer);
		 sprintf(TempTextBuffer,"*****************************\r\n");
		 WriteText(hLogFile,TempTextBuffer);
		 sprintf(TempTextBuffer,"%s (code %x) at 0x%p\r\n",
			  	         e.what(), e.code(),e.where());
	 	 WriteText(hLogFile,TempTextBuffer);
		 sprintf(TempTextBuffer,"*****************************\r\n");
		 WriteText(hLogFile,TempTextBuffer);
         CoUninitialize();
         CloseHandle(hLogFile);
         return(COM_OBJECT_EXECEPTION_OCCURRED);
		}
		catch(...)
		{
		}
       }

 try 
  {
   sprintf(TempTextBuffer,"\r\n\r\n");
   WriteText(hLogFile,TempTextBuffer);
  CoUninitialize();
  CloseHandle(hLogFile);
  }
 catch(...)
  {
  }

  //printf("Return val %d\n",RetVal);
  return(RetVal);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static void PrintUsage(_TCHAR* argv[])                                            //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static void PrintUsage(_TCHAR* argv[])
{
	fprintf(stderr, "Usage: %s <options> \n",argv[0]);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "        -c <CLSID>        - COM CLSID\n");
	fprintf(stderr, "        -o <outputfile>   - Output Filename\n");
	fprintf(stderr, "        -t                - Test COM Object\n");
    fprintf(stderr, "        -g                - Generate Interface Listing\n");
    fprintf(stderr, "        -l                - Load in Internet Explorer\n");
	fprintf(stderr, "        -p                - Test PARAMS (PropertyBag) in Internet Explorer\n");
    fprintf(stderr, "        -s                - Test PARAMS (Binary Scan) in Internet Explorer\n");
	fprintf(stderr, "        -i                - Print COM object information\n");

}

////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static void ParseArguments(int argc, _TCHAR* argv[])                              //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static void ParseArguments(int argc, _TCHAR* argv[])
{
	int i;
	char *c;
	if (argc < 2)
	{
		PrintUsage(argv);
		exit(2);
	}
   
	//get arguments
	for(i = 1; i < argc; i++)
	{
		if (argv[i][0] != '-')
		{
			fprintf(stderr, "Error in command line\n");
			PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (CLSID_String) free(CLSID_String);
			exit(2);
		}
		c = strchr((char *)"co", argv[i][1]);
		if (c != NULL)
			if ((i == (argc-1)) || (argv[i+1][0] == '-'))
			{
				fprintf(stderr, "option %s requires an argument\n", argv[i]);
			     if (OutputFileName) free(OutputFileName);
			     if (CLSID_String) free(CLSID_String);
				exit(1);
			}
			switch (argv[i][1])
			{
			case 'c':
				if (CLSID_String==NULL)
				{
				 i++;
				 CLSID_String=_strdup(argv[i]);
				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (CLSID_String) free(CLSID_String);
			     exit(2);
				}
				break;
			case 'o':
				if (OutputFileName==NULL)
				{
				 i++;
				 OutputFileName=_strdup(argv[i]);	
				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (CLSID_String) free(CLSID_String);
			     exit(2);
				}
				break;

			case 't':
				if (ExecutionMode==NONE)
				{
                 ExecutionMode=TEST_CONTROL;
				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (CLSID_String) free(CLSID_String);
			     exit(2);
				}
				break;
			case 'g':
				if (ExecutionMode==NONE)
				{
                 ExecutionMode=GEN_INTERFACE;
				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (CLSID_String) free(CLSID_String);
			     exit(2);
				}
				break;
			case 'l':
				if (ExecutionMode==NONE)
				{
                 DWORD MajorVersion;
                 ExecutionMode=IE_LOAD;
				 if (!GetIeVersion(&MajorVersion, NULL,NULL, NULL))
				 {
				  if (MajorVersion>=7) ZoneID=1;
				  else ZoneID=3;
				 }
				 else ZoneID=3;

				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (CLSID_String) free(CLSID_String);
			     exit(2);
				}
				break;
			case 'p':
				if (ExecutionMode==NONE)
				{
                 DWORD MajorVersion;
                 ExecutionMode=PARAMS_IN_IE_PROPBAG;
				 if (!GetIeVersion(&MajorVersion, NULL,NULL, NULL))
				 {
				  if (MajorVersion>=7) ZoneID=1;
				  else ZoneID=3;
				 }
				 else ZoneID=3;
				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (CLSID_String) free(CLSID_String);
			     exit(2);
				}
				break;
			case 's':
				if (ExecutionMode==NONE)
				{
                 DWORD MajorVersion;
                 ExecutionMode=PARAMS_IN_IE_BINARY_SCAN;
				 if (!GetIeVersion(&MajorVersion, NULL,NULL, NULL))
				 {
				  if (MajorVersion>=7) ZoneID=1;
				  else ZoneID=3;
				 }
				 else ZoneID=3;

				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (CLSID_String) free(CLSID_String);
			     exit(2);
				}
				break;

			case 'i':
				if (ExecutionMode==NONE)
				{
                 ExecutionMode=PRINT_COM_OBJECT_INFORMATION;
				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (CLSID_String) free(CLSID_String);
			     exit(2);
				}
				break;


			case '?':
				PrintUsage(argv);
				exit(2);
				break;

			default:
			     fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (CLSID_String) free(CLSID_String);
			     exit(2);
				 break;

			}
	}

if (CLSID_String==NULL)
 {
	 printf("Error: CLSID not specified -c <CLSID>\n");
	 if (OutputFileName) free(OutputFileName);
	 exit(1);
 }
if (ExecutionMode==NONE)
{
	printf("Error: Execution mode not specified use -t or -g <CLSID>\n");
	if (OutputFileName) free(OutputFileName);
	if (CLSID_String) free(CLSID_String);
	exit(1);
}

}
///////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                          //
/// static int GetIeVersion(DWORD * Major, DWORD * Minor,DWORD *BuildNo, DWORD *SubBuildNo)  //
///                                                                                          //
///////////////////////////////////////////////////////////////////////////////////////////////

static int GetIeVersion(DWORD * Major, DWORD * Minor,DWORD *BuildNo, DWORD *SubBuildNo)
{
 HKEY hKey;

 if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Internet Explorer", 0,KEY_QUERY_VALUE,&hKey) == ERROR_SUCCESS)
 {
  char VersionString[256];
  DWORD VersionStringSize=sizeof(VersionString);

  if (RegQueryValueEx( hKey, "Version", NULL, NULL, (LPBYTE)VersionString, &VersionStringSize)==ERROR_SUCCESS)
  {
   char *SubStr;
   
   SubStr=strtok (VersionString,".");
   if ((SubStr) && (Major)) *Major=atoi(SubStr);
   else if (Major) *Major=0;

   SubStr=strtok (NULL,".");
   if ((SubStr) && (Minor)) *Minor=atoi(SubStr);
   else if (Minor) *Minor=0;

   SubStr=strtok (NULL,".");
   if ((SubStr) && (BuildNo)) *BuildNo=atoi(SubStr);
   else if (BuildNo) *BuildNo=0;

   SubStr=strtok (NULL,".");
   if ((SubStr) && (SubBuildNo)) *SubBuildNo=atoi(SubStr);
   else if (SubBuildNo) *SubBuildNo=0;

   RegCloseKey(hKey);
   return(0);
  }
  else RegCloseKey(hKey);
 }
 return(-1);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static WriteText(HANDLE hFile,char *Text)                                         //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////

static int WriteText(HANDLE hFile,char *Text)
{
   DWORD BytesToWrite=(DWORD)strlen(Text);
   DWORD BytesWritten=0;
   
   while (BytesToWrite)
   {
     if(WriteFile(hFile, Text, BytesToWrite, &BytesWritten, 0)==0)
	 {
	  return -1;
	 }
	Text+= BytesWritten;
    BytesToWrite-=BytesWritten;
   }
   return(0);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static int Get_CLSID_Description()                                                //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int Get_CLSID_Description(TCHAR *CLSID_String,TCHAR *Description,DWORD DescriptionLength)
{
 TCHAR KeyString[1024];
 HKEY  hKey;
 int  RetVal=SUCCESS;
 DWORD Type;
 TCHAR trim[ ] = TEXT(" \0");

 sprintf(KeyString,"Software\\Classes\\CLSID\\%s",CLSID_String);
 if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, KeyString, 0, KEY_QUERY_VALUE, &hKey )!=
	 ERROR_SUCCESS) 
     {
	   return(REG_OPEN_KEY_CLSID_CLSID_STRING_FAILED);
	 }
 if (RegQueryValueEx(hKey, NULL, NULL, &Type, (BYTE *)Description, 
	                &DescriptionLength )!=ERROR_SUCCESS) 
 {
   strcpy(Description,"[DESCRIPTION NOT AVAILABLE]");
 }
 else
 {
  StrTrim(Description, trim);
  if (_tcsclen(Description)==0)
   strcpy(Description,"[DESCRIPTION NOT AVAILABLE]");
 }

 
 RegCloseKey(hKey);
 return(RetVal);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static char *stristr(const char *String, const char *Pattern)                     //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////

char *stristr(const char *String, const char *Pattern)
{
  char *pptr, *sptr, *start;
  size_t  slen, plen;

  for (start = (char *)String,pptr  = (char *)Pattern,slen  = strlen(String),plen  = strlen(Pattern);
       slen >= plen;start++, slen--)
      {
            /* find start of pattern in string */
            while (toupper(*start) != toupper(*Pattern))
            {
                  start++;
                  slen--;

                  /* if pattern longer than string */

                  if (slen < plen)
                        return(NULL);
            }

            sptr = start;
            pptr = (char *)Pattern;

            while (toupper(*sptr) == toupper(*pptr))
            {
                  sptr++;
                  pptr++;

                  /* if end of pattern then pattern was found */

                  if ('\0' == *pptr)
                        return (start);
            }
      }
   return(NULL);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static void RemoveExtraneousArgs(char *Filename)                                  //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static void RemoveExtraneousArgs(char *Filename)
{
 char *substr;
 substr=stristr(Filename,".dll");
 if (substr) 
 {
  substr[4]=NULL;
  return;
 }
 substr=stristr(Filename,".exe");
 if (substr) 
 {
  substr[4]=NULL;
  return;
 }
 substr=stristr(Filename,".ocx");
 if (substr) 
 {
  substr[4]=NULL;
  return;
 }
 substr=stristr(Filename,".cpl");
 if (substr) 
 {
  substr[4]=NULL;
  return;
 }
 substr=stristr(Filename,".wsc");
 if (substr) 
 {
  substr[4]=NULL;
  return;
 }
 substr=stristr(Filename,".acm");
 if (substr) 
 {
  substr[4]=NULL;
  return;
 }
 substr=stristr(Filename,".flt");
 if (substr) 
 {
  substr[4]=NULL;
  return;
 }
 substr=stristr(Filename,".mdw");
 if (substr) 
 {
  substr[4]=NULL;
  return;
 }
 substr=stristr(Filename,".ax");
 if (substr) 
 {
  substr[3]=NULL;
  return;
 }
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static void ExpandEnvironmentVars(char *Filename,DWORD FileNameLength)            //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static void ExpandEnvironmentVars(char *Filename,DWORD FileNameLength) 
{
 char *tempstr;
 
 if (Filename==NULL) return;
 if ((tempstr=_strdup(Filename))==NULL) return;
 if (ExpandEnvironmentStrings(tempstr,Filename,FileNameLength)==0)
 {
  strcpy(Filename,tempstr);
 }
 free(tempstr);
 return;
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// int Get_COM_FileName(TCHAR *CLSID_String,TCHAR *FileName,DWORD FileNameLength)    //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int Get_COM_FileName(TCHAR *CLSID_String,TCHAR *FileName,DWORD FileNameLength)
{
 TCHAR KeyString[1024];
 HKEY  hKey;
 DWORD Count=0;
 DWORD Type;
 TCHAR trim[ ] = TEXT(" \0");
 TCHAR CLSID_Temp[512];
 DWORD CLSID_Temp_Length;
 DWORD Temp_Length;

 if (CLSID_String==NULL) return(-1);

 StrCpy(CLSID_Temp,CLSID_String);
 while(1)
 {
  sprintf(KeyString,"Software\\Classes\\CLSID\\%s\\InprocServer32",CLSID_Temp);
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, KeyString, 0, KEY_QUERY_VALUE, &hKey )==
	 ERROR_SUCCESS) 
     {
	   Temp_Length=FileNameLength;
       if (RegQueryValueEx(hKey, NULL, NULL, &Type, (BYTE *)FileName, 
	                &Temp_Length )==ERROR_SUCCESS) 
       {  
          StrTrim(FileName, trim);
          if (_tcsclen(FileName)!=0) 
		   {
            RegCloseKey(hKey);
			RemoveExtraneousArgs(FileName);
			ExpandEnvironmentVars(FileName,FileNameLength);
			DeQuoteString(FileName);
			return(SUCCESS);
		   }
	   }
	   RegCloseKey(hKey);
	 }

  sprintf(KeyString,"Software\\Classes\\CLSID\\%s\\InprocHandler32",CLSID_Temp);
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, KeyString, 0, KEY_QUERY_VALUE, &hKey )==
	 ERROR_SUCCESS) 
     {
	   Temp_Length=FileNameLength;
       if (RegQueryValueEx(hKey, NULL, NULL, &Type, (BYTE *)FileName, 
	                &Temp_Length )==ERROR_SUCCESS) 
       {  
          StrTrim(FileName, trim);
          if (_tcsclen(FileName)!=0) 
		   {
            RegCloseKey(hKey);
			RemoveExtraneousArgs(FileName);
			ExpandEnvironmentVars(FileName,FileNameLength);
			DeQuoteString(FileName);
			return(SUCCESS);
		   }
	   }
	   RegCloseKey(hKey);
	 }

  sprintf(KeyString,"Software\\Classes\\CLSID\\%s\\LocalServer32",CLSID_Temp);
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, KeyString, 0, KEY_QUERY_VALUE, &hKey )==
	 ERROR_SUCCESS) 
     {
       Temp_Length=FileNameLength;
       if (RegQueryValueEx(hKey, NULL, NULL, &Type, (BYTE *)FileName, 
	                &Temp_Length )==ERROR_SUCCESS) 
       {  
          StrTrim(FileName, trim);
          if (_tcsclen(FileName)!=0) 
		   {
            RegCloseKey(hKey);
			RemoveExtraneousArgs(FileName);
			ExpandEnvironmentVars(FileName,FileNameLength);
			DeQuoteString(FileName);
			return(SUCCESS);
		   }
	   }
	   RegCloseKey(hKey);
	 }
  sprintf(KeyString,"Software\\Classes\\CLSID\\%s\\InprocServer",CLSID_Temp);
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, KeyString, 0, KEY_QUERY_VALUE, &hKey )==
	 ERROR_SUCCESS) 
     {
       Temp_Length=FileNameLength;
       if (RegQueryValueEx(hKey, NULL, NULL, &Type, (BYTE *)FileName, 
	                &Temp_Length )==ERROR_SUCCESS) 
       {  
          StrTrim(FileName, trim);
          if (_tcsclen(FileName)!=0) 
		   {
            RegCloseKey(hKey);
			RemoveExtraneousArgs(FileName);
			ExpandEnvironmentVars(FileName,FileNameLength);
			DeQuoteString(FileName);
			return(SUCCESS);
		   }
	   }
	   RegCloseKey(hKey);
	 }
  sprintf(KeyString,"Software\\Classes\\CLSID\\%s\\LocalServer",CLSID_Temp);
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, KeyString, 0, KEY_QUERY_VALUE, &hKey )==
	 ERROR_SUCCESS) 
     {
       Temp_Length=FileNameLength;
       if (RegQueryValueEx(hKey, NULL, NULL, &Type, (BYTE *)FileName, 
	                &Temp_Length )==ERROR_SUCCESS) 
       {  
          StrTrim(FileName, trim);
          if (_tcsclen(FileName)!=0) 
		   {
            RegCloseKey(hKey);
			RemoveExtraneousArgs(FileName);
			ExpandEnvironmentVars(FileName,FileNameLength);
			DeQuoteString(FileName);
			return(SUCCESS);
		   }
	   }
	   RegCloseKey(hKey);
	 }

  sprintf(KeyString,"Software\\Classes\\CLSID\\%s\\TreatAs",CLSID_Temp);
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, KeyString, 0, KEY_QUERY_VALUE, &hKey )==
	 ERROR_SUCCESS) 
     {
       CLSID_Temp_Length=sizeof(CLSID_Temp);
       if (RegQueryValueEx(hKey, NULL, NULL, &Type, (BYTE *)CLSID_Temp, 
	                &CLSID_Temp_Length )==ERROR_SUCCESS) 
       {  
	    RegCloseKey(hKey);
	   }
	   else
	   {
		 RegCloseKey(hKey);
		 break;
	   }
	 }
  else 
  {
	 break;
  }
  Count++;
  if (Count>10) break;
 }


  strcpy(FileName,"[FILENAME NOT AVAILABLE]");
 return(-1);

}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static BOOL GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId,       //
///                               DWORD &dwId, BOOL bPrimaryEnough)                   //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////

static BOOL GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough)
{
	for (LPWORD lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData)+unBlockSize; lpwData+=2)
	{
		if (*lpwData == wLangId)
		{
			dwId = *((DWORD*)lpwData);
			return TRUE;
		}
	}

	if (!bPrimaryEnough)
		return FALSE;

	for (LPWORD lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData)+unBlockSize; lpwData+=2)
	{
		if (((*lpwData)&0x00FF) == (wLangId&0x00FF))
		{
			dwId = *((DWORD*)lpwData);
			return TRUE;
		}
	}

	return FALSE;
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// int PrintFileInfo(HANDLE handle,TCHAR *FileName)                                  //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int PrintFileInfo(HANDLE handle,TCHAR *FileName)
{
    DWORD dwHandle, dwLen;
	UINT BufLen;
	LPTSTR lpData, lpBuffer;
	VS_FIXEDFILEINFO *pFileInfo;
	DWORD	dwLangCode = 0;
	char   StringInfo[MAX_PATH];


	dwLen = GetFileVersionInfoSize (FileName, &dwHandle);
    sprintf(TempTextBuffer,"*******************************************************************************\r\n");
    WriteText(handle,TempTextBuffer);

	sprintf(TempTextBuffer,"%-20s: %s\r\n","COM Object Filename",FileName);
	WriteText(handle,TempTextBuffer);

	if (!dwLen)
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Version Info","not found");
		WriteText(handle,TempTextBuffer);
        sprintf(TempTextBuffer,"*******************************************************************************\r\n");
        WriteText(handle,TempTextBuffer);
		return -1;
	}
	if ((lpData = (LPTSTR) malloc (dwLen))==NULL)return -1;
	if (!GetFileVersionInfo (FileName, dwHandle, dwLen, lpData)) 
	{
		free (lpData);
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Version Info","not found");
		WriteText(handle,TempTextBuffer);
        sprintf(TempTextBuffer,"*******************************************************************************\r\n");
        WriteText(handle,TempTextBuffer);
		return -1;
	}
	if (!VerQueryValue (lpData, "\\", (LPVOID *) &pFileInfo, (PUINT) &BufLen)) 
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Version Info","not found");
		WriteText(handle,TempTextBuffer);
	}
	else 
	{
		sprintf(TempTextBuffer,"%-20s: %d\r\n","Major Version",HIWORD(pFileInfo->dwFileVersionMS));
		WriteText(handle,TempTextBuffer);
		sprintf(TempTextBuffer,"%-20s: %d\r\n","Minor Version",LOWORD(pFileInfo->dwFileVersionMS));
		WriteText(handle,TempTextBuffer);
		sprintf(TempTextBuffer,"%-20s: %d\r\n","Build Number",HIWORD(pFileInfo->dwFileVersionLS));
		WriteText(handle,TempTextBuffer);
		sprintf(TempTextBuffer,"%-20s: %d\r\n","Revision Number",LOWORD(pFileInfo->dwFileVersionLS));
		WriteText(handle,TempTextBuffer);
	}
	    
		// find best matching language and codepage
		VerQueryValue(lpData, _T("\\VarFileInfo\\Translation"), (LPVOID *)&pFileInfo, (PUINT) &BufLen);

		if (!GetTranslationId(pFileInfo, BufLen, GetUserDefaultLangID(), dwLangCode, FALSE))
		{
			if (!GetTranslationId(pFileInfo, BufLen, GetUserDefaultLangID(), dwLangCode, TRUE))
			{
				if (!GetTranslationId(pFileInfo, BufLen, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), dwLangCode,TRUE))
				{
					if (!GetTranslationId(pFileInfo, BufLen, MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL), dwLangCode, TRUE))
						dwLangCode = *((DWORD*)pFileInfo);
				}
			}
		}
    
   
 
    sprintf(StringInfo,"\\StringFileInfo\\%04X%04X\\%s",dwLangCode&0x0000FFFF, (dwLangCode&0xFFFF0000)>>16,
		   "ProductVersion");
	if (!VerQueryValue (lpData, StringInfo,(LPVOID *) &lpBuffer, (PUINT) &BufLen)) 
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Product Version","not found");
	}
	else
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Product Version",lpBuffer);
	}
	WriteText(handle,TempTextBuffer);

    sprintf(StringInfo,"\\StringFileInfo\\%04X%04X\\%s",dwLangCode&0x0000FFFF, (dwLangCode&0xFFFF0000)>>16,
		   "ProductName");
	if (!VerQueryValue (lpData, StringInfo,(LPVOID *) &lpBuffer, (PUINT) &BufLen)) 
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Product Name","not found");
	}
	else
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Product Name",lpBuffer);
	}
	WriteText(handle,TempTextBuffer);


	sprintf(StringInfo,"\\StringFileInfo\\%04X%04X\\%s",dwLangCode&0x0000FFFF, (dwLangCode&0xFFFF0000)>>16,
		   "CompanyName");
	if (!VerQueryValue (lpData, StringInfo,(LPVOID *) &lpBuffer, (PUINT) &BufLen)) 
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Company Name","not found");
	}
	else
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Company Name",lpBuffer);
	}
    WriteText(handle,TempTextBuffer);

	sprintf(StringInfo,"\\StringFileInfo\\%04X%04X\\%s",dwLangCode&0x0000FFFF, (dwLangCode&0xFFFF0000)>>16,
		   "LegalCopyright");
	if (!VerQueryValue (lpData, StringInfo,(LPVOID *) &lpBuffer, (PUINT) &BufLen)) 
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Legal Copyright","not found");
	}
	else
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Legal Copyright",lpBuffer);
	}
	WriteText(handle,TempTextBuffer);

	sprintf(StringInfo,"\\StringFileInfo\\%04X%04X\\%s",dwLangCode&0x0000FFFF, (dwLangCode&0xFFFF0000)>>16,
		   "Comments");
	if (!VerQueryValue (lpData, StringInfo,	(LPVOID *) &lpBuffer, (PUINT) &BufLen)) 
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Comments","not found");
	}
	else
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Comments",lpBuffer);
	}
	WriteText(handle,TempTextBuffer);

	sprintf(StringInfo,"\\StringFileInfo\\%04X%04X\\%s",dwLangCode&0x0000FFFF, (dwLangCode&0xFFFF0000)>>16,
		   "FileDescription");
	if (!VerQueryValue (lpData, StringInfo,(LPVOID *) &lpBuffer, (PUINT) &BufLen)) 
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","File Description","not found");
	}
	else
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","File Description",lpBuffer);
	}
	WriteText(handle,TempTextBuffer);

	sprintf(StringInfo,"\\StringFileInfo\\%04X%04X\\%s",dwLangCode&0x0000FFFF, (dwLangCode&0xFFFF0000)>>16,
		   "FileVersion");
	if (!VerQueryValue (lpData, StringInfo,(LPVOID *) &lpBuffer, (PUINT) &BufLen)) 
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","File Version","not found");
	}
	else
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","File Version",lpBuffer);
	}
    WriteText(handle,TempTextBuffer);

	sprintf(StringInfo,"\\StringFileInfo\\%04X%04X\\%s",dwLangCode&0x0000FFFF, (dwLangCode&0xFFFF0000)>>16,
		   "InternalName");
	if (!VerQueryValue (lpData, StringInfo,(LPVOID *) &lpBuffer, (PUINT) &BufLen)) 
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Internal Name","not found");
	}
	else
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Internal Name",lpBuffer);
	}
	WriteText(handle,TempTextBuffer);

	sprintf(StringInfo,"\\StringFileInfo\\%04X%04X\\%s",dwLangCode&0x0000FFFF, (dwLangCode&0xFFFF0000)>>16,
		   "LegalTrademarks");
	if (!VerQueryValue (lpData, StringInfo,(LPVOID *) &lpBuffer, (PUINT) &BufLen)) 
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Legal Trademarks","not found");
	}
	else
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Legal Trademarks",lpBuffer);
	}
	WriteText(handle,TempTextBuffer);

	sprintf(StringInfo,"\\StringFileInfo\\%04X%04X\\%s",dwLangCode&0x0000FFFF, (dwLangCode&0xFFFF0000)>>16,
		   "PrivateBuild");
	if (!VerQueryValue (lpData, StringInfo,(LPVOID *) &lpBuffer, (PUINT) &BufLen)) 
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Private Build","not found");
	}
	else
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Private Build",lpBuffer);
	}
	WriteText(handle,TempTextBuffer);

	sprintf(StringInfo,"\\StringFileInfo\\%04X%04X\\%s",dwLangCode&0x0000FFFF, (dwLangCode&0xFFFF0000)>>16,
		   "SpecialBuild");
	if (!VerQueryValue (lpData, StringInfo,(LPVOID *) &lpBuffer, (PUINT) &BufLen)) 
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Special Build","not found");
	}
	else
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Special Build",lpBuffer);
	}
	WriteText(handle,TempTextBuffer);

	sprintf(StringInfo,"\\StringFileInfo\\%04X%04X\\%s",dwLangCode&0x0000FFFF, (dwLangCode&0xFFFF0000)>>16,
		   "Language");
	if (!VerQueryValue (lpData, StringInfo,(LPVOID *) &lpBuffer, (PUINT) &BufLen)) 
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Language","not found");
	}
	else
	{
		sprintf(TempTextBuffer,"%-20s: %s\r\n","Language",lpBuffer);
	}
    WriteText(handle,TempTextBuffer);

	free (lpData);

    sprintf(TempTextBuffer,"*******************************************************************************\r\n");
    WriteText(handle,TempTextBuffer);
	return(0);
}

////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static int Get_CLSID_From_String()                                                //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int Get_CLSID_From_String(TCHAR *CLSID_String,CLSID *clsid)
{
 WCHAR CLSID_Str_Wide[MAX_PATH];
 DWORD CLSID_Str_Wide_Length=MAX_PATH;

 if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
	                     CLSID_String, -1, 
						 CLSID_Str_Wide, CLSID_Str_Wide_Length)==0) return(MULTIBYTE_TO_WIDE_CHAR_FAILED);
 if (CLSIDFromString(CLSID_Str_Wide, clsid)!=NOERROR) return(CLSID_FROM_STRING_FAILED);

 return(SUCCESS);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// int TestCOM_Object(CLSID *clsid)                                                  //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int TestCOM_Object(CLSID *clsid)
{
 HRESULT        hResult=E_FAIL;
 IObjectSafety *pIObjectSafety = NULL;
 int            RetVal;

 try
  {

    hResult=CoCreateInstance(*clsid, NULL, CLSCTX_LOCAL_SERVER | CLSCTX_INPROC_SERVER,
                            IID_IObjectSafety, (void**)&pIObjectSafety);
  }
 catch(...)
  {
   pIObjectSafety=NULL;
  }
 if ((hResult==S_OK) && (pIObjectSafety!=NULL))
    {
     bool IDispatchEx_SafeForScripting=false;
     bool IDispatch_SafeForScripting=false;

      try
	   {
	    hResult = pIObjectSafety->SetInterfaceSafetyOptions(IID_IDispatchEx, 
	 	          INTERFACESAFE_FOR_UNTRUSTED_CALLER, INTERFACESAFE_FOR_UNTRUSTED_CALLER);
        if (hResult==S_OK) IDispatchEx_SafeForScripting=true;
	    
		hResult = pIObjectSafety->SetInterfaceSafetyOptions(IID_IDispatch, 
		           INTERFACESAFE_FOR_UNTRUSTED_CALLER, INTERFACESAFE_FOR_UNTRUSTED_CALLER);
        if (hResult==S_OK)IDispatch_SafeForScripting=true;
	   }
      catch(...)
       {
        pIObjectSafety->Release();
        return(COM_OBJECTSAFETY_SET_INTERFACE_OPT_FAULT);
       }
      if ((IDispatchEx_SafeForScripting) || (IDispatch_SafeForScripting))
	   {
	    if (IDispatchEx_SafeForScripting)
		 {
          IDispatchEx * pIDispatchEx = NULL; 
		  hResult = pIObjectSafety->QueryInterface(IID_IDispatchEx, (void**)&pIDispatchEx);
		  if (hResult== S_OK)
		   {
            if (ExecutionMode==TEST_CONTROL)
		     {
              RetVal=Test_Dispatch_Interface(pIDispatchEx,0);
		      pIDispatchEx->Release();
			  pIObjectSafety->Release();
		      return(RetVal);
		     }
		    else if (ExecutionMode==GEN_INTERFACE)
		    {
		     RetVal=PrintInterfaceInfo(pIDispatchEx);
		     pIDispatchEx->Release();
			 pIObjectSafety->Release();
		     return(RetVal);
         	}
            else return(SUCCESS);
		   }
		  else
		  {
           if (IDispatch_SafeForScripting)
		   {
            IDispatch * pIDispatch = NULL;
  		    hResult = pIObjectSafety->QueryInterface(IID_IDispatch, (void**)&pIDispatch);
		    if (hResult== S_OK)
		     {
              if (ExecutionMode==TEST_CONTROL)
	  	        {
                 RetVal=Test_Dispatch_Interface(pIDispatch,0);
		         pIDispatch->Release();
				 pIObjectSafety->Release();
		         return(RetVal);
		        }
		      else if (ExecutionMode== GEN_INTERFACE)
		       {
		        RetVal=PrintInterfaceInfo(pIDispatch);
		        pIDispatch->Release();
				pIObjectSafety->Release();
		        return(RetVal);
		       }
              else return(SUCCESS);
		     }
		     else
		     {
              pIObjectSafety->Release();
		      return(QUERY_INTERFACE_FOR_IDISPATCH_FAILED);
		     }
		   }
		   else
		   {
            pIObjectSafety->Release();
		    return(QUERY_INTERFACE_FOR_IDISPATCH_EX_FAILED);
		   }
		  }
		 }
		else
		{
         IDispatch * pIDispatch = NULL;
		 hResult = pIObjectSafety->QueryInterface(IID_IDispatch, (void**)&pIDispatch);
		 if (hResult== S_OK)
		  {
           if (ExecutionMode==TEST_CONTROL)
		    {
             RetVal=Test_Dispatch_Interface(pIDispatch,0);
		     pIDispatch->Release();
			 pIObjectSafety->Release();
		     return(RetVal);
		    }
		   else if (ExecutionMode== GEN_INTERFACE)
	    	{
		     RetVal=PrintInterfaceInfo(pIDispatch);
		     pIDispatch->Release();
			 pIObjectSafety->Release();
		     return(RetVal);
		    }
            else return(SUCCESS);
		  }
		  else
		  {
           pIObjectSafety->Release();
		   return(QUERY_INTERFACE_FOR_IDISPATCH_FAILED);
		  }
		}

	   }
	  else
	  {
       pIObjectSafety->Release();
       return(COM_OBJECT_NOT_SCRIPT_SAFE);
	  }

	}
 else
 {
  LPOLESTR CLSID_String_Wide;
  LONG     RetValue;
  HKEY     hKeyQ;
  TCHAR    RegKey[1024];
  bool     RegistryEntrySafeForScripting=false;

  hResult = StringFromCLSID(*clsid, &CLSID_String_Wide);
  if (FAILED(hResult))
    {
	 return(STRING_FROM_CLSID_FAILED);
    }
  sprintf(RegKey, "Software\\Classes\\CLSID\\%ws\\Implemented Categories\\{7DD95801-9882-11CF-9FA9-00AA006C42C4}", CLSID_String_Wide);
  RetValue = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegKey, 0, KEY_QUERY_VALUE, &hKeyQ );
  RegCloseKey(hKeyQ);
  if (RetValue == ERROR_SUCCESS) RegistryEntrySafeForScripting=true;
  CoTaskMemFree(CLSID_String_Wide);
if (RegistryEntrySafeForScripting)
  {
   IDispatch * pIDispatch = NULL;
   
   hResult = CoCreateInstance(*clsid, NULL, CLSCTX_LOCAL_SERVER | CLSCTX_INPROC_SERVER, IID_IDispatch, (void**)&pIDispatch);
   if (hResult== S_OK)
	  {
        if (ExecutionMode==TEST_CONTROL)
		{
          RetVal=Test_Dispatch_Interface(pIDispatch,0);
		  pIDispatch->Release();
		  return(RetVal);
		}
		else if (ExecutionMode== GEN_INTERFACE)
		{
		 RetVal=PrintInterfaceInfo(pIDispatch);
		 pIDispatch->Release();
		 return(RetVal);
		}
		else return(SUCCESS);
      }
  else
	  { 
		return(QUERY_INTERFACE_FOR_IDISPATCH_FAILED);
	  }
    
  }
  else
  {
   return(COM_OBJECT_NOT_SCRIPT_SAFE);
  }
  
 }
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// int PrintComObjectInformation(CLSID *clsid)                                       //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int PrintComObjectInformation(CLSID *clsid)
{
 HRESULT        hResult=E_FAIL;
 IObjectSafety *pIObjectSafety = NULL;
 LPOLESTR CLSID_ForKB_AsWide, ProgIdAsWide;

 bool HaveIObjectSaftey=false;
 bool IDispatchEx_SafeForScripting=false;
 bool IDispatch_SafeForScripting=false;
 bool IPersist_SafeForUntrustedData=false;
 int  KillBit;

 if (ProgIDFromCLSID(*clsid,&ProgIdAsWide)==S_OK) 
 {
  sprintf(TempTextBuffer,"ProgId is : %ws\r\n",ProgIdAsWide);
  WriteText(hLogFile,TempTextBuffer);
  CoTaskMemFree(ProgIdAsWide);
 }


 if (StringFromCLSID(*clsid,&CLSID_ForKB_AsWide)==S_OK) 
 {
  KillBit=GetKillBit(CLSID_ForKB_AsWide);
  CoTaskMemFree( CLSID_ForKB_AsWide);
  if (KillBit==1)
  {
	 sprintf(TempTextBuffer,"Kill Bit : true\r\n");
     WriteText(hLogFile,TempTextBuffer);
  }
 }

 try
  {

    hResult=CoCreateInstance(*clsid, NULL, CLSCTX_LOCAL_SERVER | CLSCTX_INPROC_SERVER,
                            IID_IObjectSafety, (void**)&pIObjectSafety);
  }
 catch(...)
  {
   pIObjectSafety=NULL;
  }
 if ((hResult==S_OK) && (pIObjectSafety!=NULL))
    {
     HaveIObjectSaftey=true;
      try
	   {
	    hResult = pIObjectSafety->SetInterfaceSafetyOptions(IID_IDispatchEx, 
	 	          INTERFACESAFE_FOR_UNTRUSTED_CALLER, INTERFACESAFE_FOR_UNTRUSTED_CALLER);
        if (hResult==S_OK) IDispatchEx_SafeForScripting=true;
	    
		hResult = pIObjectSafety->SetInterfaceSafetyOptions(IID_IDispatch, 
		           INTERFACESAFE_FOR_UNTRUSTED_CALLER, INTERFACESAFE_FOR_UNTRUSTED_CALLER);
        if (hResult==S_OK)IDispatch_SafeForScripting=true;
	   	hResult = pIObjectSafety->SetInterfaceSafetyOptions(IID_IPersistPropertyBag,
		          INTERFACESAFE_FOR_UNTRUSTED_DATA, INTERFACESAFE_FOR_UNTRUSTED_DATA);

		if (hResult==S_OK) IPersist_SafeForUntrustedData=true;

	   }
      catch(...)
       {
        pIObjectSafety->Release();
        return(COM_OBJECTSAFETY_SET_INTERFACE_OPT_FAULT);
       }
     
	 sprintf(TempTextBuffer,"IDispatchEx SafeForScripting : %s\r\n",IDispatchEx_SafeForScripting?"true":"false");
     WriteText(hLogFile,TempTextBuffer);
	 sprintf(TempTextBuffer,"IDispatch SafeForScripting   : %s\r\n",IDispatch_SafeForScripting?"true":"false");
     WriteText(hLogFile,TempTextBuffer);
	 sprintf(TempTextBuffer,"IPersist SafeForUntrustedData: %s\r\n",IPersist_SafeForUntrustedData?"true":"false");
     WriteText(hLogFile,TempTextBuffer);

	 pIObjectSafety->Release(); 
	}
  else
  {
	 sprintf(TempTextBuffer,"IObjectSafety not implemented\r\n");
     WriteText(hLogFile,TempTextBuffer);
  }

  LPOLESTR CLSID_String_Wide;
  LONG     RetValue;
  HKEY     hKeyQ;
  TCHAR    RegKey[1024];
  bool     RegistryEntrySafeForScripting=false;
  bool     RegistryEntrySafeForInitializingPersistentData=false; 

  hResult = StringFromCLSID(*clsid, &CLSID_String_Wide);
  if (FAILED(hResult))
    {
	 return(STRING_FROM_CLSID_FAILED);
    }

  sprintf(RegKey, "Software\\Classes\\CLSID\\%ws\\Implemented Categories\\{7DD95801-9882-11CF-9FA9-00AA006C42C4}", CLSID_String_Wide);
  RetValue = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegKey, 0, KEY_QUERY_VALUE, &hKeyQ );
  RegCloseKey(hKeyQ);
  if (RetValue == ERROR_SUCCESS) RegistryEntrySafeForScripting=true;
 

  sprintf(RegKey, "Software\\Classes\\CLSID\\%ws\\Implemented Categories\\{7DD95802-9882-11CF-9FA9-00AA006C42C4}", CLSID_String_Wide);
  RetValue =RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegKey, 0, KEY_QUERY_VALUE, &hKeyQ );
  RegCloseKey(hKeyQ);
  if (RetValue == ERROR_SUCCESS) RegistryEntrySafeForInitializingPersistentData=true;

  CoTaskMemFree(CLSID_String_Wide);



  sprintf(TempTextBuffer,"Registry entry safe for scripting: %s\r\n",RegistryEntrySafeForScripting?"true":"false");
  WriteText(hLogFile,TempTextBuffer);
   sprintf(TempTextBuffer,"Registry entry safe for initializing persistent data: %s\r\n",RegistryEntrySafeForInitializingPersistentData?"true":"false");
  WriteText(hLogFile,TempTextBuffer);
  if (HaveIObjectSaftey)
  {
    if ((IDispatchEx_SafeForScripting) || (IDispatch_SafeForScripting))
	{
     sprintf(TempTextBuffer,"Object safe for scripting: true\r\n");
     WriteText(hLogFile,TempTextBuffer);
	}
	else
	{
     sprintf(TempTextBuffer,"Object safe for scripting: false\r\n");
     WriteText(hLogFile,TempTextBuffer);
	}
    sprintf(TempTextBuffer,"Object safe for init: %s\r\n",IPersist_SafeForUntrustedData?"true":"false");
    WriteText(hLogFile,TempTextBuffer);
  }
  else
  {
   sprintf(TempTextBuffer,"Object safe for scripting: %s\r\n",RegistryEntrySafeForScripting?"true":"false");
   WriteText(hLogFile,TempTextBuffer);
   sprintf(TempTextBuffer,"Object safe for init: %s\r\n",RegistryEntrySafeForInitializingPersistentData?"true":"false");
   WriteText(hLogFile,TempTextBuffer);
  }

 return(SUCCESS);
}

////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// int PrintInterfaceInfo(IDispatch *pIDispatch)                                     //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int PrintInterfaceInfo(IDispatch *pIDispatch)
{
 HRESULT hResult;

  ITypeInfo        * pTypeInfo;
  TYPEATTR         * pTypeAttr;
  BSTR               InterfaceName;
  char             * MethodInformation;
  char             * PropertyType;
  
  MethodInformation=new char[2048];
  MethodInformation[0]=NULL;

  if (pIDispatch->GetTypeInfo(0, 0, &pTypeInfo)!=S_OK) 
  {
    delete[] MethodInformation;
	return (GET_TYPE_INFO_FAILED);
  }
    
  pTypeInfo->GetTypeAttr(&pTypeAttr);
  
  if (pTypeInfo->GetDocumentation(-1, &InterfaceName, 0, 0, 0)!=S_OK)
  {
     sprintf(TempTextBuffer,"Unknown default interface:\r\n");
     WriteText(hLogFile,TempTextBuffer);	
  }
  else 
  {
   sprintf(TempTextBuffer,"%ws:\r\n",InterfaceName);
   WriteText(hLogFile,TempTextBuffer);
  }
  for(INT CurrrentFunction=0; CurrrentFunction < pTypeAttr->cFuncs; CurrrentFunction++) 
   {
     FUNCDESC* FunctionDescription;
     hResult = pTypeInfo->GetFuncDesc(CurrrentFunction, &FunctionDescription);
	 if (!(FunctionDescription->wFuncFlags & FUNCFLAG_FRESTRICTED))
	 {
       BSTR     methodName;
	   string_m tempstr;
       hResult |= pTypeInfo->GetDocumentation(FunctionDescription->memid, &methodName, 0, 0, 0);
       if(hResult) 
		  { 
           sprintf(TempTextBuffer,"Error In Name\r\n");
           WriteText(hLogFile,TempTextBuffer);
           pTypeInfo->ReleaseFuncDesc(FunctionDescription); 
		   continue; 
		  }
		switch(FunctionDescription->invkind)
		 {
		  case INVOKE_FUNC:          PropertyType="Method                 "; break;
	      case INVOKE_PROPERTYGET:   PropertyType="Property Get           "; break;
	      case INVOKE_PROPERTYPUT:   PropertyType="Property Put           "; break;
	      case INVOKE_PROPERTYPUTREF:PropertyType="Property Put Reference "; break;
		  default: PropertyType=""; break;
		 }

        tempstr=TypeDescriptionToString(&FunctionDescription->elemdescFunc.tdesc,pTypeInfo);
		if (tempstr)
		{
         sprintf(MethodInformation,"\t%s%s %ws(",PropertyType,getstrptr_m(tempstr),methodName);
		 strdelete_m(&tempstr);
		}
        if (FunctionDescription->cParams>0)
		{
         BSTR* rgBstrNames = new BSTR[FunctionDescription->cParams + 1];
		 UINT NumrgBstrNames=0;
		 pTypeInfo->GetNames(FunctionDescription->memid,rgBstrNames, FunctionDescription->cParams + 1, &NumrgBstrNames);
		 if (NumrgBstrNames>0) SysFreeString(rgBstrNames[0]);
		 for (INT CurrentParameter=0; CurrentParameter < FunctionDescription->cParams; CurrentParameter++) 
		 {
          if(FunctionDescription->lprgelemdescParam[CurrentParameter].paramdesc.wParamFlags != 0)
            {
             bool NeedComma=false;
             strcat(MethodInformation,"[");
             if(FunctionDescription->lprgelemdescParam[CurrentParameter].paramdesc.wParamFlags & PARAMFLAG_FIN)
               {
                strcat(MethodInformation,"in");
			    NeedComma=true;
               }
             if(FunctionDescription->lprgelemdescParam[CurrentParameter].paramdesc.wParamFlags & PARAMFLAG_FOUT)
               {
			    if (NeedComma) strcat(MethodInformation,", ");
                strcat(MethodInformation,"out");
				 NeedComma=true;
               }
             if(FunctionDescription->lprgelemdescParam[CurrentParameter].paramdesc.wParamFlags & PARAMFLAG_FRETVAL)
               {
                if (NeedComma) strcat(MethodInformation,", ");
                strcat(MethodInformation,"retval");
			    NeedComma=true;
               }
             if(FunctionDescription->lprgelemdescParam[CurrentParameter].paramdesc.wParamFlags & PARAMFLAG_FOPT)
               {
                if (NeedComma) strcat(MethodInformation,", ");
                strcat(MethodInformation,"optional");
               }
             strcat(MethodInformation,"] ");
            }                        
			tempstr=TypeDescriptionToString(&FunctionDescription->lprgelemdescParam[CurrentParameter].tdesc, pTypeInfo);
            if (tempstr)
			  {
			   strcat(MethodInformation,getstrptr_m(tempstr));
			   strdelete_m(&tempstr);
			  }
	  	    if ((CurrentParameter+1) < (INT) NumrgBstrNames)
	          {
               char *TempArgStr=new char[2048];
			   sprintf(TempArgStr," %ws",rgBstrNames[CurrentParameter+1]);
		 	   strcat(MethodInformation,TempArgStr);
               SysFreeString(rgBstrNames[CurrentParameter+1]);
			   delete [] TempArgStr;
		      }
            if(CurrentParameter < FunctionDescription->cParams - 1)  strcat(MethodInformation,", ");
            
         }
         delete[] rgBstrNames;
		}
        strcat(MethodInformation,")");
		sprintf(TempTextBuffer,"%s\r\n",MethodInformation);
		WriteText(hLogFile,TempTextBuffer);
        pTypeInfo->ReleaseFuncDesc(FunctionDescription);
	 }
   }
  if(InterfaceName)
   {
	::SysFreeString(InterfaceName);
   }

  pTypeInfo->ReleaseTypeAttr(pTypeAttr);
  delete[] MethodInformation;
  return(SUCCESS);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// int SetArgument(VARIANTARG *arg,BSTR BigString)                                   //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int SetArgument(VARIANTARG *arg,BSTR BigString)
{
 switch(arg->vt)
  {
   case VT_BSTR:
  				arg->bstrVal = BigString;
                return(0);
   case VT_PTR:
	            arg->vt=VT_BSTR;
  				arg->bstrVal = BigString;
                return(0);
   case VT_I1:
			    arg->cVal=-1;
                return(0);
   case VT_I2:
				arg->iVal=-1;
                return(0);
   case VT_I4:
 			    arg->lVal = -1; 
                return(0);
   case VT_I8:
			    arg->llVal=-1;
                return(0);     
   case VT_UI1:
			    arg->bVal=0xFF;
                return(0);
   case VT_UI2:
				arg->uiVal=0xFFFF;
                return(0);
   case VT_UI4:
 			    arg->ulVal = 0xFFFFFFFF; 
                return(0);
   case VT_UI8:
			    arg->ullVal=0xFFFFFFFFFFFFFFFF;
                return(0);
   case VT_R4:
			    arg->fltVal =0.0;
                return(0);
   case VT_R8:
			    arg->dblVal = 0.0;
                return(0);
   case VT_BOOL:
			    arg->boolVal = 10000;
                return(0);
   case (VT_BYREF|VT_BSTR):
  			    arg->pbstrVal = NULL;
                return(1);
   case (VT_BYREF|VT_I1):
				arg->pcVal=NULL;
                return(1);
   case (VT_BYREF|VT_I2):
			    arg->piVal=NULL;
                return(1);
   case (VT_BYREF|VT_I4):
 			    arg->plVal =NULL; 
                return(1);
   case (VT_BYREF|VT_I8):
			    arg->pllVal=NULL;
                return(1);     
   case (VT_BYREF|VT_UI1):
			    arg->pbVal=NULL;
                return(1);
   case (VT_BYREF|VT_UI2):
				arg->puiVal=NULL;
                return(1);
   case (VT_BYREF|VT_UI4):
 			   arg->pulVal =NULL;
               return(1);
   case (VT_BYREF|VT_UI8):
			   arg->pullVal=NULL;
               return(1);
   case (VT_BYREF|VT_R4):
			   arg->pfltVal =NULL;
               return(1);
   case (VT_BYREF|VT_R8):
			   arg->pdblVal = NULL;
               return(1);
   case (VT_BYREF|VT_BOOL):
	 	       arg->pboolVal = NULL;
               return(1);
   case (VT_BYREF|VT_DECIMAL):
			   arg->pdecVal = NULL;
               return(1);
   case (VT_BYREF):
			   arg->byref = NULL;
			   return(1);
   case  VT_UNKNOWN:
			   arg->punkVal=NULL;
               return(1);
   case  VT_DISPATCH:
		       arg->pdispVal=NULL; 
               return(1);
   case  VT_ARRAY:
			   arg->parray=NULL;
               return(1);
   case  VT_DATE:
               arg->date=-9999999999999999999999.99999999;
               return(0);
   case  (VT_BYREF|VT_UNKNOWN):
		       arg->ppunkVal=NULL;
               return(1);
   case  (VT_BYREF|VT_DISPATCH):
			   arg->ppdispVal=NULL; 
               return(1);
   case  (VT_BYREF|VT_ARRAY):
               arg->pparray=NULL;
               return(1);
   case  (VT_BYREF|VT_VARIANT):
               arg->pvarVal=NULL;
               return(1);
   default:
			 return(1);
 }
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static char * GetArgumentStringEquivalent(USHORT vt)                              //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static char * GetArgumentStringEquivalent(USHORT vt)
{
 switch(vt)
  {
   case VT_BSTR:
   case VT_PTR:
				return(PropAndMethodDispString);
   case VT_I1:
   case VT_I2:
   case VT_I4:
   case VT_I8:
	            return("<-1>");  
   case VT_UI1:
				return("<0xFF>");  
   case VT_UI2:
				return("<0xFFFF>");  
   case VT_UI4:
                return("<0xFFFFFFFF>");
   case VT_UI8:
                return("<0xFFFFFFFFFFFFFFFF>");  
   case VT_R4:
   case VT_R8:
                return("<0.0>");
   case VT_BOOL:
	            return("<10000>");
   case (VT_BYREF|VT_BSTR):
   case (VT_BYREF|VT_I1):
   case (VT_BYREF|VT_I2):
   case (VT_BYREF|VT_I4):
   case (VT_BYREF|VT_I8):
   case (VT_BYREF|VT_UI1):
   case (VT_BYREF|VT_UI2):
   case (VT_BYREF|VT_UI4):
   case (VT_BYREF|VT_UI8):
   case (VT_BYREF|VT_R4):
   case (VT_BYREF|VT_R8):
   case (VT_BYREF|VT_BOOL):
   case (VT_BYREF|VT_DECIMAL):
   case (VT_BYREF):
   case  VT_UNKNOWN:
   case  VT_DISPATCH:
   case  VT_ARRAY:
			   return("<NULL>");
   case  VT_DATE:
			   return("<-9999999999999999999999.99999999>");
   case  (VT_BYREF|VT_UNKNOWN):
   case  (VT_BYREF|VT_DISPATCH):
   case  (VT_BYREF|VT_ARRAY):
   case  (VT_BYREF|VT_VARIANT):
             return("<NULL>");
   default:
			 return("<*Unknown-Type-Not Set*>");
 }
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static string_m CustomTypeToString(HREFTYPE refType, ITypeInfo* pTypeInfo)        //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static string_m CustomTypeToString(HREFTYPE refType, ITypeInfo* pTypeInfo)
{
 ITypeInfo *pCustTypeInfo;
 string_m   ReturnValue = NULL;
 char       TempString[MAX_PATH];
 BSTR       pBstrName;

 HRESULT hr=(pTypeInfo->GetRefTypeInfo(refType, &pCustTypeInfo));
 if(hr) 
 {
	if (strcreate_m(&ReturnValue,"UnknownCustomType", 0, NULL)) return(NULL);
	return (ReturnValue);
 }
 hr = pCustTypeInfo->GetDocumentation(-1, &pBstrName, 0, 0, 0);
 if(hr)
 {
	if (strcreate_m(&ReturnValue,"UnknownCustomType", 0, NULL)) return(NULL);
	pCustTypeInfo->Release();
	return (ReturnValue);
 }

 WideCharToMultiByte(CP_ACP, 0, pBstrName, -1, TempString, MAX_PATH, 0, 0);
 if(pBstrName) ::SysFreeString(pBstrName);
 pCustTypeInfo->Release();
 if (strcreate_m(&ReturnValue,TempString, 0, NULL)) return(NULL);
 return (ReturnValue);
}

////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static string_m TypeDescriptionToString(TYPEDESC* typeDesc, ITypeInfo* pTypeInfo) //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static string_m TypeDescriptionToString(TYPEDESC* typeDesc, ITypeInfo* pTypeInfo)
{
 string_m   ReturnString;

 if (strcreate_m(&ReturnString,NULL, 0, NULL)) return(NULL);

 if(typeDesc->vt == VT_PTR)
   {
    string_m tempstr=TypeDescriptionToString(typeDesc->lptdesc, pTypeInfo);
	if (tempstr)
	{
     strcat_m(ReturnString,tempstr);
	 strdelete_m(&tempstr);
	}
     cstrcat_m(ReturnString,"*");
	return ReturnString;
   }
 if(typeDesc->vt == VT_SAFEARRAY) 
   {
    string_m tempstr=TypeDescriptionToString(typeDesc->lptdesc, pTypeInfo);
	cstrcat_m(ReturnString,"SAFEARRAY(");
	if (tempstr)
	{
     strcat_m(ReturnString,tempstr);
	 strdelete_m(&tempstr);
	}
	cstrcat_m(ReturnString,")");
	return ReturnString;
   }
 if(typeDesc->vt == VT_CARRAY) 
   {


    string_m tempstr=TypeDescriptionToString(&typeDesc->lpadesc->tdescElem, pTypeInfo);
	if (tempstr)
	 {
      strcat_m(ReturnString,tempstr);
	  strdelete_m(&tempstr);
	 }

	for(int dim=0; typeDesc->lpadesc->cDims; dim++) 
	 {
	  char tempbuffer[256];
	 
      cstrcat_m(ReturnString,"[");
	  sprintf(tempbuffer,"%d",typeDesc->lpadesc->rgbounds[dim].lLbound);
	  cstrcat_m(ReturnString,tempbuffer);
      cstrcat_m(ReturnString,"...");
	  sprintf(tempbuffer,"%d",typeDesc->lpadesc->rgbounds[dim].cElements + 
	  		                 typeDesc->lpadesc->rgbounds[dim].lLbound - 1);
	  cstrcat_m(ReturnString,tempbuffer);
	  cstrcat_m(ReturnString,"]");
	 }
	 return ReturnString;
	}
 if(typeDesc->vt == VT_USERDEFINED)
   {
    string_m tempstr=CustomTypeToString(typeDesc->hreftype, pTypeInfo);
	if (tempstr)
	 {
  	  strcat_m(ReturnString,tempstr);
	  strdelete_m(&tempstr);
	 }
	return ReturnString;
  }

 switch(typeDesc->vt)
  {
	case VT_I2: cstrcat_m(ReturnString,"short"); break; 
	case VT_I4: cstrcat_m(ReturnString,"long"); break;
	case VT_R4: cstrcat_m(ReturnString,"float"); break;
	case VT_R8: cstrcat_m(ReturnString,"double"); break;
	case VT_CY: cstrcat_m(ReturnString,"CY"); break;
	case VT_DATE: cstrcat_m(ReturnString,"DATE"); break;
	case VT_BSTR: cstrcat_m(ReturnString,"BSTR"); break;
	case VT_DISPATCH: cstrcat_m(ReturnString,"IDispatch*"); break;
	case VT_ERROR: cstrcat_m(ReturnString,"SCODE"); break;
	case VT_BOOL: cstrcat_m(ReturnString,"VARIANT_BOOL"); break;
	case VT_VARIANT: cstrcat_m(ReturnString,"VARIANT"); break;
	case VT_UNKNOWN: cstrcat_m(ReturnString,"IUnknown*"); break;
	case VT_UI1: cstrcat_m(ReturnString,"BYTE"); break;
	case VT_DECIMAL: cstrcat_m(ReturnString,"DECIMAL"); break;
	case VT_I1: cstrcat_m(ReturnString,"char"); break;
	case VT_UI2: cstrcat_m(ReturnString,"USHORT"); break;
	case VT_UI4: cstrcat_m(ReturnString,"ULONG"); break;
	case VT_I8: cstrcat_m(ReturnString,"__int64"); break;
	case VT_UI8: cstrcat_m(ReturnString,"unsigned __int64"); break;
	case VT_INT: cstrcat_m(ReturnString,"int"); break;
	case VT_UINT: cstrcat_m(ReturnString,"UINT"); break;
	case VT_HRESULT: cstrcat_m(ReturnString,"HRESULT"); break;
	case VT_VOID: cstrcat_m(ReturnString,"void"); break;
	case VT_LPSTR: cstrcat_m(ReturnString,"char*"); break;
	case VT_LPWSTR: cstrcat_m(ReturnString,"wchar_t*"); break;
	default: cstrcat_m(ReturnString,"Error"); break;
	}

	return(ReturnString);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static int Test_DispatchEx_Interface(IDispatchEx *pIDispatchEx,,DWORD level)      //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int Test_DispatchEx_Interface(IDispatchEx *pIDispatchEx,DWORD Level)
{
 return(Test_Dispatch_Interface(pIDispatchEx,Level));
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static int Test_Dispatch_Interface(IDispatch *pIDispatch,DWORD Level)             //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int Test_Dispatch_Interface(IDispatch *pIDispatch,DWORD Level)
{
  if (Level>MAXRECURSE) return(MAX_RECURSIVE_LEVEL_REACHED);

  TestProperites(pIDispatch,Level);
  TestMethods(pIDispatch,Level);
  if ((Level==0) && (COM_Object_Exeception_Occurred)) return(COM_OBJECT_EXECEPTION_OCCURRED);
  else return(SUCCESS);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static int TestProperites(IDispatch *pIDispatch, DWORD Level)                     //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int TestProperites(IDispatch *pIDispatch, DWORD Level)
{
 ITypeInfo * pTypeInfo;
 HRESULT     hResult;
 VARIANT   * pArguments = NULL;
 TYPEATTR  * pTypeAttr;
 BSTR        InterfaceName;
 char      * MethodInformation;
 BSTR        LargeTestString;
 char      * PropertyType;

 MethodInformation=new char[2048];
 MethodInformation[0]=NULL;
 LargeTestString= ::SysAllocStringLen(NULL,METHOD_AND_PROP_STRING_LENGTH-1);
 wmemset(LargeTestString,METHOD_AND_PROP_STRING_CHARACTER,METHOD_AND_PROP_STRING_LENGTH-1);
  
 
 if (pIDispatch->GetTypeInfo(0, 0, &pTypeInfo)!=S_OK) 
   { 
	::SysFreeString(LargeTestString);
	delete[] MethodInformation;
	return (GET_TYPE_INFO_FAILED); 
   }

 
 pTypeInfo->GetTypeAttr(&pTypeAttr);
 
 
 if (pTypeInfo->GetDocumentation(-1, &InterfaceName, 0, 0, 0)!=S_OK)
   {
     sprintf(TempTextBuffer,"Unknown default interface:\r\n");
     WriteText(hLogFile,TempTextBuffer);	 
   }


 for(UINT CurrrentFunction=0; CurrrentFunction < pTypeAttr->cFuncs; CurrrentFunction++) 
    {
	 FUNCDESC* FunctionDescription;
	 hResult = pTypeInfo->GetFuncDesc(CurrrentFunction, &FunctionDescription);
     if (hResult!=S_OK) 
	 {
      if (hResult == E_OUTOFMEMORY) sprintf(TempTextBuffer,"GetFuncDesc Failed (E_OUTOFMEMORY)\r\n"); 
      else if (hResult == E_INVALIDARG)  sprintf(TempTextBuffer,"GetFuncDesc Failed (E_INVALIDARG)\r\n"); 
      else sprintf(TempTextBuffer,"GetFuncDesc Failed (0x%x)\r\n",hResult); 
	  WriteText(hLogFile,TempTextBuffer);
  	  continue;
	 }
	if (!(FunctionDescription->wFuncFlags & FUNCFLAG_FRESTRICTED))
	  {
		BSTR MethodName;
		string_m tempstr;
		hResult |= pTypeInfo->GetDocumentation(FunctionDescription->memid, &MethodName, 0, 0, 0);
		if(hResult!=S_OK) 
		  { 
			sprintf(TempTextBuffer,"Error In Method Name\r\n"); 
			WriteText(hLogFile,TempTextBuffer);
			pTypeInfo->ReleaseFuncDesc(FunctionDescription); 
			continue; 
		  }
		switch(FunctionDescription->invkind)
		 {
	      case INVOKE_PROPERTYGET:   PropertyType="Property Get          "; break;
	      case INVOKE_PROPERTYPUT:   PropertyType="Property Put          "; break;
	      case INVOKE_PROPERTYPUTREF:PropertyType="Property Put Reference"; break;
		 }
        tempstr=TypeDescriptionToString(&FunctionDescription->elemdescFunc.tdesc,pTypeInfo);
		if (tempstr)
		{
		 sprintf(MethodInformation,"%s %ws(",getstrptr_m(tempstr),MethodName);
         strdelete_m(&tempstr);
		}
		if(MethodName) ::SysFreeString(MethodName);
        if (FunctionDescription->cParams>0)
	 	 {
          BSTR* rgBstrNames = new BSTR[FunctionDescription->cParams + 1];
		  UINT NumrgBstrNames=0;
		  pTypeInfo->GetNames(FunctionDescription->memid,rgBstrNames, FunctionDescription->cParams + 1, &NumrgBstrNames);
		  if (NumrgBstrNames>0) SysFreeString(rgBstrNames[0]);

		  for (INT CurrentParameter=0; CurrentParameter < FunctionDescription->cParams; CurrentParameter++) 
		    {
             if(FunctionDescription->lprgelemdescParam[CurrentParameter].paramdesc.wParamFlags != 0)
               {
                bool NeedComma=false;
                strcat(MethodInformation,"[");
                if(FunctionDescription->lprgelemdescParam[CurrentParameter].paramdesc.wParamFlags & PARAMFLAG_FIN)
                  {
                   strcat(MethodInformation,"in");
	     		   NeedComma=true;
                  }
                if(FunctionDescription->lprgelemdescParam[CurrentParameter].paramdesc.wParamFlags & PARAMFLAG_FOUT)
                  {
			       if (NeedComma) strcat(MethodInformation,", ");
                   strcat(MethodInformation,"out");
				    NeedComma=true;
                  }
                if(FunctionDescription->lprgelemdescParam[CurrentParameter].paramdesc.wParamFlags & PARAMFLAG_FRETVAL)
                  {
                   if (NeedComma) strcat(MethodInformation,", ");
                   strcat(MethodInformation,"retval");
			       NeedComma=true;
                  }
                if(FunctionDescription->lprgelemdescParam[CurrentParameter].paramdesc.wParamFlags & PARAMFLAG_FOPT)
                  {
                   if (NeedComma) strcat(MethodInformation,", ");
                   strcat(MethodInformation,"optional");
                  }
                strcat(MethodInformation,"] ");
               }                        

             tempstr=TypeDescriptionToString(&FunctionDescription->lprgelemdescParam[CurrentParameter].tdesc, pTypeInfo);
             if (tempstr) 
			  {
				 strcat(MethodInformation,getstrptr_m(tempstr));
				 strdelete_m(&tempstr);
			  }
			if ((CurrentParameter+1) < (INT) NumrgBstrNames)
			 {
              char *TempArgStr=new char[2048];
			  sprintf(TempArgStr," %ws",rgBstrNames[CurrentParameter+1]);
			  strcat(MethodInformation,TempArgStr);
              SysFreeString(rgBstrNames[CurrentParameter+1]);
			  delete [] TempArgStr;
			 }
			 strcat(MethodInformation,GetArgumentStringEquivalent(FunctionDescription->lprgelemdescParam[CurrentParameter].tdesc.vt));
			 if(CurrentParameter < FunctionDescription->cParams - 1)  strcat(MethodInformation,", ");
		    }
           delete[] rgBstrNames;
		 }
	  strcat(MethodInformation,")");

	  if (FunctionDescription->invkind == INVOKE_PROPERTYGET)
		  {
		   int HasPointers=0;
		   DISPPARAMS Parameters;

		   memset(&Parameters, 0, sizeof (Parameters));

		   Parameters.cArgs = FunctionDescription->cParams;
		   Parameters.cNamedArgs = 0;
		   Parameters.rgvarg = NULL;

		   if (Parameters.cArgs > 0)
			  {
			   pArguments = new VARIANT[Parameters.cArgs];

			   Parameters.rgvarg = pArguments;
			   memset(pArguments, 0, sizeof(VARIANT) * Parameters.cArgs); 
			   for (unsigned int i = 0; i < Parameters.cArgs; i++)
				   {
					 VariantInit(&pArguments[i]);
					 pArguments[i].vt=FunctionDescription->lprgelemdescParam[i].tdesc.vt;
					 HasPointers|=SetArgument(&pArguments[i],LargeTestString);
				   }
			  }
           if (!HasPointers)
		   {
		   VARIANT Result;
		   VariantInit(&Result);
		   Result.vt=VT_PTR;
		   Result.pdispVal=NULL;
		   UINT uArgErr;

		   sprintf(TempTextBuffer,"Invoking %s - %ws::%s\r\n",PropertyType,InterfaceName,MethodInformation);
		   WriteText(hLogFile,TempTextBuffer);

		   try {
				 pIDispatch->Invoke(FunctionDescription->memid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &Parameters,&Result,NULL,&uArgErr);
			   } 
           catch (const access_violation& e) 
            { 
				 COM_Object_Exeception_Occurred=true;
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"***   Access Violation    ***\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);
                 sprintf(TempTextBuffer,"Invoked %s - %ws::%s\r\n",PropertyType,InterfaceName,MethodInformation);
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"%s at 0x%p :Bad %s on 0x%p\r\n",
			  	         e.what(), e.where(),e.isWrite()?"write":"read",e.badAddress());
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);
            }
          catch (const win32_exception& e) 
	        {    
				 COM_Object_Exeception_Occurred=true;
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"***   Win32 Exception     ***\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);
                 sprintf(TempTextBuffer,"Invoked %s - %ws::%s\r\n",PropertyType,InterfaceName,MethodInformation);
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"%s (code %x) at 0x%p\r\n",
			  	         e.what(), e.code(),e.where());
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);

           }

	       if (Result.vt == VT_PTR && !Result.pdispVal) 
		   {
#if LOGINFO
		     sprintf(TempTextBuffer,"*****************************\r\n");
			 WriteText(hLogFile,TempTextBuffer);
			 sprintf(TempTextBuffer,"***      No Results       ***\r\n");
			 WriteText(hLogFile,TempTextBuffer);
			 sprintf(TempTextBuffer,"*****************************\r\n");  
			 WriteText(hLogFile,TempTextBuffer);
			 sprintf(TempTextBuffer,"Invoked %s - %ws::%s\r\n",PropertyType,InterfaceName,MethodInformation);
			 WriteText(hLogFile,TempTextBuffer);
		     sprintf(TempTextBuffer,"*****************************\r\n");
			 WriteText(hLogFile,TempTextBuffer);
#endif
		   }
		   if (pArguments) delete[] pArguments;
		   pArguments = NULL;
					
		  if (Result.vt == VT_DISPATCH && Result.pdispVal)
		    {
              if (pIDispatch!=Result.pdispVal)
			  {
               LARGE_INTEGER CurrentPos,Before,After,zero;
			   zero.QuadPart=0;
			   if (!SetFilePointerEx(hLogFile,zero,&CurrentPos,FILE_CURRENT))
			   {
		        sprintf(TempTextBuffer,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
			    WriteText(hLogFile,TempTextBuffer);
		        sprintf(TempTextBuffer,"Set File Pointer 1 Failed\r\n");
			    WriteText(hLogFile,TempTextBuffer);
		        sprintf(TempTextBuffer,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
			    WriteText(hLogFile,TempTextBuffer);
			   }
               sprintf(TempTextBuffer,"Recurse from   interface %ws\r\n",InterfaceName);
			   WriteText(hLogFile,TempTextBuffer);
			   if (!SetFilePointerEx(hLogFile,zero,&Before,FILE_CURRENT))
			   {
		        sprintf(TempTextBuffer,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
			    WriteText(hLogFile,TempTextBuffer);
		        sprintf(TempTextBuffer,"Set File Pointer 2 Failed\r\n");
			    WriteText(hLogFile,TempTextBuffer);
		        sprintf(TempTextBuffer,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
			    WriteText(hLogFile,TempTextBuffer);
			   }
			   Test_Dispatch_Interface(Result.pdispVal,Level+1);
			   if (!SetFilePointerEx(hLogFile,zero,&After,FILE_CURRENT))
			   {
		        sprintf(TempTextBuffer,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
			    WriteText(hLogFile,TempTextBuffer);
		        sprintf(TempTextBuffer,"Set File Pointer 3 Failed\r\n");
			    WriteText(hLogFile,TempTextBuffer);
		        sprintf(TempTextBuffer,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
			    WriteText(hLogFile,TempTextBuffer);
			   }
			   if (Before.QuadPart==After.QuadPart)
			   {
 			    if (!SetFilePointerEx(hLogFile,CurrentPos,NULL,FILE_BEGIN))
			    {
		         sprintf(TempTextBuffer,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
			     WriteText(hLogFile,TempTextBuffer);
		         sprintf(TempTextBuffer,"Set File Pointer 4 Failed\r\n");
			     WriteText(hLogFile,TempTextBuffer);
		         sprintf(TempTextBuffer,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
			     WriteText(hLogFile,TempTextBuffer);
			    }
			   }
			   else
			   {
                sprintf(TempTextBuffer,"Recurse return %ws\r\n",InterfaceName);
			    WriteText(hLogFile,TempTextBuffer);
			   }

			  }
		    }
		  try 
		    {
			  VariantClear(&Result);
			}
           catch (const access_violation& e) 
            {
#if LOG_CRASH_ON_FREE
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"***   Access Violation    ***\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);
                 sprintf(TempTextBuffer,"Invoked - VariantClear 1\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"%s at 0x%p :Bad %s on 0x%p\r\n",
			  	         e.what(), e.where(),e.isWrite()?"write":"read",e.badAddress());
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);

#endif
            }
          catch (const win32_exception& e) 
	        {
#if LOG_CRASH_ON_FREE
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"***   Win32 Exception     ***\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);
                 sprintf(TempTextBuffer,"Invoked - VariantClear 1\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"%s (code %x) at 0x%p\r\n",
			  	         e.what(), e.code(),e.where());
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);

#endif
           }
		  }
		 }
		if (FunctionDescription->invkind == INVOKE_PROPERTYPUT)
		  {
           int HasPointers=0;
		   DISPPARAMS Parameters;
		   DISPID dispidPut=DISPID_PROPERTYPUT;
		   memset(&Parameters, 0, sizeof(Parameters));

		   Parameters.cArgs = FunctionDescription->cParams;
		   Parameters.cNamedArgs = 1;
		   Parameters.rgvarg = NULL;
		   Parameters.rgdispidNamedArgs=&dispidPut;
           
		   if (Parameters.cArgs > 0)
		     {
			  pArguments = new VARIANT[Parameters.cArgs];

			  Parameters.rgvarg = pArguments;
			  memset(pArguments, 0, sizeof(VARIANT) * Parameters.cArgs); 

			  for (unsigned int i = 0; i < Parameters.cArgs; i++)
				 {
				  VariantInit(&pArguments[i]);
				  pArguments[i].vt=FunctionDescription->lprgelemdescParam[i].tdesc.vt;
				  HasPointers|=SetArgument(&pArguments[i],LargeTestString);
				 }
			 }
           if(!HasPointers)
		   {

		   VARIANT Result;
		   VariantInit(&Result);
		   Result.vt=VT_BOOL;
		   Result.boolVal=VARIANT_FALSE;
		   UINT uArgErr;

		   sprintf(TempTextBuffer,"Invoking %s - %ws::%s\r\n",PropertyType,InterfaceName,MethodInformation);
		   WriteText(hLogFile,TempTextBuffer);

		   try
		    {
			 pIDispatch->Invoke(FunctionDescription->memid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &Parameters,&Result,NULL,&uArgErr);
			} 
           catch (const access_violation& e) 
            {
				 COM_Object_Exeception_Occurred=true;
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"***   Access Violation    ***\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);
                 sprintf(TempTextBuffer,"Invoked %s - %ws::%s\r\n",PropertyType,InterfaceName,MethodInformation);
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"%s at 0x%p :Bad %s on 0x%p\r\n",
			  	         e.what(), e.where(),e.isWrite()?"write":"read",e.badAddress());
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"*****************************\r\n");
 				 WriteText(hLogFile,TempTextBuffer);
           }
          catch (const win32_exception& e) 
	        {    
				 COM_Object_Exeception_Occurred=true;
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"***   Win32 Exception     ***\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);
                 sprintf(TempTextBuffer,"Invoked %s - %ws::%s\r\n",PropertyType,InterfaceName,MethodInformation);
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"%s (code %x) at 0x%p\r\n",
			  	         e.what(), e.code(),e.where());
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);

           }
		   if (pArguments) delete[] pArguments; 
		   pArguments = NULL;
		  }
		  }
		pTypeInfo->ReleaseFuncDesc(FunctionDescription);
	  }
	}
  if(InterfaceName)
   {
	::SysFreeString(InterfaceName);
   }
  pTypeInfo->ReleaseTypeAttr(pTypeAttr);
  pTypeInfo->Release();
  ::SysFreeString(LargeTestString);
  delete[] MethodInformation;
  return(SUCCESS);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static int TestMethods(IDispatch *pIDispatch, DWORD Level)                        //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int TestMethods(IDispatch *pIDispatch,DWORD Level)
{
 ITypeInfo *pTypeInfo;
 TYPEATTR  *pTypeAttr;
 BSTR       InterfaceName;
 HRESULT    hResult;

 if (pIDispatch->GetTypeInfo(0, 0, &pTypeInfo)!=S_OK) return (GET_TYPE_INFO_FAILED); 
 pTypeInfo->GetTypeAttr(&pTypeAttr);

 if (pTypeInfo->GetDocumentation(-1, &InterfaceName, 0, 0, 0)!=S_OK)
   {
     sprintf(TempTextBuffer,"Unknown default interface:\r\n");
     WriteText(hLogFile,TempTextBuffer);
   }

 for(UINT CurrrentFunction=0; CurrrentFunction < pTypeAttr->cFuncs; CurrrentFunction++) 
   {
	FUNCDESC* FunctionDescription;
	hResult=pTypeInfo->GetFuncDesc(CurrrentFunction, &FunctionDescription);
    if (hResult!=S_OK)
	{
      if (hResult == E_OUTOFMEMORY) sprintf(TempTextBuffer,"GetFuncDesc Failed (E_OUTOFMEMORY)\r\n"); 
      else if (hResult == E_INVALIDARG)  sprintf(TempTextBuffer,"GetFuncDesc Failed (E_INVALIDARG)\r\n"); 
      else sprintf(TempTextBuffer,"GetFuncDesc Failed (0x%x)\r\n",hResult); 
	  WriteText(hLogFile,TempTextBuffer);
	  continue;
	}

	if (!(FunctionDescription->wFuncFlags & FUNCFLAG_FRESTRICTED))
	  {
			if (FunctionDescription->invkind == INVOKE_FUNC)
				TestMethod(pIDispatch, pTypeInfo, FunctionDescription,InterfaceName,Level);
	  }
		pTypeInfo->ReleaseFuncDesc(FunctionDescription);
	}
  return(SUCCESS);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static int TestMethod(IDispatch *pIDispatch,ITypeInfo *pTypeInfo,                 //
///                      FUNCDESC* FunctionDescription,BSTR InterfaceName,DWORD Level)//
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////

static int TestMethod(IDispatch *pIDispatch,ITypeInfo *pTypeInfo, FUNCDESC* FunctionDescription,BSTR InterfaceName,DWORD Level)
{  
	BSTR        LargeTestString;
	VARIANT    *pArguments = NULL;
    char       *MethodInformation;
	BSTR        MethodName;
    string_m    tempstr;
	MethodInformation=new char[2048];
    MethodInformation[0]=NULL;
	LargeTestString= ::SysAllocStringLen(NULL,METHOD_AND_PROP_STRING_LENGTH-1);
    wmemset(LargeTestString,METHOD_AND_PROP_STRING_CHARACTER,METHOD_AND_PROP_STRING_LENGTH-1);


	
	if(pTypeInfo->GetDocumentation(FunctionDescription->memid, &MethodName, 0, 0, 0)!=S_OK)
	{
      delete[] MethodInformation;
      pTypeInfo->ReleaseFuncDesc(FunctionDescription); 
	  ::SysFreeString(LargeTestString);
	  sprintf(TempTextBuffer,"Error In Name\r\n"); 
	  WriteText(hLogFile,TempTextBuffer);
      return(GET_DOCUMENTATION_FAILED);
	}
    tempstr=TypeDescriptionToString(&FunctionDescription->elemdescFunc.tdesc,pTypeInfo);
	if (tempstr)
	{
 	 sprintf(MethodInformation,"%s %ws(",getstrptr_m(tempstr) ,MethodName);
	 strdelete_m(&tempstr);
	}
	if(MethodName) ::SysFreeString(MethodName);
    if (FunctionDescription->cParams>0)
	  {
       BSTR* rgBstrNames = new BSTR[FunctionDescription->cParams + 1];
	   UINT NumrgBstrNames=0;
	   pTypeInfo->GetNames(FunctionDescription->memid,rgBstrNames, FunctionDescription->cParams + 1, &NumrgBstrNames);
	   if (NumrgBstrNames>0) SysFreeString(rgBstrNames[0]);

	   for(INT CurrentParameter=0; CurrentParameter < FunctionDescription->cParams; CurrentParameter++)
	    {
         if(FunctionDescription->lprgelemdescParam[CurrentParameter].paramdesc.wParamFlags != 0)
            {
             bool NeedComma=false;
             strcat(MethodInformation,"[");
             if(FunctionDescription->lprgelemdescParam[CurrentParameter].paramdesc.wParamFlags & PARAMFLAG_FIN)
               {
                strcat(MethodInformation,"in");
			    NeedComma=true;
               }
             if(FunctionDescription->lprgelemdescParam[CurrentParameter].paramdesc.wParamFlags & PARAMFLAG_FOUT)
               {
			    if (NeedComma) strcat(MethodInformation,", ");
                strcat(MethodInformation,"out");
				 NeedComma=true;
               }
             if(FunctionDescription->lprgelemdescParam[CurrentParameter].paramdesc.wParamFlags & PARAMFLAG_FRETVAL)
               {
                if (NeedComma) strcat(MethodInformation,", ");
                strcat(MethodInformation,"retval");
			    NeedComma=true;
               }
             if(FunctionDescription->lprgelemdescParam[CurrentParameter].paramdesc.wParamFlags & PARAMFLAG_FOPT)
               {
                if (NeedComma) strcat(MethodInformation,", ");
                strcat(MethodInformation,"optional");
               }
             strcat(MethodInformation,"] ");
            }                        

         tempstr=TypeDescriptionToString(&FunctionDescription->lprgelemdescParam[CurrentParameter].tdesc, pTypeInfo);
	     if (tempstr)
	       {
	        strcat(MethodInformation, getstrptr_m(tempstr));
	        strdelete_m(&tempstr);
	       }
		 if ((CurrentParameter+1) < (INT) NumrgBstrNames)
	       {
            char *TempArgStr=new char[2048];
			sprintf(TempArgStr," %ws",rgBstrNames[CurrentParameter+1]);
			strcat(MethodInformation,TempArgStr);
            SysFreeString(rgBstrNames[CurrentParameter+1]);
			delete [] TempArgStr;
		   }
		 strcat(MethodInformation,GetArgumentStringEquivalent(FunctionDescription->lprgelemdescParam[CurrentParameter].tdesc.vt));
	     if(CurrentParameter < FunctionDescription->cParams - 1)strcat(MethodInformation,", ");
	    }
       delete[] rgBstrNames;
	  }
	strcat(MethodInformation,")");

	DISPPARAMS Parameters;
	memset(&Parameters, 0, sizeof(Parameters));

	Parameters.cArgs = FunctionDescription->cParams;
	Parameters.cNamedArgs = 0;
	Parameters.rgvarg = NULL;

	if (Parameters.cArgs > 0)
		{
			pArguments = new VARIANT[Parameters.cArgs];

			Parameters.rgvarg = pArguments;
			memset(pArguments, 0, sizeof(VARIANT) * Parameters.cArgs); 

			for (unsigned int i = 0; i < Parameters.cArgs; i++)
			{
			 VariantInit(&pArguments[i]);
			 pArguments[i].vt=FunctionDescription->lprgelemdescParam[i].tdesc.vt;
			 SetArgument(&pArguments[i],LargeTestString);
			}
		}

		VARIANT Result;
		VariantInit(&Result);
		Result.vt=VT_BOOL;
		Result.boolVal=VARIANT_FALSE;
		UINT uArgErr;

		sprintf(TempTextBuffer,"Invoking Method                 - %ws::%s\r\n",InterfaceName,MethodInformation);
		WriteText(hLogFile,TempTextBuffer);

		try
		{
			pIDispatch->Invoke(FunctionDescription->memid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &Parameters, &Result,NULL,&uArgErr);
		} 

        catch (const access_violation& e) 
            {
				 COM_Object_Exeception_Occurred=true;
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"***   Access Violation    ***\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);
                 sprintf(TempTextBuffer,"Invoked Method - %ws::%s\r\n",InterfaceName,MethodInformation);
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"%s at 0x%p :Bad %s on 0x%p\r\n",
			  	         e.what(), e.where(),e.isWrite()?"write":"read",e.badAddress());
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);

            }
          catch (const win32_exception& e) 
	        {
				 COM_Object_Exeception_Occurred=true;
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"***   Win32 Exception     ***\r\n");
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);
                 sprintf(TempTextBuffer,"Invoked Method - %ws::%s\r\n",InterfaceName,MethodInformation);
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"%s (code %x) at 0x%p\r\n",
			  	         e.what(), e.code(),e.where());
				 WriteText(hLogFile,TempTextBuffer);
				 sprintf(TempTextBuffer,"*****************************\r\n");
				 WriteText(hLogFile,TempTextBuffer);

           }

			if (Result.vt == VT_DISPATCH && !Result.pdispVal) 
			{
#if LOGINFO
		     sprintf(TempTextBuffer,"*****************************\r\n");
			 WriteText(hLogFile,TempTextBuffer);
			 sprintf(TempTextBuffer,"***      No Results       ***\r\n");
			 WriteText(hLogFile,TempTextBuffer);
			 sprintf(TempTextBuffer,"*****************************\r\n");       
			 WriteText(hLogFile,TempTextBuffer);
			 sprintf(TempTextBuffer,"Invoked Method - %ws::%s\r\n",InterfaceName,MethodInformation);
			 WriteText(hLogFile,TempTextBuffer);
			 sprintf(TempTextBuffer,"*****************************\r\n");
		     WriteText(hLogFile,TempTextBuffer);

#endif
			}		
		
		 if (Result.vt == VT_DISPATCH && Result.pdispVal)
		  {
            if (pIDispatch!=Result.pdispVal)			 
			{
               LARGE_INTEGER CurrentPos,Before,After,zero;
			   zero.QuadPart=0;
			   if (!SetFilePointerEx(hLogFile,zero,&CurrentPos,FILE_CURRENT))
			   {
		        sprintf(TempTextBuffer,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
			    WriteText(hLogFile,TempTextBuffer);
		        sprintf(TempTextBuffer,"Set File Pointer 1 Failed\r\n");
			    WriteText(hLogFile,TempTextBuffer);
		        sprintf(TempTextBuffer,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
			    WriteText(hLogFile,TempTextBuffer);
			   }
               sprintf(TempTextBuffer,"Recurse from interface %ws\r\n",InterfaceName);
			   WriteText(hLogFile,TempTextBuffer);
			   if (!SetFilePointerEx(hLogFile,zero,&Before,FILE_CURRENT))
			   {
		        sprintf(TempTextBuffer,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
			    WriteText(hLogFile,TempTextBuffer);
		        sprintf(TempTextBuffer,"Set File Pointer 2 Failed\r\n");
			    WriteText(hLogFile,TempTextBuffer);
		        sprintf(TempTextBuffer,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
			    WriteText(hLogFile,TempTextBuffer);
			   }
			   Test_Dispatch_Interface(Result.pdispVal,Level+1);
			   if (!SetFilePointerEx(hLogFile,zero,&After,FILE_CURRENT))
			   {
		        sprintf(TempTextBuffer,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
			    WriteText(hLogFile,TempTextBuffer);
		        sprintf(TempTextBuffer,"Set File Pointer 3 Failed\r\n");
			    WriteText(hLogFile,TempTextBuffer);
		        sprintf(TempTextBuffer,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
			    WriteText(hLogFile,TempTextBuffer);
			   }
			   if (Before.QuadPart==After.QuadPart)
			   {
 			    if (!SetFilePointerEx(hLogFile,CurrentPos,NULL,FILE_BEGIN))
			    {
		         sprintf(TempTextBuffer,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
			     WriteText(hLogFile,TempTextBuffer);
		         sprintf(TempTextBuffer,"Set File Pointer 4 Failed\r\n");
			     WriteText(hLogFile,TempTextBuffer);
		         sprintf(TempTextBuffer,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
			     WriteText(hLogFile,TempTextBuffer);
			    }
			   }
			   else
			   {
                sprintf(TempTextBuffer,"Recurse return %ws\r\n",InterfaceName);
			    WriteText(hLogFile,TempTextBuffer);
			   }
			}
		  }

		try 
		{
		 VariantClear(&Result);
		} 
           catch (const access_violation& e) 
            {
#if LOG_CRASH_ON_FREE
		     sprintf(TempTextBuffer,"*****************************\r\n");
			 WriteText(hLogFile,TempTextBuffer);
		     sprintf(TempTextBuffer,"***   Access Violation    ***\r\n");
			 WriteText(hLogFile,TempTextBuffer);
			 sprintf(TempTextBuffer,"*****************************\r\n");
			 WriteText(hLogFile,TempTextBuffer);
             sprintf(TempTextBuffer,"Invoked - VariantClear 2\r\n");
		     WriteText(hLogFile,TempTextBuffer);
			 sprintf(TempTextBuffer,"%s at 0x%p :Bad %s on 0x%p\r\n",
			  	         e.what(), e.where(),e.isWrite()?"write":"read",e.where());
			 WriteText(hLogFile,TempTextBuffer);
			 sprintf(TempTextBuffer,"*****************************\r\n");
		     WriteText(hLogFile,TempTextBuffer);
#endif
            }
          catch (const win32_exception& e) 
	        {
#if LOG_CRASH_ON_FREE
			 sprintf(TempTextBuffer,"*****************************\r\n");
			 WriteText(hLogFile,TempTextBuffer);
			 sprintf(TempTextBuffer,"***   Win32 Exception     ***\r\n");
			 WriteText(hLogFile,TempTextBuffer);
			 sprintf(TempTextBuffer,"*****************************\r\n");
			 WriteText(hLogFile,TempTextBuffer);
             sprintf(TempTextBuffer,"Invoked - VariantClear 2\r\n");
		     WriteText(hLogFile,TempTextBuffer);
			 sprintf(TempTextBuffer,"%s (code %x) at 0x%p\r\n",
			  	         e.what(), e.code(),e.where());
			 WriteText(hLogFile,TempTextBuffer);
			 sprintf(TempTextBuffer,"*****************************\r\n");
		     WriteText(hLogFile,TempTextBuffer);
#endif
           }

    delete[] pArguments;
	::SysFreeString(LargeTestString);
	delete[] MethodInformation;
	return(SUCCESS);
}

////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
///static DebugProcess(DWORD ProcessId)                                                // 
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static HANDLE DebugProcess(DWORD ProcessId)
{
 DWORD   id;
 
 KillDebugMonitor=FALSE;
 MonitorDetectedIECrash=FALSE;
#pragma warning( push )
#pragma warning( disable : 4312 ) 
 DebugProcessThreadHandle = CreateThread(NULL, 0, DebugProcessThread, (LPVOID)ProcessId, CREATE_SUSPENDED, &id);
#pragma warning( pop ) 
 if (DebugProcessThreadHandle==NULL)
	 { 
	  return(NULL);
	 }
 ResumeThread(DebugProcessThreadHandle);
 return(DebugProcessThreadHandle);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
///static DWORD WINAPI  DebugProcessThread(LPVOID arg)                                // 
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static DWORD WINAPI  DebugProcessThread(LPVOID arg)
{
#pragma warning( push )
#pragma warning( disable : 4311 ) 
  DWORD       ProcessId=(DWORD)arg;
#pragma warning( pop )
  DEBUG_EVENT DebugEvent;
  BOOL        First_Exception_Event=TRUE;
  HINSTANCE   hKernel32=NULL;
  BOOL  (WINAPI *pDebugSetProcessKillOnExit)(BOOL)=NULL;
  BOOL  (WINAPI *pDebugActiveProcessStop)(DWORD)=NULL;

  hKernel32 = LoadLibrary(_T("kernel32.dll"));
  if (hKernel32)
  {
    *(FARPROC *)&pDebugSetProcessKillOnExit = GetProcAddress (hKernel32,"DebugSetProcessKillOnExit");
	*(FARPROC *)&pDebugActiveProcessStop = GetProcAddress (hKernel32,"DebugActiveProcessStop");
    if ((pDebugSetProcessKillOnExit==NULL)&& (pDebugActiveProcessStop==NULL))
	{
	  FreeLibrary(hKernel32);
      hKernel32=NULL;
	}
  }

  if(!DebugActiveProcess(ProcessId)) 
  {
    if (hKernel32) FreeLibrary(hKernel32);
	return(FAILED_TO_ATTACH_DEBUGGER);
  }
  if (pDebugSetProcessKillOnExit) pDebugSetProcessKillOnExit(TRUE);
  while(1)
  {
	if(!WaitForDebugEvent(&DebugEvent, 100))
		{
		 if (KillDebugMonitor)
		 {
          if (pDebugActiveProcessStop) pDebugActiveProcessStop(ProcessId);
		  if (hKernel32) FreeLibrary(hKernel32);
		  return(SUCCESS);
		 }
		 else continue;
		}

		// Process the debugging event code.
		switch (DebugEvent.dwDebugEventCode) 
		{ 
			case EXCEPTION_DEBUG_EVENT: 
				
                  if (!DebugEvent.u.Exception.dwFirstChance)
				  {
                          sprintf(TempTextBuffer,"*****************************\r\n");
                          WriteText(hLogFile,TempTextBuffer);
                          sprintf(TempTextBuffer,"*** IE Exception          ***\r\n");
                          WriteText(hLogFile,TempTextBuffer);
                          sprintf(TempTextBuffer,"*****************************\r\n");
                          WriteText(hLogFile,TempTextBuffer);
                          if (DebugEvent.u.Exception.ExceptionRecord.NumberParameters == 2)
                             {
                              if (DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[0] == 1)
                                   {
                                       // write error
						        	   sprintf(TempTextBuffer,"%s(0x%08x): instruction address: 0x%p, invalid write to 0x%08x\r\n",
								   	   GetExceptionName(DebugEvent.u.Exception.ExceptionRecord.ExceptionCode),
					                   DebugEvent.u.Exception.ExceptionRecord.ExceptionCode,
									   DebugEvent.u.Exception.ExceptionRecord.ExceptionAddress,
                                       DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[1]);
							           WriteText(hLogFile,TempTextBuffer);
                                   }
                                else
						        {
                                      // read error
							           sprintf(TempTextBuffer,"%s(0x%08x): instruction address: 0x%p, invalid read from 0x%08x\r\n",
								   	   GetExceptionName(DebugEvent.u.Exception.ExceptionRecord.ExceptionCode),
					                   DebugEvent.u.Exception.ExceptionRecord.ExceptionCode,
									   DebugEvent.u.Exception.ExceptionRecord.ExceptionAddress,
                                       DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[1]);
							           WriteText(hLogFile,TempTextBuffer);
                                }
							 }
                          else
                           {
					            sprintf(TempTextBuffer,"%s(0x%08x): instruction address: 0x%p\r\n",
								GetExceptionName(DebugEvent.u.Exception.ExceptionRecord.ExceptionCode),
					            DebugEvent.u.Exception.ExceptionRecord.ExceptionCode,
							    DebugEvent.u.Exception.ExceptionRecord.ExceptionAddress);
							    WriteText(hLogFile,TempTextBuffer);
                           }
                          sprintf(TempTextBuffer,"*****************************\r\n");
                          WriteText(hLogFile,TempTextBuffer);
                          MonitorDetectedIECrash=TRUE;
				       
				 }
				 
                if ((DebugEvent.u.Exception.dwFirstChance) && 
					(DebugEvent.u.Exception.ExceptionRecord.ExceptionCode==EXCEPTION_BREAKPOINT))
					  ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, DBG_CONTINUE);

				else ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
				

				if (MonitorDetectedIECrash)
				{
                 if (pDebugActiveProcessStop) pDebugActiveProcessStop(ProcessId);
				 if (hKernel32) FreeLibrary(hKernel32);
				 return(INTERNET_EXPLORER_CRASHED);
				}
				break;
	        case CREATE_THREAD_DEBUG_EVENT: 
	        case CREATE_PROCESS_DEBUG_EVENT: 
	        case EXIT_THREAD_DEBUG_EVENT: 
	        case EXIT_PROCESS_DEBUG_EVENT: 
	        case LOAD_DLL_DEBUG_EVENT: 
	        case UNLOAD_DLL_DEBUG_EVENT: 
	 		default:
					ContinueDebugEvent(DebugEvent.dwProcessId, 
			                           DebugEvent.dwThreadId, 
			                           DBG_CONTINUE);
				    break;

		} 
  }

}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static char * GetExceptionName (DWORD ExceptionCode )                             // 
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////

static char * GetExceptionName(DWORD ExceptionCode )
{
	switch ( ExceptionCode )
	{
		case EXCEPTION_ACCESS_VIOLATION	:        return "EXCEPTION ACCESS VIOLATION";
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "EXCEPTION ARRAY BOUNDS EXCEEDED";
		case EXCEPTION_BREAKPOINT:               return "EXCEPTION BREAKPOINT";
		case EXCEPTION_DATATYPE_MISALIGNMENT:    return "EXCEPTION DATATYPE MISALIGNMENT";
		case EXCEPTION_FLT_DENORMAL_OPERAND:     return "EXCEPTION FLT DENORMAL OPERAND";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "EXCEPTION FLT DIVIDE BY ZERO";
		case EXCEPTION_FLT_INEXACT_RESULT:       return "EXCEPTION FLT INEXACT RESULT";
		case EXCEPTION_FLT_INVALID_OPERATION:    return "EXCEPTION FLT INVALID OPERATION";
		case EXCEPTION_FLT_OVERFLOW:             return "EXCEPTION FLT OVERFLOW";
		case EXCEPTION_FLT_STACK_CHECK:          return "EXCEPTION FLT STACK_CHECK";
		case EXCEPTION_FLT_UNDERFLOW:            return "EXCEPTION FLT UNDERFLOW";
		case EXCEPTION_ILLEGAL_INSTRUCTION:      return "EXCEPTION ILLEGAL INSTRUCTION";
		case EXCEPTION_IN_PAGE_ERROR:            return "EXCEPTION IN PAGE ERROR";
		case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "EXCEPTION INT DIVIDE BY ZERO";
		case EXCEPTION_INT_OVERFLOW:             return "EXCEPTION INT OVERFLOW";
		case EXCEPTION_INVALID_DISPOSITION:      return "EXCEPTION INVALID DISPOSITION";
		case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION NONCONTINUABLE EXCEPTION";
		case EXCEPTION_PRIV_INSTRUCTION:         return "EXCEPTION PRIV INSTRUCTION";
		case EXCEPTION_SINGLE_STEP:              return "EXCEPTION SINGLE STEP";
		case EXCEPTION_STACK_OVERFLOW:           return "EXCEPTION STACK OVERFLOW";
		default:                                 return "EXCEPTION UNKNOWN"; 
	}

}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static void StopDebugger(HANDLE DebugThreadHandle)                                // 
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static void StopDebugger(HANDLE DebugThreadHandle)
{
 KillDebugMonitor=TRUE;
 WaitForSingleObject(DebugThreadHandle,1000);
 CloseHandle(DebugThreadHandle);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// BSTR ToBSTR( T a_Str )                                                            // 
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////

template< class T >
BSTR ToBSTR( T a_Str )
{
     return( _bstr_t(a_Str).copy() ) ;
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static int IELoadUrl(LPCTSTR Url)                                                 // 
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int IELoadUrl(char * Url) 
{
  IWebBrowser2   *IeBrowserInterface=NULL; //Create a IWebBrowser variable 
  HRESULT         hResult;
  VARIANT         var;
  BSTR            bstrURL = NULL;
  DWORD           ProcId;
  HANDLE          DebugThreadHandle;

  bstrURL = ToBSTR(Url);

  if (KillAllProcessesByName("iexplore.exe")) printf("Kill all iexplore.exe failed\n"); 

  try
  {
   hResult= CoCreateInstance(CLSID_InternetExplorer,NULL,CLSCTX_SERVER,IID_IWebBrowser2, 
                                  ( void**) &IeBrowserInterface); 
   if ((hResult!=S_OK)||(IeBrowserInterface==NULL)) return(FAILED_TO_START_BROWSER);
   ZeroMemory  (&var,sizeof(VARIANT)); 
   IeBrowserInterface->put_Visible(VARIANT_TRUE);
   if ((GetProcessIdByName("iexplore.exe",&ProcId))!=0)
   {
	IeBrowserInterface->Quit();
    IeBrowserInterface->Release();
    return(FAILED_TO_GET_PROCESS_ID); 
   }
  }
 catch(...)
  {
   if (bstrURL) SysFreeString(bstrURL);
   return(FAILED_TO_START_BROWSER);
  }
  if ((DebugThreadHandle=DebugProcess(ProcId))==NULL)
  {
	IeBrowserInterface->Quit();
    IeBrowserInterface->Release();
    return(FAILED_TO_ATTACH_DEBUGGER);
  }
   try
  {
  IeBrowserInterface->Navigate(bstrURL,&var,&var,&var,&var);
  Sleep(1500);
  IeBrowserInterface->Navigate(bstrURL,&var,&var,&var,&var);
  Sleep(800);
  }
  catch (const access_violation& e) 
  {
    COM_Object_Exeception_Occurred=true;
    sprintf(TempTextBuffer,"*****************************\r\n");
    WriteText(hLogFile,TempTextBuffer);
    sprintf(TempTextBuffer,"*** IE Access Violation   ***\r\n");
    WriteText(hLogFile,TempTextBuffer);
    sprintf(TempTextBuffer,"*****************************\r\n");
    WriteText(hLogFile,TempTextBuffer);
    sprintf(TempTextBuffer,"%s at 0x%p :Bad %s on 0x%p\r\n",
			  	         e.what(), e.where(),e.isWrite()?"write":"read",e.badAddress());
    WriteText(hLogFile,TempTextBuffer);
    sprintf(TempTextBuffer,"*****************************\r\n");
    WriteText(hLogFile,TempTextBuffer);
	if (bstrURL) SysFreeString(bstrURL);
	IeBrowserInterface->Quit();
	IeBrowserInterface->Release();
	StopDebugger(DebugThreadHandle);
	return(INTERNET_EXPLORER_CRASHED);
   }
  catch (const win32_exception& e) 
   {
    COM_Object_Exeception_Occurred=true;
    sprintf(TempTextBuffer,"*****************************\r\n");
    WriteText(hLogFile,TempTextBuffer);
    sprintf(TempTextBuffer,"*** IE Win32 Exception    ***\r\n");
    WriteText(hLogFile,TempTextBuffer);
    sprintf(TempTextBuffer,"*****************************\r\n");
    WriteText(hLogFile,TempTextBuffer);
    sprintf(TempTextBuffer,"%s (code %x) at 0x%p\r\n",
			  	         e.what(), e.code(),e.where());
    WriteText(hLogFile,TempTextBuffer);
    sprintf(TempTextBuffer,"*****************************\r\n");
    WriteText(hLogFile,TempTextBuffer);
	if (bstrURL) SysFreeString(bstrURL);
	IeBrowserInterface->Quit();
	IeBrowserInterface->Release();
	StopDebugger(DebugThreadHandle);
	return(INTERNET_EXPLORER_CRASHED);
   }
   
	if (bstrURL) SysFreeString(bstrURL);
	IeBrowserInterface->Quit();
    IeBrowserInterface->Release();
	StopDebugger(DebugThreadHandle);
	if (MonitorDetectedIECrash) return(INTERNET_EXPLORER_CRASHED);
    else return(SUCCESS); 
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static int IELoadUrlPARAMS(LPCTSTR Url,DWORD Timeout)                             // 
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int IELoadUrlPARAMS(char * Url,DWORD Timeout) 
{
  IWebBrowser2   *IeBrowserInterface=NULL; //Create a IWebBrowser variable 
  HRESULT         hResult;
  VARIANT         var;
  BSTR            bstrURL = NULL;
  DWORD           ProcId;
  HANDLE          DebugThreadHandle;

  bstrURL = ToBSTR(Url);

  if (KillAllProcessesByName("iexplore.exe")) printf("Kill all iexplore.exe failed\n"); 

  try
  {
   hResult= CoCreateInstance(CLSID_InternetExplorer,NULL,CLSCTX_SERVER,IID_IWebBrowser2, 
                                  ( void**) &IeBrowserInterface); 
   if ((hResult!=S_OK)||(IeBrowserInterface==NULL)) return(FAILED_TO_START_BROWSER);
   ZeroMemory  (&var,sizeof(VARIANT)); 
   IeBrowserInterface->put_Visible(VARIANT_TRUE);
   if ((GetProcessIdByName("iexplore.exe",&ProcId))!=0)
   {
	IeBrowserInterface->Quit();
    IeBrowserInterface->Release();
    return(FAILED_TO_GET_PROCESS_ID); 
   }
  }
 catch(...)
  {
   if (bstrURL) SysFreeString(bstrURL);
   return(FAILED_TO_START_BROWSER);
  }
  if ((DebugThreadHandle=DebugProcess(ProcId))==NULL)
  {
	IeBrowserInterface->Quit();
    IeBrowserInterface->Release();
    return(FAILED_TO_ATTACH_DEBUGGER);
  }
   try
  {
  IeBrowserInterface->Navigate(bstrURL,&var,&var,&var,&var);
  Sleep(Timeout);
  ActivateControlInIE();
  Sleep(1000);
  IeBrowserInterface->Navigate(bstrURL,&var,&var,&var,&var);
  Sleep(1000);
  }
  catch (const access_violation& e) 
  {
    COM_Object_Exeception_Occurred=true;
    sprintf(TempTextBuffer,"*****************************\r\n");
    WriteText(hLogFile,TempTextBuffer);
    sprintf(TempTextBuffer,"*** IE Access Violation   ***\r\n");
    WriteText(hLogFile,TempTextBuffer);
    sprintf(TempTextBuffer,"*****************************\r\n");
    WriteText(hLogFile,TempTextBuffer);
    sprintf(TempTextBuffer,"%s at 0x%p :Bad %s on 0x%p\r\n",
			  	         e.what(), e.where(),e.isWrite()?"write":"read",e.badAddress());
    WriteText(hLogFile,TempTextBuffer);
    sprintf(TempTextBuffer,"*****************************\r\n");
    WriteText(hLogFile,TempTextBuffer);
	if (bstrURL) SysFreeString(bstrURL);
	IeBrowserInterface->Quit();
	IeBrowserInterface->Release();
	StopDebugger(DebugThreadHandle);
	return(INTERNET_EXPLORER_CRASHED);
   }
  catch (const win32_exception& e) 
   {
    COM_Object_Exeception_Occurred=true;
    sprintf(TempTextBuffer,"*****************************\r\n");
    WriteText(hLogFile,TempTextBuffer);
    sprintf(TempTextBuffer,"*** IE Win32 Exception    ***\r\n");
    WriteText(hLogFile,TempTextBuffer);
    sprintf(TempTextBuffer,"*****************************\r\n");
    WriteText(hLogFile,TempTextBuffer);
    sprintf(TempTextBuffer,"%s (code %x) at 0x%p\r\n",
			  	         e.what(), e.code(),e.where());
    WriteText(hLogFile,TempTextBuffer);
    sprintf(TempTextBuffer,"*****************************\r\n");
    WriteText(hLogFile,TempTextBuffer);
	if (bstrURL) SysFreeString(bstrURL);
	IeBrowserInterface->Quit();
	IeBrowserInterface->Release();
	StopDebugger(DebugThreadHandle);
	return(INTERNET_EXPLORER_CRASHED);
   }
   
	if (bstrURL) SysFreeString(bstrURL);
	IeBrowserInterface->Quit();
    IeBrowserInterface->Release();
	StopDebugger(DebugThreadHandle);
	if (MonitorDetectedIECrash) return(INTERNET_EXPLORER_CRASHED);
    else return(SUCCESS); 
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static int Create_HTML_Test_File(char *FileName,char *CLSID)                      //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int Create_HTML_Test_File(char *FileName,char *CLSID)
{
  char   ADS_FileName[MAX_PATH];
  HANDLE hFile, hStream;
  char   TempBuff[1024];


  if (FileName==NULL) return(-1);

  strcpy(ADS_FileName,FileName);
  strcat(ADS_FileName,":Zone.Identifier");

  hFile = CreateFile( FileName,
                      GENERIC_WRITE,
                     FILE_SHARE_WRITE,
                                NULL,
                         OPEN_ALWAYS,
                                   0,
                                NULL );
   if( hFile == INVALID_HANDLE_VALUE ) return(-1);

   hStream = CreateFile( ADS_FileName,
                                GENERIC_WRITE,
                             FILE_SHARE_WRITE,
                                         NULL,
                                  OPEN_ALWAYS,
                                            0,
                                         NULL );

  if( hStream == INVALID_HANDLE_VALUE ) 
  {
    CloseHandle(hFile);
	DeleteFile(FileName);
	return(-1);
  }
 if (WriteText(hFile,"<!--\r\n"))
  {
   CloseHandle(hFile);
   CloseHandle(hStream);
   return(-1);
  }
 sprintf(TempBuff,"COM Object - %s %s\r\n",CLSID_String,CLSID_Description);
 if (WriteText(hFile,TempBuff))
  {
   CloseHandle(hFile);
   CloseHandle(hStream);
   return(-1);
  }

 if (HaveCOM_Filename) PrintFileInfo(hFile,COM_FileName);

if (WriteText(hFile,"-->\r\n"))
  {
   CloseHandle(hFile);
   CloseHandle(hStream);
   return(-1);
  }
  sprintf(TempBuff,"<object id=TestObj classid=\"CLSID:%s\" style=\"width:100%;height:350\"></object>",CLSID);
  if (WriteText(hFile,TempBuff))
  {
   CloseHandle(hFile);
   CloseHandle(hStream);
   DeleteFile(FileName);
   return(-1);
  }
  
  sprintf(TempBuff,"[ZoneTransfer]\nZoneId=%d\n",ZoneID);
  if (WriteText(hStream,TempBuff))
  {
   CloseHandle(hFile);
   CloseHandle(hStream);
   DeleteFile(FileName);
   return(-1);
  }
  CloseHandle(hFile);
  CloseHandle(hStream);
  return(0);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static int Generate_HTML_PARAMS_Test_File(char *FileName,char *CLSID)               //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int Generate_HTML_PARAMS_Test_File(char *FileName,char *CLSID)
{
  char   ADS_FileName[MAX_PATH];
  HANDLE hFile, hStream;

  char   TempBuff[1025];
  char   *BIG_BIG_String=NULL;
  TStringList *pNode;

  if (FileName==NULL) return(-1);

  if (HaveCOM_Filename==false) return(-1);

   if (OpenFileName(COM_FileName)==0) 
	    {
		  DWORD i;
		  for (i=0;i<NumberOfSectionsToCheck;i++)
		  {
			  StringsASCII(ControlLoadedInMemory+SectionsToCheck[i].Offset,SectionsToCheck[i].Length);
			  StringsUnicode(ControlLoadedInMemory+SectionsToCheck[i].Offset,SectionsToCheck[i].Length);

		  }
		}
   else return(-1);

  strcpy(ADS_FileName,FileName);
  strcat(ADS_FileName,":Zone.Identifier");

  hFile = CreateFile( FileName,
                      GENERIC_WRITE,
                     FILE_SHARE_WRITE,
                                NULL,
                         OPEN_ALWAYS,
                                   0,
                                NULL );
   if( hFile == INVALID_HANDLE_VALUE ) return(-1);

   hStream = CreateFile( ADS_FileName,
                                GENERIC_WRITE,
                             FILE_SHARE_WRITE,
                                         NULL,
                                  OPEN_ALWAYS,
                                            0,
                                         NULL );

  if( hStream == INVALID_HANDLE_VALUE ) 
  {
    CloseHandle(hFile);
	DeleteFile(FileName);
	LocalFree(ControlLoadedInMemory);
	DeleteStringList();
	return(-1);
  }

 if (WriteText(hFile,"<!--\r\n"))
  {
   CloseHandle(hFile);
   CloseHandle(hStream);
   return(NULL);
  }
 sprintf(TempBuff,"COM Object - %s %s\r\n",CLSID_String,CLSID_Description);
 if (WriteText(hFile,TempBuff))
  {
   CloseHandle(hFile);
   CloseHandle(hStream);
   return(NULL);
  }

 if (HaveCOM_Filename) PrintFileInfo(hFile,COM_FileName);

if (WriteText(hFile,"-->\r\n"))
  {
   CloseHandle(hFile);
   CloseHandle(hStream);
   return(NULL);
  }

  sprintf(TempBuff,"<object id=TestObj classid=\"CLSID:%s\" style=\"width:100%;height:350\">\r\n",CLSID);
  if (WriteText(hFile,TempBuff))
  {
   CloseHandle(hFile);
   CloseHandle(hStream);
   DeleteFile(FileName);
   LocalFree(ControlLoadedInMemory);
   DeleteStringList();
   return(-1);
  }
  if ((BIG_BIG_String=(char *)malloc(PARAM_STRING_LENGTH+1))==NULL)
  {
   CloseHandle(hFile);
   CloseHandle(hStream);
   DeleteFile(FileName);
   LocalFree(ControlLoadedInMemory);
   DeleteStringList();
   return(-1);
   }
  memset(BIG_BIG_String,PARAM_STRING_CHARACTER,PARAM_STRING_LENGTH);
  BIG_BIG_String[PARAM_STRING_LENGTH]=0;

  for (pNode = StringListHead; pNode != NULL; pNode = pNode->pNext)
   {
    sprintf(TempBuff,"<param NAME=\"%s\" VALUE=\"",pNode->String);
   
	if (WriteText(hFile,TempBuff))
     {
      CloseHandle(hFile);
      CloseHandle(hStream);
      DeleteFile(FileName);
      LocalFree(ControlLoadedInMemory);
      DeleteStringList();
	  free(BIG_BIG_String);
      return(-1);
     }

	 if (WriteText(hFile,BIG_BIG_String))
      {
      CloseHandle(hFile);
      CloseHandle(hStream);
      DeleteFile(FileName);
      LocalFree(ControlLoadedInMemory);
      DeleteStringList();
	  free(BIG_BIG_String);
      return(-1);
     }
	if (WriteText(hFile,"\">\r\n"))
     {
      CloseHandle(hFile);
      CloseHandle(hStream);
      DeleteFile(FileName);
      LocalFree(ControlLoadedInMemory);
      DeleteStringList();
	  free(BIG_BIG_String);
      return(-1);
     }
   }
   if (WriteText(hFile,"</object>\r\n"))
     {
      CloseHandle(hFile);
      CloseHandle(hStream);
      DeleteFile(FileName);
      LocalFree(ControlLoadedInMemory);
      DeleteStringList();
	  free(BIG_BIG_String);
      return(-1);
     }
   
  free(BIG_BIG_String);
  sprintf(TempBuff,"[ZoneTransfer]\nZoneId=%d\n",ZoneID);
  if (WriteText(hStream,TempBuff))
  {
   CloseHandle(hFile);
   CloseHandle(hStream);
   DeleteFile(FileName);
   LocalFree(ControlLoadedInMemory);
   DeleteStringList();
   return(-1);
  }
  CloseHandle(hFile);
  CloseHandle(hStream);
  LocalFree(ControlLoadedInMemory);
  DeleteStringList();
  return(0);
}

////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static HANDLE Create_HTML_PARAMS_Test_File(char *FileName,char *CLSID)               //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static HANDLE Create_HTML_PARAMS_Test_File(char *FileName,char *CLSID)
{
  char   ADS_FileName[MAX_PATH];
  HANDLE hFile, hStream;
  char   TempBuff[1025];

  if (FileName==NULL) return(NULL);


  strcpy(ADS_FileName,FileName);
  strcat(ADS_FileName,":Zone.Identifier");

  hFile = CreateFile( FileName,
                      GENERIC_WRITE,
                     FILE_SHARE_WRITE,
                                NULL,
                         OPEN_ALWAYS,
                                   0,
                                NULL );
   if( hFile == INVALID_HANDLE_VALUE ) return(NULL);

   hStream = CreateFile( ADS_FileName,
                                GENERIC_WRITE,
                             FILE_SHARE_WRITE,
                                         NULL,
                                  OPEN_ALWAYS,
                                            0,
                                         NULL );

  if( hStream == INVALID_HANDLE_VALUE ) 
  {
    CloseHandle(hFile);
	DeleteFile(FileName);
	return(NULL);
  }
  sprintf(TempBuff,"[ZoneTransfer]\nZoneId=%d\n",ZoneID);
  if (WriteText(hStream,TempBuff))
  {
   CloseHandle(hFile);
   CloseHandle(hStream);
   DeleteFile(FileName);
   return(NULL);
  }

 if (WriteText(hFile,"<!--\r\n"))
  {
   CloseHandle(hFile);
   CloseHandle(hStream);
   return(NULL);
  }
 sprintf(TempBuff,"COM Object - %s %s\r\n",CLSID_String,CLSID_Description);
 if (WriteText(hFile,TempBuff))
  {
   CloseHandle(hFile);
   CloseHandle(hStream);
   return(NULL);
  }

 if (HaveCOM_Filename) PrintFileInfo(hFile,COM_FileName);

if (WriteText(hFile,"-->\r\n"))
  {
   CloseHandle(hFile);
   CloseHandle(hStream);
   return(NULL);
  }

  sprintf(TempBuff,"<object id=TestObj classid=\"CLSID:%s\" style=\"width:100%;height:350\">\r\n",CLSID);
  if (WriteText(hFile,TempBuff))
  {
   CloseHandle(hFile);
   CloseHandle(hStream);
   return(NULL);
  }
 CloseHandle(hStream);
 return(hFile);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static void DeleteTempFile(char *FileName)                                        //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static void DeleteTempFile(char *FileName)
{
 DWORD RetryCount=0;
 DWORD LastError;
 while(1)
 {
   if (DeleteFile(FileName)==0)
   {
    LastError=GetLastError();
    if (LastError==ERROR_SHARING_VIOLATION)
	{
     if (RetryCount<10)
	  {
       RetryCount++;
       Sleep(2000);
	   continue;
	  }
	 else break;
	}
	else break;
   }
   else break;
 }
}

//---------------------------------------------------------------------------
int OpenFileName(char* FileName)
{
	HANDLE	hFile;
	IMAGE_DOS_HEADER		image_dos_header;
    IMAGE_NT_HEADERS		image_nt_headers;
    IMAGE_SECTION_HEADER	image_section_header;
	DWORD   FileSize;
    DWORD   dwBytesRead;
	DWORD   dwRO_first_section;
	DWORD   i;
    char *  FullFileName;
    MemoryNumBytes=0;

	FullFileName=FindFile(FileName);
	if (FullFileName==NULL) return(-1);
	hFile=CreateFile(FullFileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
	                 NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile==INVALID_HANDLE_VALUE) return(-1);
	FileSize=GetFileSize(hFile,0);
	if(FileSize==0)
	{
	 CloseHandle(hFile);
	 return(-1);
	}
	ControlLoadedInMemory = (BYTE *)LocalAlloc(LPTR,FileSize);
	if(ControlLoadedInMemory == NULL)
	{
		CloseHandle(hFile);
		return(-1);

	}
	if (ReadFile(hFile,ControlLoadedInMemory,FileSize,&dwBytesRead,NULL)==0)
	{
	 LocalFree(ControlLoadedInMemory);
	 ControlLoadedInMemory=NULL;
	 CloseHandle(hFile);
	 return(-1);
	}
	else CloseHandle(hFile);
   if (dwBytesRead!=FileSize)
   {

	 LocalFree(ControlLoadedInMemory);
	 ControlLoadedInMemory=NULL;
	 return(-1);
   }
    MemoryNumBytes=dwBytesRead;
	
	CopyMemory(&image_dos_header,ControlLoadedInMemory,sizeof(IMAGE_DOS_HEADER));
	CopyMemory(&image_nt_headers,
		       ControlLoadedInMemory+image_dos_header.e_lfanew,
			   sizeof(IMAGE_NT_HEADERS));
	dwRO_first_section=image_dos_header.e_lfanew+sizeof(IMAGE_NT_HEADERS);

	if(image_dos_header.e_magic!=IMAGE_DOS_SIGNATURE)// MZ
	{
	 LocalFree(ControlLoadedInMemory);
	 ControlLoadedInMemory=NULL;
	 MemoryNumBytes=0;
	 return(-1);
	}
	if(image_nt_headers.Signature!=IMAGE_NT_SIGNATURE)// PE00
	{
	 LocalFree(ControlLoadedInMemory);
	 ControlLoadedInMemory=NULL;
	 MemoryNumBytes=0;
	 return(-1);
	}
	DWORD SectionNum=image_nt_headers.FileHeader.NumberOfSections;
	for( i=0;i<SectionNum;i++) 
	{
		CopyMemory(&image_section_header,ControlLoadedInMemory+dwRO_first_section+i*sizeof(IMAGE_SECTION_HEADER),
			sizeof(IMAGE_SECTION_HEADER));
        if ((strcmp((const char *)image_section_header.Name,".data")==0)||
            (strcmp((const char *)image_section_header.Name,".rdata")==0)||
			(strcmp((const char *)image_section_header.Name,".text")==0))

		{
		 SectionsToCheck[NumberOfSectionsToCheck].Offset=image_section_header.PointerToRawData;
	     SectionsToCheck[NumberOfSectionsToCheck].Length=image_section_header.SizeOfRawData;
         NumberOfSectionsToCheck++;
		}
	}
 return(0);
}

//---------------------------------------------------------------------------
static void StringsASCII(BYTE	 *Memory,DWORD Size) 
{
  static char String[128];
  int StringLength;

  StringLength = 0;
  if (Size > 0) 
  {
    size_t i;
    unsigned char c;
	for (i = 0; i < Size; i++) 
	{
      c = Memory[i];
        
	  if ((StringLength==0) && (islower(c))) continue;
      if (((!isalpha(c))&& (c!='_')) ||
		  (StringLength >= (sizeof(String)-1))) 
	  {
       if (StringLength >= MIN_STRING_LENGTH)
	   {
	    String[StringLength++] = 0;
	    if (FindString(String)==NULL) InsertString(String);
	   }
	    StringLength = 0;
	  }
 	  else String[StringLength++] = c;
	}
  
  if (StringLength >=MIN_STRING_LENGTH) 
  {
    String[StringLength++] = 0;
    if (FindString(String)==NULL) InsertString(String);
    StringLength = 0;
  }
  }
 
}
//---------------------------------------------------------------------------
static void StringsUnicode(BYTE	 *Memory,DWORD Size) {
  static char String[128];
  int StringLength;

  StringLength = 0;
  if (Size > 0) 
  {
    size_t i;
    unsigned char c1,c2;
	for (i = 0; i < Size; i++) 
	{
      c1 = Memory[i];
	  if ((i+1)==Size)break;
	  c2=Memory[i+1];
	  if (c2!=0) continue;
	  else i++;
      if ((StringLength==0) && (islower(c1))) continue;  
      if (((!isalpha(c1))&& (c1!='_')) ||
		  (StringLength >= (sizeof(String)-1))) 
	  {
       if (StringLength >= MIN_STRING_LENGTH)
	   {
	    String[StringLength++] = 0;
		if (FindString(String)==NULL) InsertString(String);
	   }
	   StringLength = 0;
	  }
 	  else String[StringLength++] = c1;
	}
  
  if (StringLength >= MIN_STRING_LENGTH) 
  {
    String[StringLength++] = 0;
    if (FindString(String)==NULL) InsertString(String);
    StringLength = 0;
  }
  }
 
}

//---------------------------------------------------------------------------
static TStringList * InsertString(char *String)
{
 TStringList *pNode= new TStringList;
 if (pNode==NULL) return(NULL);
 if ((pNode->String=_strdup(String))==NULL)
 {
  delete pNode;
  return(NULL);
 }
  if (StringListHead == NULL)
   {
    StringListHead = pNode;
    pNode->pPrev = NULL;
   }
 else
   {
    StringListTail->pNext = pNode;
    pNode->pPrev = StringListTail;
   }
 StringListTail = pNode;
 pNode->pNext = NULL;
 return(pNode);
}
//---------------------------------------------------------------------------
static TStringList * FindString(char *String)
{
  TStringList *pNode;

  if (String==NULL) return(NULL);

  for (pNode = StringListHead; pNode != NULL; pNode = pNode->pNext)
   {
	   if (strcmp(pNode->String,String)==0) return(pNode);
   }
  return(NULL);
}
//---------------------------------------------------------------------------
static void RemoveString(TStringList *pNode)
{
   if (pNode->pPrev == NULL)
      StringListHead = pNode->pNext;
   else
      pNode->pPrev->pNext = pNode->pNext;
   if (pNode->pNext == NULL)
      StringListTail = pNode->pPrev;
   else
      pNode->pNext->pPrev = pNode->pPrev;
   free(pNode->String);
   delete pNode;
}
//---------------------------------------------------------------------------
static void  DeleteStringList(void)
{
   while (StringListHead != NULL)
      RemoveString(StringListHead);
}
//---------------------------------------------------------------------------
static void PrintStringList(void)
{
  TStringList *pNode;
  char TempTextBuffer[1024];
  for (pNode = StringListHead; pNode != NULL; pNode = pNode->pNext)
   {
	   sprintf(TempTextBuffer,"%s\r\n",pNode->String);
	   WriteText(hLogFile,TempTextBuffer);
	   
   }
}
//---------------------------------------------------------------------------
static char * FindFile(char *FileName)
{
	size_t   length;
    char     *buffer;
	static    TCHAR FullFileName[MAX_PATH];
    TCHAR trim[ ] = TEXT(" ");
	if (FileExists(FileName))
	{
	 strcpy(FullFileName,FileName);
	 return(FullFileName);
	}
    length = GetEnvironmentVariable( "PATH", 0, 0 );
    if (length<=0) return(NULL);
	buffer = new char[length];
    GetEnvironmentVariable( "PATH", buffer, (DWORD)length );
    char *directory = strtok(buffer, ";");
    while( directory != NULL )
    {
        strcpy(FullFileName,directory);
		StrTrim(FullFileName, trim);
		length=strlen(FullFileName);
		if (length<=0)
		{
         delete[] buffer;
		 return(NULL);
		}
		if (FullFileName[length-1]!='\\') strcat(FullFileName,"\\");
        strcat(FullFileName,FileName);
		ExpandEnvironmentVars(FullFileName,MAX_PATH); 
		DeQuoteString(FileName);
        if( FileExists( FullFileName ) )
        {
          delete[] buffer;
		  return(FullFileName);
        }
        directory = strtok( NULL, ";" );
    }
    delete[] buffer;

    return NULL;

} 

////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// STDMETHODIMP MyIPropertyBag::Read(LPCOLESTR , VARIANT *, IErrorLog *)             //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP MyIPropertyBag::Read(LPCOLESTR pwszPropName, VARIANT *pVar, IErrorLog *pErrorLog)
{
 HaveParamsToTest|=GenerateVariantText(*pVar,pwszPropName,TestFileHandle);
 return(E_FAIL);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
///STDMETHODIMP MyIPropertyBag::Write(LPCOLESTR , VARIANT *)                          //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP MyIPropertyBag::Write(LPCOLESTR pwszPropName, VARIANT *pVar)
{
	HaveParamsToTest|=GenerateVariantText(*pVar,pwszPropName,TestFileHandle);
	return(S_OK);
}

////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// int TestParamsPropertyBag(CLSID *clsid)                                           //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int TestParamsPropertyBag(CLSID *clsid)
{
 HRESULT        hResult=E_FAIL;
 IObjectSafety *pIObjectSafety = NULL;
 
 HaveParamsToTest=FALSE;
 strcat(lpPathBuffer,PARAMS_HTML);
 DeleteTempFile(lpPathBuffer);

 LargePropertyBagTestString=(char *) malloc(PROPERTY_BAG_STRING_LENGTH+1);
 memset(LargePropertyBagTestString,PROPERTY_BAG_STRING_CHARACTER,PROPERTY_BAG_STRING_LENGTH);
 LargePropertyBagTestString[PROPERTY_BAG_STRING_LENGTH]=0;


 try
  {

    hResult=CoCreateInstance(*clsid, NULL, CLSCTX_LOCAL_SERVER | CLSCTX_INPROC_SERVER,
                            IID_IObjectSafety, (void**)&pIObjectSafety);
  }
 catch(...)
  {
   pIObjectSafety=NULL;
  }
 if ((hResult==S_OK) && (pIObjectSafety!=NULL))
    {

     bool IPersist_SafeForUntrustedData=false;

      try
	   {
	   
	   	hResult = pIObjectSafety->SetInterfaceSafetyOptions(IID_IPersistPropertyBag,
		          INTERFACESAFE_FOR_UNTRUSTED_DATA, INTERFACESAFE_FOR_UNTRUSTED_DATA);

		if (hResult==S_OK) {
			IPersist_SafeForUntrustedData=true;
		}

	   }
      catch(...)
       {
        pIObjectSafety->Release();
		if (LargePropertyBagTestString) 
		{
 		 free(LargePropertyBagTestString);
		 LargePropertyBagTestString=NULL;
		}
        return(COM_OBJECTSAFETY_SET_INTERFACE_OPT_FAULT);
       }
       if (IPersist_SafeForUntrustedData)
	   {
         IPersistPropertyBag *pIPersistPropertyBag=NULL;
		 hResult = pIObjectSafety->QueryInterface(IID_IPersistPropertyBag, (void**)&pIPersistPropertyBag);
		 if (hResult== S_OK)
		  {
		     int RetVal=SUCCESS;
		     if ((TestFileHandle=Create_HTML_PARAMS_Test_File(lpPathBuffer,CLSID_String))!=NULL)
			 {
			   MyIPropertyBag iIPropertyBag;
		       pIPersistPropertyBag->InitNew();
               try
			   {
			     pIPersistPropertyBag->Load(&iIPropertyBag,NULL);
				 pIPersistPropertyBag->Save(&iIPropertyBag,TRUE,TRUE);
			   }
			   catch (...) {}
			   pIPersistPropertyBag->Release();
			   pIObjectSafety->Release();
			   WriteText(TestFileHandle,"</object>\r\n");
			   CloseHandle(TestFileHandle);
			   if (HaveParamsToTest)
			   {
			    DWORD LoadTime=ComputeInitialLoadTime(lpPathBuffer);
				if (LoadTime)
			        RetVal=IELoadUrlPARAMS(lpPathBuffer,LoadTime); 
 			   }
 			 }
			 else
			 {
			   pIPersistPropertyBag->Release();
			   pIObjectSafety->Release();
			 }
			 if (LargePropertyBagTestString) 
		        {
		         free(LargePropertyBagTestString);
		         LargePropertyBagTestString=NULL;
		        }

		     return(RetVal);
		  }
		  else
		  {
           pIObjectSafety->Release();
		   if (LargePropertyBagTestString) 
		    {
		     free(LargePropertyBagTestString);
		     LargePropertyBagTestString=NULL;
		    }
		   return(QUERY_INTERFACE_FOR_IPERSISTPROPERTYBAG_FAILED);
		  }

	   }
	  else
	  {
       pIObjectSafety->Release();
		if (LargePropertyBagTestString) 
		{
		 free(LargePropertyBagTestString);
		 LargePropertyBagTestString=NULL;
		}
       return(COM_OBJECT_NOT_SAFE_FOR_UNTRUSTED_DATA);
	  }

	}
 else
 {
  LPOLESTR CLSID_String_Wide;
  LONG     RetValue;
  HKEY     hKeyQ;
  TCHAR    RegKey[1024];
  bool     RegistryEntrySafeForInitializingPersistentData=false;  
   
  hResult = StringFromCLSID(*clsid, &CLSID_String_Wide);
  if (FAILED(hResult))
    {
	 return(STRING_FROM_CLSID_FAILED);
    }
  sprintf(RegKey, "Software\\Classes\\CLSID\\%ws\\Implemented Categories\\{7DD95802-9882-11CF-9FA9-00AA006C42C4}", CLSID_String_Wide);
  RetValue =RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegKey, 0, KEY_QUERY_VALUE, &hKeyQ );
  RegCloseKey(hKeyQ);
  if (RetValue == ERROR_SUCCESS) RegistryEntrySafeForInitializingPersistentData=true;
  CoTaskMemFree(CLSID_String_Wide);
if (RegistryEntrySafeForInitializingPersistentData)
  {
   IPersistPropertyBag *pIPersistPropertyBag=NULL;   
   hResult = CoCreateInstance(*clsid, NULL, CLSCTX_LOCAL_SERVER | CLSCTX_INPROC_SERVER, IID_IPersistPropertyBag, (void**)&pIPersistPropertyBag);
   if (hResult== S_OK)
	  {
		int RetVal=SUCCESS;
		if ((TestFileHandle=Create_HTML_PARAMS_Test_File(lpPathBuffer,CLSID_String))!=NULL)
		   {
			MyIPropertyBag iIPropertyBag;
		    pIPersistPropertyBag->InitNew();
            try
			{
			 pIPersistPropertyBag->Load(&iIPropertyBag,NULL);
			 pIPersistPropertyBag->Save(&iIPropertyBag,TRUE,TRUE);
			}
			catch (...) {}
			pIPersistPropertyBag->Release();
			WriteText(TestFileHandle,"</object>\r\n");
			CloseHandle(TestFileHandle);
			if (HaveParamsToTest)
			{
			 DWORD LoadTime=ComputeInitialLoadTime(lpPathBuffer);
			   if (LoadTime)
			        RetVal=IELoadUrlPARAMS(lpPathBuffer,LoadTime); 
			}
 		  }
		else  pIPersistPropertyBag->Release();
	   if (LargePropertyBagTestString) 
		 {
		  free(LargePropertyBagTestString);
		  LargePropertyBagTestString=NULL;
		 }
       
		return(RetVal);
      }
  else
	  { 
		if (LargePropertyBagTestString) 
		{
		 free(LargePropertyBagTestString);
		 LargePropertyBagTestString=NULL;
		}
		return(QUERY_INTERFACE_FOR_IPERSISTPROPERTYBAG_FAILED);
	  }
    
  }
  else
  {
	  if (LargePropertyBagTestString) 
	  {
       free(LargePropertyBagTestString);
	   LargePropertyBagTestString=NULL;
	  }
   return(COM_OBJECT_NOT_SAFE_FOR_UNTRUSTED_DATA);
  }
  
 }
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// int TestParamsViaBinaryScan(CLSID *clsid)                                          //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int TestParamsViaBinaryScan(CLSID *clsid)
{
 HRESULT        hResult=E_FAIL;
 IObjectSafety *pIObjectSafety = NULL;
 
 strcat(lpPathBuffer,PARAMS_HTML);
 DeleteTempFile(lpPathBuffer);

 try
  {

    hResult=CoCreateInstance(*clsid, NULL, CLSCTX_LOCAL_SERVER | CLSCTX_INPROC_SERVER,
                            IID_IObjectSafety, (void**)&pIObjectSafety);
  }
 catch(...)
  {
   pIObjectSafety=NULL;
  }
 if ((hResult==S_OK) && (pIObjectSafety!=NULL))
    {

     bool IPersist_SafeForUntrustedData=false;

      try
	   {
	   
	   	hResult = pIObjectSafety->SetInterfaceSafetyOptions(IID_IPersistPropertyBag,
		          INTERFACESAFE_FOR_UNTRUSTED_DATA, INTERFACESAFE_FOR_UNTRUSTED_DATA);

		if (hResult==S_OK) {
			IPersist_SafeForUntrustedData=true;
		}
        pIObjectSafety->Release();
	   }
      catch(...)
       {
        pIObjectSafety->Release();
        return(COM_OBJECTSAFETY_SET_INTERFACE_OPT_FAULT);
       }
       if (IPersist_SafeForUntrustedData)
	   {
        if (Generate_HTML_PARAMS_Test_File(lpPathBuffer,CLSID_String)!=0)  return(FAILED_TO_CREATE_IE_TEST_FILE);
	    DWORD LoadTime=ComputeInitialLoadTime(lpPathBuffer);
	    if (LoadTime) return(IELoadUrlPARAMS(lpPathBuffer,LoadTime)); 
		else return(FAILED_TO_CREATE_IE_TEST_FILE);
	   }
	  else return(COM_OBJECT_NOT_SAFE_FOR_UNTRUSTED_DATA);
	}
 else
 {
  LPOLESTR CLSID_String_Wide;
  LONG     RetValue;
  HKEY     hKeyQ;
  TCHAR    RegKey[1024];
  bool     RegistryEntrySafeForInitializingPersistentData=false;  
   
  hResult = StringFromCLSID(*clsid, &CLSID_String_Wide);
  if (FAILED(hResult))
    {
	 return(STRING_FROM_CLSID_FAILED);
    }
  sprintf(RegKey, "Software\\Classes\\CLSID\\%ws\\Implemented Categories\\{7DD95802-9882-11CF-9FA9-00AA006C42C4}", CLSID_String_Wide);
  RetValue =RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegKey, 0, KEY_QUERY_VALUE, &hKeyQ );
  RegCloseKey(hKeyQ);
  if (RetValue == ERROR_SUCCESS) RegistryEntrySafeForInitializingPersistentData=true;
  CoTaskMemFree(CLSID_String_Wide);
if (RegistryEntrySafeForInitializingPersistentData)
  {
    if (Generate_HTML_PARAMS_Test_File(lpPathBuffer,CLSID_String)!=0)  return(FAILED_TO_CREATE_IE_TEST_FILE);
	DWORD LoadTime=ComputeInitialLoadTime(lpPathBuffer);
	if (LoadTime) return(IELoadUrlPARAMS(lpPathBuffer,LoadTime)); 
    else return(FAILED_TO_CREATE_IE_TEST_FILE);
  }
  else
  {
   return(COM_OBJECT_NOT_SAFE_FOR_UNTRUSTED_DATA);
  }
  
 }
}
//---------------------------------------------------------------------------

static BOOL GenerateVariantText(VARIANT &var,LPCOLESTR pwszPropName,HANDLE hfile)
{
 char   TempBuff[1024];
 switch(var.vt)
  {
   case VT_BSTR:
   case VT_I1:
   case VT_I2:
   case VT_I4:
   case VT_I8:
   case VT_UI1:
   case VT_UI2:
   case VT_UI4:
   case VT_UI8:
   case VT_R4:
   case VT_R8:
   case VT_BOOL:
   case VT_DATE:
        break;
   default:
			 return(FALSE);
  }

 
 sprintf(TempBuff,"<PARAM NAME=\"%ws\" VALUE=\"",pwszPropName);
 WriteText(TestFileHandle,TempBuff);

 switch(var.vt)
  {
   case VT_BSTR:
	            WriteText(hfile,LargePropertyBagTestString);
				break;
   case VT_I1:
   case VT_I2:
   case VT_I4:
   case VT_I8:
	            WriteText(hfile,"-1");
	            break;  
   case VT_UI1:
	            WriteText(hfile,"0xFF");
	            break;  
   case VT_UI2:
	            WriteText(hfile,"0xFFFF");
	            break;   
   case VT_UI4:
			    WriteText(hfile,"0xFFFFFFFF");
	            break;  

   case VT_UI8:
			    WriteText(hfile,"0xFFFFFFFFFFFFFFFF");
	            break;  

   case VT_R4:
   case VT_R8:
	            WriteText(hfile,"0.0");
	            break;  
   case VT_BOOL:
	            WriteText(hfile,"10000");
	            break;  
   case  VT_DATE:
	           WriteText(hfile,"-9999999999999999999999.99999999");
               break;

  }

 sprintf(TempBuff,"\">\r\n",pwszPropName);
 WriteText(TestFileHandle,TempBuff);
 return(TRUE);
}

////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// BOOL CALLBACK EnumChildWindowCallback(HWND hwnd, LPARAM lParam)                   //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static BOOL CALLBACK EnumChildWindowCallback(HWND hwnd, LPARAM lParam)
{
TCHAR ClassName[200],TextBuffer[200];

GetClassName(hwnd,ClassName,sizeof(ClassName));
GetWindowText(hwnd,TextBuffer,sizeof(TextBuffer));
_tcslwr(ClassName);
_tcslwr(TextBuffer);

if (lParam==NULL)
{
  if (_tcsstr(ClassName,"internet explorer_server")) 
  {
   if (EnumChildWindows((HWND)hwnd, EnumChildWindowCallback,(LPARAM)0xFEED))
        { 
         
	    }

  }
}
else
{
#if 0
 if (_tcsstr(ClassName,"internet explorer_objectoverlay"))
  {
#endif
  RECT Rect;
   if (GetClientRect(hwnd,&Rect))
   {
       POINT Point;
	   Point.y=(Rect.bottom+Rect.top)/2;
	   Point.x=(Rect.left+Rect.right)/2;
	   if (ClientToScreen(hwnd,&Point))
	   {
         double fScreenWidth    = ::GetSystemMetrics( SM_CXSCREEN )-1; 
         double fScreenHeight  = ::GetSystemMetrics( SM_CYSCREEN )-1; 
         double fx = Point.x*(65535.0f/fScreenWidth);
         double fy = Point.y*(65535.0f/fScreenHeight);
		 mouse_event(MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE,(DWORD)fx,(DWORD)fy,0,0);
		 mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
		 mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
	   }
   }
#if 0
 }
#endif
}
return(TRUE);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam)                //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam)
{
  if ((GetWindowLong(hwnd,GWL_STYLE) & WS_VISIBLE))
  {
    TCHAR ClassName[200];
    GetClassName(hwnd,ClassName,sizeof(ClassName));
	if (_tcsstr(ClassName,"IEFrame"))
	{
     if (EnumChildWindows((HWND)hwnd, EnumChildWindowCallback,(LPARAM)NULL))
        { 
         
	    }
	}
  }
 return(TRUE);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
///  static void ActivateControlInIE(void)                                            //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static void ActivateControlInIE(void)
{
	EnumWindows(EnumWindowsCallback,0);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
///  static DWORD ComputeInitialLoadTime(char *FileName)                              //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////

static DWORD ComputeInitialLoadTime(char *FileName)
{
  DWORD TimeInMs=0;
  LARGE_INTEGER FileSize;

  HANDLE h = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL,  // security attributes
                         OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
  if (!h) return(TimeInMs);

  if (GetFileSizeEx(h, &FileSize))
  {
   TimeInMs=((DWORD)(FileSize.QuadPart/PARAMS_LOAD_MBYTES_PER_SEC)+PARAMS_LOAD_TIMEOUT_MIN);
  }
  CloseHandle(h);
  return(TimeInMs);
}
//---------------------------------------------------------------------------

void PrintVariant(VARIANT &var)
	{
		char TempTextBuffer[1024];
	if(var.vt==VT_UI1)
		{ sprintf(TempTextBuffer,"VT_UI1:%u\r\n",var.bVal); }
	else if(var.vt==VT_I2)
		{ sprintf(TempTextBuffer,"VT_I2:%d\r\n",var.iVal); }
	else if(var.vt==VT_I4)
		{ sprintf(TempTextBuffer,"I4:%d\r\n",var.lVal); }
	else if(var.vt==VT_R4)
		{ sprintf(TempTextBuffer,"VT_R4:%f\r\n",var.fltVal); }
	else if(var.vt==VT_R8)
		{ sprintf(TempTextBuffer,"VT_R8:%lf\r\n",var.dblVal); }
	else if(var.vt==VT_BOOL)
		{ 
		if(var.boolVal==0)
			{ sprintf(TempTextBuffer,"VT_BOOL:false\r\n"); }
		else
			{ sprintf(TempTextBuffer,"VT_BOOL:true\r\n"); }
		}
	else if(var.vt==VT_BSTR)
	{  sprintf(TempTextBuffer,"VT_BSTR:%0.10s....{%d}\r\n",var.bstrVal,SysStringLen(var.bstrVal)); }
	else sprintf(TempTextBuffer,"[UNKNOWN]\r\n");
	WriteText(hLogFile,TempTextBuffer);
	}

#define COM_KILL_BIT TEXT("SOFTWARE\\Microsoft\\Internet Explorer\\ActiveX Compatibility")
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// int GetKillBit(WCHAR *CLSID_Str_Wide)                                             //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int GetKillBit(WCHAR *CLSID_Str_Wide)
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

//---------------------------------------------------------------------------
static void dumphex(unsigned char *dptr,int size) { 
 
 int cnt,counter=0; 
 
 
 while(size>0) { 
 
  (size>16)? cnt=16 :cnt=size; 
 
  printf("%08lx   ",counter); 
 
  for (int i=0;i<cnt;i++) printf("%02x ",dptr[counter+i]); 
 
  for (int i=0;i<16-cnt;i++) printf("   "); 
 
  printf(" "); 
 
  for (int i=0;i<cnt;i++) 
 
      (isprint(dptr[counter+i])) ? printf("%c",dptr[counter+i]):printf("."); 
 
  printf("\n"); 
 
  counter+=16; 
 
  size-=16; 
 
 } 
 
 return; 
 
} 
//---------------------------------------------------------------------------
static int DeQuoteString(char * String)
{
 char *In,*Out;

 In = String; 
 Out = In;
 size_t Length=strlen(String)+1;

 for ( size_t i = 0; i < Length; i++, In++ )
  {
   if ( *In != '"' )   *Out++ = *In;
  }
 return ((int) (Out - String) );
} 
//---------------------------------------------------------------------------
#if 0
static char * VariantToString(VARIANT &var)
{

}
//---------------------------------------------------------------------------
#endif