#include "SigEdPch.hpp"
#include "tHeightFieldVertexPaintPanel.hpp"
#include "tHeightFieldPaintCursor.hpp"
#include "tWxSlapOnSpinner.hpp"
#include "tWxToolsPanelSlider.hpp"
#include "tEditorAppWindow.hpp"


namespace Sig
{
	namespace
	{
		const f32 cNudgeAmount = 0.1f;
	}
	
	class tHeightFieldVertexBrushSlider : public tWxToolsPanelSlider
	{
		tHeightFieldVertexPaintPanel* mParent;
	public:
		tHeightFieldVertexBrushSlider( tHeightFieldVertexPaintPanel* parent, wxWindow* windowParent, const char* labelName, f32 initialValue = 0.5f )
			: tWxToolsPanelSlider( windowParent, labelName, &parent->fGuiApp( ).fMainWindow( ), initialValue )
			, mParent( parent ) { }
		virtual void fOnValueChanged( ) { mParent->fOnSlidersChanged( ); }
	};

	class tTerrainPaintRaise : public tHeightFieldVertexPaintCursor
	{
	public:
		tTerrainPaintRaise( tEditorCursorControllerButton* button )
			: tHeightFieldVertexPaintCursor( button )
		{
			fMainWindow( ).fSetStatus( "Paint Terrain [Raise]" );
		}

		virtual void fOnTick( )
		{
			tHeightFieldVertexPaintCursor::fOnTick( );
		}

		virtual void fModifyVertex( tModifyVertexArgs& args )
		{
			args.mVertArray.fIndex( args.mX, args.mZ ).mLocalHeight += args.mSign * args.mPaintStrength * ( 60.0f * args.mDt );
		}

	};
	class tTerrainPaintRaiseCursorButton : public tEditorCursorControllerButton
	{
		tHeightFieldVertexPaintPanel* mButtonContainer;
	public:
		tTerrainPaintRaiseCursorButton( tHeightFieldVertexPaintPanel* buttonContainer, tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PaintTerrainUpSel" ), wxBitmap( "PaintTerrainUpDeSel" ), "Raise Brush - hold shift to lower" )
			, mButtonContainer( buttonContainer )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			tHeightFieldVertexPaintCursor* terrainCursor = new tTerrainPaintRaise( this );
			mButtonContainer->fUpdateParametersOnCursor( terrainCursor );
			mButtonContainer->fAddCursorHotKeys( terrainCursor );
			return tEditorCursorControllerPtr( terrainCursor );
		}
	};


	class tTerrainPaintLower : public tHeightFieldVertexPaintCursor
	{
	public:
		tTerrainPaintLower( tEditorCursorControllerButton* button )
			: tHeightFieldVertexPaintCursor( button )
		{
			fMainWindow( ).fSetStatus( "Paint Terrain [Lower]" );
		}
		virtual void fOnTick( )
		{
			tHeightFieldVertexPaintCursor::fOnTick( );
		}
		virtual void fModifyVertex( tModifyVertexArgs& args )
		{
			args.mVertArray.fIndex( args.mX, args.mZ ).mLocalHeight -= args.mSign * args.mPaintStrength * ( 60.0f * args.mDt );
		}
	};
	class tTerrainPaintLowerCursorButton : public tEditorCursorControllerButton
	{
		tHeightFieldVertexPaintPanel* mButtonContainer;
	public:
		tTerrainPaintLowerCursorButton( tHeightFieldVertexPaintPanel* buttonContainer, tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PaintTerrainDownSel" ), wxBitmap( "PaintTerrainDownDeSel" ), "Lower Brush - hold shift to raise" )
			, mButtonContainer( buttonContainer )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			tHeightFieldVertexPaintCursor* terrainCursor = new tTerrainPaintLower( this );
			mButtonContainer->fUpdateParametersOnCursor( terrainCursor );
			mButtonContainer->fAddCursorHotKeys( terrainCursor );
			return tEditorCursorControllerPtr( terrainCursor );
		}
	};



