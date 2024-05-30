#include "BasePch.hpp"
#include "tTextureReference.hpp"
#include "tResourceDepot.hpp"


namespace Sig { namespace Gfx
{

	tTextureReference::tTextureReference( )
		: mLip( 0 )
		, mDynamic( 0 )
		, mRaw( 0 )
		, mFilterMode( tTextureFile::cFilterModeWithMip )
		, mAddressModeU( tTextureFile::cAddressModeWrap )
		, mAddressModeV( tTextureFile::cAddressModeWrap )
		, mAddressModeW( tTextureFile::cAddressModeWrap )
	{
	}
	tTextureReference::tTextureReference( tNoOpTag )
	{
	}
	tTextureReference::tTextureReference( const tTextureReference& other )
	{
		fSafeCopy( other );
	}
	tTextureReference& tTextureReference::operator=( const tTextureReference& other )
	{
		if( this != &other )
		{
			fSafeDestroy( );
			fSafeCopy( other );
		}
		return *this;
	}
	tTextureReference::~tTextureReference( )
	{
		fSafeDestroy( );
	}
	void tTextureReference::fSetLoadInPlace( tLoadInPlaceResourcePtr* lip )
	{
		fSafeDestroy( );
		mLip = lip;
	}
	void tTextureReference::fSetDynamic( tResource* dynamic )
	{
		fSafeDestroy( );
		mDynamic = dynamic;
		if( mDynamic )
			mDynamic->fAddRef( );
	}
	void tTextureReference::fSetRaw( tPlatformHandle raw )
	{
		fSafeDestroy( );
		mRaw = raw;
#if defined( platform_xbox360 ) || defined( platform_pcdx9 )
		if( raw )
		{
			IDirect3DBaseTexture9 * texture = (IDirect3DBaseTexture9 *)raw;
			texture->AddRef( );
		}
#else
		log_warning( "Current platform raw texture handles not reference counted by tTextureReference" );
#endif

	}
	void tTextureReference::fSetDynamic( tResourceDepot& depot, const tResourceId& rid )
	{
		tResourcePtr r = depot.fQuery( rid );
		fSetDynamic( r.fGetRawPtr( ) );
	}
	void tTextureReference::fSetSamplingModes( 
		tFilterMode filterMode, 
		tAddressMode addrModeAll )
	{
		fSetSamplingModes( filterMode, addrModeAll, addrModeAll, addrModeAll );
	}
	void tTextureReference::fSetSamplingModes( 
		tFilterMode filterMode, 
		tAddressMode addrModeU ,
		tAddressMode addrModeV ,
		tAddressMode addrModeW )
	{
		mFilterMode		= filterMode;
		mAddressModeU	= addrModeU;
		mAddressModeV	= addrModeV;
		mAddressModeW	= addrModeW;
	}

	static void fLogTextureError( const tResource & texture )
	{
		const char * loadState = "unloaded";
		if( texture.fLoaded( ) )
			loadState = "loaded";
		else if( texture.fLoading( ) )
			loadState = "loading";
		else if( texture.fLoadFailed( ) )
			loadState = "load failed";

		log_warning( "Texture file " << texture.fGetPath( ) << " - state:" << loadState );
	}

