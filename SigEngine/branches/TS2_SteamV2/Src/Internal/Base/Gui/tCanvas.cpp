#include "BasePch.hpp"
#include "tCanvas.hpp"
#include "Gfx/tScreen.hpp"
#include "tCanvasFrameOutline.hpp"

namespace Sig { namespace Gui
{
	tCanvas::tCanvas( )
		: mParent( 0 )
		, mDeleteSelf( false )
		, mDisabled( false )
		, mInvisible( false )
		, mAcceptsCanvasEvents( true )
		, mVirtualMode( false )
	{
	}
	tCanvas::~tCanvas( )
	{
	}
	void tCanvas::fRemoveFromParent( )
	{
		if( fParent( ) )
			fParent( )->fRemoveChild( tCanvasPtr( this ) );
	}
	void tCanvas::fSetRgbTint( const Math::tVec3f& tint )
	{
		mLocalXform.mRgbaTint.x = tint.x;
		mLocalXform.mRgbaTint.y = tint.y;
		mLocalXform.mRgbaTint.z = tint.z;
		fOnTintChanged( );
	}

	void tCanvas::fSetRgbTint( f32 r, f32 g, f32 b )
	{
		fSetRgbTint( Math::tVec3f( r, g, b ) );
	}

	void tCanvas::fSetRgbaTint( const Math::tVec4f& tint )
	{
		mLocalXform.mRgbaTint = tint;
		fOnTintChanged( );
	}

	void tCanvas::fSetRgbaTint( f32 r, f32 g, f32 b, f32 a )
	{
		fSetRgbaTint( Math::tVec4f( r, g, b, a ) );
	}

	void tCanvas::fSetAlpha( f32 alpha )
	{
		if( mLocalXform.mRgbaTint.w == alpha )
			return; // only interested in EXACT equality here, the point is just to avoid recursive updates when we're continually calling SetAlpha with the same value

		mLocalXform.mRgbaTint.w = alpha;
		fOnTintChanged( );
	}
	void tCanvas::fSetPosition( const Math::tVec3f& p )
	{
		if( mLocalXform.mPosition != p )
		{
			mLocalXform.mPosition = p;
			fOnMoved( );
		}
	}
	void tCanvas::fSetPosition( float x, float y, float z )
	{
		fSetPosition( Math::tVec3f( x, y, z ) );
	}
	void tCanvas::fSetXPos( f32 x )
	{
		if( mLocalXform.mPosition.x != x )
		{
			mLocalXform.mPosition.x = x;
			fOnMoved( );
		}
	}
	void tCanvas::fSetYPos( f32 y )
	{
		if( mLocalXform.mPosition.y != y )
		{
			mLocalXform.mPosition.y = y;
			fOnMoved( );
		}
	}
	void tCanvas::fSetZPos( f32 z )
	{
		if( mLocalXform.mPosition.z != z )
		{
			mLocalXform.mPosition.z = z;
			fOnMoved( );
		}
	}
	void tCanvas::fSetAngle( f32 angle )
	{
		if( mLocalXform.mAngle != angle )
		{
			mLocalXform.mAngle = angle;
			fOnMoved( );
		}
	}
	void tCanvas::fSetScale( const Math::tVec2f& scale )
	{
		if( mLocalXform.mScale != scale )
		{
			mLocalXform.mScale = scale;
			fOnMoved( );
		}
	}

	void tCanvas::fSetScale( f32 sx, f32 sy )
	{
		fSetScale( Math::tVec2f( sx, sy ) );
	}

	void tCanvas::fTranslate( const Math::tVec3f& dp )
	{
		fSetPosition( fPosition( ) + dp );
	}

	void tCanvas::fTranslate( f32 x, f32 y, f32 z )
	{
		fTranslate( Math::tVec3f( x, y, z ) );
	}

	void tCanvas::fRotate( f32 dt )
	{
		fSetAngle( fAngle( ) + dt );
	}

	void tCanvas::fIncrementScale( const Math::tVec2f& ds )
	{
		fSetScale( fScale( ) + ds );
	}

	void tCanvas::fCenterInParent( )
	{
		if( mParent )
		{
			fSetPosition( Math::tVec3f( mParent->fLocalRect( ).fCenter( ), mLocalXform.mPosition.z ) );
		}
	}
	Sig::b32 tCanvas::fParentIsInvisible() const
	{
		if( !mParent )
			return mInvisible;
		else if( mInvisible )
			return true;
		else
			return mParent->fParentIsInvisible( );
	}

