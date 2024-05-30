#ifndef __tGameMode__
#define __tGameMode__

namespace Sig
{
	class tGameMode
	{
	public:
		enum tState
		{
			cStateFrontEnd,
			cStateNormal,
			cStatePurgatory,
			cStateCount,
		};
		enum tFlags
		{
			cFlagCoOp = ( 1 << 0 ),
			cFlagVersus = ( 1 << 1 ),
			cFlagOnline = ( 1 << 2 ),
			cMaskPlayWithOthers = cFlagCoOp | cFlagVersus | cFlagOnline,
		};
	public: // create
		tGameMode( ) : mState( cStateCount ), mFlags( 0 ), pad0( 0 ), pad1( 0 ) { }
		void fSetState( tState state, u32 flags = 0 ) { mState = state; mFlags = flags; }
		void fSetInvalid( ) { mState = cStateCount; mFlags = 0; }
		void fAddCoOpFlag( ) { mFlags |= cFlagCoOp; }
		void fAddVersusFlag( ) { mFlags |= cFlagVersus; }
		void fAddOnlineFlag( ) { mFlags |= cFlagOnline; }
		void fRemoveOnlineFlag( ) { mFlags &= ~cFlagOnline; }
	public: // compare
		inline b32 operator==( const tGameMode& other ) const { return mState == other.mState && mFlags == other.mFlags; }
	public: // query
		tState fState( ) const { return ( tState )mState; }
		u32 fFlags( ) const { return mFlags; }
		b32 fInvalid( ) const { return mState == cStateCount; }
		b32 fIsFrontEnd( ) const { return mState == cStateFrontEnd; }
		b32 fIsPurgatory( ) const { return mState == cStatePurgatory; }
		b32 fIsSinglePlayer( ) const { return !fIsMultiPlayer( ); }
		b32 fIsVersus( ) const { return fTestBits( mFlags, cFlagVersus ); }
		b32 fIsCoOp( ) const { return fTestBits( mFlags, cFlagCoOp ); }
		b32 fIsSinglePlayerOrCoop( ) const { return fIsSinglePlayer( ) || fIsCoOp( ); }
		b32 fIsMultiPlayer( ) const { return fTestBits( mFlags, cMaskPlayWithOthers ); }
		b32 fIsLocal( ) const { return !fIsNet( ); }
		b32 fIsNet( ) const { return fTestBits( mFlags, cFlagOnline ); }
		b32 fIsSplitScreen( ) const { return fIsMultiPlayer( ) && fIsLocal( ); }
	public:
		template<class tArchive>
		void fSaveLoad( tArchive & archive )
		{
			archive.fSaveLoad( mState );
			archive.fSaveLoad( mFlags );
		}

	private:
		u8 mState;
		u8 mFlags;
		u8 pad0;
		u8 pad1;
	};
}

#endif//__tGameMode__
