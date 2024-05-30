#ifndef __tCodeResourceLoader__
#define __tCodeResourceLoader__
#include "tResource.hpp"

namespace Sig
{

	///
	/// \brief TODO document
	template<class tResourceType>
	class tCodeResourceBuffer : public tGenericBuffer
	{
		class tWrapper : public tRefCounter, public tResourceType { };
		tRefCounterPtr<tWrapper>	mResBuffer;

	public:
		virtual void				fAlloc( u32 numBytes ) { mResBuffer = fNewRefCounterPtr<tWrapper>( ); }
		virtual void				fFree( ) { mResBuffer.fRelease( ); }
		virtual Sig::byte*			fGetBuffer( ) { return ( Sig::byte* )mResBuffer.fGetRawPtr( ); }
		virtual const Sig::byte*	fGetBuffer( ) const { return ( Sig::byte* )mResBuffer.fGetRawPtr( ); }
		virtual u32					fGetBufferSize( ) const { return sizeof( tWrapper ); }
	};

	///
	/// \brief TODO document
	template<class tResourceType>
	class tCodeResourceLoader : public tResourceLoader
	{
	public:

		virtual void		fInitiate( tResource* res )
		{
			fSetResourceBuffer( res, NEW tCodeResourceBuffer<tResourceType>( ) );
			tGenericBuffer* resBuffer = fGetResourceBuffer( res );
			resBuffer->fAlloc( sizeof( tResourceType ) );
		}

		virtual void		fCancel( ) { }

		virtual tLoadResult	fUpdate( ) { return cLoadSuccess; }
	};

}


#endif//__tCodeResourceLoader__
