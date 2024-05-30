#include "ToolsPch.hpp"
#include "Fxml.hpp"
#include "tSceneGraphFile.hpp"
#include "tExporterToolbox.hpp"
#include "tFxmlConverter.hpp"
#include "tLoadInPlaceFileBase.hpp"


// editor
#include "Editor/tEditablePropertyTypes.hpp"
#include "Editor/tEditablePropertyColor.hpp"
#include "FxEditor/tSigFxParticleSystem.hpp"
#include "FxEditor/tSigFxAttractor.hpp"
#include "FxEditor/tSigFxMeshSystem.hpp"
#include "FxEditor/tSigFxLight.hpp"

namespace Sig { namespace Fxml
{
	using namespace FX;
	///
	/// \section Global functions.
	///

	const char* fGetFileExtension( )
	{
		return tFxFile::fGetFileExtension( 0 );
	}

	b32 fIsFxmlFile( const tFilePathPtr& path )
	{
		return tFxFile::fIsFile( path );
	}

	tFilePathPtr fFxmlPathToFxb( const tFilePathPtr& path )
	{
		return tFxFile::fFxmlPathToFxb( path );
	}

	tFilePathPtr fFxbPathToFxml( const tFilePathPtr& path )
	{
		return tFxFile::fFxbPathToFxml( path );
	}

}}


namespace Sig
{
	using namespace FX;

	template< class tSerializer >
	void fSerializeXmlObject( tSerializer& s, Math::tEulerAnglesf& o )
	{
		s( "x", o.x );
		s( "y", o.y );
		s( "z", o.z );
	}
	
	template< class tSerializer >
	void fSerializeXmlObject( tSerializer& s, Math::tQuatf& o )
	{
		s( "x", o.x );
		s( "y", o.y );
		s( "z", o.z );
		s( "w", o.w );
	}

	template< class tSerializer >
	void fSerializeXmlObject( tSerializer& s, tKeyframe& o )
	{
		f32 x = o.fX( );
		s( "T", x );
		o.fSetX( x );

		u32 id = o.fGetID( );

		if( id == Rtti::fGetClassId< f32 >( ) )
		{
			f32 val = o.fValue< f32 >( );
			s( "Value", val );
			o.fSetValue< f32 >( val );
		}
		else if( id == Rtti::fGetClassId< Math::tVec2f >( ) )
		{
			Math::tVec2f val = o.fValue< Math::tVec2f >( );
			s( "Value", val );
			o.fSetValue< Math::tVec2f >( val );
		}
		else if( id == Rtti::fGetClassId< Math::tVec3f >( ) )
		{
			Math::tVec3f val = o.fValue< Math::tVec3f >( );
			s( "Value", val );
			o.fSetValue< Math::tVec3f >( val );
		}
		else if( id == Rtti::fGetClassId< Math::tVec4f >( ) )
		{
			Math::tVec4f val = o.fValue< Math::tVec4f >( );
			s( "Value", val );
			o.fSetValue< Math::tVec4f >( val );
		}
		else if( id == Rtti::fGetClassId< Math::tQuatf >( ) )
		{
			Math::tQuatf val = o.fValue< Math::tQuatf >( );
			s( "Value", val );
			o.fSetValue< Math::tQuatf >( val );
		}
		else
		{
			sigassert( !"Unrecognized keyframe type!" );
		}
	}

	template< class tSerializer >
	void fSerializeXmlObject( tSerializer& s, tGraph& o )
	{
		f32 min = o.fMinRandomness( );
		f32 max = o.fMaxRandomness( );
		b32 keep = o.fKeepLastKeyValue( );
		b32 useLerp = o.fUseLerp( );
		b32 useRandoms = o.fUseRandoms( );

		if( min > max )
			fSwap( min, max );

		s( "MinRandomness", min );
		s( "MaxRandomness", max );
		s( "KeeplastValue", keep );
		s( "Keyframes", o.mKeyframes );
		s( "UseLerp", useLerp );
		s( "UseRandoms", useRandoms );		

		o.fSetMinRandomness( min );
		o.fSetMaxRandomness( max );
		o.fSetKeepLastKeyValue( keep );
		o.fSetUseLerp( useLerp );
		o.fSetUseRandoms( useRandoms );

		o.fBuildValues( );
	}