	Sig::b32 tCanvas::fIsVirtualMode( ) const
	{
		if( !mParent )
		{
			return mVirtualMode;
		}
		else if( mVirtualMode )
		{
			return true;
		}

		return mParent->fIsVirtualMode( );
	}

	Math::tVec3f tCanvas::fCalculateScreenPosition() const
	{
		// Recursive Termination Condition
		if( !mParent )
			return mLocalXform.mPosition;

		return mParent->fCalculateScreenPosition( ) + mLocalXform.mPosition;
	}
	void tCanvas::fSetBounds( const tRect& rect )
	{
		mLocalXform.mRect = rect;
		fUpdateWorldBounds( true );
	}
	void tCanvas::fUpdateWorldXform( b32 notifyParentOfBoundsChange )
	{
		if( mParent )
		{
			const tCanvasXform& parentXform = mParent->fWorldXform( );

			// inherit color
			mWorldXform.mRgbaTint = parentXform.mRgbaTint * mLocalXform.mRgbaTint;

			// angle simply accumulates
			mWorldXform.mAngle = parentXform.mAngle + mLocalXform.mAngle;

			// technically scale is more complex than just multiplying, but anything else
			// could potentially introduce shear - this is basically akin to accumulating
			// scale in object space... this is actually more useful... but just be aware
			// that rotating and scaling a parent could have unexpected results on children
			mWorldXform.mScale = parentXform.mScale * mLocalXform.mScale;

			// position is computed as you would expect, which means 
			// incorporating the rotation and scale of the parent

			const f32 sint = Math::fSin( parentXform.mAngle );
			const f32 cost = Math::fCos( parentXform.mAngle );
			const f32 x0 =  parentXform.mScale.x * cost; const f32 y0 = -parentXform.mScale.y * sint;
			const f32 x1 =  parentXform.mScale.x * sint; const f32 y1 =  parentXform.mScale.y * cost;

			mWorldXform.mPosition.x = parentXform.mPosition.x + Math::tVec2f( x0, y0 ).fDot( mLocalXform.mPosition.fXY( ) );
			mWorldXform.mPosition.y = parentXform.mPosition.y + Math::tVec2f( x1, y1 ).fDot( mLocalXform.mPosition.fXY( ) );
			mWorldXform.mPosition.z = parentXform.mPosition.z + mLocalXform.mPosition.z;
		}
		else
			mWorldXform = mLocalXform;

		fUpdateWorldBounds( notifyParentOfBoundsChange );
	}
	void tCanvas::fUpdateWorldBounds( b32 notifyParentOfBoundsChange )
	{
		Math::tMat3f worldXform = Math::tMat3f::cIdentity;
		worldXform.fSetTranslation( fWorldXform( ).mPosition );

		const f32 sint = Math::fSin( fWorldXform( ).mAngle );
		const f32 cost = Math::fCos( fWorldXform( ).mAngle );

		worldXform(0,0) = fWorldXform( ).mScale.x * cost; worldXform(0,1) = -fWorldXform( ).mScale.y * sint;
		worldXform(1,0) = fWorldXform( ).mScale.x * sint; worldXform(1,1) =  fWorldXform( ).mScale.y * cost;

		const tRect& localBounds = fLocalXform( ).mRect;

		const Math::tVec2f p0 = worldXform.fXformPoint( Math::tVec3f( localBounds.mL, localBounds.mT, 0.f ) ).fXY( );
		const Math::tVec2f p1 = worldXform.fXformPoint( Math::tVec3f( localBounds.mR, localBounds.mT, 0.f ) ).fXY( );
		const Math::tVec2f p2 = worldXform.fXformPoint( Math::tVec3f( localBounds.mL, localBounds.mB, 0.f ) ).fXY( );
		const Math::tVec2f p3 = worldXform.fXformPoint( Math::tVec3f( localBounds.mR, localBounds.mB, 0.f ) ).fXY( );

		mWorldXform.mRect.mT = fMin( fMin( p0.y, p1.y ), fMin( p2.y, p3.y ) );
		mWorldXform.mRect.mL = fMin( fMin( p0.x, p1.x ), fMin( p2.x, p3.x ) );
		mWorldXform.mRect.mB = fMax( fMax( p0.y, p1.y ), fMax( p2.y, p3.y ) );
		mWorldXform.mRect.mR = fMax( fMax( p0.x, p1.x ), fMax( p2.x, p3.x ) );

		if( mParent && notifyParentOfBoundsChange )
			mParent->fOnChildBoundsChanged( );
	}
	void tCanvas::fUpdateWorldTint( )
	{
		if( mParent )
		{
			const tCanvasXform& parentXform = mParent->fWorldXform( );

			// inherit color
			mWorldXform.mRgbaTint = parentXform.mRgbaTint * mLocalXform.mRgbaTint;
		}
		else
			mWorldXform.mRgbaTint = mLocalXform.mRgbaTint;
	}
	void tCanvas::fOnMoved( )
	{
		fUpdateWorldXform( true );
	}
	void tCanvas::fOnTintChanged( )
	{
		fUpdateWorldTint( );
	}
	void tCanvas::fOnParentMoved( )
	{
		fUpdateWorldXform( false );
	}
	void tCanvas::fOnParentTintChanged( )
	{
		fUpdateWorldTint( );
	}
}}

