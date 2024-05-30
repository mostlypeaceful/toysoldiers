#ifndef __tSceneGraphFile__
#define __tSceneGraphFile__
#include "tEntityDef.hpp"
#include "tEntityData.hpp"

namespace Sig
{

	///
	/// \brief TODO document
	struct base_export tSceneGraphDefaultLight
	{
		declare_reflector( );
	public:
		Math::tVec3f mFrontColor;
		Math::tVec3f mBackColor;
		Math::tVec3f mRimColor;
		Math::tVec3f mAmbientColor;
		Math::tVec3f mDirection;
		b32 mCastShadow;

		tSceneGraphDefaultLight( ) { fZeroOut( this ); }
		tSceneGraphDefaultLight( tNoOpTag ) { }
	};

	struct base_export tSceneLODSettings
	{
		declare_reflector( );
	public:
		u32 mFadeSetting;
		f32 mFadeOverride;
		f32 mLODMediumDistanceOverride;
		f32 mLODFarDistanceOverride;

		tSceneLODSettings( ) { fZeroOut( this ); }
		tSceneLODSettings( tNoOpTag ) { }
	};

	///
	/// \brief TODO document
	class base_export tSceneGraphFile : public tLoadInPlaceFileBase, public tEntityDefProperties
	{
		declare_reflector( );
		declare_lip_version( );
		implement_rtti_serializable_base_class(tSceneGraphFile, 0x54EEDB93);
	public:
		typedef tDynamicArray< tLoadInPlacePtrWrapper< tEntityDef > > tObjectArray;
	public:
		Math::tAabbf mBounds;
		Math::tMat3f mSkeletonBinding;
		Math::tMat3f mSkeletonBindingInv;
		tObjectArray mObjects;
		tSceneGraphDefaultLight* mDefaultLight;
		tSceneLODSettings* mLODSettings;
		tLoadInPlaceResourceId* mTilePackagePath;
		tLoadInPlaceRuntimeObject< tResourcePtr > mTilePackage;
		tEntityDataArray* mEntityData; //Supplemental "user" and tools data that needs to be serialized.

	public:
		static u32			fGetNumFileExtensions( );
		static const char*	fGetFileExtension( u32 i = 0 );
		static b32			fIsSceneGraphFile( const tFilePathPtr& path );
		static u32			fGetNumSigmlFileExtensions( );
		static const char*	fGetSigmlFileExtension( u32 i = 0 );
		static b32			fIsSigmlFile( const tFilePathPtr& path );
		static b32			fIsMshmlFile( const tFilePathPtr& path );
		static b32			fIsSigmlFileExclusive( const tFilePathPtr& path );
		static tFilePathPtr fSigmlPathToSigb( const tFilePathPtr& path );
		static tFilePathPtr fSigbPathToSigml( const tFilePathPtr& path );
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathML2B( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathB2ML( path ); }
	public:
		tSceneGraphFile( );
		tSceneGraphFile( tNoOpTag );
		~tSceneGraphFile( );

		virtual void fGatherRuntimeResources( tResource& ownerResource ) OVERRIDE; // Loads a tile package if no override is set.
		virtual void fOnFileLoaded( const tResource& ownerResource ) OVERRIDE;
		virtual void fOnSubResourcesLoaded( const tResource& ownerResource, b32 success ) OVERRIDE;
		virtual void fOnFileUnloading( const tResource& ownerResource ) OVERRIDE;

		void fCollectEntities( const tCollectEntitiesParams& params ) const;
	};

} // ::Sig

#endif//__tSceneGraphFile__
