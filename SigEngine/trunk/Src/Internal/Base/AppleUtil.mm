#include "BasePch.hpp"
#if defined( platform_ios )
#include "AppleUtil.hpp"
#include <unistd.h>
#import <Foundation/Foundation.h>
//#import <Foundation/NSPathUtilities.h>

namespace Sig { namespace AppleUtil {
	void fGetCurrentDirectory( std::string& out ) {
		char buffer[256] = {0};
		char* result = getcwd( buffer, sizeof(buffer)-1 );
		sigassert(result);
		out = result;
	}
	void fSetCurrentDirectory( const char* path ) {
		int error = chdir(path);
		sigassert(!error);
	}
	void fGetApplicationDirectory( std::string& out ) {
		NSString *path = [[NSBundle mainBundle] bundlePath];
		out = [path cStringUsingEncoding:[NSString defaultCStringEncoding]];
	}
}}

#endif // defined( platform_ios )
