#include "BasePch.hpp"
#include "tVertexFormatVRam.hpp"

namespace Sig { namespace Gfx
{
	tVertexFormatVRam::tVertexFormatVRam( )
		: mPlatformHandle( 0 )
	{
	}

	tVertexFormatVRam::tVertexFormatVRam( tNoOpTag )
		: tVertexFormat( cNoOpTag )
	{
	}

	tVertexFormatVRam::tVertexFormatVRam( const tDevicePtr& device, const tVertexFormat& vtxFormat )
		: mPlatformHandle( 0 )
	{
		fAllocate( device, vtxFormat );
	}

	tVertexFormatVRam::tVertexFormatVRam( const tVertexFormat& vtxFormat )
		: mPlatformHandle( 0 )
	{
		tVertexFormat::operator=( vtxFormat );
	}

	tVertexFormatVRam::~tVertexFormatVRam( )
	{
		fDeallocate( );
	}

	tVertexFormatVRam::tVertexFormatVRam( const tVertexFormatVRam& other )
	{
		sigassert( !"not allowed" );
	}

	tVertexFormatVRam& tVertexFormatVRam::operator=( const tVertexFormatVRam& other )
	{
		sigassert( !"not allowed" );
		return *this;
	}

	void tVertexFormatVRam::fAllocate( const tDevicePtr& device, const tVertexFormat& vtxFormat )
	{
		fDeallocate( );
		tVertexFormat::operator=( vtxFormat );
		fAllocateInternal( device );
	}

	void tVertexFormatVRam::fPseudoAllocate( const tVertexFormat& vtxFormat )
	{
		fDeallocate( );
		tVertexFormat::operator=( vtxFormat );
	}

	void tVertexFormatVRam::fAllocateInPlace( const tDevicePtr& device )
	{
		fAllocateInternal( device );
	}

	void tVertexFormatVRam::fRelocateInPlace( ptrdiff_t delta )
	{
		tVertexFormat::fRelocateInPlace( delta );
	}

	void tVertexFormatVRam::fDeallocate( )
	{
		fDeallocateInternal( );
		mPlatformHandle = 0;
		tVertexFormat::operator=( tVertexFormat( ) );
	}

	void tVertexFormatVRam::fDeallocateInPlace( )
	{
		fDeallocateInternal( );
		mPlatformHandle = 0;
	}

	void tVertexFormatVRam::fReset( const tVertexElement* elems, u32 numElems )
	{
		tVertexFormat::fReset( elems, numElems );
	}

}}