	class tTerrainPaintAverage : public tHeightFieldVertexPaintCursor
	{
	public:
		tTerrainPaintAverage( tEditorCursorControllerButton* button )
			: tHeightFieldVertexPaintCursor( button )
		{
			fMainWindow( ).fSetStatus( "Paint Terrain [Average]" );
		}
		virtual void fOnTick( )
		{
			tHeightFieldVertexPaintCursor::fOnTick( );
		}
		virtual void fModifyVertex( tModifyVertexArgs& args )
		{
			tEditableTerrainGeometry::tEditableVertex& ev = args.mVertArray.fIndex( args.mX, args.mZ );
			const f32 d = ev.mLocalHeight - args.mVertArray.fAvgHeight( );

			if( args.mSign * d < 0.f && !args.mCtrl )
				return;

			const f32 t = fClamp( args.mPaintStrength * ( 5.0f * args.mDt ), 0.f, 1.f );
			ev.mLocalHeight = Math::fLerp( ev.mLocalHeight, args.mVertArray.fAvgHeight( ), t );
		}
	};
	class tTerrainPaintAverageCursorButton : public tEditorCursorControllerButton
	{
		tHeightFieldVertexPaintPanel* mButtonContainer;
	public:
		tTerrainPaintAverageCursorButton( tHeightFieldVertexPaintPanel* buttonContainer, tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PaintTerrainAverageSel" ), wxBitmap( "PaintTerrainAverageDeSel" ), "Average Brush - ctrl to do total average, hold shift to switch raise/lower behavior" )
			, mButtonContainer( buttonContainer )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			tHeightFieldVertexPaintCursor* terrainCursor = new tTerrainPaintAverage( this );
			mButtonContainer->fUpdateParametersOnCursor( terrainCursor );
			mButtonContainer->fAddCursorHotKeys( terrainCursor );
			return tEditorCursorControllerPtr( terrainCursor );
		}
	};


	class tTerrainPaintSample : public tHeightFieldVertexPaintCursor
	{
		f32 mLocalHeight;
	public:
		tTerrainPaintSample( tEditorCursorControllerButton* button )
			: tHeightFieldVertexPaintCursor( button )
			, mLocalHeight( 0.f )
		{
			fMainWindow( ).fSetStatus( "Paint Terrain [Sample]" );
		}
		virtual void fOnTick( )
		{
			tHeightFieldVertexPaintCursor::fOnTick( );
		}
		virtual void fModifyVertex( tModifyVertexArgs& args )
		{
			if( args.mCtrl )
			{
				mLocalHeight = mLastHoverIntersection.y - mPaintEntity->fDynamicCast< tEditableObject >( )->fObjectToWorld( ).fGetTranslation( ).y;
			}
			else
			{
				tEditableTerrainGeometry::tEditableVertex& ev = args.mVertArray.fIndex( args.mX, args.mZ );
				const f32 d = ev.mLocalHeight - mLocalHeight;
				if( args.mSign * d > 0.f )
				{
					const f32 t = fClamp( args.mPaintStrength * ( 20.0f * args.mDt ), 0.f, 1.f );
					ev.mLocalHeight = Math::fLerp( ev.mLocalHeight, mLocalHeight, t );
				}
			}
		}
	};
	class tTerrainPaintSampleCursorButton : public tEditorCursorControllerButton
	{
		tHeightFieldVertexPaintPanel* mButtonContainer;
	public:
		tTerrainPaintSampleCursorButton( tHeightFieldVertexPaintPanel* buttonContainer, tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PaintTerrainSampleSel" ), wxBitmap( "PaintTerrainSampleDeSel" ), "Sample Brush - ctrl to sample, hold shift to switch raise/lower behavior" )
			, mButtonContainer( buttonContainer )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			tHeightFieldVertexPaintCursor* terrainCursor = new tTerrainPaintSample( this );
			mButtonContainer->fUpdateParametersOnCursor( terrainCursor );
			mButtonContainer->fAddCursorHotKeys( terrainCursor );
			return tEditorCursorControllerPtr( terrainCursor );
		}
	};




