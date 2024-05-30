//------------------------------------------------------------------------------
// \file tReplay.hpp - 21 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tReplay__
#define __tReplay__
#include "Input/tGamepad.hpp"
#include "tFileWriter.hpp"
#include "tFileReader.hpp"

namespace Sig
{
	///
	/// \class tReplay
	/// \brief 
	struct tReplay
	{
		typedef tDynamicArray<byte> tContext;
		typedef tGrowableArray<Input::tGamepad::tStateData> tInputBuffer;
		tDynamicArray< tContext > mContexts;
		tDynamicArray< tInputBuffer > mInputs;
	};

	///
	/// \class tPlaycorder
	/// \brief 
	class base_export tPlaycorder : public tRefCounter
	{
	public:

		tPlaycorder( 
			u32 contextCount, 
			u32 inputCount,
			const tFilePathPtr & outputFolder,
			u32 machinesNeeded,
			const char * sessionName,
			const char * userName,
			b32 sync );

		~tPlaycorder( );

		b32		fIsOpen( ) const { return mWriter.fIsOpen( ); }
		const tFilePathPtr & fSyncPath( ) const { return mSyncPath; }
		
		
		// Game specific context data for the replay
		void	fSetContext( u32 id, u32 dataLength, const byte * data );

		template<class t>
		void	fSetContextAs( u32 id, const t & data )
		{
			fSetContext( id, sizeof( data ), (const byte *)&data );
		}
		
		// Input data for each player
		void	fMarkInputFrame( ); // Call before setting input for the frame
		void	fSetInput( u32 id, const Input::tGamepad::tStateData & data );

	private:

		void	fOpen( const tFilePathPtr & path );
		void	fPush( );

		u32 mInputFrames;
		tReplay mReplay;
		tFileWriter mWriter;
		tFilePathPtr mSyncPath;
	};

	///
	/// \class tReplayer
	/// \brief
	class base_export tReplayer : public tRefCounter
	{
	public:

		tReplayer( );

		b32		fOpen( const tFilePathPtr & path );
		void	fClose( );
		b32		fIsOpen( ) { return mReader.fIsOpen ( ); }
		const tFilePathPtr & fSyncPath( ) const { return mSyncPath; }

		// Game specific context data for the replay
		const tDynamicArray<byte> & fGetContext( u32 id );

		template< class t>
		const t & fGetContextAs( u32 id )
		{
			return *(const t *)fGetContext( id ).fBegin( );
		}

		// Input data for each player
		void	fMarkInputFrame( ); // Call before aquiring inputs for the frame
		b32		fHasInputAvailable( ) const;
		u32		fGetInputCount( ) const;
		
		const Input::tGamepad::tStateData &	fGetInput( u32 id );

	private:

		void	fPull( );

		u32 mInputFrame;
		tReplay mReplay;
		tFileReader mReader;
		tFilePathPtr mSyncPath;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tPlaycorder );
	define_smart_ptr( base_export, tRefCounterPtr, tReplayer );
}

#endif//__tReplay__
