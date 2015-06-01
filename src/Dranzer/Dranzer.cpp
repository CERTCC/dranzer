// Dranzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <tlhelp32.h>
#include "WindowMonitor.h"
#include "KillBit.h"
#include "GetCOM_Objects.h"
#include "TestErrors.h"
#include "KillApplication.h"

#define RELEASE_VERSION "RELEASE_19"
#define COM_OBJECT_TEST_TIME_LIMIT_IN_SECONDS 80
#if _DEBUG
#define TESTANDREPORT TEXT("..//..//TestAndReport//Debug//TestAndReport.exe")
#else
#define TESTANDREPORT TEXT("..//..//TestAndReport//Release//TestAndReport.exe")
#endif
#define HASH_TABLE_SIZE 256

typedef struct _Hash_Entry
{
 CLSID                clsid;
 struct _Hash_Entry * Next;
} Hash_Entry;

typedef struct
{
 unsigned int        Table_Size;	
 Hash_Entry       ** Hash_Tables;
} Hash_Table;
 
typedef enum 
{
 NONE,
 GEN_BASE_COM_LIST,
 GEN_KILL_BIT_LIST,
 GEN_INTERFACE_LISTINGS,
 TEST_INTERFACES,
 LOAD_IN_IE,
 PARAMS_IN_IE_PROPBAG,
 PARAMS_IN_IE_BINARY_SCAN,
 EMIT_VERSION_INFO,
 PRINT_COM_OBJECT_INFO
} TExecutionMode;

static DWORD WINAPI  COM_TestThreadProcRegistry(LPVOID arg);
static DWORD WINAPI  COM_TestThreadProcInputFile(LPVOID arg); 
static int           TestCOMObject(COM_ObjectInfoType *COM_ObjectInfo,char *LogFile);
static BOOL          CtrlHandler(DWORD fdwCtrlType);
static void          PrintError(DWORD dw);
static void          WriteTextToLog(char *Text);
static int           AppendFile(HANDLE hAppend, char *FileNameToAppend);
static void          PrintUsage(_TCHAR* argv[]);
static void          ParseArguments(int argc, _TCHAR* argv[]);
static void          DeleteTempResultsFile(char *FileName);
static void          GenerateComBaseline(char *FileName,bool List_Objects_Killbit);
static void          EmitVersionInfo(char *FileName);
static               Hash_Table * Create_Hash_Table(unsigned int Size);
static unsigned int  HashCLSID(CLSID clsid);
static void          Free_Hash_Table(Hash_Table * Table);
static int           Insert_Hash_Table(Hash_Table * Table,CLSID clsid);
static int           Remove_Hash_Table(Hash_Table * Table,CLSID clsid);    
static CLSID *       Lookup_Hash(Hash_Table * Table,CLSID clsid);
static void          MoveResultsFile(WCHAR *CLSID_Str_Wide);
static void          DeleteHTMLResults(void);
TExecutionMode       ExecutionMode=NONE;
static HANDLE        TestHarnessKillEvent=NULL;
static HANDLE        hFile=NULL; 
static DWORD         NumberOfFailedComTests=0;
static DWORD         NumberOfHungComObjects=0;
static DWORD         NumberOfComObjects=0;
static DWORD         NumberOfComObjectsWithKillBit=0;
static DWORD         NumberOfComObjectsWithoutKillBit=0;
static DWORD         NumberOfComObjectsNotScriptSafe=0;
static DWORD         NumberOfComObjectsNotSafeForInitialization=0;
static DWORD         NumberOfComObjectsWithOutTypeInfo=0;
static DWORD         NumberOfComPassTest=0;
static DWORD         OtherCOM_Errors=0;
static BOOL          UseInputFile=false;
static BOOL          UseNoTestList=false;
static FILE         *InputFile=NULL;
static Hash_Table   *CLSID_Hash_Table=NULL;
static char          szTempName[MAX_PATH];  
static char          lpPathBuffer[MAX_PATH-14+1];
static TProcessSnapShot *SnapShotOfProcesses=NULL;

int _tmain(int argc, _TCHAR* argv[])
{
 HANDLE  COM_TestThread=NULL;
 DWORD   id;
 DWORD   WaitResult;
 char    TextWrite[1024];
 DWORD   dwRetVal;


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


 TestHarnessKillEvent = CreateEvent (NULL,  // No security attributes
                                    FALSE, // Manual-reset event
                                    FALSE, // Initial state is signaled
                                    NULL); // Object name

 if (TestHarnessKillEvent==NULL) 
	{
     printf("Failed to Create Kill Event Object\n");
	 return(-1);
	}

SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, TRUE);
if ((ExecutionMode!=LOAD_IN_IE) && 
	(ExecutionMode!=PARAMS_IN_IE_PROPBAG) &&
	(ExecutionMode!=PARAMS_IN_IE_BINARY_SCAN)) WindowMonitorStart(true);
else WindowMonitorStart(false);
 if ((SnapShotOfProcesses=GetSnapShotOfProcesses())==NULL)
  {
   printf("Could not get snap shot of running processes\n");
   return (-1);
  }

if (UseInputFile)
COM_TestThread = CreateThread(NULL, 0, COM_TestThreadProcInputFile, (LPVOID)NULL, CREATE_SUSPENDED, &id);

else COM_TestThread = CreateThread(NULL, 0, COM_TestThreadProcRegistry, (LPVOID)NULL, CREATE_SUSPENDED, &id);
if (COM_TestThread==NULL)
	 { 
 	  CloseHandle(TestHarnessKillEvent);
	  return(-1);
	 }
 ResumeThread(COM_TestThread);
 WaitResult=WaitForSingleObject(COM_TestThread,INFINITE);
 CloseHandle(COM_TestThread);
 CloseHandle(TestHarnessKillEvent);
 if (UseInputFile) fclose(InputFile);
 WindowMonitorStop();
 FreeSnapShot(SnapShotOfProcesses);
 
