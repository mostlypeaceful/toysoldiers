#include "ToolsPch.hpp"
#include "tSigFxAttractor.hpp"
#include "Gfx/tMaterialFile.hpp"
#include "Fxml.hpp"
#include "Editor/tEditorSelectionList.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "tResourceDepot.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tSingleVectorRenderable, just for visualization
	//------------------------------------------------------------------------------
	tSingleVectorRenderable::tSingleVectorRenderable(
		const Gfx::tMaterialPtr& material, 
		const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
		const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator )
		: mCurrentScale( 0.f )
	{
		fResetDeviceObjects( material, geometryAllocator, indexAllocator );
		fGenerate( 1.f );
	}

	tSingleVectorRenderable::~tSingleVectorRenderable( )
	{
		
	}

	void tSingleVectorRenderable::fResetDeviceObjects( const Gfx::tMaterialPtr& material, const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator )
	{
		mMaterial = material;
		mGeometry.fResetDeviceObjects( geometryAllocator, indexAllocator );
	}
	
	void tSingleVectorRenderable::fGenerate( f32 scale )
	{
		if( mCurrentScale == scale )
			return;

		mCurrentScale = scale;

		tGrowableArray< Gfx::tSolidColorRenderVertex > verts;
		tGrowableArray< u16 > ids;

		const u32 numSlices = 16;
		const u32 numVertsPerAxis = 2 + 2 * numSlices;
		const f32 longAxisLen = mCurrentScale;
		const f32 shortAxisLen = 0.1f;

		for( u32 iaxis = 0; iaxis < 3; ++iaxis )
		{
			if( iaxis != 1 )
				continue;

			const u32 axisColor = Gfx::tVertexColor( iaxis==0?1.f:0.f, iaxis==1?1.f:0.f, iaxis==2?1.f:0.f ).fForGpu( );

			const u32 iu = ( iaxis + 1 ) % 3;
			const u32 iv = ( iaxis + 2 ) % 3;

			// first we create the cylinder

			Math::tVec3f centralAxis = Math::tVec3f::cZeroVector;
			centralAxis.fAxis( iaxis ) = -longAxisLen;

			Math::tVec3f radialU = Math::tVec3f::cZeroVector;
			radialU.fAxis( iu ) = shortAxisLen;

			Math::tVec3f radialV = Math::tVec3f::cZeroVector;
			radialV.fAxis( iv ) = shortAxisLen;

			// insert both "end-points" first
			verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f::cZeroVector, axisColor ) );
			verts.fPushBack( Gfx::tSolidColorRenderVertex( 1.25f * centralAxis, axisColor ) );

			// insert the two "end-circles"
			for( u32 islice = 0; islice < numSlices; ++islice )
			{
				const f32 u = std::sin( Math::c2Pi * ( f32 )islice / ( f32 )numSlices );
				const f32 v = std::cos( Math::c2Pi * ( f32 )islice / ( f32 )numSlices );
				const Math::tVec3f p = u * radialU + v * radialV;
				const Math::tVec3f n = Math::tVec3f( p ).fNormalize( );

				const f32 shade = fMax( 0.f, 0.5f * n.x + n.y ) + 0.5f;
				const u32 axisColor = Gfx::tVertexColor( iaxis==0?shade:0.f, iaxis==1?shade:0.f, iaxis==2?shade:0.f ).fForGpu( );

				verts.fPushBack( Gfx::tSolidColorRenderVertex( p, axisColor ) );
				verts.fPushBack( Gfx::tSolidColorRenderVertex( p + centralAxis, axisColor ) );
			}

			// create the triangles
			for( u32 islice = 0; islice < numSlices; ++islice )
			{
				const u32 baseVertex = 0;//iaxis * numVertsPerAxis;

				const u32 prevSlice = ( islice == 0 ) ? numSlices - 1 : islice - 1;
				const u32 currSlice = islice;

				ids.fPushBack( baseVertex + 0 );
				ids.fPushBack( baseVertex + 2 + prevSlice * 2 );
				ids.fPushBack( baseVertex + 2 + currSlice * 2 );
				//fAddLastTriangle( mAxisTris[ iaxis ], verts, ids );

				ids.fPushBack( baseVertex + 2 + prevSlice * 2 );
				ids.fPushBack( baseVertex + 2 + prevSlice * 2 + 1 );
				ids.fPushBack( baseVertex + 2 + currSlice * 2 );
				//fAddLastTriangle( mAxisTris[ iaxis ], verts, ids );

				ids.fPushBack( baseVertex + 2 + currSlice * 2 );
				ids.fPushBack( baseVertex + 2 + prevSlice * 2 + 1 );
				ids.fPushBack( baseVertex + 2 + currSlice * 2 + 1 );
				//fAddLastTriangle( mAxisTris[ iaxis ], verts, ids );

				ids.fPushBack( baseVertex + 1 );
				ids.fPushBack( baseVertex + 2 + currSlice * 2 + 1 );
				ids.fPushBack( baseVertex + 2 + prevSlice * 2 + 1 );
				//fAddLastTriangle( mAxisTris[ iaxis ], verts, ids );
			}
		}

		fBake( verts, ids );
	}

	void tSingleVectorRenderable::fBake( const tGrowableArray<Gfx::tSolidColorRenderVertex>& verts, const tGrowableArray<u16>& ids )
	{
		u32 numPrims = verts.fCount( ) * 2;
		if( !mGeometry.fAllocateGeometry( *mMaterial, verts.fCount( ), ids.fCount( ), numPrims ) )
		{
			fSetRenderBatch( Gfx::tRenderBatchPtr( ) );
			return; // couldn't get no geometry!
		}

		// copy vertices over
		Sig::byte* gpuVerts = mGeometry.fLockBuffer( );
		fMemCpy( gpuVerts, verts.fBegin( ), verts.fCount( ) * mGeometry.fGetGeometryAllocator( )->fVertexFormat( ).fVertexSize( ) );
		mGeometry.fUnlockBuffer( gpuVerts );

		// and now the indices...
		Sig::byte* gpuIndices = mGeometry.fLockIndices( );
		fMemCpy( gpuIndices, ids.fBegin( ), ids.fCount( ) * mGeometry.fGetIndexAllocator( )->fIndexFormat( ).mSize );
		mGeometry.fUnlockIndices( gpuIndices );

		fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
	}

	//------------------------------------------------------------------------------
	// tSigFxAttractor
	//------------------------------------------------------------------------------
	tSigFxAttractor::tSigFxAttractor( tEditableObjectContainer& container )
		: tEditableObject( container )
	{
		mToolData.fReset( NEW_TYPED( FX::tToolAttractorData )() );
		fCommonCtor( *container.fGetResourceDepot( ) );
	}
	
	tSigFxAttractor::tSigFxAttractor( tEditableObjectContainer& container, const Fxml::tFxAttractorObject& fxa )
		: tEditableObject( container )
	{
		fDeserializeBaseObject( &fxa );

		u32 id = mGuid;
		if( fxa.mOpened )
			id = fxa.mToolData->fId( );
		mToolData.fReset( NEW_TYPED( FX::tToolAttractorData )( fxa.mToolData.fGetRawPtr(), id ) );

		fCommonCtor( *container.fGetResourceDepot( ) );

		fSetAttractorName( fxa.mAttractorName );
	}

	void tSigFxAttractor::fClone( const tSigFxAttractor& attractor )
	{
		FX::tParticleAttractorPtr data = attractor.fGetAttractor( );
		mToolData.fReset( NEW_TYPED( FX::tToolAttractorData )( mToolData.fGetRawPtr(), mGuid ) );
		mAttractor->fSetData( mToolData->fCreateBinaryData() );

		std::string cloneName = attractor.fAttractorName( ).fCStr( );
		cloneName += " Clone";
		mAttractor->fSetAttractorName( tStringPtr( cloneName ) );
	}

	tSigFxAttractor::~tSigFxAttractor( )
	{
		mSingleVector->fDeleteImmediate( );
	}

	void tSigFxAttractor::fCommonCtor( tResourceDepot& resDep )
	{
		mAttractor.fReset( new FX::tParticleAttractor( ) );
		mAttractor->fSetData( mToolData->fCreateBinaryData() );
		mAttractor->fUpdateGraphValues( 0.f );

		//Math::tVec4f color = tRandom::fSubjectiveRand( ).fColor( 0.3f );
		mDummySphere->fSetRgbaTint( Math::tVec4f( 0.65f, 0.75f, 0.85f, 0.3f ) );
		mAttractor->fSpawnImmediate( *this );

		Math::tMat3f directionGeomOffset = Math::tMat3f::cIdentity;
		directionGeomOffset.fSetTranslation( Math::tVec3f( 0.f, 0.f, 1.f ) );

		tEditableObjectContainer& container = fGetContainer( );
		mSingleVector = new tSingleVectorRenderable( container.fGetSolidColorMaterial( )
													,container.fGetSolidColorGeometryAllocator( )
													,container.fGetSolidColorIndexAllocator( ) );

		mSingleVector->fSpawnImmediate( *this );

		mHidden = false;
		mLastOpenGraphIdx = 0;
	}


	void tSigFxAttractor::fOnSpawn( )
	{
		fOnPause( false );
		tEditableObject::fOnSpawn( );
	}

	void tSigFxAttractor::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListThinkST );
		}
		else
		{
			fRunListInsert( cRunListThinkST );
		}
		tEditableObject::fOnPause( paused );
	}

	void tSigFxAttractor::fThinkST( f32 dt )
	{
		// Sync the graphs over if there are any changes.
		fSyncToBinary( );

		f32 scale = mAttractor->fCurrentScale( );
		Math::tAabbf bounds( -scale, scale );
		fSetLocalSpaceMinMax( bounds.mMin, bounds.mMax );

		Math::tMat3f xform( scale * fObjectToWorld( ).fGetScale( ) );
		xform.fSetTranslation( mAttractor->fWorldPosition( ) );
		
		mDummySphere->fSetRgbaTint( Math::tVec4f( mAttractor->fCurrentColor( ).fXYZ( ), 0.3f ) );
		mDummySphere->fSetInvisible( mHidden );
		mDummySphere->fMoveTo( xform );
		mSelectionBox->fMoveTo( xform );
		
		mSingleVector->fSetInvisible( mHidden );

		if( mSingleVector )
		{
			FX::tForceType type = mAttractor->fForceType( );

			if( type == FX::cGravity || type == FX::cPlaneCollide )
			{
				mSingleVector->fSetRgbaTint( Math::tVec4f( 1.f ) );

				f32 gravity = mAttractor->fCurrentGravity( );
				mSingleVector->fGenerate( gravity );

				const Math::tMat3f xform( mAttractor->fObjectToWorld( ) );
				Math::tMat3f vectorxform( Math::tQuatf( xform ), mAttractor->fWorldPosition( ) );
				mSingleVector->fMoveTo( vectorxform );
			}
			else
			{
				mSingleVector->fSetRgbaTint( Math::tVec4f( 0.f ) );
			}
		}
	}

	void tSigFxAttractor::fSetLocalSpaceMinMax( const Math::tVec3f& min, const Math::tVec3f& max )
	{
		tEditableObject::fSetLocalSpaceMinMax( min, max );
		mDummySphere->fSetObjectSpaceBox( fObjectSpaceBox( ) );
		mDummySphere->fSetParentRelativeXform( fObjectSpaceBox( ).fAdjustMatrix( Math::tMat3f::cIdentity, 0.25f ) );
	}

	Sigml::tObjectPtr tSigFxAttractor::fSerialize( b32 clone ) const
	{
		Fxml::tFxAttractorObject* fxa = new Fxml::tFxAttractorObject( );
		fSerializeBaseObject( fxa, clone );
		fxa->mName = fAttractorName( ).fCStr( );
		fxa->mAttractorName = fAttractorName( );
		fxa->mToolData.fReset( mToolData.fGetRawPtr() );
		return Sigml::tObjectPtr( fxa );
	}

	void tSigFxAttractor::fSyncToBinary( )
	{
		FX::tBinaryAttractorData& state = mAttractor->fGetAttractorData( );
		state.mType = mToolData->fForceType( );
		state.mParticleMustBeInRadius = mToolData->fParticleMustBeInRadius( );
		state.mAffectParticlesDirection = mToolData->fAffectParticlesDirection( );
		state.mId = mToolData->fId( ); // Hopefully this one shouldn't be changing
		state.mFlags = mToolData->fFlags( );


		// Check for changes to the graph shapes and update the built versions on the particle system if necessary.
		for( u32 i = 0; i < mToolData->fNumGraphs( ); ++i )
		{
			if( !mToolData->fGraph( i )->fGetDirty( ) )
				continue;

			state.mGraphs[ i ]->fCopyFromGraph( mToolData->fGraph( i ) );

			mToolData->fGraph( i )->fClean( );
		}
	}
}

