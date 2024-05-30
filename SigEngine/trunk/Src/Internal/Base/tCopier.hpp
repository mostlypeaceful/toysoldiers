#ifndef __tCopier__
#define __tCopier__

namespace Sig
{
	/// \brief Copier class provides compile-time identification of built-in versus non-built-in
	/// types; in the case of built-in types, an optimized code-path is selected (again, at compile-time,
	/// using templates) to do the copying. This generic template provides the non-optimized (but
	/// safe for all object-types) code-path.
	template<class t, bool isBuiltInType = ( tIsBuiltInType<t>::cIs || tIsPointer<t>::cIs )>
	class tCopier
	{
	public:

		static void inline fCopy( t* dst, const t* src, u32 numItems )
		{
			for( u32 i = 0; i < numItems; ++i )
				dst[i] = src[i];
		}

		static void inline fCopyOverlapped( t* dst, const t* src, u32 numItems )
		{
			// If the src is at or after the dst we can copy forwards
			if( fPtrDiff( src, dst ) >= 0 ) 
				fCopy( dst, src, numItems );

			// Otherwise copy backwards
			else
			{
				for( s32 i = numItems - 1; i >= 0; --i )
					dst[i] = src[i];
			}
		}

		static void inline fAssign( t* dst, const t& object, u32 numItems )
		{
			for( u32 i = 0; i < numItems; ++i )
				dst[i] = object;
		}

		static void inline fAssignToDestroy( t* dst, const t& object, u32 numItems )
		{
			fAssign( dst, object, numItems );
		}
	};

	/// \brief This is the template specialization providing the optimized code path for built-in types.
	/// The reason this code-path isn't chosen for non-built in types is because they may
	/// have non-trivial constructurs/destructors/assignment operators.
	template<class t>
	class tCopier<t,true>
	{
	public:

		static void inline fCopy( t* dst, const t* src, u32 numItems )
		{
			fMemCpy( dst, src, numItems * sizeof(t) );
		}

		static void inline fCopyOverlapped( t* dst, const t* src, u32 numItems )
		{
			fMemMove( dst, src, numItems * sizeof(t) );
		}

		static void inline fAssign( t* dst, const t& object, u32 numItems )
		{
			for( u32 i = 0; i < numItems; ++i )
				dst[i] = object;
		}

		static void inline fAssignToDestroy( t* dst, const t& object, u32 numItems )
		{
			// nothing to do, no destructors need to be called
		}
	};
}

#endif //ndef __tCopier__