	template< class tSerializer >
	void fSerializeXmlObject( tSerializer& s, FX::tToolParticleSystemState& o )
	{
		o.fSerializeXmlObject< tSerializer >( s );
	}

	template< class tSerializer >
	void fSerializeXmlObject( tSerializer& s, FX::tState& o )
	{
		o.fSerializeXmlObject< tSerializer >( s );
	}

	template< class tSerializer >
	void fSerializeXmlObject( tSerializer& s, tToolAttractorData& o )
	{
		o.fSerializeXmlObject< tSerializer >( s );
	}

	template< class tSerializer >
	void fSerializeXmlObject( tSerializer& s, tAttractorData& o )
	{
		o.fSerializeXmlObject< tSerializer >( s );
	}

	template< class tSerializer >
	void fSerializeXmlObject( tSerializer& s, FxMeshSystem::tData& o )
	{
		o.fSerializeXmlObject< tSerializer >( s );
	}

	template< class tSerializer >
	void fSerializeXmlObject( tSerializer& s, tToolAnimatedLightData& o )
	{
		o.fSerializeXmlObject< tSerializer >( s );
	}
}


namespace Sig { namespace Fxml
{
	register_rtti_factory( tFxAttractorObject, false );
	register_rtti_factory( tFxParticleSystemObject, false );
	register_rtti_factory( tFxMeshSystemObject, false );
	register_rtti_factory( tFxLightObject, false );

	template< class tSerializer >
	void fSerializeXmlObject( tSerializer& s, tFxMeshSystemObject& o )
	{
		s( "FxMeshSystemName", o.mFxMeshSystemName );
		s( "ParticleSystemToSyncWith", o.mParticleSystemToSyncWith );
		s( "MeshResourceFile", o.mMeshResourceFile );
		s( "FxMeshSystemData", o.mFxMeshSystemData );
	}

	template< class tSerializer >
	void fSerializeXmlObject( tSerializer& s, tFxAttractorObject& o )
	{
		s( "AttractorName", o.mAttractorName );
		s( "ToolData", o.mToolData );

		// Remove the old Attractor Data slot whenever saving.
		if( s.fIn( ) )
			s( "AttractorData", o.mAttractorData );
		o.mOpened = true;

		// If old attractor data was loaded, copy it over to be new and better.
		if( o.mAttractorData && o.mAttractorData->mToolData )
		{
			o.mToolData.fReset( NEW_TYPED( tToolAttractorData )( o.mAttractorData->mToolData, o.mAttractorData->mToolData->fId( ) ) );
			delete o.mAttractorData->mToolData;
			o.mAttractorData->mToolData = NULL;
			o.mAttractorData.fRelease( );
		}
	}

	template< class tSerializer >
	void fSerializeXmlObject( tSerializer& s, tFxLightObject& o )
	{
		s( "LightData", o.mData );
	}

