#ifndef __tDeviceResource__
#define __tDeviceResource__

namespace Sig { namespace Gfx
{
	class tDevice;
	class tDevicePlatformBase;

	class base_export tDeviceResource
	{
		friend class tDevicePlatformBase;
	private:
		tDevice* mDevicePtr;
	public:
		static tGrowableArray< std::string > gLogs;
		static void fLogAlloc( std::string tag, void* addr );
		static void fRemAlloc( std::string tag, void* addr );
		static void fDumpLogs( );

		tDeviceResource( );
		virtual ~tDeviceResource( );
		tDevice* fDevice( ) const { return mDevicePtr; }
		void fRegisterWithDevice( tDevice* device );
		void fUnregisterWithDevice( );
		virtual void fOnDeviceLost( tDevice* device ) = 0;
		virtual void fOnDeviceReset( tDevice* device ) = 0;
	};

}}

#endif//__tDeviceResource__
