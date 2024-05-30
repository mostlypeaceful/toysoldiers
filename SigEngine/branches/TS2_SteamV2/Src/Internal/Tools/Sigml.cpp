#include "ToolsPch.hpp"
#include "Sigml.hpp"
#include "tSceneGraphFile.hpp"
#include "tProjectFile.hpp"
#include "tExporterToolbox.hpp"
#include "tSigmlConverter.hpp"
#include "tMesh.hpp"
#include "tMeshEntity.hpp"
#include "Scripts/tScriptFile.hpp"
#include "tSkeletonFile.hpp"
#include <limits>

// editor
#include "Editor/tEditableSgFileRefEntity.hpp"
#include "Editor/tEditablePropertyTypes.hpp"
#include "Editor/tEditablePropertyColor.hpp"

// from bullet
#include "btBulletCollisionCommon.h"

namespace Sig { namespace Sigml
{
	///
	/// \section Global functions.
	///

	u32 fGetNumFileExtensions( )
	{
		return tSceneGraphFile::fGetNumSigmlFileExtensions( );
	}

	const char* fGetFileExtension( u32 i )
	{
		return tSceneGraphFile::fGetSigmlFileExtension( i );
	}

	b32 fIsSigmlFile( const tFilePathPtr& path )
	{
		return tSceneGraphFile::fIsSigmlFile( path );
	}

	tFilePathPtr fSigmlPathToSigb( const tFilePathPtr& path )
	{
		return tSceneGraphFile::fSigmlPathToSigb( path );
	}

	tFilePathPtr fSigbPathToSigml( const tFilePathPtr& path )
	{
		return tSceneGraphFile::fSigbPathToSigml( path );
	}

	///
	/// \section tUvwSetTris
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tUvwSetTris& o )
	{
		s( "SetName", o.mSetName );
		s( "UvwTris", o.mUvwTris );
	}

