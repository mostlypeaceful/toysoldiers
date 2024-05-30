#ifndef __tSource__
#define __tSource__

namespace Sig { namespace Audio
{

	class tRTPCSmoother
	{
	public:
		tStringPtr	mRTPCName;
		u32			mRTPCNameID;
		f32			mTargetValue;
		f32			mCurrentValue;
		f32			mMaxChange; // Per second

		tRTPCSmoother( u32 id = ~0, f32 initValue = 0.f, f32 maxChange = 0.f )
			: mRTPCNameID( id )
			, mTargetValue( initValue )
			, mCurrentValue( initValue )
			, mMaxChange( maxChange )
		{ }

		tRTPCSmoother( const tStringPtr& name, f32 initValue = 0.f, f32 maxChange = 0.f )
			: mRTPCNameID( ~0 )
			, mRTPCName( name )
			, mTargetValue( initValue )
			, mCurrentValue( initValue )
			, mMaxChange( maxChange )
		{ }
		
		void fStep( f32 dt );
	};

	///
	/// \brief Contains the logic for playing a sound in the game.
	///  Essentially wraps the Wwise "Game Object".
	///   Sources can sink events, manage real time values, manage sound bank life
	///   , be attached to moving entities.

	/// Important note: There's no need to actually spawn one of these entities into the scene graph.
	///  You can create one on the stack, position it, and call fHandleEvent to play one shot sounds.

	class tSource;
	typedef tRefCounterPtr<tSource> tSourcePtr;

	class tSource : public tEntity
	{
		debug_watch( tSource );
		declare_uncopyable( tSource );
		define_dynamic_cast( tSource, tEntity );
	public:
		explicit tSource( const char *debugName = NULL );
		explicit tSource( b32 globalValues ); // sets switches and rtpcs and events on global scope
		virtual ~tSource( );

		virtual void fOnDelete( );
		virtual void fOnSpawn( );
		virtual void fOnPause( b32 paused );
		virtual void fThinkST( f32 dt );

		void fSetEnabled( b32 enabled ) { mEnabled = enabled; }
		b32  fEnabled( ) const			{ return mEnabled; }

		void fSetListenerMask( u32 mask );

		void fHandleEvent( u32 e );
		void fHandleEvent( const tStringPtr& e );
		void fHandleEvent( const char* e );

		void fSetSwitch( u32 group, u32 value );
		void fSetSwitch( const char* group, const char* value );
		void fSetSwitch( const tStringPtr& group, const tStringPtr& value );
		void fResetSwitch( u32 group );
		void fResetSwitch( const char* group );
		void fResetSwitch( const tStringPtr& group );

		void fSetGameParam( u32 param, f32 value );
		void fSetGameParam( const char* param, f32 value );
		void fSetGameParam( const tStringPtr& param, f32 value );

        void fPostTrigger( const u32 t );
        void fPostTrigger( const tStringPtr& t );
        void fPostTrigger( const char* t );

		void fStepSmoothers( f32 dt );

		inline tRTPCSmoother* fFindSmoother( const tStringPtr& name )
		{
			tRTPCSmoother* smoother = NULL;
			for( u32 i = 0; i < mSmoothers.fCount( ); ++i )
				if( mSmoothers[ i ].mRTPCName == name )
					return &mSmoothers[ i ];
			return NULL;
		}
		inline tRTPCSmoother* fFindSmoother( u32 id )
		{
			tRTPCSmoother* smoother = NULL;
			for( u32 i = 0; i < mSmoothers.fCount( ); ++i )
				if( mSmoothers[ i ].mRTPCNameID == id )
					return &mSmoothers[ i ];
			return NULL;
		}

		template< typename p >
		void fSetGameParamSmooth( const p& param, f32 value, f32 maxChange )
		{
			if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )

			profile( cProfilePerfAudioEvent );

			tRTPCSmoother* smoother = fFindSmoother( param );
			if( !smoother )
			{
				mSmoothers.fPushBack( tRTPCSmoother( param, value, maxChange ) );
				smoother = mSmoothers.fBegin( );
			}

			smoother->mTargetValue = value;
			smoother->mMaxChange = maxChange;
		}

		void fUpdatePosition( );
		void fLockToListener( u32 index ); //follows listener instead of having its own position

		void fAddEventLinkedChild( tSource* source ) { sigassert( source != this ); mEventLinkedChildren.fPushBack( tSourcePtr( source ) ); }
		void fAddSwitchAndParamsLinkedChild( tSource* source ) { sigassert( source != this ); mSwitchAndParamsLinkedChildren.fPushBack( tSourcePtr( source ) ); }

		static void fExportScriptInterface( tScriptVm& vm );

	private:
		u32 mObjectID;

		u8  mLockedToListener;
		b8  mEnabled;
		u8  pad1;
		u8  pad2;

		// Children linking allows multiple audio sources to respond to the same event or switch
		tGrowableArray<tSourcePtr> mEventLinkedChildren;
		tGrowableArray<tSourcePtr> mSwitchAndParamsLinkedChildren;
		tGrowableArray<tRTPCSmoother> mSmoothers;
	};


}}


#endif//__tSource__

