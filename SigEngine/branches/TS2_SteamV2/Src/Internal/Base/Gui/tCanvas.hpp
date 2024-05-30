#ifndef __tCanvas__
#define __tCanvas__

namespace Sig { namespace Gfx
{
	class tScreen;
}}

namespace Sig { namespace Gui
{
	typedef Math::tRect tRect;

	class tCanvasFrame;

	struct tCanvasXform
	{
		tRect			mRect;
		Math::tVec4f	mRgbaTint;
		Math::tVec3f	mPosition;
		Math::tVec2f	mScale;
		f32				mAngle;

		tCanvasXform( )
			: mRgbaTint( Math::tVec4f::cOnesVector )
			, mPosition( Math::tVec3f::cZeroVector )
			, mScale( Math::tVec2f::cOnesVector )
			, mAngle( 0.f )
		{
		}
	};

	class base_export tCanvas : public tUncopyable, public tRefCounter
	{
		friend class tCanvasPtr;
		friend class tCanvasFrame;
		define_dynamic_cast_base( tCanvas );
	public:
		tCanvas( );
		virtual ~tCanvas( );
		virtual void fClearChildren( ) { }
		virtual void fOnTickCanvas( f32 dt ) { }
		virtual void fOnRenderCanvas( Gfx::tScreen& screen ) const { }
		virtual void fSetScissorRect( const Math::tRect& rect ) { }
		virtual b32  fHandleCanvasEvent( const Logic::tEvent& e ) { return false; } // callback for all sorts of various events that need to inform game code
		
		b32						fAcceptsCanvasEvents( ) { return mAcceptsCanvasEvents; }
		void					fSetAcceptsCanvasEvents( b32 accepts ) { mAcceptsCanvasEvents = accepts; }
		void					fRemoveFromParent( );
		void					fDeleteSelf( ) { mDeleteSelf = true; }
		tCanvasFrame*			fParent( ) const { return mParent; }
		const tCanvasXform&		fLocalXform( ) const { return mLocalXform; }
		const tCanvasXform&		fWorldXform( ) const { return mWorldXform; }
		const tRect&			fLocalRect( ) const { return mLocalXform.mRect; }
		const tRect&			fWorldRect( ) const { return mWorldXform.mRect; }
		const Math::tVec4f&		fRgbaTint( ) const { return mLocalXform.mRgbaTint; }
		Math::tVec3f			fRgbTint( ) const { return mLocalXform.mRgbaTint.fXYZ( ); }
		f32						fAlpha( ) const { return mLocalXform.mRgbaTint.w; }
		const Math::tVec3f&		fPosition( ) const { return mLocalXform.mPosition; }
		const Math::tVec3f&		fWorldPosition( ) const { return mWorldXform.mPosition; }
		f32						fXPos( ) const { return mLocalXform.mPosition.x; }
		f32						fYPos( ) const { return mLocalXform.mPosition.y; }
		f32						fZPos( ) const { return mLocalXform.mPosition.z; }
		f32						fWorldXPos( ) const { return mWorldXform.mPosition.x; }
		f32						fWorldYPos( ) const { return mWorldXform.mPosition.y; }
		f32						fWorldZPos( ) const { return mWorldXform.mPosition.z; }
		f32						fAngle( ) const { return mLocalXform.mAngle; }
		const Math::tVec2f&		fScale( ) const { return mLocalXform.mScale; }
		void					fSetRgbTint( const Math::tVec3f& tint );
		void					fSetRgbTint( f32 r, f32 g, f32 b );
		void					fSetRgbaTint( const Math::tVec4f& tint );
		void					fSetRgbaTint( f32 r, f32 g, f32 b, f32 a );
		void					fSetAlpha( f32 alpha );
		void					fSetAlphaClamp( f32 alpha ) { fSetAlpha( fClamp( alpha, 0.f, 1.f ) ); }
		void					fSetPosition( const Math::tVec3f& p );
		void					fSetPosition( float x, float y , float z );
		void					fSetXPos( f32 x );
		void					fSetYPos( f32 y );
		void					fSetZPos( f32 z );
		void					fSetAngle( f32 angle );
		void					fSetScale( const Math::tVec2f& scale );
		void					fSetScale( f32 sx, f32 sy );
		void					fSetUniformScale( f32 scale ) { fSetScale( Math::tVec2f( scale, scale ) ); }
		void					fTranslate( const Math::tVec3f& dp );
		void					fTranslate( f32 x, f32 y, f32 z );
		void					fRotate( f32 dt );
		void					fIncrementScale( const Math::tVec2f& ds );
		void					fCenterInParent( );
		b32						fDisabled( ) const { return mDisabled; } // prevents OnTickCanvas from being called
		void					fSetDisabled( b32 disabled ) { mDisabled = disabled; }
		b32						fInvisible( ) const { return mInvisible; } // prevents OnTickRender from being called
		b32						fParentIsInvisible( ) const;
		void					fSetInvisible( b32 invisible ) { mInvisible = invisible; }
		Math::tVec3f			fCalculateScreenPosition( ) const;

