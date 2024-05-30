//------------------------------------------------------------------------------
// \file tReplay.cpp - 21 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tSync.hpp"
#include "tReplay.hpp"
#include "FileSystem.hpp"
#include "EndianUtil.hpp"
#include "tCmdLineOption.hpp"

namespace Sig
{
	namespace
	{
		static const u32 cReadWriteFrequency = 10;
	}

	//------------------------------------------------------------------------------
	static tFilePathPtr fBuildPath( const tFilePathPtr & folder, const char * session, const char * user )
	{
		std::stringstream ss;

		// Build and test the filename
		ss << "r_" << session << "_" << user << ".rply";
		if( ss.str().length( ) > FileSystem::fMaxFileNameLength( ) )
		{
			log_warning( 0, "Path for replay could not be built: File name is too long." );
			return tFilePathPtr( );
		}


		tFilePathPtr ret = tFilePathPtr::fConstructPath( folder, tFilePathPtr( ss.str( ) ) );
		if( ret.fLength( ) > FileSystem::fMaxPathLength( ) )
		{
			log_warning( 0, "Path for replay could not be built: File path is too long" );
			return tFilePathPtr( );
		}

		return ret;
	}

	//------------------------------------------------------------------------------
	template<class t>
	static void fWrite( tGrowableArray<byte> & buffer, const t & out )
	{
		sigassert( tIsBuiltInType<t>::cIs && "This Write only accepts built in types" );
		buffer.fInsert( buffer.fCount( ), (const byte *)&out, sizeof( out ) );
	}

	//------------------------------------------------------------------------------
	static void fWrite( tGrowableArray<byte> & buffer, const tReplay::tInputBufferGamepad & input )
	{
		const u32 count = input.fCount( );
		fWrite( buffer, count );

		for( u32 i = 0; i < count; ++i )
			buffer.fInsert( buffer.fCount( ), (const byte*)&input[ i ], sizeof( input[ i ] ) );
	}


	//------------------------------------------------------------------------------
	static void fWrite( tGrowableArray<byte> & buffer, const tReplay::tInputBufferKeyboard & input )
	{
		const u32 count = input.fCount( );
		fWrite( buffer, count );

		for( u32 i = 0; i < count; ++i )
			buffer.fInsert( buffer.fCount( ), (const byte*)&input[ i ], sizeof( input[ i ] ) );
	}


	//------------------------------------------------------------------------------
	static void fWrite( tGrowableArray<byte> & buffer, const tReplay::tInputBufferMouse & input )
	{
		const u32 count = input.fCount( );
		fWrite( buffer, count );

		for( u32 i = 0; i < count; ++i )
			buffer.fInsert( buffer.fCount( ), (const byte*)&input[ i ], sizeof( input[ i ] ) );
	}



	//------------------------------------------------------------------------------
	template< class t >
	static void fWrite( tGrowableArray<byte> & buffer, const tDynamicArray< t > & out )
	{
		const u32 count = out.fCount( );
		fWrite( buffer, count );

		if( tIsBuiltInType< t >::cIs )
		{
			buffer.fInsert( buffer.fCount( ), (const byte *)out.fBegin( ), out.fTotalSizeOf( ) );
		}
		else
		{
			for( u32 i = 0; i < count; ++i )
				fWrite( buffer, out[ i ] );
		}
	}

	//------------------------------------------------------------------------------
	template<class t>
	static void fRead( tFileReader & read, t & out )
	{
		sigassert( tIsBuiltInType<t>::cIs && "This Read only accepts built in types" );
		read( &out, sizeof( out ) );
	}

	//------------------------------------------------------------------------------
	static void fRead( tFileReader & reader, tGrowableArray<Input::tGamepad::tStateData> & input )
	{
		u32 count; 
		fRead( reader, count );

		if( !count )
			return;

		input.fGrowCount( count );
		reader( input.fEnd( ) - count, count * input.fElementSizeOf( ) );
	}

