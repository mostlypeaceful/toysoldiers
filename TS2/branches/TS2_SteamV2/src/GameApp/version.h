#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

// YEAR
#define VERSION_MAJOR               15
// MONTH
#define VERSION_MINOR               3
// DAY
#define VERSION_REVISION            11
// BUILD NUMBER
#define VERSION_BUILD               0053
// CHANGELIST
#define VERSION_CHANGELIST          95217
 
#define VER_FILE_DESCRIPTION_STR    "Toy Soldiers: Complete Game Executable"
#define VER_FILE_VERSION            VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD
#define VER_FILE_VERSION_STR        STRINGIZE(VERSION_MAJOR)        \
                                    "." STRINGIZE(VERSION_MINOR)    \
                                    "." STRINGIZE(VERSION_REVISION) \
                                    "." STRINGIZE(VERSION_BUILD)
 
#define VER_PRODUCTNAME_STR         "Toy Soldiers: Complete"
#define VER_PRODUCT_VERSION         VER_FILE_VERSION
#define VER_PRODUCT_VERSION_STR     VER_FILE_VERSION_STR "(" STRINGIZE(VERSION_CHANGELIST) ")"
#define VER_ORIGINAL_FILENAME_STR   "Game.exe"
#define VER_INTERNAL_NAME_STR       VER_ORIGINAL_FILENAME_STR
#define VER_COPYRIGHT_STR           "Copyright (C) 2014 Microsoft Corporation"
#define VER_COMPANY_NAME_STR        "Signal Studios"

#ifdef _DEBUG
  #define VER_VER_DEBUG             VS_FF_DEBUG
#else
  #define VER_VER_DEBUG             0
#endif
 
#define VER_FILEOS                  VOS_NT_WINDOWS32
#define VER_FILEFLAGS               VER_VER_DEBUG
#define VER_FILETYPE                VFT_APP