		bool					fDisabledFromScript( ) const { return mDisabled!=0; } // prevents OnTickCanvas from being called
		void					fSetDisabledFromScript( bool disabled ) { mDisabled = disabled; }
		bool					fInvisibleFromScript( ) const { return mInvisible!=0; } // prevents OnTickRender from being called
		void					fSetInvisibleFromScript( bool invisible ) { mInvisible = invisible; }

		// Set canvas to virtual mode. Canvas is not visible to user, but works the same as non-virtual in all other respects.
		void					fSetVirtualMode( b32 isVirtualMode ) { mVirtualMode = isVirtualMode; }
		// Get canvas virtual mode.
		b32						fGetVirtualMode( ) const { return mVirtualMode; }
		// Check if a canvas or parent(s) are in virtual mode.
		b32						fIsVirtualMode( ) const;

	protected:
		void fSetBounds( const tRect& rect );
		void fUpdateWorldXform( b32 notifyParentOfBoundsChange );
		void fUpdateWorldBounds( b32 notifyParentOfBoundsChange );
		void fUpdateWorldTint( );
		virtual void fOnMoved( );
		virtual void fOnTintChanged( );
		virtual void fOnParentMoved( );
		virtual void fOnParentTintChanged( );

	private:

		int fThisAsIntForScript( ) const { return (int)this; }
		bool fAcceptsCanvasEventsFromScript( ) { return fAcceptsCanvasEvents( ) ? true : false; }
		void fSetAcceptsCanvasEventsFromScript( bool accepts ) { fSetAcceptsCanvasEvents( accepts ); }

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tCanvasFrame*	mParent;
		b8				mDeleteSelf;
		b8				mDisabled;
		b8				mInvisible;
		b8				mAcceptsCanvasEvents;
		tCanvasXform	mLocalXform;
		tCanvasXform	mWorldXform;
		b8				mVirtualMode;					// Canvas is a virtual canvas, not visible.
	};

	class tCanvasPtr : public tScriptOrCodeObjectPtr<tCanvas>
	{
	public:
		tCanvasPtr( ) { }
		explicit tCanvasPtr( const Sqrat::Object& o ) : tScriptOrCodeObjectPtr<tCanvas>( o ) { }
		explicit tCanvasPtr( tCanvas* c ) : tScriptOrCodeObjectPtr<tCanvas>( c ) { }
		tCanvas* fCanvas( ) const { return fCodeObject( ); }
		tCanvasFrame& fToCanvasFrame( );
		const tCanvasFrame& fToCanvasFrame( ) const;
		void fDeleteSelf( );
		void fRemoveFromParent( );
		void fOnTickCanvas( f32 dt );
		void fClearChildren( );
		b32  fHandleCanvasEvent( const Logic::tEvent& event );
	};

	class tCanvasPtrList : public tGrowableArray<tCanvasPtr> { };

	class tCanvasFrame : public tCanvas
	{
		friend class tCanvas;
		define_dynamic_cast( tCanvasFrame, tCanvas );
	private:
		tCanvasPtrList mChildren;

	public:
		tCanvasFrame( );
		virtual ~tCanvasFrame( );
		void fAddChild( const tCanvasPtr& child );
		void fRemoveChild( const tCanvasPtr& child );
		void fShiftPivot( const Math::tVec2f& dp );
		void fCenterPivot( );
		u32  fChildCount( ) const { return mChildren.fCount( ); }
		virtual void fClearChildren( );
		virtual void fOnTickCanvas( f32 dt );
		virtual void fOnRenderCanvas( Gfx::tScreen& screen ) const;
		virtual void fSetScissorRect( const Math::tRect& rect );
		virtual b32  fHandleCanvasEvent( const Logic::tEvent& e );

		void fSetIgnoreBoundsChanged( b32 ignore ) { mIgnoreBoundsChange = ignore; }

	protected:
		virtual void fOnMoved( );
		virtual void fOnTintChanged( );
		virtual void fOnParentMoved( );
		virtual void fOnParentTintChanged( );
		void fOnChildBoundsChanged( );
		void fAddChild( const Sqrat::Object& child ) { fAddChild( tCanvasPtr( child ) ); }
		void fRemoveChild( const Sqrat::Object& child ) { fRemoveChild( tCanvasPtr( child ) ); }

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

		b32 mIgnoreBoundsChange;
	};

}}

#endif//__tCanvas__