	b32 tTextureReference::fApply( const tDevicePtr& device, u32 slot ) const
	{
		tPlatformHandle raw = 0;

		if( mDynamic )
		{
			const tTextureFile* t = mDynamic->fCast<tTextureFile>( ); 
#ifdef sig_logging
			if( !t )
				fLogTextureError( *mDynamic );
#endif
			sigassert( t );
			if( !t )
				return false;
			raw = t->fGetPlatformHandle( );
		}
		else if( mLip )
		{
			const tTextureFile* t = mLip->fGetResourcePtr( )->fCast<tTextureFile>( ); 
#ifdef sig_logging
			if( !t )
				fLogTextureError( *mLip->fGetResourcePtr( ) );
#endif
			sigassert( t );
			if( !t )
				return false;
			raw = t->fGetPlatformHandle( );
		}
		else if( mRaw )
			raw = mRaw;
		else
			return false;

		tTextureFile::fApply( raw, device, slot, mFilterMode, ( tAddressMode )mAddressModeU, ( tAddressMode )mAddressModeV, ( tAddressMode )mAddressModeW );
		return true;
	}
	const tTextureFile* tTextureReference::fGetTextureFile( ) const
	{
		if( mDynamic )
			return mDynamic->fCast< tTextureFile >( );
		if( mLip && mLip->fGetResourcePtr( ) )
			return mLip->fGetResourcePtr( )->fCast< tTextureFile >( );
		return 0;
	}
	const tFilePathPtr& tTextureReference::fGetTexturePath( ) const
	{
		if( mDynamic )
			return mDynamic->fGetPath( );
		if( mLip && mLip->fGetResourcePtr( ) )
			return mLip->fGetResourcePtr( )->fGetPath( );
		return tFilePathPtr::cNullPtr;
	}
	b32 tTextureReference::fResourceEquals( const tTextureReference & other ) const
	{
		if( mDynamic ) 
			return mDynamic == other.mDynamic;

		if( mRaw )
			return mRaw == other.mRaw;

		if( mLip ) 
			return ( other.mLip && mLip->fGetResourceId( ) == other.mLip->fGetResourceId( ) );

		return other.fNull( );
	}
	void tTextureReference::fSafeCopy( const tTextureReference& other )
	{
		mLip			= other.mLip;
		mDynamic		= other.mDynamic;
		mRaw			= other.mRaw;
		mAddressModeU	= other.mAddressModeU;
		mAddressModeV	= other.mAddressModeV;
		mAddressModeW	= other.mAddressModeW;
		mFilterMode		= other.mFilterMode;
		sigassert( 
			( !mLip && !mDynamic && !mRaw ) ||
			(  mLip && !mDynamic && !mRaw ) ||
			( !mLip &&  mDynamic && !mRaw ) ||
			( !mLip && !mDynamic &&  mRaw )  );
		if( mDynamic )
			mDynamic->fAddRef( );

#if defined( platform_xbox360 ) || defined( platform_pcdx9 )
		if( mRaw )
		{
			IDirect3DBaseTexture9 * texture = (IDirect3DBaseTexture9 *)mRaw;
			texture->AddRef( );
		}
#else
		log_warning( "Current platform raw texture handles not reference counted by tTextureReference" );
#endif

	}
	void tTextureReference::fSafeDestroy( )
	{
		if( mDynamic )
		{
			mDynamic->fDecRef( );
			if( mDynamic->fRefCount( ) == 0 )
				delete mDynamic;
		}

#if defined( platform_xbox360 ) || defined( platform_pcdx9 )
		if( mRaw )
		{
			IDirect3DBaseTexture9 * texture = (IDirect3DBaseTexture9 *)mRaw;
			texture->Release( );
			mRaw = NULL;
		}
#else
		mRaw			= 0;
		log_warning( "Current platform raw texture handles not reference counted by tTextureReference" );
#endif

		mLip			= 0;
		mDynamic		= 0;
		mAddressModeU	= tTextureFile::cAddressModeWrap;
		mAddressModeV	= tTextureFile::cAddressModeWrap;
		mAddressModeW	= tTextureFile::cAddressModeWrap;
		mFilterMode		= tTextureFile::cFilterModeWithMip;
	}
	void tTextureReference::fSetAddressModesScript( u32 all )
	{
		this->fSetSamplingModes( mFilterMode, (tAddressMode)all );
	}
	void tTextureReference::fSetAddressModesScript( u32 u, u32 v, u32 w )
	{
		this->fSetSamplingModes( mFilterMode, (tAddressMode)u, (tAddressMode)v, (tAddressMode)w );
	}

	void tTextureReference::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tTextureReference, Sqrat::NoCopy<tTextureReference> > classDesc( vm.fSq( ) );

		classDesc
			.Overload<void (tTextureReference::*)(u32)>( "SetAddressModes", &tTextureReference::fSetAddressModesScript )
			.Overload<void (tTextureReference::*)(u32, u32, u32)>( "SetAddressModes", &tTextureReference::fSetAddressModesScript )
			;

		vm.fNamespace(_SC("Gfx")).Bind(_SC("TextureReference"), classDesc);

		vm.fConstTable( ).Const(_SC("ADDRESS_MODE_WRAP"),			tTextureFile::cAddressModeWrap);
		vm.fConstTable( ).Const(_SC("ADDRESS_MODE_CLAMP"),			tTextureFile::cAddressModeClamp);
		vm.fConstTable( ).Const(_SC("ADDRESS_MODE_MIRROR"),			tTextureFile::cAddressModeMirror);
		vm.fConstTable( ).Const(_SC("ADDRESS_MODE_BORDER_WHITE"),	tTextureFile::cAddressModeBorderWhite);
		vm.fConstTable( ).Const(_SC("ADDRESS_MODE_BORDER_BLACK"),	tTextureFile::cAddressModeBorderBlack);
	}

}}