namespace Sig { namespace Gui
{
	tCanvasFrame& tCanvasPtr::fToCanvasFrame( )
	{
		sigassert( fCanvas( ) );
		tCanvasFrame* o = fCanvas( )->fDynamicCast< tCanvasFrame >( );
		sigassert( o );
		return *o;
	}

	const tCanvasFrame& tCanvasPtr::fToCanvasFrame( ) const
	{
		sigassert( fCanvas( ) );
		const tCanvasFrame* o = fCanvas( )->fDynamicCast< tCanvasFrame >( );
		sigassert( o );
		return *o;
	}

	void tCanvasPtr::fDeleteSelf( )
	{
		if( fCanvas( ) )
			fCanvas( )->fDeleteSelf( );
	}
	void tCanvasPtr::fRemoveFromParent( )
	{
		if( fCanvas( ) && fCanvas( )->fParent( ) )
			fCanvas( )->fParent( )->fRemoveChild( *this );
	}
	void tCanvasPtr::fOnTickCanvas( f32 dt )
	{
		if( fIsNull( ) )
			return;
		else if( fCanvas( )->mDeleteSelf )
			fRemoveFromParent( );
		else if( fCanvas( )->fDisabled( ) )
			return;
		else if( fIsCodeOwned( ) )
			fCanvas( )->fOnTickCanvas( dt );
		else
		{
			Sqrat::Function f( fScriptObject( ), "OnTick" );
			sigassert( !f.IsNull( ) && "tCanvas::fOnTickCanvas" );
			f.Execute( dt );
		}
	}
	void tCanvasPtr::fClearChildren( )
	{
		if( fIsNull( ) )
			return;
		else if( fIsCodeOwned( ) )
			fCanvas( )->fClearChildren( );
		else
		{
			Sqrat::Function f( fScriptObject( ), "ClearChildren" );
			sigassert( !f.IsNull( ) && "tCanvas::fClearChildren" );
			f.Execute( );
		}
	}

	b32 tCanvasPtr::fHandleCanvasEvent( const Logic::tEvent& event )
	{
		if( fIsNull( ) )
			return false;
		else if( !fCanvas( )->fAcceptsCanvasEvents( ) )
			return false;
		else if( fCanvas( )->fDisabled( ) )
			return false;
		else if( fIsCodeOwned( ) )
			return fCanvas( )->fHandleCanvasEvent( event );
		else
		{
			Sqrat::Function f( fScriptObject( ), "HandleCanvasEvent" );
			sigassert( !f.IsNull( ) && "tCanvas::fHandleCanvasEvent" );
			return f.Evaluate<bool>( event ) != 0;
		}
	}
}}

namespace Sig { namespace Gui
{
	tCanvasFrame::tCanvasFrame( )
		: mIgnoreBoundsChange( false )
	{
#ifdef sig_devmenu
		if( tCanvasFrameOutline::fDebugDrawEnabled( ) )
		{
			fAddChild( tCanvasPtr( NEW tCanvasFrameOutline( ) ) );
		}
#endif//sig_devmenu
	}

	tCanvasFrame::~tCanvasFrame( )
	{
		fClearChildren( );
	}