	class tTerrainPaintTrench : public tHeightFieldVertexPaintCursor
	{
		tWxSlapOnSpinner* mTrenchHeight;
	public:
		tTerrainPaintTrench( tEditorCursorControllerButton* button, tWxSlapOnSpinner* trenchHeight )
			: tHeightFieldVertexPaintCursor( button )
			, mTrenchHeight( trenchHeight )
		{
			fMainWindow( ).fSetStatus( "Paint Terrain [Trench]" );
			mTrenchHeight->fEnableControl( );
		}
		virtual ~tTerrainPaintTrench( )
		{
			mTrenchHeight->fDisableControl( );
		}
		virtual void fOnTick( )
		{
			if( mTrenchHeight->fHasFocus( ) )
				fMainWindow( ).fSetDialogInputActive( );

			tHeightFieldVertexPaintCursor::fOnTick( );
		}
		virtual void fModifyVertex( tModifyVertexArgs& args )
		{
			tEditableTerrainGeometry::tEditableVertex& ev = args.mVertArray.fIndex( args.mX, args.mZ );

			if( args.mDistToCenter > args.mFalloff )
				return;

			const f32 offset = -mTrenchHeight->fGetValue( );
			const f32 signedOffset = offset * args.mSign;
			const f32 base = args.mSign < 0.f ? args.mVertArray.fMinHeight( ) : args.mVertArray.fMaxHeight( );
			const f32 h = base + signedOffset;

			const f32 t = 1.f - std::powf( fMin( 1.f, 1.f * args.mDistToCenter / args.mFalloff ), 3.f );
			const f32 timeBasedT = fMin( 1.f, t * args.mRawStrength * 20.0f * args.mDt );
			ev.mLocalHeight = Math::fLerp( ev.mLocalHeight, h, timeBasedT );
		}
	};
	class tTerrainPaintTrenchCursorButton : public tEditorCursorControllerButton
	{
		tHeightFieldVertexPaintPanel*	mButtonContainer;
		tWxSlapOnSpinner*			mTrenchHeight;
	public:
		tTerrainPaintTrenchCursorButton( tHeightFieldVertexPaintPanel* buttonContainer, tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PaintTerrainTrenchSel" ), wxBitmap( "PaintTerrainTrenchDeSel" ), "Trench Brush - hold shift to create mound" )
			, mButtonContainer( buttonContainer )
			, mTrenchHeight( 0 )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			tHeightFieldVertexPaintCursor* terrainCursor = new tTerrainPaintTrench( this, mTrenchHeight );
			mButtonContainer->fUpdateParametersOnCursor( terrainCursor );
			mButtonContainer->fAddCursorHotKeys( terrainCursor );
			return tEditorCursorControllerPtr( terrainCursor );
		}
		void fAddGui( wxWindow* parent )
		{
			mTrenchHeight = new tWxSlapOnSpinner( parent, "Trench Height", -9999.f, +9999.f, 0.1f, 1, 100 );
			mTrenchHeight->fSetValue( 2.f );
			mTrenchHeight->fDisableControl( );
		}
	};



	class tTerrainPaintQuadEnable : public tHeightFieldVertexPaintCursor
	{
	public:
		tTerrainPaintQuadEnable( tEditorCursorControllerButton* button )
			: tHeightFieldVertexPaintCursor( button )
		{
			fMainWindow( ).fSetStatus( "Paint Terrain [Quad Enable]" );
		}

		virtual void fOnTick( )
		{
			tHeightFieldVertexPaintCursor::fOnTick( );
		}

