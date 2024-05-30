#ifndef __tHeightFieldPaintCursor__
#define __tHeightFieldPaintCursor__
#include "tEditorCursorControllerButton.hpp"
#include "Editor/tEditableTerrainEntity.hpp"
#include "Gfx/tSolidColorLines.hpp"

namespace Sig
{
	class tModifyTerrainHeightAction;
	class tModifyTerrainGroundCoverAction;

	class tModifyTerrainMaterialAction : public tEditorButtonManagedCursorAction
	{
		tEntityPtr mEntity;
		tTextureSysRam::tSurface mStartMask, mEndMask;
		tTextureSysRam::tSurface mStartIds, mEndIds;
	public:
		tModifyTerrainMaterialAction( tToolsGuiMainWindow& mainWindow, const tEntityPtr& entity  )
			: tEditorButtonManagedCursorAction( mainWindow )
			, mEntity( entity )
		{
			mStartMask.fCopyCpu( mStartMask.fGetReferenceDevice( ), mEntity->fDynamicCast< tEditableTerrainGeometry >( )->fGetMaskTexture( ) );
			mStartIds.fCopyCpu( mStartIds.fGetReferenceDevice( ), mEntity->fDynamicCast< tEditableTerrainGeometry >( )->fGetMtlIdsTexture( ) );
			fSetIsLive( true );
		}
		virtual void fUndo( )
		{
			mEntity->fDynamicCast< tEditableTerrainGeometry >( )->fGetMaskTexture( ).fCopyCpu( Gfx::tDevice::fGetDefaultDevice( ), mStartMask );
			mEntity->fDynamicCast< tEditableTerrainGeometry >( )->fGetMtlIdsTexture( ).fCopyCpu( Gfx::tDevice::fGetDefaultDevice( ), mStartIds );
			mEntity->fDynamicCast< tEditableTerrainGeometry >( )->fUpdateDynamicTextureReferences( );
		}
		virtual void fRedo( )
		{
			mEntity->fDynamicCast< tEditableTerrainGeometry >( )->fGetMaskTexture( ).fCopyCpu( Gfx::tDevice::fGetDefaultDevice( ), mEndMask );
			mEntity->fDynamicCast< tEditableTerrainGeometry >( )->fGetMtlIdsTexture( ).fCopyCpu( Gfx::tDevice::fGetDefaultDevice( ), mEndIds );
			mEntity->fDynamicCast< tEditableTerrainGeometry >( )->fUpdateDynamicTextureReferences( );
		}
		virtual void fEnd( )
		{
			mEndMask.fCopyCpu( mEndMask.fGetReferenceDevice( ), mEntity->fDynamicCast< tEditableTerrainGeometry >( )->fGetMaskTexture( ) );
			mEndIds.fCopyCpu( mEndIds.fGetReferenceDevice( ), mEntity->fDynamicCast< tEditableTerrainGeometry >( )->fGetMtlIdsTexture( ) );
			tEditorButtonManagedCursorAction::fEnd( );
		}
	};
	///
	/// \brief Base cursor type for all height-field painting, handles common goodies.
	class tHeightFieldPaintCursor : public tEditorButtonManagedCursorController
	{
	protected:
		b32 mPainting;

		Math::tVec2f mRadiusRange;
		
		f32 mSize;
		f32 mStrength;
		f32 mFalloff;
		f32 mShape; ///< [0,1], blends between circle and square

		tEntityPtr mPaintEntity;
		Gfx::tSolidColorLinesPtr mCursorLines;

	public:
		tHeightFieldPaintCursor( tEditorCursorControllerButton* button );
		virtual void fOnNextCursor( tEditorCursorController* nextController );
		virtual tEntityPtr fFilterHoverObject( const tEntityPtr& newHoverObject );

		inline void fSetSize( f32 size )			{ mSize = size; }
		inline void fSetStrength( f32 strength )	{ mStrength = strength; }
		inline void fSetFalloff( f32 falloff )		{ mFalloff = falloff; }
		inline void fSetShape( f32 shape )			{ mShape = shape; }

	protected:
		f32 fComputeRadius( ) const;
		f32 fComputeStrength( ) const;
		f32 fComputeFalloff( ) const;
		f32 fComputeShape( ) const;

		virtual void fOnTick( );
		virtual void fDoPaintAction( ) = 0;
		virtual void fBeginAction( ) = 0;
		virtual void fEndAction( ) = 0;

		virtual void fOnRenderCursor( 
			f32 radius, 
			const Math::tVec3f & centerInLocal, 
			tEditableTerrainGeometry * etg,
			u32 innerRingColor,
			tGrowableArray< Gfx::tSolidColorRenderVertex > & renderVerts ) { }

		void fBeginPaint( );
		void fEndPaint( );
		f32  fInnerRadius( f32 outerRadius ) const;
		void fHandleCursor( );
		void fRenderCursor( f32 radius, const Math::tVec3f& centerInLocal, tEditableTerrainGeometry* etg );
		void fAddRing( 
			tEditableTerrainGeometry* etg,
			f32 radius, 
			const Math::tVec3f& centerInLocal, 
			u32 color,
			tGrowableArray< Gfx::tSolidColorRenderVertex >& renderVerts );
		void fSubDivideCursorLine( 
			tEditableTerrainGeometry* etg,
			const Math::tVec3f& p0, const Math::tVec3f& p1,
			tGrowableArray< Gfx::tSolidColorRenderVertex >& addTo, 
			u32 vtxColor, u32 currentDepth );

