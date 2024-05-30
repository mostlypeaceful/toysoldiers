#include "BasePch.hpp"
#include "tMaterial.hpp"
#include "tMaterialFile.hpp"
#include "tTextureFile.hpp"
#include "tCamera.hpp"
#include "tDrawCall.hpp"
#include "tRenderContext.hpp"
#include "tResourceDepot.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		void fApplyObjectToWorldVS_FaceNone( Math::tMat3f& objectToWorldPos, Math::tMat3f& objectToWorldNormal, const tDrawCall& drawCall, const tRenderContext& context )
		{
			objectToWorldNormal		= drawCall.fRenderInstance( ).fRI_ObjectToWorld( );
			objectToWorldPos		= objectToWorldNormal;
		}
		void fApplyObjectToWorldVS_FaceAll( Math::tMat3f& objectToWorldPos, Math::tMat3f& objectToWorldNormal, const tDrawCall& drawCall, const tRenderContext& context )
		{
			const tRenderInstance& instance = drawCall.fRenderInstance( );

			// for facing, we transform the normal "normally" (as though it weren't facing)
			objectToWorldNormal = instance.fRI_ObjectToWorld( );

			const Math::tMat3f* objectToLocal = instance.fRI_ObjectToLocal( );
			const Math::tMat3f* localToObject = instance.fRI_LocalToObject( );

			if( objectToLocal && localToObject )
			{
				const Math::tMat3f localToWorld = instance.fRI_ObjectToWorld( ) * *localToObject;

				// get the scale of the object
				const Math::tVec3f scale = localToWorld.fGetScale( );

				// align final matrix to camera axes, but account for local-to-world scale
				objectToWorldPos.fXAxis( context.mCamera->fXAxis( ) * scale.x );
				objectToWorldPos.fYAxis( context.mCamera->fYAxis( ) * scale.y );
				objectToWorldPos.fZAxis( context.mCamera->fZAxis( ) * scale.z );

				// now offset by original object to local
				objectToWorldPos = objectToWorldPos * *objectToLocal;

				// set position
				objectToWorldPos.fSetTranslation( instance.fRI_ObjectToWorld( ).fGetTranslation( ) );
			}
			else
			{
				// get the scale of the object
				const Math::tVec3f scale = instance.fRI_ObjectToWorld( ).fGetScale( );

				// align final matrix to camera axes, but account for object scale
				objectToWorldPos.fXAxis( context.mCamera->fXAxis( ) * scale.x );
				objectToWorldPos.fYAxis( context.mCamera->fYAxis( ) * scale.y );
				objectToWorldPos.fZAxis( context.mCamera->fZAxis( ) * scale.z );

				// set position
				objectToWorldPos.fSetTranslation( instance.fRI_ObjectToWorld( ).fGetTranslation( ) );
			}
		}
		void fApplyObjectToWorldVS_FaceX( Math::tMat3f& objectToWorldPos, Math::tMat3f& objectToWorldNormal, const tDrawCall& drawCall, const tRenderContext& context )
		{
			fApplyObjectToWorldVS_FaceAll( objectToWorldPos, objectToWorldNormal, drawCall, context );
		}
		void fApplyObjectToWorldVS_FaceY( Math::tMat3f& objectToWorldPos, Math::tMat3f& objectToWorldNormal, const tDrawCall& drawCall, const tRenderContext& context )
		{
			fApplyObjectToWorldVS_FaceAll( objectToWorldPos, objectToWorldNormal, drawCall, context );
		}
		void fApplyObjectToWorldVS_FaceZ( Math::tMat3f& objectToWorldPos, Math::tMat3f& objectToWorldNormal, const tDrawCall& drawCall, const tRenderContext& context )
		{
			fApplyObjectToWorldVS_FaceAll( objectToWorldPos, objectToWorldNormal, drawCall, context );
		}
		void fApplyObjectToWorldVS_FaceXY( Math::tMat3f& objectToWorldPos, Math::tMat3f& objectToWorldNormal, const tDrawCall& drawCall, const tRenderContext& context )
		{
			fApplyObjectToWorldVS_FaceAll( objectToWorldPos, objectToWorldNormal, drawCall, context );
		}
		void fApplyObjectToWorldVS_FaceYZ( Math::tMat3f& objectToWorldPos, Math::tMat3f& objectToWorldNormal, const tDrawCall& drawCall, const tRenderContext& context )
		{
			fApplyObjectToWorldVS_FaceAll( objectToWorldPos, objectToWorldNormal, drawCall, context );
		}
		void fApplyObjectToWorldVS_FaceXZ( Math::tMat3f& objectToWorldPos, Math::tMat3f& objectToWorldNormal, const tDrawCall& drawCall, const tRenderContext& context )
		{
			fApplyObjectToWorldVS_FaceAll( objectToWorldPos, objectToWorldNormal, drawCall, context );
		}

		typedef void (*tApplyObjectToWorldVS)( Math::tMat3f& objectToWorldPos, Math::tMat3f& objectToWorldNormal, const tDrawCall& drawCall, const tRenderContext& context );
		
		tFixedArray<tApplyObjectToWorldVS,8> gApplyObjectToWorldVSLUT;

		define_static_function( fSetupApplyObjectToWorldVSLUT )
		{
			gApplyObjectToWorldVSLUT[ 0 ]															= fApplyObjectToWorldVS_FaceNone;
			gApplyObjectToWorldVSLUT[ tMaterial::cFaceX ]											= fApplyObjectToWorldVS_FaceX;
			gApplyObjectToWorldVSLUT[ tMaterial::cFaceY ]											= fApplyObjectToWorldVS_FaceY;
			gApplyObjectToWorldVSLUT[ tMaterial::cFaceZ ]											= fApplyObjectToWorldVS_FaceZ;
			gApplyObjectToWorldVSLUT[ tMaterial::cFaceX | tMaterial::cFaceY ]						= fApplyObjectToWorldVS_FaceXY;
			gApplyObjectToWorldVSLUT[ tMaterial::cFaceY | tMaterial::cFaceZ ]						= fApplyObjectToWorldVS_FaceYZ;
			gApplyObjectToWorldVSLUT[ tMaterial::cFaceX | tMaterial::cFaceZ ]						= fApplyObjectToWorldVS_FaceXZ;
			gApplyObjectToWorldVSLUT[ tMaterial::cFaceX | tMaterial::cFaceY | tMaterial::cFaceZ ]	= fApplyObjectToWorldVS_FaceAll;
		}
	}

	tMaterial::tMaterial( )
		: mMaterialFile( 0 )
		, mMaterialFlags( 0 )
	{
	}

	tMaterial::tMaterial( tNoOpTag )
		: mRenderState( cNoOpTag )
	{
	}

	tMaterial::~tMaterial( )
	{
		fCleanupOwnedMaterialFileResourcePtr( );
	}

	void tMaterial::fSetMaterialFileResourcePtrUnOwned( tLoadInPlaceResourcePtr* mtlFileResPtr )
	{
		fCleanupOwnedMaterialFileResourcePtr( );

		mMaterialFile = mtlFileResPtr;
	}

	void tMaterial::fSetMaterialFileResourcePtrOwned( tResourceDepot& resDepot, const tFilePathPtr& materialFilePath )
	{
		fSetMaterialFileResourcePtrOwned( resDepot.fQuery( tResourceId::fMake< tMaterialFile >( materialFilePath ) ) );
	}

	void tMaterial::fSetMaterialFileResourcePtrOwned( const tResourcePtr& resourcePtr )
	{
		sigassert( resourcePtr->fGetClassId( ) == Rtti::fGetClassId< tMaterialFile >( ) );

		fCleanupOwnedMaterialFileResourcePtr( );

		mMaterialFlags |= cBehaviorOwnsMaterialFileResource;
		mMaterialFile = NEW tLoadInPlaceResourcePtr;	
		mMaterialFile->mResourcePtr.fConstruct( );
		mMaterialFile->mResourcePtr.fTreatAsObject( ) = resourcePtr;
	}

	void tMaterial::fSetFacingFlags( b32 x, b32 y, b32 z )
	{
		// clear facing flags
		mMaterialFlags &= ~cFaceMask;

		// now or in specified flags
		if( x ) mMaterialFlags |= cFaceX;
		if( y ) mMaterialFlags |= cFaceY;
		if( z ) mMaterialFlags |= cFaceZ;
	}

	void tMaterial::fApplyObjectToWorldVS( const tDevicePtr& device, u32 posId, u32 normalId, const tDrawCall& drawCall, const tRenderContext& context ) const
	{
		const tRenderInstance& instance = drawCall.fRenderInstance( );

		if( (mMaterialFlags & cFaceMask) == 0 )
		{
			fApplyMatrix3VS( device, posId, instance.fRI_ObjectToWorld( ) );
			if( normalId != posId )
				fApplyMatrix3VS( device, normalId, instance.fRI_ObjectToWorld( ) );
		}
		else
		{
			Math::tMat3f objectToWorldPos, objectToWorldNormal;

			gApplyObjectToWorldVSLUT[ mMaterialFlags & cFaceMask ]( objectToWorldPos, objectToWorldNormal, drawCall, context );

			fApplyMatrix3VS( device, posId, objectToWorldPos );

			if( normalId != posId )
				fApplyMatrix3VS( device, normalId, objectToWorldNormal );
		}
	}

	void tMaterial::fApplyTexture( const tDevicePtr& device, u32& slot, tLoadInPlaceResourcePtr* texture ) const
	{
		if( texture )
		{
			const tResourcePtr& texResource = texture->fGetResourcePtr( );
			const tTextureFile* t = texResource->fCast<tTextureFile>( );
			sigassert( t );

			t->fApply( device, slot );
			++slot;
		}
	}

	void tMaterial::fCleanupOwnedMaterialFileResourcePtr( )
	{
		if( mMaterialFlags & cBehaviorOwnsMaterialFileResource )
		{
			mMaterialFlags &= ~cBehaviorOwnsMaterialFileResource;
			if( mMaterialFile )
			{
				mMaterialFile->mResourcePtr.fDestroy( );
				delete mMaterialFile;
				mMaterialFile = 0;
			}
		}
	}

}}