		virtual void fModifyVertex( tModifyVertexArgs& args )
		{
			if( args.mShift )
				args.mVertArray.fIndex( args.mX, args.mZ ).mQuadState = tHeightFieldMesh::cQuadNormal;
			else
				args.mVertArray.fIndex( args.mX, args.mZ ).mQuadState = tHeightFieldMesh::cQuadRemoved;
		}

	};
	class tTerrainPaintQuadEnableCursorButton : public tEditorCursorControllerButton
	{
		tHeightFieldVertexPaintPanel* mButtonContainer;
	public:
		tTerrainPaintQuadEnableCursorButton( tHeightFieldVertexPaintPanel* buttonContainer, tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PaintTerrainQuadSel" ), wxBitmap( "PaintTerrainQuadDeSel" ), "Quad Disable Brush - hold shift to re-enable" )
			, mButtonContainer( buttonContainer )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			tHeightFieldVertexPaintCursor* terrainCursor = new tTerrainPaintQuadEnable( this );
			mButtonContainer->fUpdateParametersOnCursor( terrainCursor );
			mButtonContainer->fAddCursorHotKeys( terrainCursor );
			return tEditorCursorControllerPtr( terrainCursor );
		}
	};

	



	class tTerrainCursorVertexSizeIncHotKey : public tEditorHotKey
	{
		tHeightFieldVertexPaintPanel* mOwner;
	public:
		tTerrainCursorVertexSizeIncHotKey( tHeightFieldVertexPaintPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton1, 0 ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorSize( +cNudgeAmount ); }
	};
	class tTerrainCursorVertexSizeDecHotKey : public tEditorHotKey
	{
		tHeightFieldVertexPaintPanel* mOwner;
	public:
		tTerrainCursorVertexSizeDecHotKey( tHeightFieldVertexPaintPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton1, tEditorHotKey::cOptionShift ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorSize( -cNudgeAmount ); }
	};

	class tTerrainCursorVertexStrengthIncHotKey : public tEditorHotKey
	{
		tHeightFieldVertexPaintPanel* mOwner;
	public:
		tTerrainCursorVertexStrengthIncHotKey( tHeightFieldVertexPaintPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton2, 0 ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorStrength( +cNudgeAmount ); }
	};
	class tTerrainCursorVertexStrengthDecHotKey : public tEditorHotKey
	{
		tHeightFieldVertexPaintPanel* mOwner;
	public:
		tTerrainCursorVertexStrengthDecHotKey( tHeightFieldVertexPaintPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton2, tEditorHotKey::cOptionShift ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorStrength( -cNudgeAmount ); }
	};

	class tTerrainCursorVertexFalloffIncHotKey : public tEditorHotKey
	{
		tHeightFieldVertexPaintPanel* mOwner;
	public:
		tTerrainCursorVertexFalloffIncHotKey( tHeightFieldVertexPaintPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton3, 0 ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorFalloff( +cNudgeAmount ); }
	};
	class tTerrainCursorVertexFalloffDecHotKey : public tEditorHotKey
	{
		tHeightFieldVertexPaintPanel* mOwner;
	public:
		tTerrainCursorVertexFalloffDecHotKey( tHeightFieldVertexPaintPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton3, tEditorHotKey::cOptionShift ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorFalloff( -cNudgeAmount ); }
	};

	class tTerrainCursorVertexShapeIncHotKey : public tEditorHotKey
	{
		tHeightFieldVertexPaintPanel* mOwner;
	public:
		tTerrainCursorVertexShapeIncHotKey( tHeightFieldVertexPaintPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton4, 0 ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorShape( +cNudgeAmount ); }
	};
	class tTerrainCursorVertexShapeDecHotKey : public tEditorHotKey
	{
		tHeightFieldVertexPaintPanel* mOwner;
	public:
		tTerrainCursorVertexShapeDecHotKey( tHeightFieldVertexPaintPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton4, tEditorHotKey::cOptionShift ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorShape( -cNudgeAmount ); }
	};

