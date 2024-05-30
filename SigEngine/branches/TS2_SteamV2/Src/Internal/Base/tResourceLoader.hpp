#ifndef __tResourceLoader__
#define __tResourceLoader__
#include "tGenericBuffer.hpp"

namespace Sig
{
	class tResource;

	///
	/// \brief Smart pointer type for resources.
	define_smart_ptr( base_export, tRefCounterPtr, tResource );

	///
	/// \brief TODO document
	class /*base_export*/ tResourceLoader : public tRefCounter
	{
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

		Memory::tAllocStamp			fMakeStamp( ) const;

	protected:

		inline const tResourcePtr&	fGetResource( ) const { return mResource; }
		inline b32					fGetCancel( ) const { return mCancel; }

		void				fSetFileTimeStamp( u64 timeStamp );
		void				fSetResourceBuffer( tGenericBuffer* newDerivedBuffer );
		tGenericBuffer*		fGetResourceBuffer( );
		void				fSetSelfOnResource( );

	};

	typedef tResourceLoader* tResourceLoaderPtr;

}

#endif//__tResourceLoader__
