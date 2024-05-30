#include "BasePch.hpp"
#if defined( platform_metro )
#include "DumpExceptions.hpp"
#include "MetroUtil.hpp"
#include "Debug/tDebugger.hpp"

namespace Sig { namespace Debug
{
	void fDumpException( const char* preamble, Platform::COMException^ com )
	{
		ignore_unused(preamble);
		ignore_unused(com);

		log_warning( 0, preamble );
		log_warning( 0, "	Message: " << com->Message );
		log_warning( 0, "	HRESULT: " << std::hex << (u32)com->HResult << " (" << MetroUtil::fErrorCodeToString(com->HResult) << ")" );
		break_if_debugger();
	}

	void fDumpException( const char* preamble, Platform::Exception^ ex )
	{
		Platform::COMException^ com = dynamic_cast<Platform::COMException^>(ex);
		if( com )
		{
			fDumpException(preamble,com);
			return;
		}

		ignore_unused(preamble);
		ignore_unused(ex);

		log_warning( 0, preamble );
		log_warning( 0, "	Message: " << ex->Message );
		break_if_debugger();
	}
}}
#endif // defined( platform_metro )