	tHeightFieldVertexPaintPanel::tHeightFieldVertexPaintPanel( tEditorAppWindow* appWindow, tWxToolsPanel* parent )
		: tWxToolsPanelTool( parent, "Terrain Height Painting", "Terrain Height Painting", "TerrainHeight" )
		, mAppWindow( appWindow )
		, mSizeSlider( 0 )
		, mStrengthSlider( 0 )
		, mFalloffSlider( 0 )
		, mShapeSlider( 0 )
	{
		tEditorCursorControllerButtonGroup* brushGroup = new tEditorCursorControllerButtonGroup( this, "Brush Type", false );
		tTerrainPaintRaiseCursorButton* raise	= new tTerrainPaintRaiseCursorButton( this, brushGroup );
		tTerrainPaintLowerCursorButton* lower	= new tTerrainPaintLowerCursorButton( this, brushGroup );
		tTerrainPaintAverageCursorButton* avg	= new tTerrainPaintAverageCursorButton( this, brushGroup );
		tTerrainPaintSampleCursorButton* sample	= new tTerrainPaintSampleCursorButton( this, brushGroup );
		tTerrainPaintTrenchCursorButton* trench = new tTerrainPaintTrenchCursorButton( this, brushGroup );
		tTerrainPaintQuadEnableCursorButton* quad	= new tTerrainPaintQuadEnableCursorButton( this, brushGroup );
		brushGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 8 );

		tWxSlapOnGroup* propsGroup = new tWxSlapOnGroup( fGetMainPanel( ), "Brush Properties", false );
		mSizeSlider = new tHeightFieldVertexBrushSlider( this, propsGroup->fGetMainPanel( ), "Size", 0.25f );
		mStrengthSlider = new tHeightFieldVertexBrushSlider( this, propsGroup->fGetMainPanel( ), "Strength", 0.50f );
		mFalloffSlider = new tHeightFieldVertexBrushSlider( this, propsGroup->fGetMainPanel( ), "Focus", 0.25f );
		mShapeSlider = new tHeightFieldVertexBrushSlider( this, propsGroup->fGetMainPanel( ), "Shape", 0.00f );
		trench->fAddGui( propsGroup->fGetMainPanel( ) );
		propsGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 8 );
	}

	void tHeightFieldVertexPaintPanel::fAddCursorHotKeys( tHeightFieldPaintCursor* cursorBase )
	{
		tHeightFieldVertexPaintCursor* cursor = dynamic_cast< tHeightFieldVertexPaintCursor* >( cursorBase );
		if( !cursor )
			return;
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorVertexSizeIncHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorVertexSizeDecHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorVertexStrengthIncHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorVertexStrengthDecHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorVertexFalloffIncHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorVertexFalloffDecHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorVertexShapeIncHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorVertexShapeDecHotKey( this ) ) );
	}

	void tHeightFieldVertexPaintPanel::fOnSlidersChanged( )
	{
		tEditorCursorControllerPtr cursor = fGuiApp( ).fCurrentCursor( );
		fUpdateParametersOnCursor( dynamic_cast<tHeightFieldPaintCursor*>( cursor.fGetRawPtr( ) ) );
	}

	void tHeightFieldVertexPaintPanel::fUpdateParametersOnCursor( tHeightFieldPaintCursor* cursor )
	{
		if( !cursor )
			return;
		cursor->fSetSize( mSizeSlider->fGetValue( ) );
		cursor->fSetStrength( mStrengthSlider->fGetValue( ) );
		cursor->fSetFalloff( mFalloffSlider->fGetValue( ) );
		cursor->fSetShape( mShapeSlider->fGetValue( ) );
	}

	void tHeightFieldVertexPaintPanel::fNudgeCursorSize( f32 delta )
	{
		mSizeSlider->fSetValue( mSizeSlider->fGetValue( ) + delta );
		fOnSlidersChanged( );
	}

	void tHeightFieldVertexPaintPanel::fNudgeCursorStrength( f32 delta )
	{
		mStrengthSlider->fSetValue( mStrengthSlider->fGetValue( ) + delta );
		fOnSlidersChanged( );
	}

	void tHeightFieldVertexPaintPanel::fNudgeCursorFalloff( f32 delta )
	{
		mFalloffSlider->fSetValue( mFalloffSlider->fGetValue( ) + delta );
		fOnSlidersChanged( );
	}

	void tHeightFieldVertexPaintPanel::fNudgeCursorShape( f32 delta )
	{
		mShapeSlider->fSetValue( mShapeSlider->fGetValue( ) + delta );
		fOnSlidersChanged( );
	}

}
