#include "ToolsPch.hpp"
#include "tEditable3dGridEntity.hpp"
#include "tEditableObjectContainer.hpp"
#include "t3dGridEntity.hpp"
#include "tEditablePropertyTypes.hpp"
#include "tSigmlConverter.hpp"

namespace Sig { namespace
{
	const f32 gDefaultRadius = 1.0f; // can never change this without breaking all existing dummy objects

	static const char cStrCellCounts[]="3dGrid.CellCounts";
}}

namespace Sig { namespace Sigml
{
	class tools_export t3dGridObject : public tObject
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( t3dGridObject, 0xD647667 );
	public:
		t3dGridObject( );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, t3dGridObject& o )
	{
	}

	register_rtti_factory( t3dGridObject, false );

	t3dGridObject::t3dGridObject( )
	{
	}

	void t3dGridObject::fSerialize( tXmlSerializer& s )
	{
		fSerializeXmlObject( s, *this );
	}
	void t3dGridObject::fSerialize( tXmlDeserializer& s )
	{
		fSerializeXmlObject( s, *this );
	}

	tEntityDef* t3dGridObject::fCreateEntityDef( tSigmlConverter& sigmlConverter )
	{
		t3dGridEntityDef* entityDef = new t3dGridEntityDef( );
		fConvertEntityDefBase( entityDef, sigmlConverter );
		const Math::tVec3i cellCounts = mEditableProperties.fGetValue<Math::tVec3i>( cStrCellCounts, Math::tVec3i(1,1,1) );
		entityDef->mCellCounts.x = ( u32 )cellCounts.x;
		entityDef->mCellCounts.y = ( u32 )cellCounts.y;
		entityDef->mCellCounts.z = ( u32 )cellCounts.z;
		entityDef->mBounds = Math::tAabbf( Math::tVec3f( -gDefaultRadius ), Math::tVec3f( +gDefaultRadius ) );
		return entityDef;
	}

	tEditableObject* t3dGridObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tEditable3dGridEntity( container, *this );
	}

}}




namespace Sig
{
	namespace
	{
		static const Math::tVec4f cDefaultTint = Math::tVec4f( 1.0f, 1.0f, 0.5f, 0.5f );
		static const Math::tVec4f cOverlappingTint = Math::tVec4f( 1.0f, 0.0f, 0.0f, 0.5f );
	}

	tEditable3dGridEntity::tEditable3dGridEntity( tEditableObjectContainer& container )
		: tEditableObject( container )
	{
		fAddEditableProperties( );
		fCommonCtor( );
	}

	tEditable3dGridEntity::tEditable3dGridEntity( tEditableObjectContainer& container, const Sigml::t3dGridObject& ao )
		: tEditableObject( container )
	{
		fAddEditableProperties( );
		fDeserializeBaseObject( &ao );
		fCommonCtor( );
	}

	void tEditable3dGridEntity::fAddEditableProperties( )
	{
		//mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( fEditablePropStateIndex( ), 0, -1, 254, 1, 0 ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyVec3i( cStrCellCounts, Math::tVec3i(1,1,1), 1, 1024, 1 ) ) );
	}

	void tEditable3dGridEntity::fCommonCtor( )
	{
		mCurrentTint = cDefaultTint;
		mRenderState = Gfx::tRenderState::cDefaultColorTransparent;
		mRenderState.fEnableDisable( Gfx::tRenderState::cPolyTwoSided, true );

		// set the dummy object's bounds as our own
		Math::tAabbf localSpaceBox = mContainer.fGetDummyBoxTemplate( ).fGetBounds( );
		localSpaceBox.mMin *= gDefaultRadius;
		localSpaceBox.mMax *= gDefaultRadius;
		fSetLocalSpaceMinMax( localSpaceBox.mMin, localSpaceBox.mMax );

		const Math::tMat3f xform( gDefaultRadius );

		// setup the box object
		mDummyBox->fSetRgbaTint( mCurrentTint );
		mDummyBox->fSetParentRelativeXform( xform );
		mDummyBox->fSetInvisible( false );

		// create lines
		mGridLines.fReset( new Gfx::tWorldSpaceLines( ) );
		mGridLines->fResetDeviceObjects( 
			Gfx::tDevice::fGetDefaultDevice( ), 
			mContainer.fGetSolidColorMaterial( ), 
			mContainer.fGetSolidColorGeometryAllocator( ), 
			mContainer.fGetSolidColorIndexAllocator( ) );
		mGridLines->fSpawnImmediate( *this );

		tEditableObject::fUpdateStateTint( );

		fRefreshLines( );
	}

	tEditable3dGridEntity::~tEditable3dGridEntity( )
	{
	}

	std::string tEditable3dGridEntity::fGetToolTip( ) const
	{
		if( fIsFrozen( ) ) return std::string( );
		const std::string name = tEditableObject::fGetToolTip( );
		if( name.length( ) > 0 )
			return "3dGrid - " + name;
		return "3dGrid";
	}

