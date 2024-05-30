#ifndef __tSceneGraphFile__
#define __tSceneGraphFile__
#include "tEntityDef.hpp"

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

		tSceneLODSettings( ) { fZeroOut( this ); }
		tSceneLODSettings( tNoOpTag ) { }
	};

	///
	/// \class tTileSetChance
	/// \brief Contains a GUID and a chance weight for which tile
	/// set to pick for this pigment.
	struct base_export tPigmentChoice
	{
		declare_reflector( );
	public:
		u32 mGuid;
		f32 mChanceWeight;

		tPigmentChoice( ) : mGuid( 0 ), mChanceWeight( 1.f ) { }
		tPigmentChoice( tNoOpTag ) { }
	};

	struct base_export tPigment
	{
		declare_reflector( );
	public:
		u32 mGuid;
		tDynamicArray< tPigmentChoice > mPigmentChoices;

		tPigment( ) : mGuid( 0 ) { }
		tPigment( tNoOpTag ) : mPigmentChoices( tNoOpTag( ) ) { }
	};

	struct base_export tTileDef
	{
		declare_reflector( );
	public:
		//std::string mRelativePath; // Relative to the tile set's root which is relative to res.
		f32 mChanceWeight;

		tTileDef( ) : mChanceWeight( 1.f ) { }
		tTileDef( tNoOpTag ) { }
	};

	struct base_export tTileSet
	{
		declare_reflector( );
	public:
		u32 mGuid;
		//std::string mDir; // Root tile set dir relative to res.
		tDynamicArray< tTileDef > mTileDefs;

		tTileSet( ) : mGuid( 0 ) { }
		tTileSet( tNoOpTag ) : mTileDefs( tNoOpTag( ) ) { }
	};

	struct base_export tTilePackage
	{
		declare_reflector( );
	public:
		tDynamicArray< tPigment > mPigments;
		tDynamicArray< tTileSet > mTileSets;

		tTilePackage( ) { fZeroOut( this ); }
		tTilePackage( tNoOpTag ) : mPigments( tNoOpTag( ) ), mTileSets( tNoOpTag( ) ) { }
	};

	///
	/// \brief TODO document
	class base_export tSceneGraphFile : public tLoadInPlaceFileBase, public tEntityDefProperties
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tSceneGraphFile, 0x54EEDB93 );
	public:
		typedef tDynamicArray< tLoadInPlacePtrWrapper< tEntityDef > > tObjectArray;
	public:
		static const u32 cVersion;
	public:
		Math::tAabbf mBounds;
		Math::tMat3f mSkeletonBinding;
		Math::tMat3f mSkeletonBindingInv;
		tObjectArray mObjects;
		tSceneGraphDefaultLight* mDefaultLight;
		tSceneLODSettings* mLODSettings;
		tTilePackage* mTilePackage;
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
	public:
		tSceneGraphFile( );
		tSceneGraphFile( tNoOpTag );
		~tSceneGraphFile( );

		virtual void fOnFileLoaded( const tResource& ownerResource );
		virtual void fOnSubResourcesLoaded( const tResource& ownerResource );
		virtual void fOnFileUnloading( );

		void fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const;
	};

}

namespace Sig
{
	template<>
	class tResourceConvertPath<tSceneGraphFile>
	{
	public:
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathML2B( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathB2ML( path ); }
	};
}

#endif//__tSceneGraphFile__
