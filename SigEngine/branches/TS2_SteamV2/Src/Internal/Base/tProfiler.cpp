#include "BasePch.hpp"
#include "tProfiler.hpp"
#include "tResourceDepot.hpp"
#include "Threads/tThread.hpp"
#include "Gfx/tScreen.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gui/tColoredQuad.hpp"
#include "Gui/tText.hpp"

#if defined( sig_profile ) && defined( sig_devmenu )

namespace Sig
{
	namespace
	{
		enum tProfilerDisplayMode
		{
			cProfilerDisplayNone,
			cProfilerDisplayPerf,
			cProfilerDisplayMem,
			cProfilerDisplayCount
		};

		u32 gLocalRandGroupSeed = 314158880;
		static tRandom gLocalGroupRand( gLocalRandGroupSeed );

		u32 gLocalRandCatSeed = 314159265;
		static tRandom gLocalCategoryRand( gLocalRandCatSeed );

		static const f32 cTopPct		= 0.15f;
		static const f32 cBotPct		= 0.85f;
		static const f32 cLeftPct		= 0.2f;
		static const f32 cIndent		= 0.0125f;
		static const f32 cRightPct		= 0.8f;
		static const f32 cWidthPct		= cRightPct - cLeftPct;
		static const f32 cMaxMs			= 33.333f;
		static const f32 cMaxMB			= 256.0f;
		static const u32 cNumTickMarks	= 5;
		static s32 gDisplayMode			= cProfilerDisplayNone;
		static s32 gDisplayGroup		= -1;
		static b32 gShuttingDown		= false;
		static b32 gDisplayGroupDirty	= false;

		struct tPerfDisplayCategory
		{
			tStringPtr	mDisplayName;
			f32			mTimeMs;
			tPerfDisplayCategory( ) 
				: mTimeMs( 0.f ) { }
			explicit tPerfDisplayCategory( const tStringPtr& displayName, f32 timeMs = 0.f ) 
				: mDisplayName( displayName ), mTimeMs( timeMs ) { }
			inline b32 operator==( const tPerfDisplayCategory& cat ) const { return mDisplayName == cat.mDisplayName; }
			inline b32 operator==( const tStringPtr& name ) const { return mDisplayName == name; }
		};

		struct tPerfDisplayGroup
		{
			tStringPtr	mDisplayName;
			f32			mTimeMs;
			tGrowableArray< tPerfDisplayCategory > mCategories;

			tPerfDisplayGroup( ) 
				: mTimeMs( 0.f ) { }
			explicit tPerfDisplayGroup( const tStringPtr& displayName, f32 timeMs = 0.f ) 
				: mDisplayName( displayName ), mTimeMs( timeMs ) { }
			inline b32 operator==( const tPerfDisplayGroup& cat ) const { return mDisplayName == cat.mDisplayName; }
			inline b32 operator==( const tStringPtr& name ) const { return mDisplayName == name; }
		};

		struct tPerfDisplayGroupList
		{
			tGrowableArray< tPerfDisplayGroup > mGroups;
			u32 mTotalItems;

			tPerfDisplayGroupList( ) : mTotalItems( 0 ) { }
			void fClear( )
			{
				mTotalItems = 0;
				mGroups.fSetCount( 0 );
			}
		};

		static void fObtainPerfDisplayCategories( tPerfDisplayGroupList& list, const tGrowableArray< tProfiler::tPerfCategory >& categories )
		{
			list.fClear( );

			// find group to expand
			tStringPtr expand;
			if( gDisplayGroup >= 0 && gDisplayGroup < (s32)tProfiler::fInstance( ).fPerfGroupNames( ).fCount( ) )
				expand = tProfiler::fInstance( ).fPerfGroupNames( )[ gDisplayGroup ];

			for( u32 i = 0; i < categories.fCount( ); ++i )
			{
				const tProfiler::tPerfCategory& realCat = categories[ i ];

				if( realCat.mGroupName.fNull( ) ) continue;

				tPerfDisplayGroup dg( realCat.mGroupName );
				tPerfDisplayGroup* displayGroup = list.mGroups.fFind( dg );
				if( !displayGroup )
				{
					list.mGroups.fPushBack( dg );
					displayGroup = &list.mGroups.fBack( );
					++list.mTotalItems;
				}

				if( !realCat.mExcludeFromGroupTime ) displayGroup->mTimeMs += realCat.mTimeMs / realCat.mNumberOfThreads;

				if( realCat.mGroupName == expand )
				{
					tPerfDisplayCategory dc( realCat.mDisplayName );
					tPerfDisplayCategory* displayCat = displayGroup->mCategories.fFind( dc );
					if( !displayCat )
					{
						displayGroup->mCategories.fPushBack( dc );
						displayCat = &displayGroup->mCategories.fBack( );
						++list.mTotalItems;
					}

					displayCat->mTimeMs += realCat.mTimeMs / realCat.mNumberOfThreads;
				}
			}
		}

