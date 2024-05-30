#ifndef __Debug_DumpExceptions__
#define __Debug_DumpExceptions__

namespace Sig { namespace Debug
{
#if defined( platform_metro )
	void base_export fDumpException( const char* preamble, Platform::COMException^ ex  );
	void base_export fDumpException( const char* preamble, Platform::Exception^ ex  );
#endif
}}

#endif //ndef __Debug_DumpExceptions__
