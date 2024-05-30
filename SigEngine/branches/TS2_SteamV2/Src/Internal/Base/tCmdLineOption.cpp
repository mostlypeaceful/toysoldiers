#include "BasePch.hpp"
#include "tCmdLineOption.hpp"

namespace Sig
{
	tCmdLineOption::tCmdLineOption( )
		: mFound( false )
	{
	}

	tCmdLineOption::tCmdLineOption( const char* optionOriginal, const std::string& cmdLineBuffer )
		: mFound( false )
	{
		const char* cmdLine = cmdLineBuffer.c_str( );

		const char* find = 0;
		const char* prefixes[ ] = { "/", "-", "+" };

		tFixedArray<char,512> option;
		fZeroOut( option );

		for( u32 attempt = 0; !find && attempt < array_length( prefixes ); ++attempt )
		{
			option[0] = 0;
			strcat( option.fBegin( ), prefixes[ attempt ] );
			strcat( option.fBegin( ), optionOriginal );
			find = StringUtil::fStrStrI( cmdLine, option.fBegin( ) );

			if( find )
			{
				// we found something that's potentially a match

				const char lastChar = *( find + strlen( option.fBegin( ) ) );
				if( lastChar && !isspace( lastChar ) )
				{
					// doesn't count, i.e., we were looking for -s, and
					// it seemed like we found it, but really we found -something;
					// however, we still need to continue searching; i.e., just
					// because we found -something doesn't mean there's not a -s
					// later on
					cmdLine = find + 1;
					find = 0;
					--attempt;
				}
			}
		}

		if( find )
		{
			mFound = true;
			find += strlen( option.fBegin( ) );
			while( *find && isspace( *find ) )
				++find;

			b32 quotesTerminate = false;
			if( *find == '\"' )
			{
				quotesTerminate = true;
				++find;
			}

			if( *find )
			{
				mOption = find;

				u32 endIndex = 0;
				for( ; endIndex < mOption.size( ); ++endIndex )
				{
					if( quotesTerminate )
					{
						if( mOption[endIndex] == '\"' )
							break;
					}
					else if( isspace( mOption[endIndex] ) )
						break;
				}

				mOption.resize( endIndex );
			}
		}
	}

}