		void fChangeProfilerDisplayMode( tDevCallback::tArgs& args )
		{
			if( args.mEvent == tDevCallback::cEventTypeSetValue )
			{
				if( args.mValueText == "Performance" ) gDisplayMode = cProfilerDisplayPerf;
				else if( args.mValueText == "Memory" ) gDisplayMode = cProfilerDisplayMem;
				else gDisplayMode = cProfilerDisplayNone;
			}
			else
				gDisplayMode = Math::fModulus( gDisplayMode + args.fGetIncrementValue( ), (s32)cProfilerDisplayCount );
			
			switch( gDisplayMode )
			{
			case cProfilerDisplayNone:	args.mValueText = "None"; break;
			case cProfilerDisplayPerf:	args.mValueText = "Performance"; break;
			case cProfilerDisplayMem:	args.mValueText = "Memory"; break;
			default:					args.mValueText = "None"; break;
			}
		}

		void fChangeProfilerDisplayGroup( tDevCallback::tArgs& args )
		{
			tGrowableArray< tStringPtr >& groupNames = tProfiler::fInstance( ).fPerfGroupNames( );
			
			if( args.mEvent == tDevCallback::cEventTypeSetValue )
			{
				gDisplayGroup = -1;
				
				for( u32 i = 0; i < groupNames.fCount( ); ++i )
				{
					if( args.mValueText == std::string( groupNames[ i ].fCStr( ) ) )
					{
						gDisplayGroup = i;
						break;
					}
				}
			}
			else
				gDisplayGroup = Math::fModulus( gDisplayGroup + args.fGetIncrementValue( ), (s32)groupNames.fCount( ) + 1 );
			
			if( gDisplayGroup >= 0 && gDisplayGroup < (s32)groupNames.fCount( ) )
				args.mValueText = groupNames[ gDisplayGroup ].fCStr( );
			else
				args.mValueText = "None";

			gDisplayGroupDirty = true;
		}
	}

	devcb( Debug_Profiler_DisplayMode, "None", make_delegate_cfn( tDevCallback::tFunction, fChangeProfilerDisplayMode ) );
	devcb( Debug_Profiler_DisplayPerfGroup, "None", make_delegate_cfn( tDevCallback::tFunction, fChangeProfilerDisplayGroup ) );

	class tProfilerDisplayEntry : public tRefCounter
	{
		Gui::tColoredQuad								mBar;
		Gui::tText										mName;
		Gui::tText										mCurrentValue;
		tFixedArray<Gui::tColoredQuad,cNumTickMarks>	mTickMarks;
		tFixedArray<Gui::tText,cNumTickMarks>			mTickMarkNames;
		u32												mIndentDepth;

