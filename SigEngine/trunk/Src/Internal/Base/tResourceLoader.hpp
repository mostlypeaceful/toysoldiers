#ifndef __tResourceLoader__
#define __tResourceLoader__
#include "tGenericBuffer.hpp"

namespace Sig
{
	class tResource;
	class tAsyncFileReader;

	///
	/// \brief Smart pointer type for resources.
	define_smart_ptr( base_export, tRefCounterPtr, tResource );

	///
	/// \brief TODO document
	class /*base_export*/ tResourceLoader : public tRefCounter
	{
		declare_uncopyable( tResourceLoader );
		define_dynamic_cast_base( tResourceLoader );

	private:
		tResourcePtr mResource;
		b32			 mCancel;
		
	public:

		enum tLoadResult { cLoadSuccess, cLoadFailure, cLoadCancel, cLoadPending };

		tResourceLoader( tResource* res );
		virtual ~tResourceLoader( ) { }

		virtual void				fInitiate( ) = 0;
		virtual void				fCancel( ) { mCancel = true; }
		virtual tLoadResult			fUpdate( ) = 0;
		virtual	s32					fGetLoadStage( ) { return 0; }

		Memory::tAllocStamp			fMakeStamp( u32 size ) const;

		inline b32					fGetCancel( ) const { return mCancel; }

	protected:

		inline const tResourcePtr&	fGetResource( ) const { return mResource; }

		void				fSetFileTimeStamp( u64 timeStamp );
		void				fSetResourceBuffer( tGenericBuffer* newDerivedBuffer );
		tGenericBuffer*		fGetResourceBuffer( );
		void				fSetSelfOnResource( );

	};

	typedef tResourceLoader* tResourceLoaderPtr;

}

#endif//__tResourceLoader__
