//------------------------------------------------------------------------------
// \file tSkeletonFile.hpp - 02 Jul 2008
// \author Max Wagner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tSkeletonFile__
#define __tSkeletonFile__
#include "tHashTable.hpp"

namespace Sig
{
	///
	/// \brief Building block of tSkeletonFile (represents a single joint transform in an animated skeleton;
	/// this type however provides just the non-animated data associated with a bone/joint).
	class base_export tBone
	{
		declare_reflector( );
	public:
		typedef tDynamicArray< u32 > tChildList;
	public:
		s32						mMasterIndex;
		tLoadInPlaceStringPtr*	mName;
		s32						mParent;
		Math::tMat3f			mRefPose;
		Math::tMat3f			mRefPoseInv;
		Math::tPRSXformf		mRefLocalPose;
		tChildList				mChildren;
	public:
		tBone( );
		tBone( tNoOpTag );
	};

	///
	/// \class tSkeletonMap
	/// \brief Provides information for mapping from a different skeleton
	/// to the skeleton file containing this map
	class base_export tSkeletonMap
	{
		declare_reflector( );
	public:

		static const u32 cInvalidIndex = ~0;

		struct tBoneMap
		{
			declare_reflector( );

			tBoneMap( ) 
				: mTargetBoneIndex( cInvalidIndex )
				, mSrc2TgtXformIndex( cInvalidIndex ){ }
			tBoneMap( tNoOpTag ) { }
			
			u32 mTargetBoneIndex;
			u32 mSrc2TgtXformIndex;
		};

	public:

		tLoadInPlaceStringPtr*				mSrcSkeletonPath;
		tDynamicArray< tBoneMap >			mSrc2TgtBones; // One for each src bone
		tDynamicArray< Math::tPRSXformf >	mSrc2TgtXforms;

	public:

		tSkeletonMap( );
		tSkeletonMap( tNoOpTag );

	};

	///
	/// \brief Encapsulates a single referenced skeleton structure; this type supplies only the structural
	/// information of the underlying bones (parent/child hierarchy, reference pose transforms), with no
	/// animation data.
	class base_export tSkeletonFile : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		declare_lip_version( );
		implement_rtti_serializable_base_class(tSkeletonFile, 0x2F5D3B8F);
	public:
		static const char*	fGetFileExtension( );
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathML2B( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathB2ML( path ); }
	public:
		typedef tHashTable<tHashTablePtrInt, const tBone*, tHashTableNoResizePolicy>		tBoneMap;
		typedef tLoadInPlaceRuntimeObject<tBoneMap>											tBoneMapStorage;
		typedef tDynamicArray< tBone >														tMasterBoneArray;
		typedef tDynamicArray< u16 >														tPreOrderTraversal;
		typedef tDynamicArray< tSkeletonMap >												tSkeletonMapArray;
	public:
		tBone				mReferenceFrame;
		tMasterBoneArray	mMasterBoneList;
		tBoneMapStorage		mBonesByName;
		tPreOrderTraversal	mPreOrderTraversal;
		tSkeletonMapArray	mSkeletonMaps;
		s32					mRoot;
	public:
		tSkeletonFile( );
		tSkeletonFile( tNoOpTag );
		~tSkeletonFile( );

		inline const tBone& fBone( u32 masterBoneIndex ) const { return mMasterBoneList[ masterBoneIndex ]; }
		inline const tBone& fParent( u32 masterBoneIndex ) const { return mMasterBoneList[ mMasterBoneList[ masterBoneIndex ].mParent ]; }
		inline const tBoneMap& fGetBoneMap( ) const { return mBonesByName.fTreatAsObject( ); }
		const tBone* fFindBone( const tStringPtr& name ) const { const tBone** find = fGetBoneMap( ).fFind( name.fGetHashValue( ) ); return find ? *find : 0; }
		const tBone* fFindBone( const tLoadInPlaceStringPtr* name ) const { return fFindBone( name->fGetStringPtr( ) ); }
		const tSkeletonMap* fFindSkeletonMap(  const tStringPtr & skeletonPath  ) const;

		virtual void fOnFileLoaded( const tResource& ownerResource );
		virtual void fOnFileUnloading( const tResource& ownerResource );
	};

} // ::Sig

#endif//__tSkeletonFile__