	void tCanvasFrame::fOnMoved( )
	{
		tCanvas::fOnMoved( );
		for( u32 i = 0; i < mChildren.fCount( ); ++i )
			if( mChildren[ i ].fCanvas( ) )
				mChildren[ i ].fCanvas( )->fOnParentMoved( );
	}
	void tCanvasFrame::fOnTintChanged( )
	{
		tCanvas::fOnTintChanged( );
		for( u32 i = 0; i < mChildren.fCount( ); ++i )
			if( mChildren[ i ].fCanvas( ) )
				mChildren[ i ].fCanvas( )->fOnParentTintChanged( );
	}
	void tCanvasFrame::fOnParentMoved( )
	{
		tCanvas::fOnParentMoved( );
		for( u32 i = 0; i < mChildren.fCount( ); ++i )
			if( mChildren[ i ].fCanvas( ) )
				mChildren[ i ].fCanvas( )->fOnParentMoved( );
	}
	void tCanvasFrame::fOnParentTintChanged( )
	{
		tCanvas::fOnParentTintChanged( );
		for( u32 i = 0; i < mChildren.fCount( ); ++i )
			if( mChildren[ i ].fCanvas( ) )
				mChildren[ i ].fCanvas( )->fOnParentTintChanged( );
	}
	void tCanvasFrame::fOnChildBoundsChanged( )
	{
		if( !mIgnoreBoundsChange )
		{
			mLocalXform.mRect.fInvalidate( );
			Math::tMat3f childLocalMat = Math::tMat3f::cIdentity;
			for( u32 i = 0; i < mChildren.fCount( ); ++i )
			{
				tCanvas* child = mChildren[ i ].fCanvas( );
				if( !child ) continue;

				const Sig::Gui::tCanvasXform childLocalXform = child->fLocalXform( );

				childLocalMat = Math::tMat3f::cIdentity;
				childLocalMat.fSetTranslation( childLocalXform.mPosition );

				const f32 sint = Math::fSin( childLocalXform.mAngle );
				const f32 cost = Math::fCos( childLocalXform.mAngle );

				childLocalMat( 0, 0 ) = childLocalXform.mScale.x * cost; childLocalMat( 0, 1 ) = -childLocalXform.mScale.y * sint;
				childLocalMat( 1, 0 ) = childLocalXform.mScale.x * sint; childLocalMat( 1, 1 ) = childLocalXform.mScale.y * cost;

				const Math::tVec2f p0 = childLocalMat.fXformPoint( Math::tVec3f( childLocalXform.mRect.mL, childLocalXform.mRect.mT, 0.f ) ).fXY( );
				const Math::tVec2f p1 = childLocalMat.fXformPoint( Math::tVec3f( childLocalXform.mRect.mR, childLocalXform.mRect.mT, 0.f ) ).fXY( );
				const Math::tVec2f p2 = childLocalMat.fXformPoint( Math::tVec3f( childLocalXform.mRect.mL, childLocalXform.mRect.mB, 0.f ) ).fXY( );
				const Math::tVec2f p3 = childLocalMat.fXformPoint( Math::tVec3f( childLocalXform.mRect.mR, childLocalXform.mRect.mB, 0.f ) ).fXY( );

				float childLocalTop = fMin( fMin( p0.y, p1.y ), fMin( p2.y, p3.y ) );
				float childLocalLeft = fMin( fMin( p0.x, p1.x ), fMin( p2.x, p3.x ) );
				float childLocalBottom = fMax( fMax( p0.y, p1.y ), fMax( p2.y, p3.y ) );
				float childLocalRight = fMax( fMax( p0.x, p1.x ), fMax( p2.x, p3.x ) );

				mLocalXform.mRect.mT = fMin( mLocalXform.mRect.mT, childLocalTop );
				mLocalXform.mRect.mL = fMin( mLocalXform.mRect.mL, childLocalLeft );
				mLocalXform.mRect.mB = fMax( mLocalXform.mRect.mB, childLocalBottom );
				mLocalXform.mRect.mR = fMax( mLocalXform.mRect.mR, childLocalRight );
			}

			fUpdateWorldXform( true );
		}
	}
	void tCanvasFrame::fAddChild( const tCanvasPtr& child )
	{
		sigassert( child.fCanvas( ) );

		if( child.fCanvas( )->mParent )
		{
			if( child.fCanvas( )->mParent == this )
				return;
			else
				child.fCanvas( )->mParent->fRemoveChild( child );
		}

		child.fCanvas( )->mParent = this;
		mChildren.fPushBack( child );

		child.fCanvas( )->fOnParentMoved( );

		fOnChildBoundsChanged( );
	}
	void tCanvasFrame::fRemoveChild( const tCanvasPtr& child )
	{
		sigassert( child.fCanvas( ) );

		if( child.fCanvas( )->mParent != this )
			return;

		child.fCanvas( )->mParent = 0;
		mChildren.fFindAndErase( child );

		fOnChildBoundsChanged( );
	}
	void tCanvasFrame::fClearChildren( )
	{
		for( u32 i = 0; i < mChildren.fCount( ); ++i )
			mChildren[ i ].fClearChildren( );
		mChildren.fDeleteArray( );
	}
	void tCanvasFrame::fShiftPivot( const Math::tVec2f& dp )
	{
		for( u32 i = 0; i < mChildren.fCount( ); ++i )
		{
			tCanvas* child = mChildren[ i ].fCanvas( );
			if( child )
				child->fTranslate( Math::tVec3f( -dp, 0.f ) );
		}
		fTranslate( Math::tVec3f( dp, 0.f ) );
	}
	void tCanvasFrame::fCenterPivot( )
	{
		fShiftPivot( 0.5f * fLocalRect( ).fWidthHeight( ) );
	}
	void tCanvasFrame::fOnTickCanvas( f32 dt )
	{
		if( fDisabled( ) )
			return;

		for( u32 i = 0; i < mChildren.fCount( ); ++i )
			mChildren[ i ].fOnTickCanvas( dt );
	}
	void tCanvasFrame::fOnRenderCanvas( Gfx::tScreen& screen ) const
	{
		if( fInvisible( ) )
			return;

		for( u32 i = 0; i < mChildren.fCount( ); ++i )
			if( mChildren[ i ].fCanvas( ) )
				mChildren[ i ].fCanvas( )->fOnRenderCanvas( screen );
	}
	void tCanvasFrame::fSetScissorRect( const Math::tRect& rect )
	{
		for( u32 i = 0; i < mChildren.fCount( ); ++i )
			if( mChildren[ i ].fCanvas( ) )
				mChildren[ i ].fCanvas( )->fSetScissorRect( rect );
	}
	b32 tCanvasFrame::fHandleCanvasEvent( const Logic::tEvent& e )
	{
		if( fAcceptsCanvasEvents( ) )
		{
			for( u32 i = 0; i < mChildren.fCount( ); ++i )
			{
				const b32 handled = mChildren[ i ].fHandleCanvasEvent( e );
				if( handled )
					return true;
			}
		}
		return false;
	}
}}


