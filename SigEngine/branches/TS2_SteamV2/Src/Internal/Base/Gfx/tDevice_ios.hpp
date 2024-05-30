#if defined( platform_ios )
#ifndef __tDevice_ios__
#define __tDevice_ios__
#ifndef __tDevice__
#error This file must be included from tDevice.hpp!
#endif//__tDevice__

namespace Sig { namespace Gfx
{
	struct tDeviceCaps;
	struct tScreenCreationOptions;

	class base_export tDevice : public tDevicePlatformBase
	{
	public:
		explicit tDevice( const tScreenCreationOptions& opts );
		~tDevice( );
		inline b32 fRequiresDeviceReset( ) const { return false; }

	private:
		void fSetDefaultState( );
	};

}}


#endif//__tDevice_ios__
#endif//#if defined( platform_ios )
