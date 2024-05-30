using System.Collections;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace GetLatest
{
    // http://groups.google.com/group/microsoft.public.dotnet.framework/msg/4f423c3d4c59b627
    public class ProcessUtility
    {
        public static void fKillProcessTree( int processToKillId )
        {
            // Kill the children
            foreach( int childProcessId in fGetChildProcessIds( processToKillId ) )
            {
                using( Process child = Process.GetProcessById( childProcessId ) )
                    child.Kill( );
            }

            // Kill the parent
            using( Process thisProcess = Process.GetProcessById( processToKillId ) )
                thisProcess.Kill( );
        }

        public static int fGetParentProcessId( int processId )
        {
            int ParentID = 0;

            int hProcess = OpenProcess( eDesiredAccess.PROCESS_QUERY_INFORMATION, false, processId );
            if( hProcess != 0 )
            {
                try
                {
                    PROCESS_BASIC_INFORMATION pbi = new PROCESS_BASIC_INFORMATION( );
                    int pSize = 0;
                    if( NtQueryInformationProcess( hProcess, PROCESSINFOCLASS.ProcessBasicInformation, ref pbi, pbi.Size, ref pSize ) != -1 )
                        ParentID = pbi.InheritedFromUniqueProcessId;
                }
                finally
                {
                    CloseHandle( hProcess );
                }
            }

            return ( ParentID );
        }

        public static int[ ] fGetChildProcessIds( int parentProcessId )
        {
            ArrayList myChildren = new ArrayList( );

            foreach( Process proc in Process.GetProcesses( ) )
            {
                int currentProcessId = proc.Id;
                proc.Dispose( );

                if( parentProcessId == fGetParentProcessId( currentProcessId ) )
                {
                    // Add this one
                    myChildren.Add( currentProcessId );

                    // Add any of its children
                    myChildren.AddRange( fGetChildProcessIds( currentProcessId ) );
                }
            }

            return ( int[ ] ) myChildren.ToArray( typeof( int ) );
        }

        [DllImport( "KERNEL32.DLL" )]
        private static extern int OpenProcess( eDesiredAccess dwDesiredAccess, bool bInheritHandle, int dwProcessId );

        [DllImport( "KERNEL32.DLL" )]
        private static extern int CloseHandle( int hObject );

        [DllImport( "NTDLL.DLL" )]
        private static extern int NtQueryInformationProcess( int hProcess, PROCESSINFOCLASS pic, ref PROCESS_BASIC_INFORMATION pbi, int cb, ref int pSize );

        private enum PROCESSINFOCLASS : int
        {
            ProcessBasicInformation = 0,
            ProcessQuotaLimits,
            ProcessIoCounters,
            ProcessVmCounters,
            ProcessTimes,
            ProcessBasePriority,
            ProcessRaisePriority,
            ProcessDebugPort,
            ProcessExceptionPort,
            ProcessAccessToken,
            ProcessLdtInformation,
            ProcessLdtSize,
            ProcessDefaultHardErrorMode,
            ProcessIoPortHandlers,
            ProcessPooledUsageAndLimits,
            ProcessWorkingSetWatch,
            ProcessUserModeIOPL,
            ProcessEnableAlignmentFaultFixup,
            ProcessPriorityClass,
            ProcessWx86Information,
            ProcessHandleCount,
            ProcessAffinityMask,
            ProcessPriorityBoost,
            MaxProcessInfoClass
        };

        [StructLayout( LayoutKind.Sequential )]
        private struct PROCESS_BASIC_INFORMATION
        {
            public int ExitStatus;
            public int PebBaseAddress;
            public int AffinityMask;
            public int BasePriority;
            public int UniqueProcessId;
            public int InheritedFromUniqueProcessId;

            public int Size
            {
                get { return ( 6 * 4 ); }
            }
        };

        private enum eDesiredAccess : int
        {
            DELETE = 0x00010000,
            READ_CONTROL = 0x00020000,
            WRITE_DAC = 0x00040000,
            WRITE_OWNER = 0x00080000,
            SYNCHRONIZE = 0x00100000,
            STANDARD_RIGHTS_ALL = 0x001F0000,

            PROCESS_TERMINATE = 0x0001,
            PROCESS_CREATE_THREAD = 0x0002,
            PROCESS_SET_SESSIONID = 0x0004,
            PROCESS_VM_OPERATION = 0x0008,
            PROCESS_VM_READ = 0x0010,
            PROCESS_VM_WRITE = 0x0020,
            PROCESS_DUP_HANDLE = 0x0040,
            PROCESS_CREATE_PROCESS = 0x0080,
            PROCESS_SET_QUOTA = 0x0100,
            PROCESS_SET_INFORMATION = 0x0200,
            PROCESS_QUERY_INFORMATION = 0x0400,
            PROCESS_ALL_ACCESS = SYNCHRONIZE | 0xFFF
        }
    }
}