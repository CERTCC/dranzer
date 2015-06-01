#ifndef _TESTERRORS_H_
#define _TESTERRORS_H_

#define PARAMS_HTML "IE_PARAMS_COM.html"
#define IELOAD_HTML "IeLoadCOM.html"
/*
============================================================
|                     ERORR INCLUDE                        |
|----------------------------------------------------------|
*/
typedef enum
  {
   ERROR_MIN                                   = -13,
   BUFFER_OVERRUN_FAULT                        = -13,
   FAILED_TO_CREATE_IE_TEST_FILE               = -12,
   FAILED_TO_ATTACH_DEBUGGER                   = -11,
   FAILED_TO_GET_PROCESS_ID                    = -10,
   INTERNET_EXPLORER_CRASHED                   = -9,
   FAILED_TO_START_BROWSER                     = -8,
   COM_OBJECT_EXECEPTION_OCCURRED              = -7,
   CANT_CREATE_TEST_RESULTS_FILE               = -6,
   USER_ABORT                                  = -5,
   CREATE_PROCESS_FAILED                       = -4,
   COM_OBJECTSAFETY_SET_INTERFACE_OPT_FAULT    = -3,
   COM_COCREATE_INSTANCE_IOBJECTSAFETY_FAULT   = -2,
   COM_OBJECT_OPERATION_HUNG                   = -1,
   SUCCESS                                     =  0,
   BAD_ARGUMENT_COUNT                          =  1,
   NULL_POINTER_ERROR                          =  2, 
   BUFFER_OVERRUN_FAULT_CRT_GENERATED          =  3,
   MULTIBYTE_TO_WIDE_CHAR_FAILED               =  4,
   CLSID_FROM_STRING_FAILED                    =  5,
   REG_OPEN_KEY_CLSID_CLSID_STRING_FAILED      =  6,
   STRING_FROM_CLSID_FAILED                    =  7,
   COM_OBJECT_NOT_SCRIPT_SAFE                  =  8,
   COM_OBJECT_NOT_SAFE_FOR_UNTRUSTED_DATA      =  9,
   QUERY_INTERFACE_FOR_IDISPATCH_EX_FAILED     =  10,
   QUERY_INTERFACE_FOR_IDISPATCH_FAILED        =  11,
   QUERY_INTERFACE_FOR_IPERSISTPROPERTYBAG_FAILED =12,
   GET_TYPE_INFO_FAILED                        =  13,
   MAX_RECURSIVE_LEVEL_REACHED                 =  14,
   GET_DOCUMENTATION_FAILED                    =  15,
   APPEND_FILE_FAILED                          =  16
 
  } ERROR_RETURNS;
/*
============================================================
|                     EXTERNALS                            |
|----------------------------------------------------------|
*/
extern char  * ErrorList[];

/*
============================================================
|                     PROTOTYPES                           |
|----------------------------------------------------------|
*/
                      
  char *ErrorString(int);
                      /*  Returns a pointer to a character string    */      
                      /*  describing the the error input.            */        
                      /*  A pointer is returned if the call succeeds */
                      /*  NULL is returned if the the error number   */
                      /*  is invalid.                                */  
/*
============================================================
|                 END ERORR INCLUDE                        |
|----------------------------------------------------------|
*/
#endif