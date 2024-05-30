#ifndef __tTextureReference__
#define __tTextureReference__
#include "tTextureFile.hpp"

namespace Sig
{
	class tResource;
	class tResourceDepot;
	class tResourceId;
}

namespace Sig { namespace Gfx
{
	class tDevicePtr;

	class base_export tTextureReference
	{
		declare_reflector( );

	public:
		typedef tTextureFile::tAddressMode		tAddressMode;
		typedef tTextureFile::tFilterMode		tFilterMode;
		typedef tTextureFile::tPlatformHandle	tPlatformHandle;

	private:
		tLoadInPlaceResourcePtr*		mLip;
		tResource*						mDynamic;
		tPlatformHandle					mRaw;
		tEnum<tFilterMode,u8>			mFilterMode;
		u8								mAddressModeU;
		u8								mAddressModeV;
		u8								mAddressModeW;

	public:
		tTextureReference( );
		tTextureReference( tNoOpTag );
		tTextureReference( const tTextureReference& other );
		tTextureReference& operator=( const tTextureReference& other );
		~tTextureReference( );
		void fSetLoadInPlace( tLoadInPlaceResourcePtr* lip );
		void fSetDynamic( tResource* dynamic );
		void fSetDynamic( tResourceDepot& depot, const tResourceId& rid );
		void fSetRaw( tPlatformHandle raw );
		void fSetSamplingModes( 
			tFilterMode filterMode, 
			tAddressMode addrModeAll = tTextureFile::cAddressModeWrap );
		void fSetSamplingModes( 
			tFilterMode filterMode, 
			tAddressMode addrModeU,
			tAddressMode addrModeV,
			tAddressMode addrModeW );
		b32  fApply( const tDevicePtr& device, u32 slot ) const;
		tLoadInPlaceResourcePtr*	fGetLip( ) const { return mLip; }
		tResource*					fGetDynamic( ) const { return mDynamic; }
		tPlatformHandle				fGetRaw( ) const { return mRaw; }
		const tTextureFile*			fGetTextureFile( ) const;
		const tFilePathPtr&			fGetTexturePath( ) const;

		b32 fResourceEquals( const tTextureReference & other ) const;
		b32 fNull( ) const { return !mLip && !mDynamic && !mRaw; }

		tAddressMode fAddressModeU( ) const { return (tAddressMode)mAddressModeU; }
		tAddressMode fAddressModeV( ) const { return (tAddressMode)mAddressModeV; }
		tAddressMode fAddressModeW( ) const { return (tAddressMode)mAddressModeW; }

		static void fClearBoundTextures( const tDevicePtr& device, u32 begin, u32 end );


	private:
		void fSafeCopy( const tTextureReference& other );
		void fSafeDestroy( );

		void fSetAddressModesScript( u32 all );
		void fSetAddressModesScript( u32 u, u32 v, u32 w );


	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	};

}}


#endif//__tTextureReference__
