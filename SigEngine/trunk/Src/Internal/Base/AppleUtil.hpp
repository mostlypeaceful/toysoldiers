#ifndef __AppleUtil__
#define __AppleUtil__
#if defined( platform_ios )

namespace Sig { namespace AppleUtil {
	base_export void		fGetCurrentDirectory( std::string& out );
	base_export void		fSetCurrentDirectory( const char* path );
	base_export void		fGetApplicationDirectory( std::string& out );
}}

namespace Sig { namespace OsUtil {
	using namespace ::Sig::AppleUtil;
}}

#endif // defined( platform_ios )
#endif // __AppleUtil__