	public:
		tProfilerDisplayEntry( Gfx::tScreen& screen, const tStringPtr& name, u32 indentDepth, const tFixedArray<std::string,cNumTickMarks>& tickMarkNames, b32 showTickNames, const Math::tVec4f& color )
			: mIndentDepth( indentDepth )
		{
			Gfx::tDefaultAllocators& defGuiAllocators = Gfx::tDefaultAllocators::fInstance( );

			const Math::tVec4f rgba = color;
			mBar.fSetRect( Math::tVec2f::cOnesVector );
			mBar.fSetRgbaTint( rgba );

			tResourcePtr devFont = defGuiAllocators.mResourceDepot->fQuery( tResourceId::fMake<Gui::tFont>( Gui::tFont::fDevFontPath( ) ) );
			mName.fSetFont( devFont );
			mName.fBake( name.fCStr( ), 0, Gui::tText::cAlignRight );
			mName.fSetRgbaTint( 1.f );

			mCurrentValue.fSetFont( devFont );
			mCurrentValue.fBake( "", 0, Gui::tText::cAlignLeft );
			mCurrentValue.fSetRgbaTint( 1.f );

			for( u32 i = 0; i < mTickMarks.fCount( ); ++i )
			{
				mTickMarks[ i ].fSetRect( Math::tVec2f( 2.f, mName.fHeight( ) ) );
				mTickMarks[ i ].fSetRgbaTint( 1.f );
			}

			if( showTickNames )
			{
				for( u32 i = 0; i < mTickMarks.fCount( ); ++i )
				{
					mTickMarkNames[ i ].fSetFont( devFont );
					mTickMarkNames[ i ].fBake( tickMarkNames[ i ].c_str( ), 0, Gui::tText::cAlignCenter );
					mTickMarkNames[ i ].fSetRgbaTint( 1.f );
				}
			}
		}
		f32 fRender( f32 y, Gfx::tScreen& screen, f32 actualValue, f32 maxValue )
		{
			const f32 xIndent =  mIndentDepth * cIndent * screen.fCreateOpts( ).mBackBufferWidth;
			const f32 x = xIndent + cLeftPct * screen.fCreateOpts( ).mBackBufferWidth;
			const f32 barWidth = ( actualValue / maxValue ) * cWidthPct * screen.fCreateOpts( ).mBackBufferWidth;
			const Math::tVec3f scale = Math::tVec3f( barWidth, mName.fHeight( ), 1.f );

			mBar.fSetPosition( Math::tVec3f( x, y, 0.0001f ) );
			mBar.fSetRect( Math::tVec2f( scale.x, scale.y ) );
			screen.fAddScreenSpaceDrawCall( mBar.fDrawCall( ) );

			mName.fSetPosition( Math::tVec3f( x - 5.f, y, 0.0f ) );
			Gfx::tDrawCall name = mName.fDrawCall( );
			if( name.fValid( ) )
				screen.fAddScreenSpaceDrawCall( name );

			std::stringstream ss; ss << std::fixed << std::setprecision( 1 ) << actualValue;
			mCurrentValue.fBake( ss.str( ).c_str( ), 0, Gui::tText::cAlignLeft );
			mCurrentValue.fSetPosition( Math::tVec3f( mBar.fPosition( ).x + 6.f, y, 0.0f ) );
			screen.fAddScreenSpaceDrawCall( mCurrentValue.fDrawCall( ) );

			for( u32 i = 1; i < mTickMarks.fCount( ); ++i )
			{
				const f32 tt = i / ( mTickMarks.fCount( ) - 1.f );
				const f32 tx = xIndent + Math::fLerp( cLeftPct * screen.fCreateOpts( ).mBackBufferWidth, cRightPct * screen.fCreateOpts( ).mBackBufferWidth, tt );
				mTickMarks[ i ].fSetPosition( Math::tVec3f( tx, y, 0.f ) );
				screen.fAddScreenSpaceDrawCall( mTickMarks[ i ].fDrawCall( ) );
				mTickMarkNames[ i ].fSetPosition( Math::tVec3f( tx, y, 0.f ) );

				Gfx::tDrawCall tickMarkName = mTickMarkNames[ i ].fDrawCall( );
				if( tickMarkName.fValid( ) )
					screen.fAddScreenSpaceDrawCall( tickMarkName );
			}

			return mBar.fLocalRect( ).fHeight( );
		}
	};
	typedef tRefCounterPtr<tProfilerDisplayEntry> tProfilerDisplayPtr;

