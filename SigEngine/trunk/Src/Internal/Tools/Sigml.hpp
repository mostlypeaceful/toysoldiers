#ifndef __Sigml__
#define __Sigml__
#include "Editor/tEditableProperty.hpp"
#include "iSigEdPlugin.hpp"

namespace Sig
{
	class tResourceId;
	class tLoadInPlaceFileBase;
	class tSceneGraphFile;
	struct tSceneGraphDefaultLight;
	class tEntity;
	class tEditableObject;
	class tEntityDef;
	class tEntityDefProperties;
	class tSigmlConverter;
	class tFxmlConverter;
	class tEditableObjectContainer;
}

namespace Sig { namespace Gfx
{
	class tMaterial;
	class tRenderState;
}}

namespace Sig { namespace Sigml
{
	class tFile;

	tools_export u32 fGetNumFileExtensions( );
	tools_export const char* fGetFileExtension( u32 i = 0 );
	tools_export b32 fIsSigmlFile( const tFilePathPtr& path );
	tools_export tFilePathPtr fSigmlPathToSigb( const tFilePathPtr& path );
	tools_export tFilePathPtr fSigbPathToSigml( const tFilePathPtr& path );

	typedef tGrowableArray< std::string >  tStringArray;
	typedef tGrowableArray< Math::tVec3u > tTriArray;
	typedef tGrowableArray< Math::tVec3f > tVertexArray;
	typedef tGrowableArray< Math::tVec3f > tNormalArray;
	typedef tGrowableArray< Math::tVec3f > tTangentArray;
	typedef tGrowableArray< Math::tVec3f > tBinormalArray;
	typedef tGrowableArray< Math::tVec3f > tUvwArray;
	typedef tGrowableArray< Math::tVec4f > tRgbaArray;

	struct tools_export tUvwSetTris
	{
		std::string mSetName;
		tTriArray	mUvwTris;
	};

	struct tools_export tUvwSetVerts
	{
		b32			mDefault;
		std::string mSetName;
		tUvwArray	mUvws;

		tUvwSetVerts( ) : mDefault( false ) { }
	};

	typedef tGrowableArray< tUvwSetTris >	tUvwSetTrisArray;
	typedef tGrowableArray< tUvwSetVerts >	tUvwSetVertsArray;

	struct tools_export tRgbaSetTris
	{
		std::string mSetName;
		tTriArray	mRgbaTris;
	};

	struct tools_export tRgbaSetVerts
	{
		b32			mDefault;
		std::string mSetName;
		tRgbaArray	mRgbas;

		tRgbaSetVerts( ) : mDefault( false ) { }
	};

	typedef tGrowableArray< tRgbaSetTris >	tRgbaSetTrisArray;
	typedef tGrowableArray< tRgbaSetVerts >	tRgbaSetVertsArray;

	struct tools_export tMaterialRenderOptions
	{
		b32 mTwoSided;
		b32 mFlipBackFaceNormal;
		b32 mTransparency;
		u32 mAlphaCutOut;
		b32 mAdditive;
		u32 mZBufferTest; // 0 is use default (based on other render states), 1 is force-on, 2 is force-off
		u32 mZBufferWrite; // 0 is use default (based on other render states), 1 is force-on, 2 is force-off
		b32 mFaceX;
		b32 mFaceY;
		b32 mFaceZ;
		b32 mXparentDepthPrepass;
		f32 mSortOffset;

		tMaterialRenderOptions( );
		void fToRenderState( Gfx::tRenderState& rs );
	};