	//------------------------------------------------------------------------------
	static void fRead( tFileReader & reader, tGrowableArray<Input::tKeyboard::tStateData> & input )
	{
		u32 count; 
		fRead( reader, count );

		if( !count )
			return;

		input.fGrowCount( count );
		reader( input.fEnd( ) - count, count * input.fElementSizeOf( ) );
	}

	//------------------------------------------------------------------------------
	static void fRead( tFileReader & reader, tGrowableArray<Input::tMouse::tStateData> & input )
	{
		u32 count; 
		fRead( reader, count );

		if( !count )
			return;

		input.fGrowCount( count );
		reader( input.fEnd( ) - count, count * input.fElementSizeOf( ) );
	}

	//------------------------------------------------------------------------------
	template< class t >
	static void fRead( tFileReader & reader, tDynamicArray< t > & out )
	{
		u32 count;
		reader( &count, sizeof( count ) );
		out.fNewArray( count );

		if( tIsBuiltInType<t>::cIs )
		{
			reader( out.fBegin( ), out.fTotalSizeOf( ) );	
		}
		else
		{
			for( u32 i = 0; i < count; ++i )
				fRead( reader, out[ i ] );
		}
	}

	//------------------------------------------------------------------------------
	// tPlaycorder
	//------------------------------------------------------------------------------
	tPlaycorder::tPlaycorder( 
		u32 contextCount, 
		u32 inputCount,
		const tFilePathPtr & outputFolder,
		u32 machinesNeeded,
		const char * sessionName,
		const char * userName,
		b32 sync )
		: mInputFrames( 0 )
	{
		mReplay.mContexts.fNewArray( contextCount );
		mReplay.mInputsGamepad.fNewArray( inputCount );
		mReplay.mInputsKeyboard.fNewArray( inputCount );
		mReplay.mInputsMouse.fNewArray( inputCount );

		tFilePathPtr path = fBuildPath( outputFolder, sessionName, userName );

		if( sync )
			mSyncPath = tFilePathPtr::fSwapExtension( path, ".sync" );
		
		fOpen( path );

		// Write replayer cmdline style information
		if( fIsOpen( ) )
		{
			std::stringstream ss;
			ss	<< "-machineCount " << machinesNeeded 
				<< " -sessionName " << sessionName
				<< " -userName "	<< userName;

			if( sync )
				ss << " -syncFile "	<< mSyncPath.fCStr( );

			std::string output = ss.str( );
			
			// Swap for the pc, which will be reading this information
			s32 length = output.length( );
			EndianUtil::fSwapForTargetPlatform( &length, sizeof(length), cPlatformPcDx9 );

			mWriter( &length, sizeof(length) );
			mWriter( output.c_str( ), output.length( ) + 1 );
		}
	}

	//------------------------------------------------------------------------------
	tPlaycorder::~tPlaycorder( )
	{
		// Flush an remaining data
		fPush( );
	}

	//------------------------------------------------------------------------------
	void tPlaycorder::fSetContext( u32 id, u32 dataLength, const byte * data )
	{
		sigassert( !mInputFrames && "Contexts cannot be set after first input frame has been marked" );
		sigassert( id < mReplay.mContexts.fCount( ) && "Context id is out of bounds" );

		if( mReplay.mContexts[ id ].fCount( ) != dataLength )
			mReplay.mContexts[ id ].fNewArray( dataLength );

		fMemCpy( mReplay.mContexts[ id ].fBegin( ), data, dataLength );
	}

	//------------------------------------------------------------------------------
	void tPlaycorder::fSetInput( u32 id, const Input::tGamepad::tStateData & data )
	{
		sigassert( id < mReplay.mInputsGamepad.fCount( ) && "Input id is out of bounds" );

		if( mReplay.mInputsGamepad[ id ].fCount( ) < mInputFrames )
			mReplay.mInputsGamepad[ id ].fGrowCount( 1 );

		sigassert( mReplay.mInputsGamepad[ id ].fCount( ) == mInputFrames && "An input set was missed" );
		mReplay.mInputsGamepad[ id ].fBack( ) = data;
	}