	void tEditable3dGridEntity::fOnMoved( b32 recomputeParentRelative )
	{
		tEditableObject::fOnMoved( recomputeParentRelative );

		tGrowableArray<tEditable3dGridEntity*> otherGrids;
		mContainer.fCollectAllByType<tEditable3dGridEntity>( otherGrids );

		b32 intersects = false;
		for( u32 i = 0; i < otherGrids.fCount( ); ++i )
		{
			if( otherGrids[ i ] == this )
				continue; // don't test against myself

			Math::tObbf otherObb = otherGrids[ i ]->fWorldSpaceObb( );
			otherObb = otherObb.fTransform( fWorldToObject( ) );

			if( Math::tIntersectionAabbObb<f32>( otherObb, fObjectSpaceBox( ) ).fIntersects( ) )
			{
				intersects = true;
				break;
			}
		}
		mCurrentTint = intersects ? cOverlappingTint : cDefaultTint;
		mDummyBox->fSetRgbaTint( mCurrentTint );
	}

	void tEditable3dGridEntity::fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint )
	{
		tEditableObject::fUpdateStateTint( entity, rgbaTint );
		if( &entity == mDummyBox.fGetRawPtr( ) )
		{
			if( fIsFrozen( ) )
				mDummyBox->fSetRgbaTint( Math::tVec4f( rgbaTint.x, rgbaTint.y, rgbaTint.z, 0.25f ) );
			else
				mDummyBox->fSetRgbaTint( mCurrentTint );
		}
	}

	Sigml::tObjectPtr tEditable3dGridEntity::fSerialize( b32 clone ) const
	{
		Sigml::t3dGridObject* ao = new Sigml::t3dGridObject( );
		fSerializeBaseObject( ao, clone );
		return Sigml::tObjectPtr( ao );
	}

	void tEditable3dGridEntity::fNotifyPropertyChanged( tEditableProperty& property )
	{
		tEditableObject::fNotifyPropertyChanged( property );

		if( property.fGetName( ) == cStrCellCounts )
			fRefreshLines( );
	}

	void tEditable3dGridEntity::fRefreshLines( )
	{
		if( !mGridLines )
			return;

		const u32 vtxColor = Gfx::tVertexColor( 0.f, 1.f, 0.f, 1.f ).fForGpu( );

		tGrowableArray<Gfx::tSolidColorRenderVertex> solidColorVerts;

		const Math::tVec3i cellCounts = mEditableProperties.fGetValue<Math::tVec3i>( cStrCellCounts, Math::tVec3i(1,1,1) );
		//log_line( 0, "cellCounts = (" << cellCounts.x << ", " << cellCounts.y << ", " << cellCounts.z << ")" );

		for( s32 x = 0; x < cellCounts.x-1; ++x )
		{
			const f32 t = (x+1.f) / cellCounts.x;
			fAddPlane( solidColorVerts, vtxColor, Math::tVec3f(Math::fLerp(-1.f,+1.f,t),0.f,0.f), 1.1f* Math::tVec3f::cYAxis, 1.1f* Math::tVec3f::cZAxis );
		}
		for( s32 y = 0; y < cellCounts.y-1; ++y )
		{
			const f32 t = (y+1.f) / cellCounts.y;
			fAddPlane( solidColorVerts, vtxColor, Math::tVec3f(0.f,Math::fLerp(-1.f,+1.f,t),0.f), 1.1f* Math::tVec3f::cXAxis, 1.1f* Math::tVec3f::cZAxis );
		}
		for( s32 z = 0; z < cellCounts.z-1; ++z )
		{
			const f32 t = (z+1.f) / cellCounts.z;
			fAddPlane( solidColorVerts, vtxColor, Math::tVec3f(0.f,0.f,Math::fLerp(-1.f,+1.f,t)), 1.1f* Math::tVec3f::cXAxis, 1.1f* Math::tVec3f::cYAxis );
		}

		mGridLines->fSetGeometry( solidColorVerts, false );
	}

	void tEditable3dGridEntity::fAddPlane( tGrowableArray<Gfx::tSolidColorRenderVertex>& verts, u32 vtxColor, const Math::tVec3f& center, const Math::tVec3f& e0, const Math::tVec3f& e1 )
	{
		verts.fPushBack( Gfx::tSolidColorRenderVertex( center - e0 + e1, vtxColor ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( center + e0 + e1, vtxColor ) );

		verts.fPushBack( Gfx::tSolidColorRenderVertex( center + e0 + e1, vtxColor ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( center + e0 - e1, vtxColor ) );

		verts.fPushBack( Gfx::tSolidColorRenderVertex( center + e0 - e1, vtxColor ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( center - e0 - e1, vtxColor ) );

		verts.fPushBack( Gfx::tSolidColorRenderVertex( center - e0 - e1, vtxColor ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( center - e0 + e1, vtxColor ) );
	}

}