	///
	/// \section tUvwSetVerts
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tUvwSetVerts& o )
	{
		s( "Default", o.mDefault );
		s( "SetName", o.mSetName );
		s( "Uvws", o.mUvws );
	}

	///
	/// \section tRgbaSetTris
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tRgbaSetTris& o )
	{
		s( "SetName", o.mSetName );
		s( "RgbaTris", o.mRgbaTris );
	}

	///
	/// \section tRgbaSetVerts
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tRgbaSetVerts& o )
	{
		s( "Default", o.mDefault );
		s( "SetName", o.mSetName );
		s( "Rgbas", o.mRgbas );
	}

	///
	/// \section tMaterialRenderOptions
	///

	tMaterialRenderOptions::tMaterialRenderOptions( )
		: mTwoSided( false )
		, mFlipBackFaceNormal( false )
		, mTransparency( false )
		, mAlphaCutOut( 0 )
		, mAdditive( false )
		, mZBufferTest( 0 )
		, mZBufferWrite( 0 )
		, mFaceX( false )
		, mFaceY( false )
		, mFaceZ( false )
		, mSortOffset( 0.f )
	{
	}

	void tMaterialRenderOptions::fToRenderState( Gfx::tRenderState& rs )
	{
		if( mTransparency )
		{
			rs = Gfx::tRenderState::cDefaultColorTransparent;
			rs.fSetCutOutThreshold( mAlphaCutOut );
		}
		else if( mAlphaCutOut > 0 )
		{
			rs = Gfx::tRenderState::cDefaultColorCutOut;
			rs.fSetCutOutThreshold( mAlphaCutOut );
		}
		else
		{
			rs = Gfx::tRenderState::cDefaultColorOpaque;
			rs.fSetCutOutThreshold( 0 );
		}

		// check for additive
		if( mAdditive )
		{
			rs.fEnableDisable( Gfx::tRenderState::cAlphaBlend, true );
			rs.fSetSrcBlendMode( Gfx::tRenderState::cBlendOne );
			if( mTransparency )
				rs.fSetDstBlendMode( Gfx::tRenderState::cBlendOneMinusSrcAlpha );
			else
				rs.fSetDstBlendMode( Gfx::tRenderState::cBlendOne );
		}

		// check for two sided
		if( mTwoSided )
			rs.fEnableDisable( Gfx::tRenderState::cPolyTwoSided, true );

		// check for explicit z-testing
		if( mZBufferTest != 0 )
			rs.fEnableDisable( Gfx::tRenderState::cDepthBuffer, mZBufferTest==1 );

		// check for explicit z-writing
		if( mZBufferWrite != 0 )
			rs.fEnableDisable( Gfx::tRenderState::cDepthWrite, mZBufferWrite==1 );
	}


	///
	/// \section tSubMesh
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tSubMesh& o )
	{
		s( "MtlIndex", o.mMtlIndex );
		s( "VertexTris", o.mVertexTris );
		s( "NormalTris", o.mNormalTris );
		s( "TangentBinormalTris", o.mTangentBinormalTris );
		s( "UvwSetTris", o.mUvwSetTris );
		s( "RgbaSetTris", o.mRgbaSetTris );
	}

	tSubMesh::tSubMesh( )
		: mMtlIndex( ~0 )
	{
	}

	///
	/// \section tSkin
	///

	void fSerializeXmlObject( tXmlDeserializer& s, tSkin::tVertex& o )
	{
		u32 count = 0;
		s.fCurRoot( ).fGetAttribute( "count", count );
		o.fNewArray( count );

		std::stringstream ss;
		s.fCurRoot( ).fGetContents( ss );

		for( u32 i = 0; i < o.fCount( ); ++i )
			ss >> o[i].mBoneIndex >> o[i].mBoneWeight;
	}

	void fSerializeXmlObject( tXmlSerializer& s, tSkin::tVertex& o )
	{
		s.fCurRoot( ).fSetAttribute( "count", o.fCount( ) );

		std::stringstream ss;

		for( u32 i = 0; i < o.fCount( ); ++i )
		{
			ss << o[i].mBoneIndex << ' ' << o[i].mBoneWeight;
			if( i < o.fCount( ) - 1 )
				ss << "  ";
		}

		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ), false );
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tSkin& o )
	{
		s( "BoneNames", o.mBoneNames );
		s( "Vertices", o.mVertices );
	}

	///
	/// \section tMesh
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tMesh& o )
	{
		s( "AabbMin", o.mAabb.mMin );
		s( "AabbMax", o.mAabb.mMax );
		s( "BoundingSphere", o.mBoundingSphere );
		s( "SubMeshes", o.mSubMeshes );
		s( "Vertices", o.mVertices );
		s( "Normals", o.mNormals );
		s( "Tangents", o.mTangents );
		s( "Binormals", o.mBinormals );
		s( "UvwSetVerts", o.mUvwSetVerts );
		s( "RgbaSetVerts", o.mRgbaSetVerts );

		if( s.fIn( ) || o.mSkin )
			s( "Skin", o.mSkin );
	}

	void tMesh::fComputeTriArraySubMeshUnion( tTriArray& triArray ) const
	{
		for( u32 i = 0; i < mSubMeshes.fCount( ); ++i )
		{
			triArray.fJoin( mSubMeshes[ i ]->mVertexTris );
		}
	}

	b32 tMesh::fFindDefaultRgbaSet( u32& indexOut ) const
	{
		for( u32 i = 0; i < mRgbaSetVerts.fCount( ); ++i )
		{
			if( mRgbaSetVerts[ i ].mDefault )
			{
				indexOut = i;
				return true;
			}
		}

		indexOut = mRgbaSetVerts.fCount( );
		return false;
	}

	///
	/// \section tObjectProperties
	///

	namespace
	{
		template<class tStringPtrType>
		tStringPtrType fGetEditableStringProperty( const tEditablePropertyTable& editableProps, const char* propName )
		{
			tStringPtrType o;

			tEditablePropertyPtr* find = editableProps.fFind( propName );
			if( find )
			{
				std::string str;
				(*find)->fGetData( str );

				if( str.length( ) > 0 )
				{
					o = tStringPtrType( str.c_str( ) );
				}
			}

			return o;		
		}

		void fCollectEntityDefTags( 
			const tEditablePropertyTable& editableProps, 
			tEntityDefProperties::tTagPropertyList& entityDefProps, 
			tLoadInPlaceFileBase& converter, 
			const char* groupName )
		{
			const tProjectFile& projFile = tProjectFile::fGetCurrentProjectFileCached( );

			// extract the list of game-specific user properties from the set of general editable properties
			tGrowableArray<tEditablePropertyPtr> gameProperties;
			editableProps.fGetGroup( groupName, gameProperties );

			// go through all game properties, check type, and convert to binary serializable types
			const u32 gamePropsNameStrLen = ( u32 )strlen( groupName );
			for( u32 i = 0; i < gameProperties.fCount( ); ++i )
			{
				const tEditablePropertyPtr& prop = gameProperties[ i ];
				if( prop->fClassId( ) == Rtti::fGetClassId< tEditablePropertyProjectFileTag >( ) )
				{
					u32 value; prop->fGetData( value );
					const u32 tagBitIndex = projFile.fToTagBitIndexFromKey( value );
					if( tagBitIndex != ~0 )
						entityDefProps.fPushBack( tagBitIndex );
					else
						log_warning( 0, "Could not find game tag by key: " << value );
				}
				else
					log_warning( 0, "Unrecognized game tag type [" << std::hex << prop->fClassId( ) << "], name = [" << prop->fGetName( ) << "]" );
			}
		}
		void fCollectEntityDefEnums( 
			const tEditablePropertyTable& editableProps, 
			tEntityDefProperties::tEnumPropertyList& entityDefProps, 
			tLoadInPlaceFileBase& converter, 
			const char* groupName )
		{
			const tProjectFile& projFile = tProjectFile::fGetCurrentProjectFileCached( );

			// extract the list of game-specific user properties from the set of general editable properties
			tGrowableArray<tEditablePropertyPtr> gameProperties;
			editableProps.fGetGroup( groupName, gameProperties );

			// go through all game properties, check type, and convert to binary serializable types
			const u32 gamePropsNameStrLen = ( u32 )strlen( groupName );
			for( u32 i = 0; i < gameProperties.fCount( ); ++i )
			{
				const tEditablePropertyPtr& prop = gameProperties[ i ];
				if( prop->fClassId( ) == Rtti::fGetClassId< tEditablePropertyProjectFileEnum >( ) )
				{
					const tEditablePropertyProjectFileEnum* enumProp = dynamic_cast<const tEditablePropertyProjectFileEnum*>( prop.fGetRawPtr( ) );
					
					const tProjectFile::tGameEnumeratedType* enumT = enumProp->fEnumType( );
					if( !enumT )
					{
						log_warning( 0, "Could not find enum type by key: " << enumProp->fEnumTypeKey( ) );
					}
					else
					{
						u32 key = enumT->mKey;
						u32 valueKey;
						enumProp->fGetData( valueKey );

						if( key != ~0 && valueKey != ~0 )
						{
							u32 index = enumT->fFindValueIndexByKey( valueKey );
							if( index == ~0 )
							{
								log_warning( 0, "Could not find enum (key: " << key << ") value by key: " << valueKey );
							}
							else
								entityDefProps.fPushBack( tEntityDefProperties::tEnumProperty( key, index ) );
						}
						else
							log_warning( 0, "Programmer Error key: " << key << " valueKey: " << valueKey );
					}
				}
				else
					log_warning( 0, "Unrecognized game property type [" << std::hex << prop->fClassId( ) << "], name = [" << prop->fGetName( ) << "]" );
			}
		}
	}

	void tObjectProperties::fAddFadeSettingsEditableProperties( tEditablePropertyTable& editableProperties )
	{
		editableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( tObjectProperties::fEditablePropFadeOverride( ), 0.f, 0.f, 10000.f, 1.f, 0 ) ) );

		tDynamicArray<std::string> fadeNames(4);
		fadeNames[0] = "Never";
		fadeNames[1] = "Near";
		fadeNames[2] = "Medium";
		fadeNames[3] = "Far";
		editableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyEnum( tObjectProperties::fEditablePropFadeSetting( ), fadeNames ) ) );
	}

	tStringPtr tObjectProperties::fGetEntityNameProperty( ) const
	{
		return fGetEditableStringProperty<tStringPtr>( mEditableProperties, fEditablePropObjectName( ) );
	}

	void tObjectProperties::fGetScriptSource( tFilePathPtr& path, std::string& func ) const
	{
		const std::string scriptText = fGetEditableStringProperty<std::string>( mEditableProperties, fEditablePropScriptName( ) );
		if( StringUtil::fStrStrI( scriptText.c_str( ), ".nut" ) )
			path = tFilePathPtr( scriptText );
		else
			func = scriptText;
	}

	tFilePathPtr tObjectProperties::fGetSkeletonSourcePath( ) const
	{
		return fGetEditableStringProperty<tFilePathPtr>( mEditableProperties, fEditablePropSkeletonName( ) );
	}

	tStringPtr tObjectProperties::fGetSkeletonBindingName( ) const
	{
		return fGetEditableStringProperty<tStringPtr>( mEditableProperties, fEditablePropSkeletonBindingName( ) );
	}

	tStringPtr tObjectProperties::fGetBoneAttachmentProperty( ) const
	{
		return fGetEditableStringProperty<tStringPtr>( mEditableProperties, fEditablePropBoneAttachment( ) );
	}

	b32 tObjectProperties::fGetContextAnimsDisabled( ) const
	{
		return mEditableProperties.fGetValue< b32 >( fEditablePropDisableContextAnims( ), false );
	}

	b32 tObjectProperties::fGetIsInvisible( ) const
	{
		return mEditableProperties.fGetValue< b32 >( fEditablePropInvisible( ), false );
	}

	b32 tObjectProperties::fGetIsShadowCaster( ) const
	{
		return mEditableProperties.fGetValue< b32 >( fEditablePropCastShadow( ), false );
	}

	b32 tObjectProperties::fGetIsShadowReceiver( ) const
	{
		return mEditableProperties.fGetValue< b32 >( fEditablePropReceiveShadow( ), false );
	}

	void tObjectProperties::fAddEntityDefProperties( tEntityDefProperties* entityDef, tLoadInPlaceFileBase& converter ) const
	{
		// check for entity/object name
		const tStringPtr entityName = fGetEntityNameProperty( );
		if( entityName.fLength( ) > 0 )
		{
			const std::string trimmedName = StringUtil::fEatWhiteSpace( entityName.fCStr( ) );
			entityDef->mName = converter.fAddLoadInPlaceStringPtr( trimmedName.c_str( ) );
		}

		// check for script
		tFilePathPtr scriptSourcePath;
		std::string scriptCall;
		fGetScriptSource( scriptSourcePath, scriptCall );
		if( scriptSourcePath.fLength( ) > 0 )
		{
			const tFilePathPtr scriptPath = tFilePathPtr::fSwapExtension( scriptSourcePath, tScriptFile::fGetFileExtension( ) );
			entityDef->mScriptFile = converter.fAddLoadInPlaceResourcePtr( tResourceId::fMake<tScriptFile>( scriptPath ) );
		}
		else if( scriptCall.length( ) > 0 )
		{
			entityDef->mOnEntityCreateOverride = converter.fAddLoadInPlaceStringPtr( scriptCall.c_str( ) );
			const b32 immediateModeScript = ( scriptCall.find_first_of( ".=()" ) != std::string::npos );
			if( immediateModeScript )
				entityDef->mCreationFlags.mCreateFlags |= tEntityCreationFlags::cFlagImmediateScript;
		}

		// check for skeleton
		const tFilePathPtr skelSourcePath = fGetSkeletonSourcePath( );
		if( skelSourcePath.fLength( ) > 0 )
		{
			const tFilePathPtr skelPath = tFilePathPtr::fSwapExtension( skelSourcePath, tSkeletonFile::fGetFileExtension( ) );
			entityDef->mSkeletonFile = converter.fAddLoadInPlaceResourcePtr( tResourceId::fMake<tSkeletonFile>( skelPath ) );
		}

		// check for bone attachment
		const tStringPtr boneAttachment = fGetBoneAttachmentProperty( );
		if( boneAttachment.fLength( ) > 0 )
			entityDef->mBoneAttachment = converter.fAddLoadInPlaceStringPtr( boneAttachment.fCStr( ) );

		fCollectEntityDefTags( mEditableProperties, entityDef->mTagProperties, converter, fEditablePropGameTagName( ) );
		fCollectEntityDefEnums( mEditableProperties, entityDef->mEnumProperties, converter, fEditablePropGameEnumName( ) );
	}

	///
	/// \section tObject
	///

	tObject::tObject( )
		: mVersion( 0 )
		, mGuid( 0 )
		, mClone( false )
		, mName( "" )
		, mLayer( "" )
		, mHidden( false )
		, mFrozen( false )
	{
	}

	void tObject::fGetDependencyFiles( tFilePathPtrList& resourcePathsOut )
	{
		tFilePathPtr scriptSourcePath; std::string dummy;
		tObjectProperties::fGetScriptSource( scriptSourcePath, dummy );
		if( scriptSourcePath.fLength( ) > 0 )
			resourcePathsOut.fFindOrAdd( scriptSourcePath );
	}

	void tObject::fConvertEntityDefBase( tEntityDef* entityDef, tLoadInPlaceFileBase& converter )
	{
		// spatial shite
		entityDef->mBounds.fInvalidate( );
		entityDef->mObjectToLocal = mXform;
		entityDef->mLocalToObject = mXform.fInverse( );

		tEntityCreationFlags creationFlags;
		creationFlags.mRenderFlags |= ( fGetIsInvisible( ) ? Gfx::tRenderableEntity::cFlagInvisible : 0 );
		creationFlags.mRenderFlags |= ( fGetIsShadowCaster( ) ? Gfx::tRenderableEntity::cFlagCastShadow : 0 );
		creationFlags.mRenderFlags |= ( fGetIsShadowReceiver( ) ? Gfx::tRenderableEntity::cFlagReceiveShadow : 0 );
		creationFlags.mCreateFlags |= ( fGetContextAnimsDisabled( ) ? tEntityCreationFlags::cFlagDisableContextAnims : 0 );
		entityDef->mCreationFlags = creationFlags;

		tObjectProperties::fAddEntityDefProperties( entityDef, converter );
	}


	///
	/// \section tGeomObject
	///

	register_rtti_factory( tGeomObject, false );

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tGeomObject& o )
	{
		s( "MeshIndex", o.mMeshIndex );
		s( "GeomType", o.mGeomType );
		s( "StateType", o.mStateType );

		s32 stateIndex = o.mStateIndex;
		s( "StateIndex", stateIndex );
		o.mStateIndex = stateIndex;
	}

	tGeomObject::tGeomObject( )
		: mMeshIndex( ~0 )
		, mStateType( tExporterToolbox::fNameStateState( ) )
		, mStateIndex( 0 )
	{
	}

	void tGeomObject::fSerialize( tXmlSerializer& s )		{ fSerializeXmlObject( s, *this ); }
	void tGeomObject::fSerialize( tXmlDeserializer& s )		{ fSerializeXmlObject( s, *this ); }
	
	tEntityDef* tGeomObject::fCreateEntityDef( tSigmlConverter& sigmlConverter )
	{
		tMeshEntityDef* entityDef = new tMeshEntityDef( );
		fConvertEntityDefBase( entityDef, sigmlConverter );

		const Sigml::tFile& sigmlFile = sigmlConverter.fSigmlFile( );

		entityDef->mMesh = sigmlConverter.fGetMesh( mMeshIndex ).fGetRawPtr( );
		const Sigml::tMeshPtr& sigmlMesh = sigmlFile.mMeshes[ mMeshIndex ];
		entityDef->mBounds = sigmlMesh->mAabb;

		// acquire sort offset from materials - strictly speaking this is kinda broken
		// for single mesh objects that reference multiple separate materials with separate sort offset -
		// what will happen is the sort offset of highest magnitude will be selected
		entityDef->mSortOffset = 0.f;
		for( u32 iSubMesh = 0; iSubMesh < sigmlMesh->mSubMeshes.fCount( ); ++iSubMesh )
		{
			const f32 sortOffset = sigmlFile.mMaterials[ sigmlMesh->mSubMeshes[ iSubMesh ]->mMtlIndex ]->mSortOffset;
			if( fAbs( sortOffset ) > fAbs( entityDef->mSortOffset ) )
				entityDef->mSortOffset = sortOffset;
		}

		// FIXME should have per sub mesh bounding
		b32 anySubMeshesHaveFacingMaterials = false;
		for( u32 iSubMesh = 0; !anySubMeshesHaveFacingMaterials && iSubMesh < sigmlMesh->mSubMeshes.fCount( ); ++iSubMesh )
			anySubMeshesHaveFacingMaterials = anySubMeshesHaveFacingMaterials || sigmlFile.mMaterials[ sigmlMesh->mSubMeshes[ iSubMesh ]->mMtlIndex ]->fIsFacing( );
		if( anySubMeshesHaveFacingMaterials )
		{
			// account for the fact that the object is facing
			const Math::tSpheref bsphere = Math::tSpheref( entityDef->mBounds );
			const f32 extendedRadius = bsphere.mRadius + bsphere.mCenter.fLength( );
			entityDef->mBounds = Math::tAabbf( Math::tSpheref( bsphere.mCenter, extendedRadius ) );
		}

		if( mStateType == tExporterToolbox::fNameStateTransition( ) )
			entityDef->mStateType = tMeshEntityDef::cStateTypeTransition;
		else // if( mStateType == tExporterToolbox::fNameStateState( ) ) default to this one
			entityDef->mStateType = tMeshEntityDef::cStateTypeState;

		entityDef->mStateIndex = mStateIndex;

		return entityDef;
	}

	tEditableObject* tGeomObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		log_warning( 0, "Geometry Objects (meshes) not supported as editable objects. Skipping object." );
		return 0;
	}

	///
	/// \section tSigmlReferenceObject
	///

	register_rtti_factory( tSigmlReferenceObject, false );

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tSigmlReferenceObject& o )
	{
		s( "ReferencePath", o.mReferencePath );
	}

	tSigmlReferenceObject::tSigmlReferenceObject( )
	{
	}

	void tSigmlReferenceObject::fSerialize( tXmlSerializer& s )		{ fSerializeXmlObject( s, *this ); }
	void tSigmlReferenceObject::fSerialize( tXmlDeserializer& s )	{ fSerializeXmlObject( s, *this ); }

	void tSigmlReferenceObject::fGetDependencyFiles( tFilePathPtrList& resourcePathsOut )
	{
		tObject::fGetDependencyFiles( resourcePathsOut );
		resourcePathsOut.fFindOrAdd( mReferencePath );
	}

	tEntityDef* tSigmlReferenceObject::fCreateEntityDef( tSigmlConverter& sigmlConverter )
	{
		tSceneRefEntityDef* entityDef = new tSceneRefEntityDef( );

		fConvertEntityDefBase( entityDef, sigmlConverter );

		// fade setting and override
		const u32 fadeSetting = mEditableProperties.fGetValue( fEditablePropFadeSetting( ), 0u );
		const f32 fadeOverride = mEditableProperties.fGetValue( fEditablePropFadeOverride( ), 0.f );
		if( fadeSetting || fadeOverride )
		{
			entityDef->mLODSettings = new tSceneLODSettings;
			entityDef->mLODSettings->mFadeSetting = fadeSetting;
			entityDef->mLODSettings->mFadeOverride = fadeOverride;
		}

		tResourceId rid;
		entityDef->mReferenceFile = sigmlConverter.fAddLoadInPlaceResourcePtr( fConstructResourceId( rid ) );
		return entityDef;
	}

	tEditableObject* tSigmlReferenceObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tEditableSgFileRefEntity( container, *this );
	}

	tResourceId& tSigmlReferenceObject::fConstructResourceId( tResourceId& rid ) const
	{
		const tFilePathPtr sigbPath = fSigmlPathToSigb( mReferencePath );
		rid = tResourceId::fMake<tSceneGraphFile>( sigbPath );
		return rid;
	}

	///
	/// \section tObjectLayer
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tObjectLayer& o )
	{
		o.fSerialize( s );
	}

	tObjectLayer::tObjectLayer( const std::string& name, tState state, const Math::tVec4f& colorRgba ) 
		: mName( name ), mState( state ), mColorRgba( colorRgba )
	{
	}
	const char* tObjectLayer::fStateText( wxColour& colourOut ) const
	{
		switch( mState )
		{
		case cStateVisible: colourOut = wxColour( 0x00, 0x88, 0x00 ); return "V";
		case cStateHidden: colourOut = wxColour( 0x00, 0x00, 0x00 ); return "H";
		case cStateFrozen: colourOut = wxColour( 0x00, 0x00, 0xff ); return "F";
		default: sigassert( !"invalid case in tObjectLayer::fStateText" ); break;
		}
		return "";
	}

	//------------------------------------------------------------------------------
	// tGroundCoverLayer
	//------------------------------------------------------------------------------
	void tGroundCoverLayer::fSetElements( const tElement elements[], u32 count )
	{
		mElements.fSetCount( count );
		for( u32 i = 0; i < count; ++i )
			mElements.fFindOrAdd( elements[ i ] );
	}

	//------------------------------------------------------------------------------
	const tGroundCoverLayer::tElement * tGroundCoverLayer::fFindElement( const tFilePathPtr & path ) const
	{
		return mElements.fFind( path );
	}

	//------------------------------------------------------------------------------
	tGroundCoverLayer::tElement * tGroundCoverLayer::fFindElement( const tFilePathPtr & path )
	{
		return mElements.fFind( path );
	}

	//------------------------------------------------------------------------------
	tGroundCoverLayer::tElement & tGroundCoverLayer::fAddElement( const tFilePathPtr & path )
	{
		tElement element;
		element.mSgPath = path;

		mElements.fPushBack( element );
		return mElements.fBack( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverLayer::fRemoveElement( const tFilePathPtr & path )
	{
		mElements.fFindAndEraseOrdered( path );
	}

	//------------------------------------------------------------------------------
	u32 tGroundCoverLayer::fComputeMaxUnitSpawns( ) const
	{
		u32 spawns = 0;

		const u32 elementCount = mElements.fCount( );
		for( u32 e = 0; e < elementCount; ++e )
			spawns += mElements[ e ].mSpawnCount;

		return spawns ? spawns : 1;
	}

	///
	/// \section tFile
	///

	namespace
	{
		static u32 gSigmlVersion = 1;
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile& o )
	{
		s.fAsAttribute( "Version", o.mVersion );

		s( "ModellingPackage", o.mModellingPackage );
		s( "SrcFile", o.mSrcFile );

		if( o.mVersion != gSigmlVersion )
		{
			log_warning( 0, "Sigml file format is out of date (exported from " << o.mSrcFile << ") -> Please re-export." );
			return;
		}

		s( "ExplicitDependencies", o.mExplicitDependencies );
		s( "Objects", o.mObjects );
		s( "Meshes", o.mMeshes );
		s( "Materials", o.mMaterials );
		s( "DiffuseMapAtlas", o.mDiffuseMapAtlas );
		s( "NormalMapAtlas", o.mNormalMapAtlas );
		s( "HeightFieldMaterialTileFactors", o.mHeightFieldMaterialTileFactors );
		s( "EditableProperties", o.mEditableProperties );
		s( "Layers", o.mLayers );
		s( "GroundCovers", o.mGroundCovers );
	}

	tFile::tFile( )
		: mVersion( gSigmlVersion )
	{
		fAddGlobalProperties( );
	}

	b32 tFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		if( !ser.fSave( path, "Sigml", *this, promptToCheckout ) )
		{
			log_warning( 0, "Couldn't save Sigml file [" << path << "]" );
			return false;
		}

		return true;
	}

	b32 tFile::fLoadXml( const tFilePathPtr& path, b32 displayErrors )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Sigml", *this ) )
		{
			log_warning( 0, "Couldn't load Sigml file [" << path << "]" );
			return false;
		}

		fAddGlobalProperties( );
		return true;
	}

	b32 tFile::fApplySkinFile( const tFilePathPtr& path )
	{
		tFile skinFile;
		if( !skinFile.fLoadXml( path ) )
			return false;
		return fApplySkinFile( skinFile );
	}

	b32 tFile::fApplySkinFile( const tFile& skinFile )
	{
		if( mMeshes.fCount( ) != skinFile.mMeshes.fCount( ) )
		{
			log_warning( 0, "Error applying skin file to mesh file: mis-match in mesh count." );
			return false;
		}

		for( u32 i = 0; i < mMeshes.fCount( ); ++i )
		{
			mMeshes[ i ]->mSkin = skinFile.mMeshes[ i ]->mSkin;
			
			if( !mMeshes[ i ]->mSkin || ( mMeshes[ i ]->mSkin->mVertices.fCount( ) != mMeshes[ i ]->mVertices.fCount( ) ) )
			{
				log_warning( 0, "Error applying skin file to mesh file: mis-match in the number of vertices within mesh." );
				return false;
			}
		}

		//log_line( 0, "Skin file applied successfully to mesh file." );
		return true;
	}

	tObjectPtr tFile::fFindByEntityName( const tStringPtr& name ) const
	{
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
		{
			if( mObjects[ i ]->fGetEntityNameProperty( ) == name )
				return mObjects[ i ];
		}
		return tObjectPtr( );
	}

	tObjectPtr tFile::fFindObjectByGuid( u32 guid ) const
	{
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
		{
			if( mObjects[ i ]->mGuid == guid )
				return mObjects[ i ];
		}
		return tObjectPtr( );
	}

	void tFile::fAddEntityDefProperties( tSceneGraphFile* sgFile, tSigmlConverter& sigmlConverter ) const
	{
		tObjectProperties::fAddEntityDefProperties( sgFile, sigmlConverter );

		if( sgFile->mSkeletonFile )
		{
			// since we have a skeleton, check for a skeleton binding
			const tStringPtr skelBinding = fGetSkeletonBindingName( );
			if( skelBinding.fLength( ) > 0 )
			{
				tObjectPtr find = sigmlConverter.fSigmlFile( ).fFindByEntityName( skelBinding );
				if( find )
				{
					sgFile->mSkeletonBinding = find->mXform;
					sgFile->mSkeletonBindingInv = find->mXform.fInverse( );
				}
			}
		}

		if( mEditableProperties.fGetValue( Sigml::tFile::fEditablePropDefaultApplyInGameName( ), false ) )
		{
			sgFile->mDefaultLight = new tSceneGraphDefaultLight( );
			fConvertToDefaultLightDesc( mEditableProperties, *sgFile->mDefaultLight );
		}

		// fade setting and override
		const u32 fadeSetting = mEditableProperties.fGetValue( fEditablePropFadeSetting( ), 0u );
		const f32 fadeOverride = mEditableProperties.fGetValue( fEditablePropFadeOverride( ), 0.f );
		if( fadeSetting || fadeOverride )
		{
			sgFile->mLODSettings = new tSceneLODSettings;
			sgFile->mLODSettings->mFadeSetting = fadeSetting;
			sgFile->mLODSettings->mFadeOverride = fadeOverride;
		}
	}

	void tFile::fAddGlobalProperties( )
	{
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyString( tObjectProperties::fEditablePropObjectName( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFileNameString( tObjectProperties::fEditablePropScriptName( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFileNameString( tObjectProperties::fEditablePropSkeletonName( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyCustomString( tObjectProperties::fEditablePropSkeletonBindingName( ) ) ) );

		fAddFadeSettingsEditableProperties( mEditableProperties );

		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyVec3f( fEditablePropDefaultLightDirectionName( ), -Math::tVec3f( 0.7f, 1.0f, 0.6f ).fNormalize( ), -1.f, +1.f, 0.01f, 2 ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( fEditablePropDefaultCastsShadowName( ), true ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( fEditablePropDefaultApplyInEditorName( ), true ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( fEditablePropDefaultApplyInGameName( ), true ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyColor( fEditablePropDefaultFrontColorName( ), tColorPickerData( 0.5f * 1.25f ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyColor( fEditablePropDefaultBackColorName( ), tColorPickerData( 0.5f * Math::tVec3f( 0.3f, 0.3f, 0.6f ) ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyColor( fEditablePropDefaultRimColorName( ), tColorPickerData( 0.f ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyColor( fEditablePropDefaultAmbientColorName( ), tColorPickerData( 0.1f ) ) ) );

		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( fEditablePropShowFogInEditor( ), true ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( fEditablePropShowGlobalLightDirection( ), false ) ) );
	}

	b32 tFile::fIsPropertyDefaultLightRelated( const tEditableProperty& property )
	{
		const std::string& propName = property.fGetName( );
		return 
			propName == fEditablePropDefaultLightDirectionName( ) ||
			propName == fEditablePropDefaultCastsShadowName( ) ||
			propName == fEditablePropDefaultApplyInEditorName( ) ||
			propName == fEditablePropDefaultApplyInGameName( ) ||
			propName == fEditablePropDefaultFrontColorName( ) ||
			propName == fEditablePropDefaultBackColorName( ) ||
			propName == fEditablePropDefaultRimColorName( ) ||
			propName == fEditablePropDefaultAmbientColorName( );
	}

	void tFile::fConvertToDefaultLightDesc( tEditablePropertyTable& properties, tSceneGraphDefaultLight& desc )
	{
		desc.mDirection = properties.fGetValue( fEditablePropDefaultLightDirectionName( ), desc.mDirection );

		if( desc.mDirection.fIsZero( ) )
			desc.mDirection.y = -1.f;

		desc.mCastShadow = properties.fGetValue( fEditablePropDefaultCastsShadowName( ), true );
		
		desc.mFrontColor = 2.f * properties.fGetValue( fEditablePropDefaultFrontColorName( ), tColorPickerData( ) ).fExpandRgba( ).fXYZ( );
		desc.mBackColor = 2.f * properties.fGetValue( fEditablePropDefaultBackColorName( ), tColorPickerData( ) ).fExpandRgba( ).fXYZ( );
		desc.mRimColor = 2.f * properties.fGetValue( fEditablePropDefaultRimColorName( ), tColorPickerData( ) ).fExpandRgba( ).fXYZ( );
		desc.mAmbientColor = properties.fGetValue( fEditablePropDefaultAmbientColorName( ), tColorPickerData( ) ).fExpandRgba( ).fXYZ( );
	}

	namespace
	{
		void fReplaceReferencesInProperties( tEditablePropertyTable& table, const tFilePathPtr& replace, const tFilePathPtr& with, u32& replaceCnt )
		{
			const std::string newPath( with.fCStr( ) );

			tEditablePropertyTable::tIteratorNoNullOrRemoved it( table.fBegin( ), table.fEnd( ) );
			for( ; it != table.fEnd( ); ++it )
			{
				tEditablePropertyFileNameString *filename = dynamic_cast<tEditablePropertyFileNameString*>( (*it).mValue.fGetRawPtr() );
				if( filename )
				{
					std::string path;
					filename->fGetData( path );

					if( stricmp( replace.fCStr( ), path.c_str( ) ) == 0 )
					{
						++replaceCnt;
						log_line( 0, "Replaced " << path << " with " << newPath );

						path = newPath;
						filename->fSetData( path );
					}
				}
			}
		}

		void fReplaceFilePathPtr( tFilePathPtr& subject, const tFilePathPtr& replace, const tFilePathPtr& with, u32& replaceCnt )
		{
			if( subject == replace )
			{
				++replaceCnt;
				log_line( 0, "Replaced " << subject << " with " << with );
				subject = with;
			}
		}
	}

	u32 tFile::fReplaceReferences( const tFilePathPtr& replace, const tFilePathPtr& with )
	{
		u32 replaceCnt = 0;

		fReplaceFilePathPtr( mDiffuseMapAtlas, replace, with, replaceCnt );
		fReplaceFilePathPtr( mNormalMapAtlas, replace, with, replaceCnt );
		fReplaceReferencesInProperties( mEditableProperties, replace, with, replaceCnt );

		for( u32 i = 0; i < mExplicitDependencies.fCount( ); ++i )
			fReplaceFilePathPtr( mExplicitDependencies[ i ], replace, with, replaceCnt );

		for( u32 i = 0; i < mObjects.fCount( ); ++i )
		{
			tSigmlReferenceObject* refObj = dynamic_cast< tSigmlReferenceObject* >( mObjects[ i ].fGetRawPtr( ) );
			if( refObj ) fReplaceFilePathPtr( refObj->mReferencePath, replace, with, replaceCnt );

			fReplaceReferencesInProperties( mObjects[ i ]->mEditableProperties, replace, with, replaceCnt );
		}

		return replaceCnt;
	}

	//------------------------------------------------------------------------------
	const tGroundCoverLayer * tFile::fFindGroundCover( u32 id ) const
	{
		const u32 count = mGroundCovers.fCount( );
		for( u32 l = 0; l < count; ++l )
		{
			if( mGroundCovers[ l ].fUniqueId( ) == id )
				return &mGroundCovers[ l ];
		}

		return NULL;
	}

}}