	//------------------------------------------------------------------------------
	void tPlaycorder::fSetInput( u32 id, const Input::tKeyboard::tStateData & data )
	{
		sigassert( id < mReplay.mInputsKeyboard.fCount( ) && "Input id is out of bounds" );

		if( mReplay.mInputsKeyboard[ id ].fCount( ) < mInputFrames )
			mReplay.mInputsKeyboard[ id ].fGrowCount( 1 );

		sigassert( mReplay.mInputsKeyboard[ id ].fCount( ) == mInputFrames && "An input set was missed" );
		mReplay.mInputsKeyboard[ id ].fBack( ) = data;
	}


	//------------------------------------------------------------------------------
	void tPlaycorder::fSetInput( u32 id, const Input::tMouse::tStateData & data )
	{
		sigassert( id < mReplay.mInputsMouse.fCount( ) && "Input id is out of bounds" );

		if( mReplay.mInputsMouse[ id ].fCount( ) < mInputFrames )
			mReplay.mInputsMouse[ id ].fGrowCount( 1 );

		sigassert( mReplay.mInputsMouse[ id ].fCount( ) == mInputFrames && "An input set was missed" );
		mReplay.mInputsMouse[ id ].fBack( ) = data;
	}

	//------------------------------------------------------------------------------
	void tPlaycorder::fMarkInputFrame( )
	{
		const b32 skipWrite = mInputFrames % cReadWriteFrequency;
		
		// Flush the data we have
		if( !skipWrite )
			fPush( );

		mInputFrames++;
	}

	//------------------------------------------------------------------------------
	void tPlaycorder::fOpen( const tFilePathPtr & path )
	{
#ifdef sig_devmenu

		if( !path.fNull( ) )
			mWriter.fOpen( path, false );

#endif// sig_devmenu
	}

	//------------------------------------------------------------------------------
	void tPlaycorder::fPush( )
	{

#ifdef sig_devmenu

		if( mWriter.fIsOpen( ) )
		{
			tGrowableArray<byte> output;

			// Write any contexts out
			if( mReplay.mContexts.fCount( ) )
			{
				sigassert( !mInputFrames && "Sanity: Contexts got set after an input frame was marked.?" );
				fWrite( output, mReplay.mContexts );
			}

			// Write any inputs out
			fWrite( output, mReplay.mInputsGamepad );
			fWrite( output, mReplay.mInputsKeyboard );
			fWrite( output, mReplay.mInputsMouse );
			
			// Kick the write operation
			mWriter( output.fBegin( ), output.fTotalSizeOf( ) );
		}

#endif// sig_devmenu

		// Reset the contexts
		mReplay.mContexts.fDeleteArray( );

		// Reset the inputs
		u32 inputCount = mReplay.mInputsGamepad.fCount( );
		for( u32 i = 0; i < inputCount; ++i )
			mReplay.mInputsGamepad[ i ].fSetCount( 0 );


		inputCount = mReplay.mInputsKeyboard.fCount( );
		for( u32 i = 0; i < inputCount; ++i )
			mReplay.mInputsKeyboard[ i ].fSetCount( 0 );


		inputCount = mReplay.mInputsMouse.fCount( );
		for( u32 i = 0; i < inputCount; ++i )
			mReplay.mInputsMouse[ i ].fSetCount( 0 );



		mInputFrames = 0;
	}

	//------------------------------------------------------------------------------
	// tReplayer
	//------------------------------------------------------------------------------
	tReplayer::tReplayer(  )
	{

	}

	//------------------------------------------------------------------------------
	b32	tReplayer::fOpen( const tFilePathPtr & path )
	{
		fClose( );

#ifdef sig_devmenu

		if( !mReader.fOpen( path ) )
			return false;
		
		// Pull the internal replayer string
		std::string cmdLine;
		{
			s32 length;
			mReader( &length, sizeof( length ) );
			EndianUtil::fSwapForTargetPlatform( &length, sizeof( length ), cPlatformPcDx9 );
			char * buffer = (char * )alloca( length + 1 );
			mReader( buffer, length + 1 );

			cmdLine = buffer;
		}

		tCmdLineOption option( "syncFile", cmdLine );
		if( option.fFound( ) )
			mSyncPath = tFilePathPtr( option.fGetTypedOption<std::string>( ) );

		// Read the contexts and the first empty input set
		fRead( mReader, mReplay.mContexts );

#endif// sig_devmenu

		fPull( );
		return true;
	}