	class tProfilerRenderImpl
	{
		tGrowableArray< tProfilerDisplayPtr > mPerfEntries;
		tGrowableArray< tProfilerDisplayPtr > mMemEntries;
	public:
		void fRenderPerf( Gfx::tScreen& screen, const tPerfDisplayGroupList& display, b32 initOnly = false )
		{
			if( mPerfEntries.fCount( ) != display.mTotalItems || gDisplayGroupDirty )
				fInitPerfEntries( screen, display );

			if( initOnly )
				return;

			f32 y = cTopPct * screen.fCreateOpts( ).mBackBufferHeight;
			u32 index = 0;
			for( u32 g = 0; g < display.mGroups.fCount( ); ++g )
			{
				const tPerfDisplayGroup& group = display.mGroups[ g ];
				y += mPerfEntries[ index++ ]->fRender( y, screen, group.mTimeMs, cMaxMs ) + 6.f;


				for( u32 c = 0; c < group.mCategories.fCount( ); ++c )
					y += mPerfEntries[ index++ ]->fRender( y, screen, group.mCategories[ c ].mTimeMs, cMaxMs ) + 6.f;
			}
		}
		void fRenderMem( 
			Gfx::tScreen& screen, 
			const tGrowableArray<tProfiler::tMemCategory>& display,
			const tGrowableArray< tPair<const Gfx::tGeometryBufferVRamAllocator*,tStringPtr> >& geomAllocators,
			const tGrowableArray< tPair<const Gfx::tIndexBufferVRamAllocator*,tStringPtr> >& idxAllocators,
			b32 initOnly = false )
		{
			const u32 totalDisplayCount = 
				display.fCount( ) +
				geomAllocators.fCount( ) +
				idxAllocators.fCount( );
			if( mMemEntries.fCount( ) != totalDisplayCount )
				fInitMemEntries( screen, display, geomAllocators, idxAllocators );

			if( initOnly )
				return;

			const f32 cGroupGap = 8.f;
			const f32 cItemGap = 1.f;

			u32 curParentId = cProfilerCategoryNull;
			f32 y = cTopPct * screen.fCreateOpts( ).mBackBufferHeight;
			for( u32 i = 0; i < display.fCount( ); ++i )
			{
				y += mMemEntries[ i ]->fRender( y, screen, display[ i ].fInMB( ), cMaxMB );

				if( i < display.fCount( ) - 1 )
				{
					const u32 nextParentId = display[ i + 1 ].mParentCategory;
					if( nextParentId == cProfilerCategoryNull )
						y += cGroupGap;
					else
						y += cItemGap;
					curParentId = nextParentId;
				}
			}

			y += cGroupGap;
			for( u32 i = 0; i < geomAllocators.fCount( ); ++i )
			{
				y += mMemEntries[ display.fCount( ) + i ]->fRender( y, screen, geomAllocators[ i ].mA->fTotalAllocated( )/(1024.f*1024.f), geomAllocators[ i ].mA->fBufferSize( )/(1024.f*1024.f) );
				y += cItemGap;
			}
			y += cGroupGap;
			for( u32 i = 0; i < idxAllocators.fCount( ); ++i )
			{
				y += mMemEntries[ display.fCount( ) + geomAllocators.fCount( ) + i ]->fRender( y, screen, idxAllocators[ i ].mA->fTotalAllocated( )/(1024.f*1024.f), idxAllocators[ i ].mA->fBufferSize( )/(1024.f*1024.f) );
				y += cItemGap;
			}
		}
	private:
		void fInitPerfEntries( Gfx::tScreen& screen, const tPerfDisplayGroupList& display )
		{
			gLocalGroupRand = tRandom( gLocalRandGroupSeed ); //reseed so we get consistant colors
			gLocalCategoryRand = tRandom( gLocalRandCatSeed );

			tFixedArray<std::string,cNumTickMarks> tickMarkNames;
			for( u32 i = 0; i < tickMarkNames.fCount( ); ++i )
			{
				std::stringstream ss; ss << std::fixed << std::setprecision( 1 ) << ( cMaxMs * ( i / ( cNumTickMarks - 1.f ) ) ) << " ms";
				tickMarkNames[ i ] = ss.str( );
			}

			mPerfEntries.fSetCount( display.mTotalItems );
			u32 pIndex = 0;

			for( u32 g = 0; g < display.mGroups.fCount( ); ++g )
			{
				const tPerfDisplayGroup& group = display.mGroups[ g ];
				mPerfEntries[ pIndex++ ].fReset( NEW tProfilerDisplayEntry( screen, group.mDisplayName, 0, tickMarkNames, pIndex==0, gLocalGroupRand.fColor( 0.75f ) ) );

				for( u32 c = 0; c < group.mCategories.fCount( ); ++c )
					mPerfEntries[ pIndex++ ].fReset( NEW tProfilerDisplayEntry( screen, group.mCategories[ c ].mDisplayName, 1, tickMarkNames, pIndex==0, gLocalCategoryRand.fColor( 0.75f ) ) );
			}

			gDisplayGroupDirty = false;
		}
		void fInitMemEntries( 
			Gfx::tScreen& screen, 
			const tGrowableArray<tProfiler::tMemCategory>& display,
			const tGrowableArray< tPair<const Gfx::tGeometryBufferVRamAllocator*,tStringPtr> >& geomAllocators,
			const tGrowableArray< tPair<const Gfx::tIndexBufferVRamAllocator*,tStringPtr> >& idxAllocators )
		{
			gLocalCategoryRand = tRandom( gLocalRandCatSeed ); //reseed so we get consistant colors

			tFixedArray<std::string,cNumTickMarks> tickMarkNames;
			for( u32 i = 0; i < tickMarkNames.fCount( ); ++i )
			{
				std::stringstream ss; ss << std::fixed << std::setprecision( 1 ) << ( cMaxMB * ( i / ( cNumTickMarks - 1.f ) ) ) << " MB";
				tickMarkNames[ i ] = ss.str( );
			}

			mMemEntries.fSetCount( display.fCount( ) + geomAllocators.fCount( ) + idxAllocators.fCount( ) );
			for( u32 i = 0; i < display.fCount( ); ++i )
				mMemEntries[ i ].fReset( NEW tProfilerDisplayEntry( screen, display[ i ].mDisplayName, 0, tickMarkNames, i==0, gLocalGroupRand.fColor( 0.75f ) ) );
			for( u32 i = 0; i < geomAllocators.fCount( ); ++i )
			{
				const f32 maxMB = geomAllocators[ i ].mA->fBufferSize( )/(1024.f*1024.f);
				for( u32 j = 0; j < tickMarkNames.fCount( ); ++j )
				{
					std::stringstream ss; ss << std::fixed << std::setprecision( 1 ) << ( maxMB * ( j / ( cNumTickMarks - 1.f ) ) ) << " MB";
					tickMarkNames[ j ] = ss.str( );
				}
				mMemEntries[ display.fCount( ) + i ].fReset( NEW tProfilerDisplayEntry( screen, geomAllocators[ i ].mB, 0, tickMarkNames, true, gLocalGroupRand.fColor( 0.75f ) ) );
			}
			for( u32 i = 0; i < idxAllocators.fCount( ); ++i )
			{
				const f32 maxMB = idxAllocators[ i ].mA->fBufferSize( )/(1024.f*1024.f);
				for( u32 j = 0; j < tickMarkNames.fCount( ); ++j )
				{
					std::stringstream ss; ss << std::fixed << std::setprecision( 1 ) << ( maxMB * ( j / ( cNumTickMarks - 1.f ) ) ) << " MB";
					tickMarkNames[ j ] = ss.str( );
				}
				mMemEntries[ display.fCount( ) + geomAllocators.fCount( ) + i ].fReset( NEW tProfilerDisplayEntry( screen, idxAllocators[ i ].mB, 0, tickMarkNames, true, gLocalGroupRand.fColor( 0.75f ) ) );
			}
		}
	};
}


