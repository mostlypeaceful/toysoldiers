#ifndef __tSharedStringBase__
#define __tSharedStringBase__

#ifdef target_tools
#	define sig_sharedstring_thread_safe
#endif//target_tools

namespace Sig
{
	class tSharedStringTable;
	class tSharedStringBase;
	typedef tSharedStringBase* (*tNewSharedStringInstance)( const char* str );

	class base_export tSharedStringBase : public tUncopyable, public tThreadSafeRefCounter
	{
	private:
		tDynamicArray<char> mCharBuffer;

	public:
		///
		/// \brief Access the raw c-style null-terminated char array pointer.
		/// \brief Note it is preferable to use fLength( ) than to access the
		/// raw pointer and call strlen( ), as fLength( ) return a pre-computed length.
		inline const char*	fCStr( ) const { return mCharBuffer.fBegin( ); }

		///
		/// \brief Query for the length of the string; note this is constant time, as
		/// the length is stored with the string.
		inline u32			fLength( ) const { return mCharBuffer.fCount( ) - 1; }

		///
		/// \brief Robust way to determine whether a non zero length string exists
		inline b32			fExists( ) const { return fLength( ) > 0; }

	protected:
		explicit tSharedStringBase( const char* str );
		void fOnDestroy( tSharedStringTable& stringTable );
	};
}

#endif//__tSharedStringBase__