	//------------------------------------------------------------------------------
	void tReplayer::fClose( )
	{
		// Destroy the replay
		mReplay.mContexts.fDeleteArray( );
		mReplay.mInputsGamepad.fDeleteArray( );
		mReplay.mInputsKeyboard.fDeleteArray( );
		mReplay.mInputsMouse.fDeleteArray( );

#ifdef sig_devmenu

		// Close the reader
		mReader.fClose( );

#endif// sig_devmenu
	}

	//------------------------------------------------------------------------------
	const tDynamicArray<byte> & tReplayer::fGetContext( u32 id )
	{
		sigassert( id < mReplay.mContexts.fCount( ) && "Context id is out of bounds" );
		return mReplay.mContexts[ id ];
	}

	//------------------------------------------------------------------------------
	b32	tReplayer::fHasInputAvailableGamepad( ) const
	{
		if( !mReplay.mInputsGamepad.fCount( ) )
			return false;

		if( mInputFrame > mReplay.mInputsGamepad[ 0 ].fCount( ) ) 
			return false;

		return true;
	}

	//------------------------------------------------------------------------------
	b32	tReplayer::fHasInputAvailableKeyboard( ) const
	{
		if( !mReplay.mInputsKeyboard.fCount( ) )
			return false;

		if( mInputFrame > mReplay.mInputsKeyboard[ 0 ].fCount( ) ) 
			return false;

		return true;
	}


	//------------------------------------------------------------------------------
	b32	tReplayer::fHasInputAvailableMouse( ) const
	{
		if( !mReplay.mInputsMouse.fCount( ) )
			return false;

		if( mInputFrame > mReplay.mInputsMouse[ 0 ].fCount( ) ) 
			return false;

		return true;
	}




	//------------------------------------------------------------------------------
	u32	tReplayer::fGetInputCountGamepad( ) const
	{
		return mReplay.mInputsGamepad.fCount( );
	}

	//------------------------------------------------------------------------------
	u32	tReplayer::fGetInputCountKeyboard( ) const
	{
		return mReplay.mInputsKeyboard.fCount( );
	}

	//------------------------------------------------------------------------------
	u32	tReplayer::fGetInputCountMouse( ) const
	{
		return mReplay.mInputsMouse.fCount( );
	}


	//------------------------------------------------------------------------------
	const Input::tGamepad::tStateData & tReplayer::fGetInputGamepad( u32 id )
	{
		sigassert( id < mReplay.mInputsGamepad.fCount( ) && "Input id is out of bounds" );
		sigassert( mInputFrame <= mReplay.mInputsGamepad[ id ].fCount( ) && "Input frame is past available input" );
		return mReplay.mInputsGamepad[ id ][ mInputFrame - 1 ];
	}

	//------------------------------------------------------------------------------
	const Input::tKeyboard::tStateData & tReplayer::fGetInputKeyboard( u32 id )
	{
		sigassert( id < mReplay.mInputsKeyboard.fCount( ) && "Input id is out of bounds" );
		sigassert( mInputFrame <= mReplay.mInputsKeyboard[ id ].fCount( ) && "Input frame is past available input" );
		return mReplay.mInputsKeyboard[ id ][ mInputFrame - 1 ];
	}

	//------------------------------------------------------------------------------
	const Input::tMouse::tStateData & tReplayer::fGetInputMouse( u32 id )
	{
		sigassert( id < mReplay.mInputsMouse.fCount( ) && "Input id is out of bounds" );
		sigassert( mInputFrame <= mReplay.mInputsMouse[ id ].fCount( ) && "Input frame is past available input" );
		return mReplay.mInputsMouse[ id ][ mInputFrame - 1 ];
	}

	//------------------------------------------------------------------------------
	void tReplayer::fMarkInputFrame( )
	{
		const b32 skipRead = mInputFrame % cReadWriteFrequency;

		if( !skipRead )
			fPull( );

		++mInputFrame;
	}