	namespace Deprecated
	{
		enum tParticleBlendMode
		{
			cNormal,
			cAdditive,
			cSubtractive,
			cReverseSubtract,
			cMin,
			cMax,
			cParticleBlendCount,
		};
		static void fConvertBlendMode( u32 oldBlendMode, u32& blendOp, u32& srcBlend, u32& dstBlend )
		{
			if( oldBlendMode >= cParticleBlendCount )
				return; // file was hopefully up to date, doesn't contain older style blend mode specification
			switch( oldBlendMode )
			{
			case cNormal:			blendOp = 0;										srcBlend = Gfx::tRenderState::cBlendSrcAlpha; dstBlend = Gfx::tRenderState::cBlendOneMinusSrcAlpha; break;
			case cAdditive:			blendOp = 0;										srcBlend = Gfx::tRenderState::cBlendSrcAlpha; dstBlend = Gfx::tRenderState::cBlendOne; break;
			case cSubtractive:		blendOp = Gfx::tRenderState::cBlendOpSubtract;		srcBlend = Gfx::tRenderState::cBlendSrcAlpha; dstBlend = Gfx::tRenderState::cBlendOneMinusSrcAlpha; break;
			case cReverseSubtract:	blendOp = Gfx::tRenderState::cBlendOpRevSubtract;	srcBlend = Gfx::tRenderState::cBlendSrcAlpha; dstBlend = Gfx::tRenderState::cBlendOneMinusSrcAlpha; break;
			case cMin:				blendOp = Gfx::tRenderState::cBlendOpMin;			srcBlend = Gfx::tRenderState::cBlendSrcAlpha; dstBlend = Gfx::tRenderState::cBlendOneMinusSrcAlpha; break;
			case cMax:				blendOp = Gfx::tRenderState::cBlendOpMax;			srcBlend = Gfx::tRenderState::cBlendSrcAlpha; dstBlend = Gfx::tRenderState::cBlendOneMinusSrcAlpha; break;
			}
		}
	}

	template< class tSerializer >
	void fSerializeXmlObject( tSerializer& s, tFxParticleSystemObject& o )
	{
		s( "ParticleSystemName", o.mParticleSystemName );
		s( "States", o.mStates );
		s( "ToolState", o.mToolState );
		s( "LocalSpace", o.mLocalSpace );
		s( "CameraDepthOffset", o.mCameraDepthOffset );
		s( "UpdateSpeedMultiplier", o.mUpdateSpeedMultiplier );
		s( "LoDFactor", o.mLodFactor );
		s( "GhostParticleFrequency", o.mGhostParticleFrequency );
		s( "GhostParticleLifetime", o.mGhostParticleLifetime );

		u32 type = ( u32 ) o.mEmitterType;
		s( "EmitterType", type );
		o.mEmitterType = ( tEmitterType ) type;

		if( s.fIn( ) )
		{
			u32 oldBlendMode = ~0;
			s( "BlendMode", oldBlendMode );
			Deprecated::fConvertBlendMode( oldBlendMode, o.mBlendOp, o.mSrcBlend, o.mDstBlend );
		}

		s( "BlendOp", o.mBlendOp );
		s( "SrcBlend", o.mSrcBlend );
		s( "mDstBlend", o.mDstBlend );

		u32 sort = ( u32 ) o.mSortMode;
		s( "SortMode", sort );
		o.mSortMode = ( tParticleSortMode ) sort;

		s( "Material", o.mMaterial );
		s( "MeshResourcePath", o.mMeshResourcePath );
	}


	//
	// tFxParticleSystemObject
	//

	void tFxParticleSystemObject::fSerialize( tXmlSerializer& s )		{ fSerializeXmlObject( s, *this ); }
	void tFxParticleSystemObject::fSerialize( tXmlDeserializer& s )		{ fSerializeXmlObject( s, *this ); }

	tFxParticleSystemObject::tFxParticleSystemObject( )
	{
		mUpdateSpeedMultiplier = 1.f;
		mLodFactor = 1.f;
		mGhostParticleFrequency = 60.f;
		mGhostParticleLifetime = 0.5f;	// as a percent of it's parents' life
		mLocalSpace = false;
		mBlendOp = 0;
		mSrcBlend = Gfx::tRenderState::cBlendSrcAlpha;
		mDstBlend = Gfx::tRenderState::cBlendOne;
	}

	void tFxParticleSystemObject::fGetDependencyFiles( tFilePathPtrList& resourcePathsOut )
	{
		tObject::fGetDependencyFiles( resourcePathsOut );
		if( mMaterial )
			mMaterial->fGetTextureResourcePaths( resourcePathsOut );
		if( mMeshResourcePath.fLength( ) > 0 )
			resourcePathsOut.fPushBack( mMeshResourcePath );
	}