// emit test engine version based on svn build revision

 sprintf(TextWrite,"*******************************************************************************\r\n");
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"Test Engine Version: $Rev: 96 $\r\n");
 WriteTextToLog(TextWrite);
 
 
 if (ExecutionMode==GEN_INTERFACE_LISTINGS)
 {
 sprintf(TextWrite,"*******************************************************************************\r\n");
 WriteTextToLog(TextWrite);

 sprintf(TextWrite,"Number of COM Objects                       %d\r\n",NumberOfComObjects);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"Number of COM Objects With Kill Bit         %d\r\n",NumberOfComObjectsWithKillBit);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"Number of COM Objects Without Kill Bit      %d\r\n",NumberOfComObjectsWithoutKillBit);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"Number of COM Objects Not Script Safe       %d\r\n",NumberOfComObjectsNotScriptSafe);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"Number of COM Objects Listings Generated    %d\r\n",NumberOfComPassTest);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"Number of COM Objects Listing Failed        %d\r\n",NumberOfFailedComTests);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"Number of COM Objects Hung During Operation %d\r\n",NumberOfHungComObjects);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"Number of COM Objects with No Type Info     %d\r\n", NumberOfComObjectsWithOutTypeInfo);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"Number of COM Objects Misc Errors           %d\r\n",OtherCOM_Errors);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"*******************************************************************************\r\n");
 WriteTextToLog(TextWrite);

 }
 else if (ExecutionMode==PRINT_COM_OBJECT_INFO)
 {
 sprintf(TextWrite,"*******************************************************************************\r\n");
 WriteTextToLog(TextWrite);

 sprintf(TextWrite,"Number of COM Objects                       %d\r\n",NumberOfComObjects);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"Number of COM Objects With Kill Bit         %d\r\n",NumberOfComObjectsWithKillBit);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"Number of COM Objects Without Kill Bit      %d\r\n",NumberOfComObjectsWithoutKillBit);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"Number of COM Objects Hung During Operation %d\r\n",NumberOfHungComObjects);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"Number of COM Objects Misc Errors           %d\r\n",OtherCOM_Errors);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"*******************************************************************************\r\n");
 WriteTextToLog(TextWrite);

 }
 else
 {
 sprintf(TextWrite,"*******************************************************************************\r\n");
 WriteTextToLog(TextWrite);

 sprintf(TextWrite,"Number of COM Objects                   %d\r\n",NumberOfComObjects);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"Number of COM Objects With Kill Bit     %d\r\n",NumberOfComObjectsWithKillBit);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"Number of COM Objects Without Kill Bit  %d\r\n",NumberOfComObjectsWithoutKillBit);
 WriteTextToLog(TextWrite);
 if ((ExecutionMode!=LOAD_IN_IE) && 
	 (ExecutionMode!=PARAMS_IN_IE_PROPBAG) &&
	 (ExecutionMode!=PARAMS_IN_IE_BINARY_SCAN))
  {
   sprintf(TextWrite,"Number of COM Objects Not Script Safe   %d\r\n",NumberOfComObjectsNotScriptSafe);
   WriteTextToLog(TextWrite);
  }
 if ((ExecutionMode==PARAMS_IN_IE_PROPBAG) ||
	 (ExecutionMode==PARAMS_IN_IE_BINARY_SCAN))
 {
  sprintf(TextWrite,"Number of COM Objects Not Safe for Init %d\r\n",NumberOfComObjectsNotSafeForInitialization);
  WriteTextToLog(TextWrite);
 }
 sprintf(TextWrite,"Number of COM Objects Passed Test       %d\r\n",NumberOfComPassTest);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"Number of COM Objects Failed Test       %d\r\n",NumberOfFailedComTests);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"Number of COM Objects Hung During Test  %d\r\n",NumberOfHungComObjects);
 WriteTextToLog(TextWrite);
 if ((ExecutionMode!=LOAD_IN_IE) && 
	 (ExecutionMode!=PARAMS_IN_IE_PROPBAG) &&
	 (ExecutionMode!=PARAMS_IN_IE_BINARY_SCAN))
  {
   sprintf(TextWrite,"Number of COM Objects with No Type Info %d\r\n", NumberOfComObjectsWithOutTypeInfo);
   WriteTextToLog(TextWrite);
  }
 sprintf(TextWrite,"Number of COM Objects Misc Errors       %d\r\n",OtherCOM_Errors);
 WriteTextToLog(TextWrite);
 sprintf(TextWrite,"*******************************************************************************\r\n");
 WriteTextToLog(TextWrite);
 }
 CloseHandle(hFile);
 if ((UseNoTestList) && (CLSID_Hash_Table)) Free_Hash_Table(CLSID_Hash_Table);
 return 0;
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
	fprintf(stderr, "        -o <outputfile>   - Output Filename\n");
	fprintf(stderr, "        -i <inputfile>    - Use input file CLSID list\n");
	fprintf(stderr, "        -d <notestfile>   - Use don't test CLSID List\n");
	fprintf(stderr, "        -g                - Generate base COM list\n");
	fprintf(stderr, "        -k                - Generate Kill Bit COM list\n");
	fprintf(stderr, "        -l                - Generate Interface Listings\n");
    fprintf(stderr, "        -b                - Load In Browser (IE)\n");
    fprintf(stderr, "        -t                - Test Interfaces Properties and Methods\n");
	fprintf(stderr, "        -p                - Test PARAMS (PropertyBag) in Internet Explorer\n");
    fprintf(stderr, "        -s                - Test PARAMS (Binary Scan) in Internet Explorer\n");
    fprintf(stderr, "        -n                - Print COM object information\n");
    fprintf(stderr, "        -v                - Print out version information\n");

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
    char *InputFileName=NULL;
	char *OutputFileName=NULL;
	char *NoTestFileName=NULL;
	if (argc < 1)
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
			if (InputFileName) free(InputFileName);
			if (NoTestFileName) free(NoTestFileName);
			exit(2);
		}
		c = strchr((char *)"io", argv[i][1]);
		if (c != NULL)
			if ((i == (argc-1)) || (argv[i+1][0] == '-'))
			{
				fprintf(stderr, "option %s requires an argument\n", argv[i]);
				PrintUsage(argv);
			    if (OutputFileName) free(OutputFileName);
			    if (InputFileName) free(InputFileName);
				if (NoTestFileName) free(NoTestFileName);
				exit(1);
			}
			switch (argv[i][1])
			{
			case 'i':
				if (!UseInputFile)
				{
				 i++;
				 UseInputFile=true;
				 InputFileName=_strdup(argv[i]);
				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (InputFileName) free(InputFileName);
				 if (NoTestFileName) free(NoTestFileName);
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
			     if (InputFileName) free(InputFileName);
				 if (NoTestFileName) free(NoTestFileName);
			     exit(2);
				}
				break;

			case 'd':
				if (!UseNoTestList)
				{
				 i++;
				 UseNoTestList=true;
				 NoTestFileName=_strdup(argv[i]);
				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (InputFileName) free(InputFileName);
				 if (NoTestFileName) free(NoTestFileName);
			     exit(2);
				}
				break;

			case 'l':
				if (ExecutionMode==NONE)
				{
                 ExecutionMode=GEN_INTERFACE_LISTINGS;
				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (InputFileName) free(InputFileName);
				 if (NoTestFileName) free(NoTestFileName);
			     exit(2);
				}
				break;
	     case 'b':
				if (ExecutionMode==NONE)
				{
                 ExecutionMode=LOAD_IN_IE;
				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (InputFileName) free(InputFileName);
				 if (NoTestFileName) free(NoTestFileName);
			     exit(2);
				}
				break;
	     case 'p':
				if (ExecutionMode==NONE)
				{
                 ExecutionMode=PARAMS_IN_IE_PROPBAG;
				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (InputFileName) free(InputFileName);
				 if (NoTestFileName) free(NoTestFileName);
			     exit(2);
				}
				break;
	     case 's':
				if (ExecutionMode==NONE)
				{
                 ExecutionMode=PARAMS_IN_IE_BINARY_SCAN;
				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (InputFileName) free(InputFileName);
				 if (NoTestFileName) free(NoTestFileName);
			     exit(2);
				}
				break;

			case 'g':
				if (ExecutionMode==NONE)
				{
                 ExecutionMode=GEN_BASE_COM_LIST;
				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (InputFileName) free(InputFileName);
				 if (NoTestFileName) free(NoTestFileName);
			     exit(2);
				}
				break;
			case 'k':
				if (ExecutionMode==NONE)
				{
                 ExecutionMode=GEN_KILL_BIT_LIST;
				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (InputFileName) free(InputFileName);
				 if (NoTestFileName) free(NoTestFileName);
			     exit(2);
				}
				break;
			case 't':
				if (ExecutionMode==NONE)
				{
                 ExecutionMode=TEST_INTERFACES;
				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (InputFileName) free(InputFileName);
				 if (NoTestFileName) free(NoTestFileName);
			     exit(2);
				}
				break;
			case 'n':
				if (ExecutionMode==NONE)
				{
                 ExecutionMode=PRINT_COM_OBJECT_INFO;
				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (InputFileName) free(InputFileName);
				 if (NoTestFileName) free(NoTestFileName);
			     exit(2);
				}
				break;

			case 'v':
				if (ExecutionMode==NONE)
				{
                 ExecutionMode=EMIT_VERSION_INFO;
				}
				else
				{
				 fprintf(stderr, "Error in command line\n");
			     PrintUsage(argv);
			     if (OutputFileName) free(OutputFileName);
			     if (InputFileName) free(InputFileName);
				 if (NoTestFileName) free(NoTestFileName);
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
			     if (InputFileName) free(InputFileName);
				 if (NoTestFileName) free(NoTestFileName);
			     exit(2);
				 break;

			}
	}
  if (ExecutionMode==NONE)
  {
   printf("Execution mode not specified use -g,-k,-l,-b, or -p \n");
   PrintUsage(argv);
   if (OutputFileName) free(OutputFileName);
   if (InputFileName) free(InputFileName);
   if (NoTestFileName) free(NoTestFileName);
   exit(2);

  }
  if ((UseNoTestList||
		UseInputFile) && 
	  ((ExecutionMode==GEN_BASE_COM_LIST)||
		(ExecutionMode==GEN_KILL_BIT_LIST)||
		(ExecutionMode==EMIT_VERSION_INFO)))	  
  {
   printf("-i,-d options not vaild with -g, -k or -v options\n");
   if (OutputFileName) free(OutputFileName);
   if (InputFileName) free(InputFileName);
   if (NoTestFileName) free(NoTestFileName); 
   exit(1);
  }

 if (UseNoTestList)
   {
    wchar_t            InputLine[512];
    size_t             InputLineLen;
    FILE *             NoTestInputFile;
	CLSID              clsid;

	NoTestInputFile=fopen(NoTestFileName,"rt");
	
	if (NoTestInputFile==NULL) 
	  {
	   fprintf(stderr, "Can't open no test input file %s\n", NoTestFileName);
	   if (NoTestFileName) free(NoTestFileName);
	   exit(1);
      }

    if ((CLSID_Hash_Table=Create_Hash_Table(HASH_TABLE_SIZE))==NULL)
     {
      printf("Failed to Create Hash Table\n");
      exit(0);
     }
	 while (fgetws(InputLine,sizeof(InputLine),NoTestInputFile))
      {
       InputLineLen=wcslen(InputLine);
       if (InputLineLen>0)
        {
         if (InputLine[InputLineLen-1]==0x0A) InputLine[InputLineLen-1]=NULL;
         if (wcslen(InputLine)==0) continue;
		 if (CLSIDFromString(InputLine, &clsid)!=NOERROR)
		    {
	          fprintf(stderr, "Error in no test input file\n");
			  fclose(NoTestInputFile);
			  if (OutputFileName) free(OutputFileName);
  	          if (InputFileName) free(InputFileName);
			  exit(1);
			}
		 if (Insert_Hash_Table(CLSID_Hash_Table,clsid)!=0)
		 {
	          fprintf(stderr, "Error build No Test CLSID Hash Table\n");
			  fclose(NoTestInputFile);
			  if (OutputFileName) free(OutputFileName);
  	          if (InputFileName) free(InputFileName);
			  exit(1);
		 }
		}
	  }
       fclose(NoTestInputFile);
   }

  if (UseInputFile)
	{
	 InputFile=fopen(InputFileName,"rt");
	
	 if (InputFile==NULL) 
	  {
       fprintf(stderr, "Can't open input file %s\n", InputFileName);
       if (OutputFileName) free(OutputFileName);
	   if (InputFileName) free(InputFileName);
	   exit(1);
      }
	  if (InputFileName) free(InputFileName);
	}
   if ((ExecutionMode==GEN_BASE_COM_LIST)||
		(ExecutionMode==GEN_KILL_BIT_LIST))
   {
    GenerateComBaseline(OutputFileName,ExecutionMode==GEN_KILL_BIT_LIST);
	if (OutputFileName) free(OutputFileName);
	exit(0);
   }
   if ((ExecutionMode==EMIT_VERSION_INFO))
   {
    EmitVersionInfo(OutputFileName);
	if (OutputFileName) free(OutputFileName);
	exit(0);
   }
  else
  {
   if (OutputFileName)
   {
    hFile = CreateFile(OutputFileName,     // file to create
                   GENERIC_WRITE,          // open for writing
                   0,                      // do not share
                   NULL,                   // default security
                   CREATE_ALWAYS,          // 
                   FILE_ATTRIBUTE_NORMAL,  // normal file
                   NULL);                  // no attr. template
    free(OutputFileName);
    if (hFile == INVALID_HANDLE_VALUE) 
    { 
     if (InputFile) fclose(InputFile);
     printf("Could not open output file ");
	 PrintError(GetLastError());
	 printf("\n");
     exit(2);
    }
   }
   else  
    {
	 hFile=GetStdHandle(STD_OUTPUT_HANDLE);
	}
  }

