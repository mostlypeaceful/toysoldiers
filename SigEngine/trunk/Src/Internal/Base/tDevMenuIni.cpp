#include "BasePch.hpp"

#ifdef sig_devmenu
#include "FileSystem.hpp"

namespace Sig
{
	namespace
	{
		static const char cCommentChar = '#';
		static const char cAssignmentChar = '=';

		char* fFindFirstCharFromSet( char* text, const char* charSet )
		{
			for( ; *text; ++text )
			{
				for( const char* i = charSet; *i; ++i )
				{
					if( *text == *i )
						return text;
				}
			}

			return text;
		}

		char* fEatWhiteSpace( char* text )
		{
			while( *text && isspace( *text ) )
				++text;
			return text;
		}

		void fTerminateAtWhiteSpace( char* text )
		{
			while( *text && !isspace( *text ) )
				++text;
			if( isspace( *text ) )
				*text = '\0';
		}

		void fTerminateAtComment( char* text )
		{
			char* commentSymbol = strchr( text, cCommentChar );
			if( commentSymbol )
				*commentSymbol = '\0';
		}

		b32 fReadValue( char* text, tDevMenuIni::tEntry& e )
		{
			e.mValue = Math::cInfinity;

			// see if it's a string
			char* openQuote = strchr( text, '"' );
			if( openQuote )
			{
				char* closeQuote = strrchr( openQuote + 1, '"' );
				if( closeQuote )
				{
					text = openQuote + 1;
					*closeQuote = '\0';

					e.mStringValue = tStringPtr( text );
					return true;
				}
			}

			// nope, either number or vector
			char* openParen = strchr( text, '(' );
			text = openParen ? ( openParen + 1 ) : text;
			char* closeParen = strchr( text, ')' );
			if( closeParen ) *closeParen = '\0';

			u32 ithValue = 0;

			for( char* endOfValue = 0; (endOfValue = strchr( text, ',' )); text = endOfValue + 1 )
			{
				*endOfValue = '\0';

				text = fEatWhiteSpace( text );
				fTerminateAtWhiteSpace( text );

				if( ithValue < e.mValue.cDimension )
					e.mValue.fAxis( ithValue++ ) = ( f32 )atof( text );
			}

			text = fEatWhiteSpace( text );
			fTerminateAtWhiteSpace( text );

			if( ithValue < e.mValue.cDimension )
				e.mValue.fAxis( ithValue++ ) = ( f32 )atof( text );

			return true;
		}
	}

	tDevMenuIni::tDevMenuIni( )
	{
		tDevVarDepot::fInstance( ).mInis.fPushBack( this );
	}

	tDevMenuIni::~tDevMenuIni( )
	{
		tDevVarDepot::fInstance( ).mInis.fFindAndEraseOrdered( this );
	}

	b32 tDevMenuIni::fReadFile( const tFilePathPtr& path )
	{
		tDynamicBuffer fileContents;
		if( !FileSystem::fReadFileToBuffer( fileContents, path, "" ) )
			return false;

		if( !fParseString( ( char* )fileContents.fBegin( ) ) )
			return false;

		return true;
	}

	b32 tDevMenuIni::fParseString( char* text )
	{
		tGrowableArray<tEntry> entries; ///< sorted by mVarName

		b32 lastLine = false;

		for( char* endOfLine = 0; !lastLine && *text && ( endOfLine = fFindFirstCharFromSet( text, "\n" ) ); text = endOfLine + 1 )
		{
			lastLine = ( *endOfLine == '\0' );
			if( !lastLine )
				*endOfLine = '\0';

			text = fEatWhiteSpace( text );
			fTerminateAtComment( text );

			if( !*text )
				continue; // whole line was white space or commented out

			// find assignment operator
			char* assignmentOp = strchr( text, cAssignmentChar );
			if( !assignmentOp )
				continue; // no assignment operator on this line, invalid

			// on the left we have variable name, on the right we have variable value
			*assignmentOp = '\0';

			// seek to value
			char* value = fEatWhiteSpace( assignmentOp + 1 );

			// read variable name
			tEntry e;
			e.mName = tStringPtr( StringUtil::fTrim( text, " \r\n\t", true, true ) );

			if( !fReadValue( value, e ) )
				continue;

			entries.fPushBack( e );
		}

		fStoreEntries( entries );

		return true;
	}

	namespace
	{
		void fLogDevMenuIniEntry( const tDevMenuIni::tEntry& entry )
		{
			log_output( Log::cFlagDevMenu, "registering ini var [" << entry.mName << "] = [" );
			if( entry.mStringValue.fExists( ) )
				log_output( 0, entry.mStringValue );
			else
			{
				for( u32 iaxis = 0; iaxis < entry.mValue.cDimension; ++iaxis )
				{
					if( entry.mValue.fAxis( iaxis ) == Math::cInfinity )
						break;
					if( iaxis > 0 )
						log_output( 0, " " );
					log_output( 0, entry.mValue.fAxis( iaxis ) );
				}
			}
			log_line( 0, "]" );

		}
	}

	void tDevMenuIni::fStoreEntries( const tGrowableArray<tEntry>& entries )
	{
		mEntries.fSetCapacity( 2 * entries.fCount( ) );
		for( u32 i = 0; i < entries.fCount( ); ++i )
		{
			fLogDevMenuIniEntry( entries[ i ] );

			mEntries.fInsert( entries[ i ].mName, entries[ i ] );

			tDevMenuItem* var = tDevVarDepot::fInstance( ).fFindVar( entries[ i ].mName.fCStr( ) );
			if( var )
			{
				if( !entries[ i ].mStringValue.fNull( ) )
				{
					std::string str( entries[ i ].mStringValue.fCStr( ) );
					var->fSetFromString( str );
				}
				else
					var->fSetFromVector( entries[ i ].mValue );
			}
		}
	}

}

#else

// Fixes no object linker warning
void tDevMenuIniCPP_NoObjFix( ) { }

#endif//sig_devmenu