	// create fxEntiyDef here
	tEntityDef* tFxParticleSystemObject::fCreateEntityDef( tFxmlConverter& fxmlConverter )
	{
		Gfx::tMaterial* mat = mMaterial->fCreateGfxMaterial( fxmlConverter, false );
		if( !mat )
		{
			log_warning( "You need to assign a custom material to your particle system!" );
			return 0;
		}

		tParticleSystemDef* entityDef = new tParticleSystemDef( );
		fConvertEntityDefBase( entityDef, fxmlConverter );

		// Use new tool state or convert old.
		if( mToolState )
		{
			entityDef->fAddFromToolState( *mToolState );
		}
		else
		{
			entityDef->fAddFromToolState( mStates[ 0 ]->fToolState( ) );
		}

		entityDef->mParticleSystemName = fxmlConverter.fAddLoadInPlaceStringPtr( mParticleSystemName.fCStr( ) );

		if( mMeshResourcePath.fLength( ) > 0 )
			entityDef->mMeshResource = fxmlConverter.fAddLoadInPlaceResourcePtr( tResourceId::fMake<tSceneGraphFile>( mMeshResourcePath ) );

		entityDef->mEmitterType = mEmitterType;
		entityDef->mLocalSpace = mLocalSpace;
		entityDef->mCameraDepthOffset = mCameraDepthOffset;
		entityDef->mUpdateSpeedMultiplier = mUpdateSpeedMultiplier;
		entityDef->mLodFactor = mLodFactor;
		entityDef->mGhostParticleFrequency = mGhostParticleFrequency;
		entityDef->mGhostParticleLifetime = mGhostParticleLifetime;
		entityDef->mRenderState = Gfx::tRenderState::cDefaultColorTransparent;
		entityDef->mRenderState.fEnableDisable( Gfx::tRenderState::cBlendOpMask, false );
		entityDef->mRenderState.fEnableDisable( mBlendOp, true );
		entityDef->mRenderState.fEnableDisable( Gfx::tRenderState::cPolyTwoSided, true );
		entityDef->mRenderState.fSetSrcBlendMode( ( Sig::Gfx::tRenderState::tBlendMode )mSrcBlend );
		entityDef->mRenderState.fSetDstBlendMode( ( Sig::Gfx::tRenderState::tBlendMode )mDstBlend );
		entityDef->mSortMode = mSortMode;
		entityDef->mMaterial = mat;
		return entityDef;
	}

	tEditableObject* tFxParticleSystemObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tSigFxParticleSystem( container, *this );
	}



	//
	// tFxAttractorObject
	//

	void tFxAttractorObject::fSerialize( tXmlSerializer& s )		{ fSerializeXmlObject( s, *this ); }
	void tFxAttractorObject::fSerialize( tXmlDeserializer& s )		{ fSerializeXmlObject( s, *this ); }

	void tFxAttractorObject::fGetDependencyFiles( tFilePathPtrList& resourcePathsOut )
	{
		tObject::fGetDependencyFiles( resourcePathsOut );
	}

	// create fxEntiyDef here
	tEntityDef* tFxAttractorObject::fCreateEntityDef( tFxmlConverter& fxmlConverter )
	{
		tParticleAttractorDef* entityDef = new tParticleAttractorDef( );
		fConvertEntityDefBase( entityDef, fxmlConverter );
		entityDef->mAttractorName = fxmlConverter.fAddLoadInPlaceStringPtr( mAttractorName.fCStr( ) );
		entityDef->fFromToolData( mToolData.fGetRawPtr() );		
		return entityDef;
	}

