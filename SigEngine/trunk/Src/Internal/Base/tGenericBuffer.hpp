#ifndef __tGenericBuffer__
#define __tGenericBuffer__

namespace Sig
{

	///
	/// \brief Abstract base class interface for generic buffer types.
	class base_export tGenericBuffer : public tUncopyable, public tRefCounter
	{
	public:

		virtual ~tGenericBuffer( ) { }

		///
		/// \brief Allocate the underlying memory for the resource.
		virtual void				fAlloc( u32 numBytes, const Memory::tAllocStamp& stamp ) = 0;

		///
		/// \brief Free the underlying memory for the resource.
		virtual void				fFree( ) = 0;

		///
		/// \brief Resizes the underlying memory allocation; copies data
		/// from old buffer into new buffer (up to the min of the new size and old size).
		virtual void				fResize( u32 newBufferSize ) = 0;

		///
		/// \brief Access the address of the resource. (non-const version)
		virtual Sig::byte*			fGetBuffer( ) = 0;

		///
		/// \brief Access the address of the resource. (const version)
		virtual const Sig::byte*	fGetBuffer( ) const = 0;

		///
		/// \brief Find out how big the underlying resource is in bytes.
		virtual u32					fGetBufferSize( ) const = 0;
	};

	typedef tRefCounterPtr<tGenericBuffer> tGenericBufferPtr;

}

#endif//__tGenericBuffer__