namespace Sig { namespace Gui
{
	namespace
	{
		static b32 fHasParent( tCanvas* canvas )
		{
			return canvas && canvas->fParent( );
		}
	}
	void tCanvas::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tCanvas, Sqrat::NoCopy<tCanvas> > classDesc( vm.fSq( ) );

		classDesc
			.Func( "DeleteSelf", &tCanvas::fDeleteSelf )
			.GlobalFunc( "HasParent", &fHasParent )
			.Prop( "Parent", &tCanvas::fParent )
			.Prop( "LocalRect", &tCanvas::fLocalRect )
			.Prop( "WorldRect", &tCanvas::fWorldRect )
			.Prop( "AcceptsCanvasEvents", &fAcceptsCanvasEventsFromScript, &fSetAcceptsCanvasEventsFromScript )
			.Prop( "ThisAsInt", &tCanvas::fThisAsIntForScript )
			.Func( "GetRgb", &tCanvas::fRgbTint )
			.Func( "GetRgba", &tCanvas::fRgbaTint )
			.Func( "GetAlpha", &tCanvas::fAlpha )
			.Overload<void (tCanvas::*)(const Math::tVec3f&)>( "SetRgb", &tCanvas::fSetRgbTint )
			.Overload<void (tCanvas::*)(float,float,float)>( "SetRgb", &tCanvas::fSetRgbTint )
			.Overload<void (tCanvas::*)(const Math::tVec4f&)>( "SetRgba", &tCanvas::fSetRgbaTint )
			.Overload<void (tCanvas::*)(float,float,float, float)>( "SetRgba", &tCanvas::fSetRgbaTint )
			.Func( "SetAlpha", &tCanvas::fSetAlpha )
			.Func( "SetAlphaClamp", &tCanvas::fSetAlphaClamp )
			.Func( "GetPosition", &tCanvas::fPosition )
			.Func( "GetXPos", &tCanvas::fXPos )
			.Func( "GetYPos", &tCanvas::fYPos )
			.Func( "GetZPos", &tCanvas::fZPos )
			.Overload<void (tCanvas::*)(const Math::tVec3f&)>( "SetPosition", &tCanvas::fSetPosition )
			.Overload<void (tCanvas::*)(float,float,float)>( "SetPosition", &tCanvas::fSetPosition )
			.Func( "SetXPos", &tCanvas::fSetXPos )
			.Func( "SetYPos", &tCanvas::fSetYPos )
			.Func( "SetZPos", &tCanvas::fSetZPos )
			.Func( "GetWorldPosition", &tCanvas::fWorldPosition )
			.Func( "GetWorldXPos", &tCanvas::fWorldXPos )
			.Func( "GetWorldYPos", &tCanvas::fWorldYPos )
			.Func( "GetWorldZPos", &tCanvas::fWorldZPos )
			.Func( "GetAngle", &tCanvas::fAngle )
			.Func( "SetAngle", &tCanvas::fSetAngle )
			.Func( "GetScale", &tCanvas::fScale )
			.Overload<void (tCanvas::*)(const Math::tVec2f&)>( "SetScale", &tCanvas::fSetScale )
			.Overload<void (tCanvas::*)(float,float)>( "SetScale", &tCanvas::fSetScale )
			.Func( "SetUniformScale", &tCanvas::fSetUniformScale )
			.Overload<void (tCanvas::*)(const Math::tVec3f&)>( "Translate", &tCanvas::fTranslate )
			.Overload<void (tCanvas::*)(float,float,float)>( "Translate", &tCanvas::fTranslate )
			.Func( "Rotate", &tCanvas::fRotate )
			.Func( "Scale", &tCanvas::fIncrementScale )
			.Func( "CenterInParent", &tCanvas::fCenterInParent )
			.Prop( "Disabled", &tCanvas::fDisabledFromScript, &tCanvas::fSetDisabledFromScript )
			.Prop( "Invisible", &tCanvas::fInvisibleFromScript, &tCanvas::fSetInvisibleFromScript )
			.Func( "ParentIsInvisible", &tCanvas::fParentIsInvisible)
			.Func( "OnTick", &tCanvas::fOnTickCanvas )
			.Func( "ClearChildren", &tCanvas::fClearChildren )
			.Func( "SetScissorRect", &tCanvas::fSetScissorRect )
			.Func( "HandleCanvasEvent", &tCanvas::fHandleCanvasEvent )
			.Func( "CalculateScreenPosition", &tCanvas::fCalculateScreenPosition )
			.Prop( "VirtualMode", &tCanvas::fGetVirtualMode, &tCanvas::fSetVirtualMode )
			;

		vm.fNamespace(_SC("Gui")).Bind(_SC("Canvas"), classDesc);
	}
void tCanvasFrame::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tCanvasFrame, tCanvas, Sqrat::NoCopy<tCanvasFrame> > classDesc( vm.fSq( ) );

		classDesc
			.Func<void (tCanvasFrame::*)( const Sqrat::Object& )>(_SC("AddChild"),		&tCanvasFrame::fAddChild)
			.Func<void (tCanvasFrame::*)( const Sqrat::Object& )>(_SC("RemoveChild"),	&tCanvasFrame::fRemoveChild)
			.Func(_SC("ClearChildren"), &tCanvasFrame::fClearChildren)
			.Func(_SC("ShiftPivot"),	&tCanvasFrame::fShiftPivot)
			.Func(_SC("CenterPivot"),	&tCanvasFrame::fCenterPivot)
			.Prop(_SC("ChildCount"),	&tCanvasFrame::fChildCount)
			.Var(_SC("IgnoreBoundsChange"),	&tCanvasFrame::mIgnoreBoundsChange)
			
			;

		vm.fNamespace(_SC("Gui")).Bind(_SC("CanvasFrame"), classDesc);
	}
}}