	//------------------------------------------------------------------------------
	void tReplayer::fPull( )
	{
		// Clear any processed inputs
		u32 inputCount = mReplay.mInputsGamepad.fCount( );
		for( u32 i = 0; i < inputCount; ++i )
		{
			tReplay::tInputBufferGamepad & input = mReplay.mInputsGamepad[ i ];
			const u32 newCount = input.fCount( ) - mInputFrame;
			if( newCount )
			{
				// Copy the memory down
				fMemMove(
					input.fBegin( ), 
					input.fBegin( ) + mInputFrame, 
					newCount * input.fElementSizeOf( ) );
			}

			input.fSetCount( newCount );
		}

		inputCount = mReplay.mInputsKeyboard.fCount( );
		for( u32 i = 0; i < inputCount; ++i )
		{
			tReplay::tInputBufferKeyboard & input = mReplay.mInputsKeyboard[ i ];
			const u32 newCount = input.fCount( ) - mInputFrame;
			if( newCount )
			{
				// Copy the memory down
				fMemMove(
					input.fBegin( ), 
					input.fBegin( ) + mInputFrame, 
					newCount * input.fElementSizeOf( ) );
			}

			input.fSetCount( newCount );
		}

		inputCount = mReplay.mInputsMouse.fCount( );
		for( u32 i = 0; i < inputCount; ++i )
		{
			tReplay::tInputBufferMouse & input = mReplay.mInputsMouse[ i ];
			const u32 newCount = input.fCount( ) - mInputFrame;
			if( newCount )
			{
				// Copy the memory down
				fMemMove(
					input.fBegin( ), 
					input.fBegin( ) + mInputFrame, 
					newCount * input.fElementSizeOf( ) );
			}

			input.fSetCount( newCount );
		}

		// Reset the index to the first
		mInputFrame = 0;

		tDynamicArray<tReplay::tInputBufferGamepad> newInputsGamepad;
		tDynamicArray<tReplay::tInputBufferKeyboard> newInputsKeyboard;
		tDynamicArray<tReplay::tInputBufferMouse> newInputsMouse;

#ifdef sig_devmenu

		fRead( mReader, newInputsGamepad );
		fRead( mReader, newInputsKeyboard );
		fRead( mReader, newInputsMouse );

#endif// sig_devmenu

		if( newInputsGamepad.fCount( ) )
		{
			// Handle first read
			if( !inputCount )
			{
				inputCount = newInputsGamepad.fCount( );
				mReplay.mInputsGamepad.fNewArray( inputCount );
			}
			
			// Copy over the new inputs
			for( u32 i = 0; i < inputCount; ++i )
			{
				mReplay.mInputsGamepad[ i ].fInsert( 
					mReplay.mInputsGamepad[ i ].fCount( ), 
					newInputsGamepad[ i ].fBegin( ), 
					newInputsGamepad[ i ].fCount( ) );
			}
		}


		if( newInputsKeyboard.fCount( ) )
		{
			// Handle first read
			if( !inputCount )
			{
				inputCount = newInputsKeyboard.fCount( );
				mReplay.mInputsKeyboard.fNewArray( inputCount );
			}
			
			// Copy over the new inputs
			for( u32 i = 0; i < inputCount; ++i )
			{
				mReplay.mInputsKeyboard[ i ].fInsert( 
					mReplay.mInputsKeyboard[ i ].fCount( ), 
					newInputsKeyboard[ i ].fBegin( ), 
					newInputsKeyboard[ i ].fCount( ) );
			}
		}

		if( newInputsMouse.fCount( ) )
		{
			// Handle first read
			if( !inputCount )
			{
				inputCount = newInputsMouse.fCount( );
				mReplay.mInputsMouse.fNewArray( inputCount );
			}
			
			// Copy over the new inputs
			for( u32 i = 0; i < inputCount; ++i )
			{
				mReplay.mInputsMouse[ i ].fInsert( 
					mReplay.mInputsMouse[ i ].fCount( ), 
					newInputsMouse[ i ].fBegin( ), 
					newInputsMouse[ i ].fCount( ) );
			}
		}


	}
}