		virtual tEntityPtr fPick( const Math::tRayf& ray, f32* bestTout = 0, tEntity* const* ignoreList = 0, u32 numToIgnore = 0 );
	};

	///
	/// \brief Cursor type for painting/modifying terrain vertex heights.
	class tHeightFieldVertexPaintCursor : public tHeightFieldPaintCursor
	{
		tRefCounterPtr<tModifyTerrainHeightAction> mCurrentAction;
	public:
		tHeightFieldVertexPaintCursor( tEditorCursorControllerButton* button );
		virtual ~tHeightFieldVertexPaintCursor( );

	protected:

		struct tModifyVertexArgs
		{
			u32 mX, mZ;
			f32 mPaintStrength, mSign, mDt;
			f32 mDistToCenter, mFalloff, mRawStrength;
			b32 mShift;
			b32 mCtrl;
			tEditableTerrainGeometry::tEditableVertices& mVertArray;

			tModifyVertexArgs( 
				u32 x, u32 z, f32 paintStrength, f32 sign, f32 dt, 
				f32 distToCenter, f32 falloff, f32 rawStrength, b32 shift, b32 ctrl,
				tEditableTerrainGeometry::tEditableVertices& vertArray )
				: mX( x )
				, mZ( z )
				, mPaintStrength( paintStrength )
				, mSign( sign )
				, mDt( dt )
				, mDistToCenter( distToCenter )
				, mFalloff( falloff )
				, mRawStrength( rawStrength )
				, mShift( shift )
				, mCtrl( ctrl )
				, mVertArray( vertArray )
			{
			}
		};

		virtual void fModifyVertex( tModifyVertexArgs& args ) = 0;

	private:
		virtual void fDoPaintAction( );
		virtual void fBeginAction( );
		virtual void fEndAction( );
	};


	///
	/// \brief Cursor type for painting/modifying terrain vertex heights.
	class tHeightFieldMaterialPaintCursor : public tHeightFieldPaintCursor
	{
		f32 mLuminosity;
		tRefCounterPtr<tModifyTerrainMaterialAction> mCurrentAction;
	public:
		tHeightFieldMaterialPaintCursor( tEditorCursorControllerButton* button );
		virtual ~tHeightFieldMaterialPaintCursor( );

		inline void fSetLuminosity( f32 l )			{ mLuminosity = l; }
		inline f32	fGetLuminosity( ) const			{ return mLuminosity; }

	protected:
		struct tPaintMaterialArgs
		{
			Gfx::tDynamicTextureVRam& mMaskTexture;
			Gfx::tDynamicTextureVRam& mMtlIdsTexture;
			f32 mCenterU, mCenterV;
			f32 mMinU, mMaxU;
			f32 mMinV, mMaxV;
			f32 mDt;

			tPaintMaterialArgs( Gfx::tDynamicTextureVRam& maskTexture, Gfx::tDynamicTextureVRam& mtlIdsTexture,
				f32 centerU, f32 centerV, f32 minU, f32 maxU, f32 minV, f32 maxV, f32 dt )
				: mMaskTexture( maskTexture ), mMtlIdsTexture( mtlIdsTexture )
				, mCenterU( centerU )
				, mCenterV( centerV )
				, mMinU( minU )
				, mMaxU( maxU )
				, mMinV( minV )
				, mMaxV( maxV )
				, mDt( dt )
			{
			}
		};

		virtual void fPaintMaterial( tPaintMaterialArgs& args ) = 0;

	private:
		virtual void fDoPaintAction( );
		virtual void fBeginAction( );
		virtual void fEndAction( );
	};

	namespace Sigml { class tGroundCoverLayer; }

	///
	/// \class tHeightFieldGroundCoverPaintCursor
	/// \brief 
	class tHeightFieldGroundCoverPaintCursor : public tHeightFieldPaintCursor
	{
	public:
		tHeightFieldGroundCoverPaintCursor( tEditorCursorControllerButton* buttonmMaskId );
		virtual ~tHeightFieldGroundCoverPaintCursor( );

		b32 fRenderCursorGrid( ) const { return mRenderCursorGrid; }
		void fSetRenderCursorGrid( b32 render ) { mRenderCursorGrid = render; }

	protected:

		struct tPaintGroundCoverArgs
		{
			u32 mX, mZ;
			f32 mPaintStrength, mDt;

			tEditableTerrainGeometry::tEditableGroundCoverMask & mMask;

			tPaintGroundCoverArgs( 
				tEditableTerrainGeometry::tEditableGroundCoverMask & mask,
				f32 sign, f32 dt)
				: mMask( mask ) 
				, mX( 0 )
				, mZ( 0 )
				, mPaintStrength( 0 )
				, mDt( dt )
			{ }
		};

		virtual void fModifyDensityTexel( tPaintGroundCoverArgs & args ) = 0;

	protected:

		virtual void fOnRenderCursor( 
			f32 radius, 
			const Math::tVec3f & centerInLocal, 
			tEditableTerrainGeometry * etg,
			u32 innerRingColor,
			tGrowableArray< Gfx::tSolidColorRenderVertex > & renderVerts );

		Sigml::tGroundCoverLayer * mPaintLayer;
		b32 mRenderCursorGrid;

	private:

		virtual void fDoPaintAction( );
		virtual void fBeginAction( );
		virtual void fEndAction( );

	private:

		
		tRefCounterPtr<tModifyTerrainGroundCoverAction> mCurrentAction;
	};

}

#endif//__tHeightFieldPaintCursor__