// end of ParseArguments()
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static void GenerateComBaseline(char *FileName,bool List_Objects_Killbit)         //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static void GenerateComBaseline(char *FileName,bool List_Objects_Killbit) 
{
 FILE              *OutputFile;
 HKEY               hKey;
 DWORD              NumObjects;
 COM_ObjectInfoType COM_ObjectInfo;
 DWORD              NumFound=0;
 DWORD              NumKillBit=0;
 int                kbval;

 if (FileName==NULL) OutputFile=stdout;
 else if( (OutputFile = fopen( FileName, "w+" )) == NULL )
 {
	 printf( "Cannot open output file :%s\n" ,FileName);
     return;
 }

 hKey=OpenCOM_ObjectList(&NumObjects);
 if (hKey==NULL)
  {
   printf("Can't open COM object database\n");
   if (FileName!=NULL) fclose(OutputFile);
   return;
  }
 if (NumObjects==0)

 {
  printf("Can't find any COM objects\n");
  CloseCOM_ObjectList(hKey);
  if (FileName!=NULL) fclose(OutputFile);
  return;
 }

 for (DWORD index=0;index<NumObjects;index++)
 {
  if (!GetCOM_ObjectInfo(hKey,index, &COM_ObjectInfo))
    {
     continue;
	}
  else
  {
   if ((kbval=GetKillBit(COM_ObjectInfo.CLSID_Str_Wide))==1) NumKillBit++;
   if (!List_Objects_Killbit)
      fprintf(OutputFile,"%ws\n",COM_ObjectInfo.CLSID_Str_Wide);
   else
   {
     if (kbval==1)
	   fprintf(OutputFile,"%ws %s\n",COM_ObjectInfo.CLSID_Str_Wide,COM_ObjectInfo.CLSID_Description);
   }
    NumFound++;
  }
 }
 CloseCOM_ObjectList(hKey);
 if (FileName!=NULL) fclose(OutputFile);
 printf("%d COM Objects Found\n", NumFound);
 printf("%d Objects with kill bit set\n",NumKillBit);
 return;
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static void EmitVersionInfo(char *FileName)                                       //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static void EmitVersionInfo(char *FileName) 
{
 FILE              *OutputFile;

 if (FileName==NULL) OutputFile=stdout;
 else if( (OutputFile = fopen( FileName, "w+" )) == NULL )
 {
	 fprintf(stderr, "Cannot open output file :%s\n" ,FileName);
     return;
 }
 if (RELEASE_VERSION && "$Rev: 96 $")
	 fprintf(OutputFile, "Dranzer Release Version: %s; Test Engine Revision: $Rev: 96 $", RELEASE_VERSION);
 else fprintf(stderr, "Cannot determine version info");

 if (FileName!=NULL) fclose(OutputFile);
 return;
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static int HashCLSID(CLSID clsid)                                                 //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static unsigned int HashCLSID(CLSID clsid)
{
 unsigned int hash=0,i;
 for (i = 0; i < 8; i ++) hash ^= ((const short *)&clsid)[i];
 return hash & (HASH_TABLE_SIZE-1);
}
////////////////////////////////////////////////////////////////////////////////////////
//                                                                                   //
// static Hash_Table * Create_Hash_Table(unsigned int Size)                          //            
//                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static Hash_Table * Create_Hash_Table(unsigned int Size)
{
 unsigned int         i;
 Hash_Table *New;
 if ( New = (Hash_Table *)malloc(sizeof(Hash_Table)))
    {
      if ( New->Hash_Tables =(Hash_Entry **)malloc(Size* sizeof(Hash_Entry *)))
         {
          New->Table_Size=Size;
          for (i=0;i<Size;i++) New->Hash_Tables[i]=NULL;
         }
      else 
         {
          free(New);
          New=NULL;
         }
    }
 return(New);
}
////////////////////////////////////////////////////////////////////////////////////////
//                                                                                   //
// static void Free_Hash_Table(Hash_Table * Table)                                   //            
//                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static void Free_Hash_Table(Hash_Table * Table)
{
 unsigned int  i;
 Hash_Entry * Entry;
 
  for (i=0;i<Table->Table_Size;i++)
      {
       while(Table->Hash_Tables[i])
            {
              Entry=Table->Hash_Tables[i];
              Table->Hash_Tables[i]=Entry->Next;
              free(Entry);
            }
      }
 free(Table->Hash_Tables);
 free(Table);
 return;
}
////////////////////////////////////////////////////////////////////////////////////////
//                                                                                    //
// static int Insert_Hash_Table(Hash_Table * Table,CLSID clsid )                       //            
//                                                                                    //
////////////////////////////////////////////////////////////////////////////////////////
static int Insert_Hash_Table(Hash_Table * Table,CLSID clsid)     
{
 unsigned int Table_Index;
 Hash_Entry *New_Entry=(Hash_Entry *)malloc(sizeof(Hash_Entry));
 if (New_Entry)
    {
     New_Entry->clsid=clsid;
     Table_Index=HashCLSID(clsid);
     New_Entry->Next=Table->Hash_Tables[Table_Index];
     Table->Hash_Tables[Table_Index]=New_Entry;
     return(0);
    }
 return(-1);
}
////////////////////////////////////////////////////////////////////////////////////////
//                                                                                    //
// static int Remove_Hash_Table(Hash_Table * Table,CLSID clsid)                       //            
//                                                                                    //
////////////////////////////////////////////////////////////////////////////////////////
static int Remove_Hash_Table(Hash_Table * Table,CLSID clsid)     
{

 Hash_Entry * Entry ,**Start;
 unsigned int          Table_Index=HashCLSID(clsid);
 
 Start=&Table->Hash_Tables[Table_Index];
 while(Entry=*Start)
      {
        if (Entry->clsid==clsid)
           {
            *Start=Entry->Next;
            free(Entry);
            return(0);
           }
        else Start=&Entry->Next;
      }
 return(-1);
}
////////////////////////////////////////////////////////////////////////////////////////
//                                                                                    //
//static CLSID* Lookup_Hash(Hash_Table * Table,CLSID clsid)                          //                                   //            
//                                                                                    //
////////////////////////////////////////////////////////////////////////////////////////
static CLSID* Lookup_Hash(Hash_Table * Table,CLSID clsid)
{
 Hash_Entry * Entry;

 unsigned int Table_Index=HashCLSID(clsid);
 Entry=Table->Hash_Tables[Table_Index];
 
 while(Entry)
     {
      if (Entry->clsid==clsid) return(&Entry->clsid);
      Entry=Entry->Next;   

     }
 return(NULL);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static BOOL CtrlHandler(DWORD fdwCtrlType)                                        //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static BOOL CtrlHandler(DWORD fdwCtrlType)
{
  SetEvent(TestHarnessKillEvent);
 return(TRUE);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static void PrintError(DWORD dw)                                                  //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static void PrintError(DWORD dw)
{
  LPVOID lpMsgBuf;

  FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );
 
  printf( "%s", lpMsgBuf); 

 LocalFree(lpMsgBuf);
 return;
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static DWORD WINAPI COM_TestThreadProcRegistry(LPVOID arg)                                //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////

static DWORD WINAPI COM_TestThreadProcRegistry(LPVOID arg) 
{
 HKEY               hKey;
 DWORD              NumObjects;
 int                KillBit;
 int                ExitCode;
 COM_ObjectInfoType COM_ObjectInfo;
 DWORD              WaitResult;
 
 szTempName[0]=NULL;

 hKey=OpenCOM_ObjectList(&NumObjects);
 if (hKey==NULL)
  {
   printf("Can't open COM object database\n");
   return(0);
  }
 if (NumObjects==0)

 {
  printf("Can't find any COM objects\n");
  CloseCOM_ObjectList(hKey);
  return(0);
 }

 for (DWORD index=0;index<NumObjects;index++)
 {
  WaitResult=WaitForSingleObject(TestHarnessKillEvent,0);
  if (WaitResult==WAIT_OBJECT_0)
  {
   char TextWrite[1024];
   sprintf(TextWrite,"*******************************************************************************\r\n");
   WriteTextToLog(TextWrite);
   sprintf(TextWrite,"%s\r\n",ErrorString(USER_ABORT));
   WriteTextToLog(TextWrite);
   sprintf(TextWrite,"*******************************************************************************\r\n");
   WriteTextToLog(TextWrite);

   printf("User Abort 1\n");
   CloseCOM_ObjectList(hKey);
   DeleteTempResultsFile(szTempName);
   DeleteHTMLResults();
   return(0);
  }
  if (!GetCOM_ObjectInfo(hKey,index, &COM_ObjectInfo))
    {
     //printf("GetCOM_ObjectInfo Failed for Index %d\n",index);
     continue;
	}
  if (UseNoTestList)
  {
   CLSID clsid;
   if (CLSIDFromString(COM_ObjectInfo.CLSID_Str_Wide, &clsid)==NOERROR)
   {
     if(Lookup_Hash(CLSID_Hash_Table,clsid))
	 {
	  continue;
	 }
   }
  }
  KillBit=GetKillBit(COM_ObjectInfo.CLSID_Str_Wide);
  NumberOfComObjects++;

  if ((((KillBit==0)||(KillBit==-1)) && (ExecutionMode!=PRINT_COM_OBJECT_INFO)) || (ExecutionMode==PRINT_COM_OBJECT_INFO))

	{
     UINT uRetVal;
     
    // Create a temporary file. 
    uRetVal = GetTempFileName(lpPathBuffer, // directory for tmp files
                              "COM",    // temp file name prefix 
                              0,            // create unique name 
                              szTempName);  // buffer for name 
     if (uRetVal == 0)
      {
        printf ("GetTempFileName failed with error %d.\n", GetLastError());
        return (0);
      }

     if ((KillBit==0)||(KillBit==-1)) NumberOfComObjectsWithoutKillBit++; 
	 else if (ExecutionMode==PRINT_COM_OBJECT_INFO) NumberOfComObjectsWithKillBit++;

     ExitCode=TestCOMObject(&COM_ObjectInfo,szTempName);
	 if (ExitCode==COM_OBJECT_NOT_SCRIPT_SAFE) NumberOfComObjectsNotScriptSafe++;
	 else if (ExitCode==COM_OBJECT_NOT_SAFE_FOR_UNTRUSTED_DATA) NumberOfComObjectsNotSafeForInitialization++;
	 else if (ExitCode>0)
	 {
	  if (ExitCode==GET_TYPE_INFO_FAILED) NumberOfComObjectsWithOutTypeInfo++;
      else OtherCOM_Errors++;
	 }
	 else if (ExitCode==SUCCESS) 
	 {
	  NumberOfComPassTest++;
	  if ((ExecutionMode==GEN_INTERFACE_LISTINGS) || (ExecutionMode==PRINT_COM_OBJECT_INFO))
	  {
		if (AppendFile(hFile,szTempName)!=SUCCESS)
		{
         char TextWrite[1024];
		 sprintf(TextWrite,"*******************************************************************************\r\n");
	     WriteTextToLog(TextWrite);
		 sprintf(TextWrite,"ERROR - Appending COM Test Log File\r\n");
	     WriteTextToLog(TextWrite);
		 sprintf(TextWrite,"*******************************************************************************\r\n");
	     WriteTextToLog(TextWrite);
		}
	  }
	 }

	 if (ExitCode==USER_ABORT)
	 {
	    char TextWrite[1024];
		sprintf(TextWrite,"*******************************************************************************\r\n");
	    WriteTextToLog(TextWrite);
	    sprintf(TextWrite,"%s\r\n",ErrorString(ExitCode));
	    WriteTextToLog(TextWrite);
		sprintf(TextWrite,"*******************************************************************************\r\n");
	    WriteTextToLog(TextWrite);

		 printf("User Abort 2\n");
		 fflush( stdout );
		 CloseCOM_ObjectList(hKey);
		 DeleteTempResultsFile(szTempName);
		 DeleteHTMLResults();
		 return(0);
	 }
     if (ExitCode<0)
	   {
	    char TextWrite[1024];

		MoveResultsFile(COM_ObjectInfo.CLSID_Str_Wide);
		sprintf(TextWrite,"*******************************************************************************\r\n");
	    WriteTextToLog(TextWrite);
		sprintf(TextWrite,"%ws-%s\r\n",COM_ObjectInfo.CLSID_Str_Wide,COM_ObjectInfo.CLSID_Description);
	    WriteTextToLog(TextWrite);
		sprintf(TextWrite,"ERROR - ");
	    WriteTextToLog(TextWrite);
	    sprintf(TextWrite,"%s (0x%x)\r\n",ErrorString(ExitCode),ExitCode);
	    WriteTextToLog(TextWrite);
		sprintf(TextWrite,"*******************************************************************************\r\n");
	    WriteTextToLog(TextWrite);
		
		if (ExitCode==COM_OBJECT_OPERATION_HUNG) NumberOfHungComObjects++;
		else NumberOfFailedComTests++;

		if (AppendFile(hFile,szTempName)!=SUCCESS)
		{
		sprintf(TextWrite,"*******************************************************************************\r\n");
	    WriteTextToLog(TextWrite);
		sprintf(TextWrite,"ERROR - Appending COM Test Log File\r\n");
	    WriteTextToLog(TextWrite);
		sprintf(TextWrite,"*******************************************************************************\r\n");
	    WriteTextToLog(TextWrite);
		}
	   }
	 else DeleteHTMLResults();
	 
      DeleteTempResultsFile(szTempName);
    
    }
  else NumberOfComObjectsWithKillBit++;
 }
 CloseCOM_ObjectList(hKey);
 return(0);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static DWORD WINAPI COM_TestThreadProcInputFile(LPVOID arg)                                //            
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////

static DWORD WINAPI COM_TestThreadProcInputFile(LPVOID arg) 
{
 wchar_t            InputLine[512];
 size_t             InputLineLen;
 DWORD              LineNumber=0;
 int                KillBit;
 int                ExitCode;
 COM_ObjectInfoType COM_ObjectInfo;
 DWORD              WaitResult;
 
 szTempName[0]=NULL;

 while (fgetws(InputLine,sizeof(InputLine),InputFile))
 {
  LineNumber++;
  WaitResult=WaitForSingleObject(TestHarnessKillEvent,0);
  if (WaitResult==WAIT_OBJECT_0)
  {
   char TextWrite[1024];
   sprintf(TextWrite,"*******************************************************************************\r\n");
   WriteTextToLog(TextWrite);
   sprintf(TextWrite,"%s\r\n",ErrorString(USER_ABORT));
   WriteTextToLog(TextWrite);
   sprintf(TextWrite,"*******************************************************************************\r\n");
   WriteTextToLog(TextWrite);
   printf("User Abort 1\n");
   DeleteTempResultsFile(szTempName);
   DeleteHTMLResults();
   return(0);
  }
  InputLineLen=wcslen(InputLine);
  if (InputLineLen>0)
  {
   if (InputLine[InputLineLen-1]==0x0A)
	   InputLine[InputLineLen-1]=NULL;
   if (wcslen(InputLine)==0) continue;
  }

  if (!GetCOM_ObjectInfo(InputLine, &COM_ObjectInfo))
    {
     printf("Syntax Error in Input Line (%d) - %ws\n",LineNumber,InputLine);
     DeleteTempResultsFile(szTempName);
     return(0);
	}
  if (UseNoTestList)
  {
   CLSID clsid;
   if (CLSIDFromString(COM_ObjectInfo.CLSID_Str_Wide, &clsid)==NOERROR)
   {
     if(Lookup_Hash(CLSID_Hash_Table,clsid))
	 {
	  continue;
	 }
   }
  }
  KillBit=GetKillBit(COM_ObjectInfo.CLSID_Str_Wide);
  NumberOfComObjects++;

  if ((((KillBit==0)||(KillBit==-1)) && (ExecutionMode!=PRINT_COM_OBJECT_INFO)) || (ExecutionMode==PRINT_COM_OBJECT_INFO))
	{
     UINT uRetVal;
     
     // Create a temporary file. 
     uRetVal = GetTempFileName(lpPathBuffer, // directory for tmp files
                              "COM",        // temp file name prefix 
                              0,            // create unique name 
                              szTempName);  // buffer for name 
     if (uRetVal == 0)
      {
        printf ("GetTempFileName failed with error %d.\n", GetLastError());
        return (0);
      }

     if ((KillBit==0)||(KillBit==-1)) NumberOfComObjectsWithoutKillBit++;
	 else if (ExecutionMode==PRINT_COM_OBJECT_INFO) NumberOfComObjectsWithKillBit++;

     ExitCode=TestCOMObject(&COM_ObjectInfo,szTempName);
	 if (ExitCode==COM_OBJECT_NOT_SCRIPT_SAFE) NumberOfComObjectsNotScriptSafe++;
	 else if (ExitCode==COM_OBJECT_NOT_SAFE_FOR_UNTRUSTED_DATA) NumberOfComObjectsNotSafeForInitialization++;
	 else if (ExitCode>0)
	 {
	  if (ExitCode==GET_TYPE_INFO_FAILED) NumberOfComObjectsWithOutTypeInfo++;
      else OtherCOM_Errors++;
	 }
	 else if (ExitCode==SUCCESS) 
	 {
	  if ((ExecutionMode!=GEN_INTERFACE_LISTINGS) && (ExecutionMode!=PRINT_COM_OBJECT_INFO))  NumberOfComPassTest++;
	  else
	  {
		if (AppendFile(hFile,szTempName)!=SUCCESS)
		{
         char TextWrite[1024];
		 sprintf(TextWrite,"*******************************************************************************\r\n");
	     WriteTextToLog(TextWrite);
		 sprintf(TextWrite,"ERROR - Appending COM Test Log File\r\n");
	     WriteTextToLog(TextWrite);
		 sprintf(TextWrite,"*******************************************************************************\r\n");
	     WriteTextToLog(TextWrite);
		}
	  }
	 }

	 if (ExitCode==USER_ABORT)
	 {
	    char TextWrite[1024];
		sprintf(TextWrite,"*******************************************************************************\r\n");
	    WriteTextToLog(TextWrite);
	    sprintf(TextWrite,"%s\r\n",ErrorString(ExitCode));
	    WriteTextToLog(TextWrite);
		sprintf(TextWrite,"*******************************************************************************\r\n");
	    WriteTextToLog(TextWrite);

		 printf("User Abort 2\n");
		 fflush( stdout );
		 DeleteTempResultsFile(szTempName);
		 DeleteHTMLResults();
		 return(0);
	 }
     if (ExitCode<0)
	   {
	    char TextWrite[1024];
		MoveResultsFile(COM_ObjectInfo.CLSID_Str_Wide);
		sprintf(TextWrite,"*******************************************************************************\r\n");
	    WriteTextToLog(TextWrite);
		sprintf(TextWrite,"%ws-%s\r\n",COM_ObjectInfo.CLSID_Str_Wide,COM_ObjectInfo.CLSID_Description);
	    WriteTextToLog(TextWrite);
		sprintf(TextWrite,"ERROR - ");
	    WriteTextToLog(TextWrite);
	    sprintf(TextWrite,"%s (0x%x)\r\n",ErrorString(ExitCode),ExitCode);
	    WriteTextToLog(TextWrite);
		sprintf(TextWrite,"*******************************************************************************\r\n");
	    WriteTextToLog(TextWrite);
		
		if (ExitCode==COM_OBJECT_OPERATION_HUNG) NumberOfHungComObjects++;
		else NumberOfFailedComTests++;

		if (AppendFile(hFile,szTempName)!=SUCCESS)
		{
		sprintf(TextWrite,"*******************************************************************************\r\n");
	    WriteTextToLog(TextWrite);
		sprintf(TextWrite,"ERROR - Appending COM Test Log File\r\n");
	    WriteTextToLog(TextWrite);
		sprintf(TextWrite,"*******************************************************************************\r\n");
	    WriteTextToLog(TextWrite);
		}
	   }
	 else DeleteHTMLResults();
      DeleteTempResultsFile(szTempName);
    
    }
  else NumberOfComObjectsWithKillBit++;
 }
 return(0);
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static void MoveResultsFile(WCHAR *CLSID_Str_Wide)                                //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static void MoveResultsFile(WCHAR *CLSID_Str_Wide)
{
        if ((ExecutionMode==LOAD_IN_IE) || 
			(ExecutionMode==PARAMS_IN_IE_PROPBAG) ||
			(ExecutionMode==PARAMS_IN_IE_BINARY_SCAN))
		{
		 char TempFileNameSource[MAX_PATH];
		 char TempFileNameDest[MAX_PATH];
         strcpy(TempFileNameSource,lpPathBuffer);
		 if (ExecutionMode==LOAD_IN_IE)
		  strcat(TempFileNameSource,IELOAD_HTML);
		 else strcat(TempFileNameSource,PARAMS_HTML);
		 sprintf(TempFileNameDest,"%ws",CLSID_Str_Wide);
         if (ExecutionMode==LOAD_IN_IE) strcat(TempFileNameDest,"_load.html");      
		 else if (ExecutionMode==PARAMS_IN_IE_PROPBAG) strcat(TempFileNameDest,"_param_pb.html");
		 else if (ExecutionMode==PARAMS_IN_IE_BINARY_SCAN) strcat(TempFileNameDest,"_param_bs.html");		  
		 CopyFile(TempFileNameSource,TempFileNameDest,FALSE);
		  DeleteTempResultsFile(TempFileNameSource);
		}
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static void DeleteHTMLResults(void)                                               //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static void DeleteHTMLResults(void)
{
 if ((ExecutionMode==LOAD_IN_IE) || 
	 (ExecutionMode==PARAMS_IN_IE_PROPBAG)||
	 (ExecutionMode==PARAMS_IN_IE_BINARY_SCAN))
    {
		 char TempFileNameSource[MAX_PATH];
         strcpy(TempFileNameSource,lpPathBuffer);
		 if (ExecutionMode==LOAD_IN_IE)
		  strcat(TempFileNameSource,IELOAD_HTML);
		 else strcat(TempFileNameSource,PARAMS_HTML);
		  DeleteTempResultsFile(TempFileNameSource);
	}
}
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static void DeleteTempResultsFile(char *FileName)                                 //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static void DeleteTempResultsFile(char *FileName)
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
////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// static int AppendFile(HANDLE hAppend, char *FileNameToAppend)                     //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int AppendFile(HANDLE hAppend, char *FileNameToAppend)    
{
HANDLE hFile;
DWORD  dwBytesRead, dwBytesWritten, RetryCount=0;

BYTE   buff[4096];
int    RetVal=SUCCESS;
if (hAppend == INVALID_HANDLE_VALUE) return(APPEND_FILE_FAILED);

while(1)
{
 hFile = CreateFile(FileNameToAppend,// File name
           GENERIC_READ,             // open for reading
           0,                        // do not share
           NULL,                     // no security
           OPEN_EXISTING,            // existing file only
           FILE_ATTRIBUTE_NORMAL,    // normal file
           NULL);                    // no attr. template


 if (hFile == INVALID_HANDLE_VALUE)
 {
  if (GetLastError()==ERROR_SHARING_VIOLATION)
  {
   if (RetryCount<5)
	{
     RetryCount++;
     Sleep(2000);
	 continue;
	}
	else
	{
 	 printf("Append Open File Failed %d\n",GetLastError());
     return(APPEND_FILE_FAILED);
	}
  }
  else
  {
 	 printf("Append Open File Failed %d\n",GetLastError());
     return(APPEND_FILE_FAILED);
  }
 }
 else break;
}

do
{
  if (ReadFile(hFile, buff, sizeof(buff), &dwBytesRead, NULL))
  {
    if (WriteFile(hAppend, buff, dwBytesRead, &dwBytesWritten, NULL)==0)
	{
     RetVal=APPEND_FILE_FAILED;
	}
  }
} while (dwBytesRead == sizeof(buff));

 CloseHandle(hFile);
 return(RetVal);
}

////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// void WriteTextToLog(char *Text)                             //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////

static void WriteTextToLog(char *Text)
{
   DWORD BytesToWrite=(DWORD)strlen(Text);
   DWORD BytesWritten=0;
   
   while (BytesToWrite)
   {
     if(WriteFile(hFile, Text, BytesToWrite, &BytesWritten, 0)==0)
	 {
      printf("File write error\n");
	  return;
	 }
	Text+= BytesWritten;
    BytesToWrite-=BytesWritten;
   }
}

////////////////////////////////////////////////////////////////////////////////////////
///                                                                                   //
/// int TestCOMObject(COM_ObjectInfoType *COM_ObjectInfo,char *LogFile)               //
///                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
static int TestCOMObject(COM_ObjectInfoType *COM_ObjectInfo,char *LogFile)
{
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    int                 ExitCode;
    char                CommandLine[1024+512];
	bool                ReTry;
    HANDLE              Handles[2];
    DWORD               WaitResult;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
    if (ExecutionMode==GEN_INTERFACE_LISTINGS)
 	  sprintf(CommandLine,"%s -g -c %ws -o %s",TESTANDREPORT,COM_ObjectInfo->CLSID_Str_Wide,LogFile);
    else if (ExecutionMode==LOAD_IN_IE)
	  sprintf(CommandLine,"%s -l -c %ws -o %s",TESTANDREPORT,COM_ObjectInfo->CLSID_Str_Wide,LogFile);
    else if (ExecutionMode==PARAMS_IN_IE_PROPBAG)
	  sprintf(CommandLine,"%s -p -c %ws -o %s",TESTANDREPORT,COM_ObjectInfo->CLSID_Str_Wide,LogFile);
    else if (ExecutionMode==PARAMS_IN_IE_BINARY_SCAN)
	  sprintf(CommandLine,"%s -s -c %ws -o %s",TESTANDREPORT,COM_ObjectInfo->CLSID_Str_Wide,LogFile);
    else if (ExecutionMode== PRINT_COM_OBJECT_INFO)
	  sprintf(CommandLine,"%s -i -c %ws -o %s",TESTANDREPORT,COM_ObjectInfo->CLSID_Str_Wide,LogFile);
	else 
	  sprintf(CommandLine,"%s -t -c %ws -o %s",TESTANDREPORT,COM_ObjectInfo->CLSID_Str_Wide,LogFile);
    // Start the child process. 
    if( !CreateProcess( 
		TESTANDREPORT,    // module name 
        CommandLine,        // Command line. 
        NULL,             // Process handle not inheritable. 
        NULL,             // Thread handle not inheritable. 
        FALSE,            // Set handle inheritance to FALSE. 
        0,                // No creation flags. 
        NULL,             // Use parent's environment block. 
        NULL,             // Use parent's starting directory. 
        &si,              // Pointer to STARTUPINFO structure.
        &pi )             // Pointer to PROCESS_INFORMATION structure.
    ) 
    {
        printf( "CreateProcess failed (%d).\n", GetLastError() );
        return(CREATE_PROCESS_FAILED);
    }
    Handles[0]=pi.hProcess;
    Handles[1]=TestHarnessKillEvent;
    // Wait until child process exits.
	ReTry=true;
	do
	{ 
	 WaitResult=WaitForMultipleObjects(2,Handles,false,COM_OBJECT_TEST_TIME_LIMIT_IN_SECONDS*1000);
	 if ((WaitResult==WAIT_TIMEOUT)||(WaitResult==(WAIT_OBJECT_0+1)))
	   {
		if (WaitResult==(WAIT_OBJECT_0+1)) 
		   {
            TerminateProcess(pi.hProcess,USER_ABORT);
            CloseHandle( pi.hProcess );
            CloseHandle( pi.hThread );
			KillProcessesNotInSnapShot(SnapShotOfProcesses);
			return(USER_ABORT);
		   }
		else
		{
		 printf("COM Object Hung During Test - Terminating Test Process\n");
         TerminateProcess(pi.hProcess,COM_OBJECT_OPERATION_HUNG);
         CloseHandle( pi.hProcess );
         CloseHandle( pi.hThread );
		 Sleep(1000);
		 KillProcessesNotInSnapShot(SnapShotOfProcesses);
		 return(COM_OBJECT_OPERATION_HUNG);
		}
	   }
	 ReTry=false;
	} while(ReTry);
 
    GetExitCodeProcess(pi.hProcess, (LPDWORD )&ExitCode);
    if (ExitCode==BUFFER_OVERRUN_FAULT_CRT_GENERATED)
          ExitCode=BUFFER_OVERRUN_FAULT;
     //printf("exit code = 0x%08X %d\n", ExitCode,ExitCode);

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
	KillProcessesNotInSnapShot(SnapShotOfProcesses);
    return(ExitCode);
}
