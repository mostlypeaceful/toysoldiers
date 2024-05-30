#ifndef __tLoadInPlaceResourceBuffer__
#define __tLoadInPlaceResourceBuffer__
#include "tGenericBuffer.hpp"
#include "Memory/tPool.hpp"

namespace Sig
{

	///
	/// \brief TODO document
	class base_export tLoadInPlaceResourceBuffer : public tGenericBuffer
	{
		debug_watch( tLoadInPlaceResourceBuffer );
		define_class_pool_new_delete( tLoadInPlaceResourceBuffer, 128 );
	private:
		Sig::byte* mBuffer;
		u32 mSize;
	public:
		tLoadInPlaceResourceBuffer( );
		virtual ~tLoadInPlaceResourceBuffer( );
		virtual void fAlloc( u32 numBytes, const Memory::tAllocStamp& stamp );
		virtual void fFree( );
		virtual void fResize( u32 newBufferSize );
		virtual Sig::byte* fGetBuffer( );
		virtual const Sig::byte* fGetBuffer( ) const;
		virtual u32 fGetBufferSize( ) const;
	};

}


#endif//__tLoadInPlaceResource__

