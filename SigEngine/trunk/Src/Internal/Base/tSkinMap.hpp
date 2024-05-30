#ifndef __tSkinMap__
#define __tSkinMap__
#include "tAnimatedSkeleton.hpp"
#include "Memory/tPool.hpp"

namespace Sig
{
	class tMesh;

	class base_export tSkinMap : public tRefCounter
	{
		debug_watch( tSkinMap );
		define_class_pool_new_delete( tSkinMap, 32 );
	private:
		Anim::tAnimatedSkeletonPtr	mAnimatedSkeleton;
		tDynamicArray< u32 >	mMasterBoneIndices;
		b16						mValid;
		b16						mInOrder;
	public:
		tSkinMap( Anim::tAnimatedSkeleton& animatedSkel, const tMesh* mesh );
		explicit tSkinMap( const tMesh* mesh );

		b32 fValid( ) const { return mValid; }
		b32 fInOrder( ) const { return mInOrder; }

		typedef tFixedArray< Math::tMat3f, 256 > tScratchMatrixPalette;

		///
		/// \brief Fills the scratch palette so the bone xforms can be sent to the gpu (in shader constants).
		void fFillScratchMatrixPalette( tScratchMatrixPalette& matPaletteOut, u32& numBonesOut ) const;

		Math::tMat3f* fBegin( ) const;
		u32 fCount( ) const;
	};
	
	define_smart_ptr( base_export, tRefCounterPtr, tSkinMap );
}

#endif//__tSkinMap__