	tEditableObject* tFxAttractorObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tSigFxAttractor( container, *this );
	}


	//
	// tFxMeshSystemObject
	//

	void tFxMeshSystemObject::fSerialize( tXmlSerializer& s )		{ fSerializeXmlObject( s, *this ); }
	void tFxMeshSystemObject::fSerialize( tXmlDeserializer& s )		{ fSerializeXmlObject( s, *this ); }

	void tFxMeshSystemObject::fGetDependencyFiles( tFilePathPtrList& resourcePathsOut )
	{
		tObject::fGetDependencyFiles( resourcePathsOut );
		resourcePathsOut.fFindOrAdd( mMeshResourceFile );
	}

	// create fxEntiyDef here
	tEntityDef* tFxMeshSystemObject::fCreateEntityDef( tFxmlConverter& fxmlConverter )
	{
		tMeshSystemDef* entityDef = new tMeshSystemDef( );
		fConvertEntityDefBase( entityDef, fxmlConverter );
		entityDef->mFxMeshSystemName = fxmlConverter.fAddLoadInPlaceStringPtr( mFxMeshSystemName.fCStr( ) );
		entityDef->mParticleSystemToSyncWith = fxmlConverter.fAddLoadInPlaceStringPtr( mParticleSystemToSyncWith.fCStr( ) );
		entityDef->mMeshResourceFile = fxmlConverter.fAddLoadInPlaceStringPtr( mMeshResourceFile.fCStr( ) );
		entityDef->fFromToolData( mFxMeshSystemData->fToolData( ) );		
		return entityDef;
	}

	tEditableObject* tFxMeshSystemObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tSigFxMeshSystem( container, *this );
	}

	//
	// tFxLightObject
	//

	void tFxLightObject::fSerialize( tXmlSerializer& s )		{ fSerializeXmlObject( s, *this ); }
	void tFxLightObject::fSerialize( tXmlDeserializer& s )		{ fSerializeXmlObject( s, *this ); }

	void tFxLightObject::fGetDependencyFiles( tFilePathPtrList& resourcePathsOut )
	{
		tObject::fGetDependencyFiles( resourcePathsOut );
	}

	// create fxEntiyDef here
	tEntityDef* tFxLightObject::fCreateEntityDef( tFxmlConverter& fxmlConverter )
	{
		FX::tAnimatedLightDef* entityDef = new FX::tAnimatedLightDef( );
		fConvertEntityDefBase( entityDef, fxmlConverter );
		entityDef->mBinaryData = mData->fCreateBinaryData( );

		// Need to copy the relevant data from creating a tLightEntityDef
		entityDef->mCastsShadows = mEditableProperties.fGetValue( tEditableLightEntity::fEditablePropCastShadows( ), false );
		entityDef->mShadowIntensity = mEditableProperties.fGetValue( tEditableLightEntity::fEditablePropShadowIntensity( ), 0.2f );

		return entityDef;
	}

	tEditableObject* tFxLightObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tSigFxLight( container, *this );
	}

	
	///
	/// \section tFile
	///

	namespace
	{
		static u32 gFxmlVersion = 1;
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile& o )
	{
		s.fAsAttribute( "Version", o.mVersion );

		if( o.mVersion != gFxmlVersion )
		{
			log_warning( "Fxml file format is out of date -> Please re-export." );
			return;
		}

		s( "Objects", o.mObjects );
		s( "Lifetime", o.mLifetime );
		s( "Flags", o.mFlags );
	}

	tFile::tFile( )
		: mVersion( gFxmlVersion )
		, mLifetime( 5.f )
		, mFlags( 0 )
	{
		
	}

	b32 tFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		if( !ser.fSave( path, "Fxml", *this, promptToCheckout ) )
		{
			log_warning( "Couldn't save Fxml file [" << path << "]" );
			return false;
		}

		return true;
	}

	b32 tFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Fxml", *this ) || mObjects.fCount( ) == 0 )
		{
			log_warning( "Couldn't load Fxml file [" << path << "]" );
			return false;
		}

		return true;
	}
}}