namespace Sig
{
	tScopedProfile::tScopedProfile( u32 category )
		: mCategory( category )
		, mTimeMs( 0.f )
	{
		if( mCategory != cProfilerCategoryNull )
		{
			begin_pix( tProfiler::fInstance( ).mPerfCategories[ mCategory ].mDisplayName.fCStr( ) );

			if( gDisplayMode == cProfilerDisplayPerf )			
				mBegin = Time::fGetStamp( );
		}
	}
	tScopedProfile::~tScopedProfile( )
	{
		if( mCategory != cProfilerCategoryNull )
		{
			if( gDisplayMode == cProfilerDisplayPerf )
			{
				mTimeMs = Time::fGetElapsedMs( mBegin, Time::fGetStamp( ) );
				tProfiler::fInstance( ).fEndProfile( *this );
			}

			end_pix( );
		}
	}

	tProfiler::tProfiler( )
		: mRenderer( NEW tProfilerRenderImpl )
	{
		const u32 numThreads = Threads::tThread::fHardwareThreadCount( );

		fAddMemCategory( cProfileMemMain,				tMemCategory( tStringPtr( "RAM - Main" ) ) );
		fAddMemCategory( cProfileMemRes,				tMemCategory( tStringPtr( "RAM - Res" ) ) );
		fAddMemCategory( cProfileMemResOver,			tMemCategory( tStringPtr( "  Over" ) ) );
		fAddMemCategory( cProfileMemResTemp,			tMemCategory( tStringPtr( "  Temp" ) ) );
		fAddMemCategory( cProfileMemSqrat,				tMemCategory( tStringPtr( "  Sqrat" ) ) );
		fAddMemCategory( cProfileMemScript,				tMemCategory( tStringPtr( "  Script" ) ) );
		fAddMemCategory( cProfileMemXAlloc,				tMemCategory( tStringPtr( "RAM - Phys" ) ) );
		fAddMemCategory( cProfileMemVideo,				tMemCategory( tStringPtr( "RAM - Video" ) ) );
		fAddMemCategory( cProfileMemTexture,			tMemCategory( tStringPtr( "  Texture" ), cProfileMemVideo ) );
		fAddMemCategory( cProfileMemGeometry,			tMemCategory( tStringPtr( "  Geometry" ), cProfileMemVideo ) );
		fAddMemCategory( cProfileMemAudio,				tMemCategory( tStringPtr( "RAM - Audio" ) ) );
		fAddMemCategory( cProfileMemAudioSys,			tMemCategory( tStringPtr( "  System" ), cProfileMemAudio ) );
		fAddMemCategory( cProfileMemAudioPhys,			tMemCategory( tStringPtr( "  Physical" ), cProfileMemAudio ) );

		fAddPerfCategory( cProfilePerfRunListAct,		tPerfCategory( tStringPtr( "RunList - Act" )	, tStringPtr( "Main" ), 1 ) );
		fAddPerfCategory( cProfilePerfScriptEventHandlers, tPerfCategory( tStringPtr( "Script Event Handlers" )	, tStringPtr( "Main" ), 1 ) );
		fAddPerfCategory( cProfilePerfGameEffects,		tPerfCategory( tStringPtr( "Game Effects" )	, tStringPtr( "Misc" ), 1 ) );
		fAddPerfCategory( cProfilePerfRunListAnimate,	tPerfCategory( tStringPtr( "RunList - Animate" ), tStringPtr( "Main" ), 1 ) );
		fAddPerfCategory( cProfilePerfRunListCollide,	tPerfCategory( tStringPtr( "RunList - Collide" ), tStringPtr( "Main" ), 1 ) );
		fAddPerfCategory( cProfilePerfRunListPhysics,	tPerfCategory( tStringPtr( "RunList - Physics" ), tStringPtr( "Main" ), 1 ) );
		fAddPerfCategory( cProfilePerfRunListMove,		tPerfCategory( tStringPtr( "RunList - Move" )	, tStringPtr( "Main" ), 1 ) );
		fAddPerfCategory( cProfilePerfRunListEffects,	tPerfCategory( tStringPtr( "RunList - Effects" ), tStringPtr( "Main" ), 1 ) );
		fAddPerfCategory( cProfilePerfRunListThink,		tPerfCategory( tStringPtr( "RunList - Think" )	, tStringPtr( "Main" ), 1 ) );
		fAddPerfCategory( cProfilePerfRunListCamera,	tPerfCategory( tStringPtr( "RunList - Camera" )	, tStringPtr( "Main" ), 1 ) );
		fAddPerfCategory( cProfilePerfRunListCoRender,	tPerfCategory( tStringPtr( "RunList - CoRender" )	, tStringPtr( "Main" ), 1 ) );
		fAddPerfCategory( cProfilePerfWorldSpaceSCST,	tPerfCategory( tStringPtr( "WorldSpaceControls" ) , tStringPtr( "Main" ), 1 ) );

		fAddPerfCategory( cProfilePerfOnSpawn,	tPerfCategory( tStringPtr( "OnSpawn" ) , tStringPtr( "Main" ), 1 ) );

		fAddPerfCategory( cProfilePerfStepBoneProxiesST, tPerfCategory( tStringPtr( "StepBoneProxiesST" )	, tStringPtr( "AnimationST" ), 1 ) );
		fAddPerfCategory( cProfilePerfCullDeadTracksST, tPerfCategory( tStringPtr( "CullDeadTracksST" )		, tStringPtr( "AnimationST" ), 1 ) );
		fAddPerfCategory( cProfilePerfSkeletonEventsST, tPerfCategory( tStringPtr( "SkeletonEventsST" )		, tStringPtr( "AnimationST" ), 1 ) );
		
		fAddPerfCategory( cProfilePerfUpdateAudio,		tPerfCategory( tStringPtr( "Update" )				, tStringPtr( "Audio" ), 1 ) );
		fAddPerfCategory( cProfilePerfAudioEvent,		tPerfCategory( tStringPtr( "Events" )				, tStringPtr( "Audio" ), 1 ) );
		
		fAddPerfCategory( cProfilePerfGatherGroundCover,	tPerfCategory( tStringPtr( "Gather" )			, tStringPtr( "GroundCover" ), 1 ) );
		fAddPerfCategory( cProfilePerfPrepareRenderables,	tPerfCategory( tStringPtr( "Prepare" )			, tStringPtr( "GroundCover" ), 1 ) );

		fAddPerfCategory( cProfilePerfParticlesMoveST,		tPerfCategory( tStringPtr( "ParticlesMoveST" )		, tStringPtr( "Effects" ), 1 ) );
		fAddPerfCategory( cProfilePerfParticlesEffectsMT,	tPerfCategory( tStringPtr( "ParticlesEffectsMT" )	, tStringPtr( "Effects" ), numThreads ) );
		fAddPerfCategory( cProfilePerfParticlesThinkST,		tPerfCategory( tStringPtr( "ParticlesMoveST" )		, tStringPtr( "Effects" ), 1 ) );
		fAddPerfCategory( cProfilePerfParticlesMeshesMoveST,		tPerfCategory( tStringPtr( "MeshesMoveST" )		, tStringPtr( "Effects" ), 1 ) );
		fAddPerfCategory( cProfilePerfParticlesMeshesEffectsMT,		tPerfCategory( tStringPtr( "MeshesEffectsMT" )	, tStringPtr( "Effects" ), numThreads ) );
		fAddPerfCategory( cProfilePerfParticlesFXThinkST,			tPerfCategory( tStringPtr( "FXEntityThinkST" )	, tStringPtr( "Effects" ), 1 ) );
		fAddPerfCategory( cProfilePerfLightEffectsMoveST,			tPerfCategory( tStringPtr( "LightsMoveST" )		, tStringPtr( "Effects" ), 1 ) );

		fAddPerfCategory( cProfilePerfDebrisLogicMoveST		, tPerfCategory( tStringPtr( "MoveST" )		, tStringPtr( "Debris" ), 1 ) );
		fAddPerfCategory( cProfilePerfDebrisLogicCoRenderMT , tPerfCategory( tStringPtr( "~CoRenderMT" )	, tStringPtr( "Debris" ), numThreads, true ) );



		fAddPerfCategory( cProfilePerfBuildLightLists,	tPerfCategory( tStringPtr( "LightLists" )		, tStringPtr( "Render" ), 1 ) );
		fAddPerfCategory( cProfilePerfBuildDisplayLists,tPerfCategory( tStringPtr( "DisplayLists" )		, tStringPtr( "Render" ), 1 ) );
		fAddPerfCategory( cProfilePerfRenderWorld,		tPerfCategory( tStringPtr( "World" )			, tStringPtr( "Render" ), 1 ) );
		fAddPerfCategory( cProfilePerfRenderShadowMaps,	tPerfCategory( tStringPtr( "ShadowMaps" )		, tStringPtr( "Render" ), 1 ) );
		fAddPerfCategory( cProfilePerfRenderShadowMapsQuery,	tPerfCategory( tStringPtr( "Query" )		, tStringPtr( "ShadowMaps" ), 1, true ) );
		fAddPerfCategory( cProfilePerfRenderShadowMapsRender,	tPerfCategory( tStringPtr( "Render" )		, tStringPtr( "ShadowMaps" ), 1, true ) );
		fAddPerfCategory( cProfilePerfOnTickCanvas,		tPerfCategory( tStringPtr( "Gui" )				, tStringPtr( "Render" ), 1 ) );
		fAddPerfCategory( cProfilePerfOnRenderCanvas,	tPerfCategory( tStringPtr( "Gui" )				, tStringPtr( "Render" ), 1 ) );
	}
	tProfiler::~tProfiler( )
	{
		fShutdown( );
	}
	void tProfiler::fAddPerfCategory( u32 id, const tPerfCategory& cat )
	{
		if( gShuttingDown ) return;

		Threads::tMutex threadLock( mCritSec );

		if( id >= mPerfCategories.fCount( ) )
			mPerfCategories.fSetCount( id + 1 );
		mPerfCategories[ id ] = cat;

		mPerfGroupNames.fFindOrAdd( cat.mGroupName );
	}
	void tProfiler::fAddMemCategory( u32 id, const tMemCategory& cat )
	{
		if( gShuttingDown ) return;

		Threads::tMutex threadLock( mCritSec );

		if( id >= mMemCategories.fCount( ) )
			mMemCategories.fSetCount( id + 1 );
		mMemCategories[ id ] = cat;
	}
	void tProfiler::fNewFrame( )
	{
		if( gShuttingDown ) return;

		Threads::tMutex threadLock( mCritSec );

		for( u32 i = 0; i < mPerfCategories.fCount( ); ++i )
			mPerfCategories[ i ].mTimeMs = 0.f;
	}
	void tProfiler::fTrackMemory( u32 id, s32 bytesDelta )
	{
		if( gShuttingDown ) return;

		Threads::tMutex threadLock( mCritSec );

		mMemCategories[ id ].mTotalBytes += bytesDelta;
		sigassert( mMemCategories[ id ].mTotalBytes >= 0 );

		// TODO REFACTOR display sub-items/parent-items in a nice intuitive way
		const u32 parentId = mMemCategories[ id ].mParentCategory;
		if( parentId != cProfilerCategoryNull )
		{
			mMemCategories[ parentId ].mTotalBytes += bytesDelta;
			sigassert( mMemCategories[ parentId ].mTotalBytes >= 0 );
		}
	}
	void tProfiler::fSetPerfTime( u32 id, f32 time )
	{
		if( gShuttingDown ) return;

		Threads::tMutex threadLock( mCritSec );

		mPerfCategories[ id ].mTimeMs += time;
	}
	void tProfiler::fTrackVRAMAllocator( const Gfx::tGeometryBufferVRamAllocator& allocator, const char* name, b32 remove )
	{
		sigassert( remove || name );

		for( u32 i = 0; i < mGeomAllocators.fCount( ); ++i )
		{
			if( mGeomAllocators[ i ].mA == &allocator )
			{
				if( remove )
					mGeomAllocators.fErase( i );
				return; // already present
			}
		}
		if( !remove )
			mGeomAllocators.fPushBack( fMakePair( &allocator, tStringPtr( name ) ) );
	}
	void tProfiler::fTrackVRAMAllocator( const Gfx::tIndexBufferVRamAllocator& allocator, const char* name, b32 remove )
	{
		sigassert( remove || name );

		for( u32 i = 0; i < mIdxAllocators.fCount( ); ++i )
		{
			if( mIdxAllocators[ i ].mA == &allocator )
			{
				if( remove )
					mIdxAllocators.fErase( i );
				return; // already present
			}
		}
		if( !remove )
			mIdxAllocators.fPushBack( fMakePair( &allocator, tStringPtr( name ) ) );
	}
	s32 tProfiler::fQueryGeomAlloc( )
	{
		s32 total = 0;
		for( u32 i = 0; i < mGeomAllocators.fCount( ); ++i )
			total += mGeomAllocators[ i ].mA->fBufferSize( );
		for( u32 i = 0; i < mIdxAllocators.fCount( ); ++i )
			total += mIdxAllocators[ i ].mA->fBufferSize( );
		return total;
	}
	void tProfiler::fRender( Gfx::tScreen& screen )
	{
		if( gShuttingDown ) return;

		if( gDisplayMode == cProfilerDisplayPerf )
		{
			tPerfDisplayGroupList display;
			fObtainPerfDisplayCategories( display, mPerfCategories );

			Threads::tMutex threadLock( mCritSec );
			mRenderer->fRenderPerf( screen, display );
		}
		else if( gDisplayMode == cProfilerDisplayMem )
		{
			Threads::tMutex threadLock( mCritSec );
			mRenderer->fRenderMem( screen, mMemCategories, mGeomAllocators, mIdxAllocators );
		}
	}
	void tProfiler::fInitialize( Gfx::tScreen& screen )
	{
		tPerfDisplayGroupList display;
		fObtainPerfDisplayCategories( display, mPerfCategories );
		mRenderer->fRenderPerf( screen, display, true );
		mRenderer->fRenderMem( screen, mMemCategories, mGeomAllocators, mIdxAllocators, true );
	}
	void tProfiler::fShutdown( )
	{
		gShuttingDown = true;
		if( mRenderer )
		{
			delete mRenderer;
			mRenderer = 0;
		}
	}
	void tProfiler::fEndProfile( const tScopedProfile& profile )
	{
		Threads::tMutex threadLock( mCritSec );

		mPerfCategories[ profile.fCategory( ) ].mTimeMs += profile.fTimeMs( );
	}

}

#else

// Fixes no object linker warning
void tProfileCPP_NoObjFix( ) { }

#endif//defined( sig_profile ) && defined( sig_devmenu )