	///
	/// \brief TODO document
	class tools_export tMaterial : 
		public Rtti::tSerializableBaseClass, 
		public tRefCounter, 
		public tMaterialRenderOptions
	{
		declare_null_reflector();
	public:
		std::string mName;
	public:
		virtual void fSerialize( tXmlSerializer& s ) = 0;
		virtual void fSerialize( tXmlDeserializer& s ) = 0;
		virtual Gfx::tMaterial* fCreateGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned = false ) = 0;
		virtual b32  fIsEquivalent( const tMaterial& other ) const = 0;
		virtual void fGetTextureResourcePaths( tFilePathPtrList& resourcePathsOut ) = 0;
		virtual b32	 fIsFacing( ) const = 0;
	};
	typedef tRefCounterPtr< tMaterial >		tMaterialPtr;
	typedef tGrowableArray< tMaterialPtr >	tMaterialPtrArray;

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tMaterial& o )
	{
		s( "Name", o.mName );
		o.fSerialize( s );
	}

	///
	/// \brief TODO document
	class tools_export tSubMesh : public tRefCounter
	{
	public:
		u32					mMtlIndex;
		tTriArray			mVertexTris;
		tTriArray			mNormalTris;
		tTriArray			mTangentBinormalTris;
		tUvwSetTrisArray	mUvwSetTris;
		tRgbaSetTrisArray	mRgbaSetTris;

	public:
		tSubMesh( );
	};
	typedef tRefCounterPtr< tSubMesh >		tSubMeshPtr;
	typedef tGrowableArray< tSubMeshPtr >	tSubMeshPtrArray;


	///
	/// \brief TODO document
	class tools_export tSkin : public tRefCounter
	{
	public:
		struct tWeight
		{
			u32 mBoneIndex;
			f32 mBoneWeight;

			tWeight( ) { fZeroOut( this ); }
			tWeight( u32 bi, f32 bw ) : mBoneIndex( bi ), mBoneWeight( bw ) { }
		};
		typedef tDynamicArray<tWeight> tVertex;

	public:
		tDynamicArray<std::string>	mBoneNames;
		tDynamicArray<tVertex>		mVertices;
	};
	typedef tRefCounterPtr< tSkin > tSkinPtr;


	///
	/// \brief TODO document
	class tools_export tMesh : public tRefCounter
	{
	public:
		Math::tAabbf		mAabb;
		Math::tSpheref		mBoundingSphere;
		tSubMeshPtrArray	mSubMeshes;
		tVertexArray		mVertices;
		tNormalArray		mNormals;
		tTangentArray		mTangents;
		tBinormalArray		mBinormals;
		tUvwSetVertsArray	mUvwSetVerts;
		tRgbaSetVertsArray	mRgbaSetVerts;
		tSkinPtr			mSkin;
		f32					mHighLodRatio;
		f32					mMediumLodRatio;
		f32					mLowLodRatio;

		tMesh( ) : mHighLodRatio( 1.0f ), mMediumLodRatio( 0.5f ), mLowLodRatio( 0.25f ) { }
		///
		/// \brief Takes the union of all the sub-meshes' tri arrays
		/// and stores them in 'triArray'.
		void fComputeTriArraySubMeshUnion( tTriArray& triArray ) const;

		///
		/// \brief Finds the index into the mRgbaSetVerts array representing
		/// the "default" color array.
		b32 fFindDefaultRgbaSet( u32& indexOut ) const;
	};
	typedef tRefCounterPtr< tMesh >		tMeshPtr;
	typedef tGrowableArray< tMeshPtr >	tMeshPtrArray;

	///
	/// \brief Provides functionality for object properties. Extracted into separate class
	/// so the functionality can be used by non tObject types.
	class tools_export tObjectProperties
	{
	public:
		static const char* fEditablePropObjectName( ) { return "$.ObjectName"; }
		static const char* fEditablePropGroundRelative( ) { return "$.SnapToGround"; }
		static const char* fEditablePropGroundOffset( ) { return "$.SnapToOffset"; }
		static const char* fEditablePropScriptName( ) { return "$.ScriptPath"; }
		static const char* fEditablePropSkeletonName( ) { return "$.SkeletonPath"; }
		static const char* fEditablePropSkeletonBindingName( ) { return "$.SkeletonBinding"; }
		static const char* fEditablePropBoneAttachment( ) { return "$.BoneAttachment"; }
		static const char* fEditablePropBoneRelativeAttachment( ) { return "$.BoneRelativeAttach"; }
		static const char* fEditablePropLockTranslation( ) { return "$.LockTranslation"; }
		static const char* fEditablePropFadeSetting( ) { return "Render.FadeSetting"; }
		static const char* fEditablePropFadeOverride( ) { return "Render.FadeOverride"; }
		static const char* fEditablePropLODMediumDistanceName( ) { return "Render.LODMediumOverride"; }
		static const char* fEditablePropLODFarDistanceName( ) { return "Render.LODFarOverride"; }
		static const char* fEditablePropShowLODFromCameraName( ) { return "Display.ShowLODFromCam"; }
		static const char* fEditablePropInvisible( ) { return "Render.Invisible"; }
		static const char* fEditablePropCastShadow( ) { return "Render.ShadowCaster"; }
		static const char* fEditablePropReceiveShadow( ) { return "Render.ShadowReceiver"; }
		static const char* fEditablePropGameTagName( ) { return "GameTag."; }
		static const char* fEditablePropGameEnumName( ) { return "GameEnum."; }
		static const char* fEditablePropShowFogInEditor( ) { return "Display.ShowFogInEditor"; }
		static const char* fEditablePropShowGlobalLightDirection( ) { return "Display.ShowGlobalLightDir"; }
		static const char* fEditablePropShowRaycastMeshName( ) { return "Display.ShowRaycastMesh"; }
		static const char* fEditablePropFreezeRaycastMeshName( ) { return "Display.FreezeRaycastMesh"; }
		static const char* fEditablePropPhysicsIgnoreChildCollision( ) { return "Physics.IgnoreChildCollision"; }
		static const char* fEditablePropPhysicsCreateStatic( ) { return "Physics.CreateStatic"; }
	public:
		mutable tEditablePropertyTable mEditableProperties;
		tEditorPluginDataContainer mPluginData;
	public:
		static void		fAddFadeSettingsEditableProperties( tEditablePropertyTable& editableProperties );
	public:
		tStringPtr		fGetEntityNameProperty( ) const;
		void			fGetScriptSource( tFilePathPtr& path, std::string& func ) const;
		tFilePathPtr	fGetSkeletonSourcePath( ) const;
		tStringPtr		fGetSkeletonBindingName( ) const;
		tStringPtr		fGetBoneAttachmentProperty( ) const;
		b32				fGetBoneRelativeAttachmentProperty( ) const;
		b32				fGetIsInvisible( ) const;
		b32				fGetIsShadowCaster( ) const;
		b32				fGetIsShadowReceiver( ) const;
		b32				fGetPhysicsDisableChildCollision( ) const;
		b32				fGetPhysicsCreateCollision( ) const;
		b32				fGetPhysicsCreateStatic( ) const;
		void			fAddEntityDefProperties( tEntityDefProperties* entityDef, tLoadInPlaceFileBase& sigmlConverter ) const;
	};

	///
	/// \brief Contains a list of visiblity sets an object belongs to.
	struct tools_export tVisibilitySet
	{
		 //Empty for always, or this type can belong to multiple visibility sets.
		tGrowableArray<std::string> mSet;
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tVisibilitySet& o )
	{
		s( "Set", o.mSet );
	}


	///
	/// \brief Base class for transform-based exported objects.
	class tools_export tObject : public Rtti::tSerializableBaseClass, public tRefCounter, public tObjectProperties
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tObject, 0x25809659 );
		define_dynamic_cast_base( tObject )

	public:
		u32							mVersion;
		u32							mGuid;
		b32							mClone;
		std::string					mName;
		std::string					mLayer;
		tVisibilitySet				mVisibilitySets;

		b32							mHidden;
		b32							mFrozen;
		tStringArray				mParentNames;
		tStringArray				mChildNames;
		Math::tMat3f				mXform;
		b32							mSelectable;

	public:
		tObject( );
		virtual void fSerialize( tXmlSerializer& s ) = 0;
		virtual void fSerialize( tXmlDeserializer& s ) = 0;
		virtual void fGetDependencyFiles( tFilePathPtrList& resourcePathsOut );
		virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter ) { return 0; }
		virtual tEntityDef* fCreateEntityDef( tFxmlConverter& fxmlConverter ) { return 0; }
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container ) = 0;

	protected:
		void fConvertEntityDefBase( tEntityDef* entityDef, tLoadInPlaceFileBase& converter );
	};
	typedef tRefCounterPtr< tObject >		tObjectPtr;
	typedef tGrowableArray< tObjectPtr >	tObjectPtrArray;

	///
	/// \brief Geometry-based object.
	class tools_export tGeomObject : public tObject
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tGeomObject, 0x85D66C0D );
	public:
		u32			mMeshIndex;
		std::string mGeomType;
		std::string mStateType;
		u32			mStateMask;

	public:
		tGeomObject( );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
	};
	typedef tRefCounterPtr< tGeomObject > tGeomObjectPtr;

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tObject& o )
	{
		s.fAsAttribute( "Version", o.mVersion );
		s( "Guid", o.mGuid );
		s( "Name", o.mName );
		s( "Layer", o.mLayer );
		s( "Visibility", o.mVisibilitySets );
		s( "Hidden", o.mHidden );
		s( "Frozen", o.mFrozen );
		s( "ParentNames", o.mParentNames );
		s( "ChildNames", o.mChildNames );
		s( "Xform", o.mXform );
		s( "EditableProperties", o.mEditableProperties );
		s( "PluginData", o.mPluginData );
		s( "Selectable", o.mSelectable );
		o.fSerialize( s );
	}

	///
	/// \brief Object representing a reference to another sigml file.
	class tools_export tSigmlReferenceObject : public tObject
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tSigmlReferenceObject, 0x157F7E32 );
		define_dynamic_cast( tSigmlReferenceObject, tObject );
	public:
		tFilePathPtr mReferencePath;
	public:
		tSigmlReferenceObject( );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual void fGetDependencyFiles( tFilePathPtrList& resourcePathsOut );
		virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );

		tResourceId& fConstructResourceId( tResourceId& rid ) const;
	};
	typedef tRefCounterPtr< tSigmlReferenceObject > tSigmlReferenceObjectPtr;


	///
	/// \brief Represents a "layer" of objects, used purely for editor and organizational purposes 
	/// (i.e., not used for any in-game purpose).
	class tools_export tObjectLayer
	{
	public:
		enum tState
		{
			cStateVisible,
			cStateHidden,
			cStateFrozen,
			cStateCount
		};
	private:
		std::string mName;
		tState mState;
		Math::tVec4f mColorRgba;
	public:
		explicit tObjectLayer( const std::string& name = "layer", tState state = cStateVisible, const Math::tVec4f& colorRgba = Math::tVec4f( 0.f, 0.f, 0.75f, 1.f ) );
		const std::string& fName( ) const { return mName; }
		void fSetName( const std::string& name ) { mName = name; }
		tState fState( ) const { return mState; }
		void fSetState( tState state ) { mState = state; }
		void fCycleState( ) { mState = ( tState )( ( ( u32 )mState + 1 ) % cStateCount ); }
		const Math::tVec4f& fColorRgba( ) const { return mColorRgba; }
		void fSetColorRgba( const Math::tVec4f& rgba ) { mColorRgba = rgba; }
		const char* tObjectLayer::fStateText( wxColour& colourOut ) const;

		template<class tSerializer>
		void fSerialize( tSerializer& s )
		{
			s( "Name", mName );
			s( "State", reinterpret_cast<u32&>( mState ) );
			s( "Color", mColorRgba );
		}
	};
	typedef tGrowableArray<tObjectLayer> tLayerList;

	///
	/// \class tGroundCoverLayer
	/// \brief 
	class tools_export tGroundCoverLayer
	{
	public:

		struct tElement {
			tFilePathPtr mSgPath;
			b32 mCastsShadow;
			f32 mFrequency;
			u32 mSpawnCount;

			inline tElement( ) : mCastsShadow( false ), mFrequency( 1 ), mSpawnCount( 1 ) { }

			inline b32 operator== ( const tElement & other ) const
			{ 
				return	other.mSgPath == mSgPath &&
						other.mFrequency == mFrequency &&
						other.mSpawnCount == mSpawnCount;
			}

			inline b32 operator== ( const tFilePathPtr & path )  const
			{ return path == mSgPath; }

			template<class tSerializer>
			void fSerializeXml( tSerializer& s )
			{ 
				s( "SgPath", mSgPath ); 
				s( "CastsShadow", mCastsShadow );
				s( "Frequency", mFrequency );
				s( "SpawnCount", mSpawnCount );
			}
		};

	public:
		
		

		tGroundCoverLayer( );
		tGroundCoverLayer( u32 uniqueId, const char * name );
		tGroundCoverLayer( const tGroundCoverLayer & other );

		u32 fUniqueId( ) const { return mUniqueId; }
		
		f32 fUnitSize( ) const { return mUnitSize; }
		void fSetUnitSize( f32 size ) { mUnitSize = size; }

		u32 fPaintUnits( ) const { return mPaintUnits; }
		void fSetPaintUnits( u32 units ) { mPaintUnits = units; }

		const std::string & fName( ) const { return mName; }
		void fSetName( const char * name ) { mName = name; }

		b32 fVisible( ) const { return mVisible; }
		void fSetVisible( b32 visible ) { mVisible = visible; }

		u32 fRotation( ) const { return mRotation; }
		void fSetRotation( u32 rot ) { mRotation = rot; }

		f32 fYRotationScale( ) const { return mYRotationScale; }
		void fSetYRotationScale( f32 scale ) { mYRotationScale = scale; }

		f32 fXZRotationScale( ) const { return mXZRotationScale; }
		void fSetXZRotationScale( f32 scale ) { mXZRotationScale = scale; }

		u32 fTranslation( )  const { return mTranslation; }
		void fSetTranslation( u32 trans ) { mTranslation = trans; }

		f32 fXZTranslationScale( ) const { return mXZTranslationScale; }
		void fSetXZTranslationScale( f32 offset ) { mXZTranslationScale = offset; }

		f32 fYTranslationScale( ) const { return mYTranslationScale; }
		void fSetYTranslationScale( f32 offset ) { mYTranslationScale = offset; }

		f32 fScaleRangeAdjustor( ) const { return mScaleRangeAdjustor; }
		void fSetScaleRangeAdjustor( f32 adj ) { mScaleRangeAdjustor = adj; }
		
		u32 fFarVisibility( ) const { return mFarVisibility; }
		void fSetFarVisibility( u32 visiblity ) { mFarVisibility = visiblity; }

		u32 fNearVisibility( ) const { return mNearVisibility; }
		void fSetNearVisibility( u32 visiblity ) { mNearVisibility = visiblity; }

		u32 fElementCount( ) const { return mElements.fCount( ); }
		const tElement * fElements( ) const { return mElements.fBegin( ); }
		void fSetElements( const tElement elements[], u32 count );
		const tElement * fFindElement( const tFilePathPtr & path ) const;
		tElement * fFindElement( const tFilePathPtr & path );
		
		tElement & fAddElement( const tFilePathPtr & path );
		void fRemoveElement( const tFilePathPtr & path );

		u32 fComputeMaxUnitSpawns( ) const;

		template<class tSerializer>
		void fSerializeXml( tSerializer& s )
		{
			s( "Id", mUniqueId );
			s( "UnitSize", mUnitSize );
			s( "PaintUnits", mPaintUnits );
			s( "Name", mName );
			s( "Visible", mVisible );
			s( "Rotation", mRotation );
			s( "YRotationScale", mYRotationScale );
			s( "XZRotationScale", mXZRotationScale );
			s( "Translation", mTranslation );
			s( "XZTranslationScale", mXZTranslationScale );
			s( "YTranslationScale", mYTranslationScale );
			s( "ScaleRangeAdjustor", mScaleRangeAdjustor );

			// Support old format with only "Visibility" - meaning far
			if( s.fIn( ) )
				s( "Visibility", mFarVisibility );

			s( "FarVisiblity", mFarVisibility );
			s( "NearVisiblity", mNearVisibility );

			s( "Elements", mElements );
		}

	private:

		f32 mUnitSize;
		u32 mPaintUnits;

		u32 mUniqueId;
		std::string mName;
		b32 mVisible;
		u32 mRotation;
		f32 mYRotationScale;
		f32 mXZRotationScale;
		u32 mTranslation;
		f32 mXZTranslationScale;
		f32 mYTranslationScale;
		f32 mScaleRangeAdjustor;
		u32 mFarVisibility;
		u32 mNearVisibility;
		tGrowableArray<tElement> mElements;
	};

	typedef tGrowableArray<tGroundCoverLayer> tGCLayerList;

	///
	/// \brief Encapsulates all the objects that make up a Sigml file.
	/// Purpose: allow building up of a file object using normal code
	/// data structures, then serializing to xml; or, alternatively,
	/// the opposite.
	class tools_export tFile : public tObjectProperties
	{
	public:
		static const char* fEditablePropDefaultLightDirectionName( ) { return "DefaultLight.Direction"; }
		static const char* fEditablePropDefaultCastsShadowName( ) { return "DefaultLight.CastsShadow"; }
		static const char* fEditablePropDefaultApplyInEditorName( ) { return "DefaultLight.ApplyInEditor"; }
		static const char* fEditablePropDefaultApplyInGameName( ) { return "DefaultLight.ApplyInGame"; }
		static const char* fEditablePropDefaultFrontColorName( ) { return "DefaultLight.ColorFront"; }
		static const char* fEditablePropDefaultBackColorName( ) { return "DefaultLight.ColorBack"; }
		static const char* fEditablePropDefaultRimColorName( ) { return "DefaultLight.ColorRim"; }
		static const char* fEditablePropDefaultAmbientColorName( ) { return "DefaultLight.ColorAmbient"; }
	public:
		u32					mVersion;
		tStringPtr			mModellingPackage;
		tFilePathPtr		mSrcFile;
		tFilePathPtrList	mExplicitDependencies;
		tObjectPtrArray		mObjects;
		tMeshPtrArray		mMeshes;
		tMaterialPtrArray	mMaterials;
		tFilePathPtr		mDiffuseMapAtlas;
		tFilePathPtr		mNormalMapAtlas;
		tDynamicArray<f32>	mHeightFieldMaterialTileFactors;
		tLayerList			mLayers;
		tLayerList			mPotentialVisibility;
		tGCLayerList		mGroundCovers;

	public:
		tFile( );
		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout );
		b32 fLoadXml( const tFilePathPtr& path );
		b32 fApplySkinFile( const tFilePathPtr& path );
		b32 fApplySkinFile( const tFile& skinFile );
		tObjectPtr fFindByEntityName( const tStringPtr& name ) const;
		tObjectPtr fFindObjectByGuid( u32 guid ) const;

		template<class T>
		T* fFindObjectByType( ) const
		{
			for( u32 i = 0; i < mObjects.fCount( ); ++i )
			{
				if( T* o = mObjects[ i ]->fDynamicCast<T>( ) )
					return o;
			}
			return NULL;
		}

		void fAddEntityDefProperties( tSceneGraphFile* sgFile, tSigmlConverter& sigmlConverter ) const;
		static b32 fIsPropertyDefaultLightRelated( const tEditableProperty& property );
		static void fConvertToDefaultLightDesc( tEditablePropertyTable& properties, tSceneGraphDefaultLight& desc );
		b32 fReplaceReferences( const tFilePathPtr& replace, const tFilePathPtr& with );

		const tGroundCoverLayer * fFindGroundCover( u32 id ) const;

	private:
		void fAddGlobalProperties( );
	};

}}

#endif//__Sigml__
