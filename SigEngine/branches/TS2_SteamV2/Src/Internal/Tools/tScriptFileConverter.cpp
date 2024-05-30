#include "ToolsPch.hpp"
#include "tScriptFileConverter.hpp"
#include "FileSystem.hpp"
#include "tFileWriter.hpp"
#include "Scripts/tScriptFile.hpp"
#include "Scripts/tScriptVm.hpp"
#include "tLoadInPlaceSerializer.hpp"

using namespace Sig::StringUtil;

namespace Sig
{
	namespace
	{
		static b32 gFirstPlatform = false;

		namespace
		{
			static const std::string cStandardExportFunctionNames[]=
			{
				std::string( "EntityOnCreate" ),
				std::string( "EntityOnChildrenCreate" ),
				std::string( "EntityOnSiblingsCreate" ),
			};
		}

		const char* fFindSpecialTag( const char* src, const char* exportTag, u32 exportTagLen )
		{
			const char* found = fStrStrI( src, exportTag );
			if( !found ) return 0;
			if( !isspace( found[exportTagLen] ) ) return 0;

			for( const char* i = found; i >= src && *i != '\n'; --i )
			{
				if( i[0]=='/' && i[1]=='/' )
				{
					// Skip this tag, it was commented out.
					return fFindSpecialTag( found+1, exportTag, exportTagLen );
				}
			}

			found += exportTagLen;
			while( isspace( *found ) )
				++found;
			return found;
		}

		b32 fIsValidVariableCharacter( char ch )
		{
			return isalpha( ch ) || fInBounds( ch, '0', '9' ) || ch == '_';
		}

		void fPut( tGrowableArray<char>& dst, const char* src )
		{
			dst.fInsert( dst.fCount( ), src, ( u32 )strlen( src ) );
		}

		void fPut( tGrowableArray<char>& dst, const char* srcStart, const char* srcEnd )
		{
			dst.fInsert( dst.fCount( ), srcStart, ( u32 )( srcEnd - srcStart ) );
		}

		b32 fReadScriptClass( tScriptFile& scriptFile, tDynamicBuffer& scriptString )
		{
			const char* src = ( const char* )scriptString.fBegin( );
			const char* classTag = "sigclass";
			const u32 classTagLen = ( u32 )strlen( classTag );

			const char* className = fFindSpecialTag( src, classTag, classTagLen );
			if( className )
			{
				const char* classNameEnd = className;
				while( *classNameEnd && !isspace( *classNameEnd ) ) ++classNameEnd;


				const std::string classNameStr = std::string( className, classNameEnd );
				if( gFirstPlatform )
					log_line( 0, "\tfound sigclass: " << classNameStr );
				scriptFile.mScriptClass = scriptFile.fAddLoadInPlaceStringPtr( classNameStr.c_str( ) );

				// skip class name
				src = classNameEnd;
				tDynamicBuffer temp;
				temp.fCreateNullTerminated( ( const Sig::byte* )src );
				scriptString = temp;
			}

			return true;
		}


		b32 fParseAndReplaceImports( tScriptFile& scriptFile, tDynamicBuffer& scriptString, const std::string& fileBasedUniqueTag, tFilePathPtrList& indirectGenFilesAddTo )
		{
			const char* src = ( const char* )scriptString.fBegin( );
			const char* importTag = "sigimport";
			const u32 importTagLen = ( u32 )strlen( importTag );

			tGrowableArray<char> dst;

			b32 commentedOut = false;
			const char* next = fFindSpecialTag( src, importTag, importTagLen );
			while( next )
			{
				fPut( dst, src, next - importTagLen - 1 );

				// import file...

				const char* fileName = next;
				while( *fileName && ( isspace( *fileName ) || *fileName == '\"' ) ) ++fileName;
				const char* fileNameEnd = fileName;
				while( *fileNameEnd && *fileNameEnd != '\"' ) ++fileNameEnd;

				const tFilePathPtr specFileNamePath = tFilePathPtr( std::string( fileName, fileNameEnd ) );

				tLoadInPlaceResourcePtr* lipResource = iAssetGenPlugin::fAddDependency( scriptFile, specFileNamePath );

				if( lipResource && fCheckExtension( specFileNamePath.fCStr( ), ".nut" ) )
					scriptFile.mScriptImports.fPushBack( lipResource );

				if( gFirstPlatform )
					log_line( 0, "\tfound sigimport: " << specFileNamePath );

				indirectGenFilesAddTo.fPushBack( specFileNamePath );

				while( *fileNameEnd && *fileNameEnd == '\"' ) ++fileNameEnd;

				src		= fileNameEnd;
				next	= fFindSpecialTag( src, importTag, importTagLen );
			}

			fPut( dst, src );
			if( dst.fCount( ) > 0 )
				dst.fBack( ) = 0; // null terminate destination character array
			else
				dst.fPushBack( 0 );

			// replace script string
			scriptString.fCreateNullTerminated( ( const Sig::byte* )dst.fBegin( ) );

			return true;
		}

		b32 fParseAndReplaceExports( tScriptFile& scriptFile, tDynamicBuffer& scriptString, const std::string& fileBasedUniqueTag )
		{
			const char* src = ( const char* )scriptString.fBegin( );
			const char* exportTag = "sigexport";
			const u32 exportTagLen = ( u32 )strlen( exportTag );

			const char* functionTag = "function";
			const u32 functionTagLen = ( u32 )strlen( functionTag );

			tGrowableArray<char> dst;

			const char* next = fFindSpecialTag( src, exportTag, exportTagLen );
			while( next )
			{
				fPut( dst, src, next - exportTagLen - 1 );

				if( strncmp( next, functionTag, functionTagLen ) == 0 )
				{
					// function export
					const char* functionName = next + functionTagLen;
					while( isspace( *functionName ) ) ++functionName;
					const char* functionNameEnd = functionName;
					while( !isspace( *functionNameEnd ) && *functionNameEnd != '(' ) ++functionNameEnd;

					const std::string functionNameStr = std::string( functionName, functionNameEnd );
					const std::string uniqueFunctionNameStr = fileBasedUniqueTag + functionNameStr;

					if( gFirstPlatform )
						log_line( 0, "\tfound sigexport: " << functionNameStr );

					// add script export function to list
					tScriptFile::tExportedFunction xfunc;
					xfunc.mExportedName = scriptFile.fAddLoadInPlaceStringPtr( functionNameStr.c_str( ) );
					xfunc.mCallableName = scriptFile.fAddLoadInPlaceStringPtr( uniqueFunctionNameStr.c_str( ) );
					xfunc.mScriptObject = tScriptFile::tScriptObjectStorage( );
					scriptFile.mExportedFunctions.fPushBack( xfunc );

					// if the script export is one of the "standard" exports, cache it for optimal runtime access
					for( u32 istdexp = 0; istdexp < tScriptFile::cStandardExportedFunctionCount; ++istdexp )
					{
						if( cStandardExportFunctionNames[ istdexp ] == functionNameStr )
						{
							if( gFirstPlatform )
								log_line( 0, "\tfound standard sigexport: " << functionNameStr );
							scriptFile.mStandardExportedFunctions[ istdexp ] = xfunc;
							break;
						}
					}

					fPut( dst, functionTag );
					fPut( dst, " " );
					fPut( dst, uniqueFunctionNameStr.c_str( ) );

					next = functionNameEnd;
				}
				//else
				//{
				//	// variable export...

				//	const char* varName = next;
				//	while( isspace( *varName ) ) ++varName;
				//	const char* varNameEnd = varName;
				//	while( fIsValidVariableCharacter( *varNameEnd ) ) ++varNameEnd;

				//	const std::string varNameStr = std::string( varName, varNameEnd );

				//	if( !commentedOut )
				//	{
				//		tScriptFile::tExportedVariable xvar;
				//		xvar.mExportedName = scriptFile.fAddLoadInPlaceStringPtr( varNameStr.c_str( ) );
				//		xvar.mCallableName = scriptFile.fAddLoadInPlaceStringPtr( varNameStr.c_str( ) );
				//		scriptFile.mExportedVariables.fPushBack( xvar );
				//	}

				//	fPut( dst, varNameStr.c_str( ) );

				//	next = varNameEnd;
				//}

				src		= next;
				next	= fFindSpecialTag( src, exportTag, exportTagLen );
			}

			fPut( dst, src );
			if( dst.fCount( ) > 0 )
				dst.fBack( ) = 0; // null terminate destination character array
			else
				dst.fPushBack( 0 );

			// replace script string
			scriptString.fCreateNullTerminated( ( const Sig::byte* )dst.fBegin( ) );

			return true;
		}

		b32 fHandleSpecialSymbols( tScriptFile& scriptFile, tDynamicBuffer& scriptString, const tFilePathPtr& inputFileName, tFilePathPtrList& indirectGenFilesAddTo )
		{
			const u32 fileBasedUniqueId = tScriptFileConverter::fGenerateUniqueFileBasedId( inputFileName );
			const std::string fileBasedUniqueTag = tScriptFileConverter::fGenerateUniqueFileBasedTag( fileBasedUniqueId );

			scriptFile.mUniqueId = fileBasedUniqueId;

			if( !fReadScriptClass( scriptFile, scriptString ) )
				return false;

			tGrowableArray< tScriptFileConverter::tExportedVariable > variables;
			tGrowableArray< std::string > groups;
			if( !tScriptFileConverter::fParseAndReplaceSigvars( &scriptFile, scriptString, fileBasedUniqueTag, variables, groups, false ) )
				return false;

			if( !fParseAndReplaceImports( scriptFile, scriptString, fileBasedUniqueTag, indirectGenFilesAddTo ) )
				return false;

			if( !fParseAndReplaceExports( scriptFile, scriptString, fileBasedUniqueTag ) )
				return false;

			return true;
		}

		b32 fConvertToScriptFile( tScriptFile& scriptFile, const tFilePathPtr& inputFileName, tDynamicBuffer& scriptString, tScriptVm& svm, tFilePathPtrList& indirectGenFilesAddTo, tPlatformId pid )
		{
			// before compiling the script string, we pre-process it for certain items
			if( !fHandleSpecialSymbols( scriptFile, scriptString, inputFileName, indirectGenFilesAddTo ) )
			{
				log_warning( 0, "Error parsing script file [" << inputFileName << "] for special tags, conversion failed." );
				return false;
			}

			// TODO REFACTOR need to re-enable this at some point: the problem is having the const table in place before compilation, which is actually feasible via project file
			const b32 compileToByteCode = false;
			const b32 endianSwap = fPlatformNeedsEndianSwap( cCurrentPlatform, pid );

			if( compileToByteCode )
			{
				// now we compile the string
				if( !svm.fCompileString( ( const char* )scriptString.fBegin( ), inputFileName.fCStr( ) ) )
				{
					log_warning( 0, "Error compiling script file [" << inputFileName << "], conversion failed." );
					return false;
				}

				// store the byte-code
				if( !svm.fGenerateByteCode( scriptFile.mByteCode, endianSwap ) )
				{
					log_warning( 0, "Error generating byte code for script file [" << inputFileName << "], conversion failed." );
					return false;
				}

				// set flag for using compiled byte code
				scriptFile.mFlags |= tScriptFile::cFlagCompiledByteCode;
			}
			else
			{
				scriptFile.mByteCode = scriptString;

				// clear flag for using compiled byte code
				scriptFile.mFlags &= ~tScriptFile::cFlagCompiledByteCode;
			}

			return true;
		}
	}


	u32 tScriptFileConverter::fGenerateUniqueFileBasedId( const tFilePathPtr& inputFileName )
	{
		const tFilePathPtr relPath = ToolsPaths::fMakeResRelative( inputFileName );
		const std::string simpleName = fStripExtension( fNameFromPath( relPath.fCStr( ) ).c_str( ) );

		const u32 hash = Hash::fGenericHash( ( const Sig::byte* )relPath.fCStr( ), relPath.fLength( ), ~0 );

		return hash;
	}

	std::string tScriptFileConverter::fGenerateUniqueFileBasedTag( u32 uniqueId )
	{
		std::stringstream ss;

		ss << "_" << std::hex << uniqueId << "_";

		return ss.str( );
	}

	b32 tScriptFileConverter::fParseAndReplaceSigvars( tScriptFile* scriptFile, tDynamicBuffer& scriptString, const std::string& fileBasedUniqueTag, tGrowableArray< tExportedVariable >& variablesOut, tGrowableArray< std::string >& groupsOut, b32 skipReplacement )
	{
		const char* src = ( const char* )scriptString.fBegin( );
		const char* varsTag = "sigvars";
		const u32 varsTagLen = ( u32 )strlen( varsTag );

		tGrowableArray<char> dst;
		const b32 releaseMode = false;

		tLoadInPlaceStringPtr *sigVarStrPtr = NULL;

		u32 lineCounter = 0;
		const char* line = src;
		while( line )
		{
			++lineCounter;

			if( *line == '\n' )
			{
				line = fReadLine( line );
				continue;
			}

			const char* nextLine = fReadLine( line );

			if( strncmp( line, varsTag, varsTagLen ) == 0 )
			{
				// sigvar line
				const char* varStart = fReadUntilNonSpace( line + varsTagLen );
				const char* varEnd = fReadUntilNewLineOrCharacter( varStart, '/' );
				std::string group( varStart, varEnd );
				groupsOut.fPushBack( fEatWhiteSpace( group ) );

				fPut( dst, "\n" ); 
			}
			else if ( *line == '@' )
			{
				// found a var
				tExportedVariable variable;
				variable.mGroupIndex = groupsOut.fCount( ) - 1;
				variable.mType = cVariableTypeEnum;
				variable.mLineNumber = lineCounter;

				const char* varStart = line + 2;
				const char* varEnd = fReadUntilNewLineOrCharacter( varStart, ']' );
				variable.mExportedName = std::string( varStart, varEnd );

				const char* dataStart = fReadUntilNonSpace( varEnd + 1 );

				// Parse description
				const char* descStart = fReadUntilNonSpace( dataStart + 1 ) + 1;
				const char* descEnd = fReadUntilNewLineOrCharacter( descStart, '"' );
				variable.mDescription = fEatWhiteSpace( std::string( descStart, descEnd ) );

				// Parse current value
				const char* valueStart = fReadUntilNonSpace( fReadUntilNewLineOrCharacter( descEnd + 1, ',' ) + 1 );
				const char* valueEnd = NULL;
				if( *valueStart == '"' ) 
				{
					variable.mType = cVariableTypePath;
					valueEnd = fReadUntilNewLineOrCharacter( valueStart + 1, '"' ) + 1;
				}
				else if( *valueStart == '(' )
				{
					variable.mType = cVariableTypeVector;
					valueEnd = fReadUntilNewLineOrCharacter( valueStart + 1, ')' ) + 1;
				}
				else
					valueEnd = fReadUntilNewLineOrCharacter( valueStart, ',' );

				variable.mCurrentValue = fEatWhiteSpace( std::string( valueStart, valueEnd ) );

				// Parse Range
				const char* rangeStart = fReadUntilNonSpace( fReadUntilNewLineOrCharacter( valueEnd + 1, '[' ) + 1 );
				const char* rangeEnd = fReadUntilNewLineOrCharacter( rangeStart, ']' );
				variable.mPotentialValues = fEatWhiteSpace( std::string( rangeStart, rangeEnd ) );
				
				if( variable.mType != cVariableTypeVector && variable.mPotentialValues.find_first_of( ':' ) != std::string::npos )
				{
					// number type
					if( variable.mPotentialValues.find_first_of( '.' ) != std::string::npos || variable.mCurrentValue.find_first_of( '.' ) != std::string::npos )
						variable.mType = cVariableTypeFloat;
					else
						variable.mType = cVariableTypeInt;
				}

				// Parse comment
				const char* commentStart = fReadUntilNewLineOrCharacter( rangeEnd + 1, '"' );
				b32 hasComment = ( *commentStart != '\n' );
				const char* commentEnd = hasComment ? fReadUntilNewLineOrCharacter( commentStart + 1, '"' ) : commentStart;
				if( hasComment ) variable.mComment = std::string( commentStart + 1, commentEnd );

				// generate a unique callable name
				variable.mCallableName = fileBasedUniqueTag + variable.mExportedName;
				variablesOut.fPushBack( variable );

				if( !releaseMode && scriptFile )
				{
					//put the new global variable in with its value

					tScriptFile::tExportedVariable xVar;
					xVar.mExportedName = scriptFile->fAddLoadInPlaceStringPtr( variable.mExportedName.c_str( ) );
					xVar.mCallableName = scriptFile->fAddLoadInPlaceStringPtr( variable.mCallableName.c_str( ) );
					scriptFile->mExportedVariables.fPushBack( xVar );

					std::string output = variable.mCallableName + " <- " + variable.fValue( ) + ";\n";
					fPut( dst, output.c_str( ) );
				}
				else
					fPut( dst, "\n" );
			}
			else
			{
				// keep this line, replace instances of sigvars
				std::string lineStr = nextLine ? std::string( line, nextLine ) : std::string( line );

				if( skipReplacement && StringUtil::fStrStrI( lineStr.c_str( ), "class" ) )
					return true;

				for( u32 i = 0; i < variablesOut.fCount( ); ++i )
				{
					std::string find = std::string( "@[" ) + variablesOut[ i ].mExportedName + "]";
					std::string replaceWith;

					if( releaseMode ) replaceWith = variablesOut[ i ].fValue( );
					else replaceWith = std::string( "::" ) + variablesOut[ i ].mCallableName;

					fReplaceAllOf( lineStr, find.c_str( ), replaceWith.c_str( ) );
				}

				fPut( dst, lineStr.c_str( ) ); 
			}

			line = nextLine;
		}

		if( dst.fCount( ) > 0 && dst.fBack( ) != 0 )
			dst.fBack( ) = 0; // null terminate destination character array
		else
			dst.fPushBack( 0 );

		// replace script string
		scriptString.fCreateNullTerminated( ( const Sig::byte* )dst.fBegin( ) );

		return true;
	}

	std::string tScriptFileConverter::tExportedVariable::fGenerateLine( ) const
	{
		std::string line = std::string( "@[" ) + mExportedName + "] { \"" + mDescription + "\", " + mCurrentValue + ", [ " + mPotentialValues + " ]";

		if( mComment.length( ) > 0 ) line += ", \"" + mComment + "\"";

		line += " }\n";

		return line;
	}

	std::string tScriptFileConverter::tExportedVariable::fValue( ) const
	{
		if( mType == cVariableTypeVector )
		{
			tGrowableArray<float> values = fValues( );

			u32 count = values.fCount( );
			if( count < 2 )
			{
				log_warning( 0, "Too few values in sigvar vector type." );
				count = 2;
			}
			else if( count > 4u )
			{
				log_warning( 0, "Too many values in sigvar vector type." );
				count = 4;
			}

			return std::string( "Math.Vec" + StringUtil::fToString( count ) + ".Construct" ) + mCurrentValue;
		}
		else
			return mCurrentValue;
	}

	tGrowableArray<float> tScriptFileConverter::tExportedVariable::fValues( ) const
	{
		sigassert( mType == cVariableTypeVector );
		// trim ( )'s from value
		std::string value( mCurrentValue.c_str( ) + 1, mCurrentValue.length( ) - 2);
		tGrowableArray<std::string> values;
		StringUtil::fSplit( values, value.c_str( ), "," );
		tGrowableArray<float> fvalues;
		for( u32 i = 0; i < values.fCount( ); ++i )
			fvalues.fPushBack( atof( values[ i ].c_str( ) ) );

		return fvalues;
	}

	void tScriptFileConverter::fDetermineInputOutputs( 
		tInputOutputList& inputOutputsOut,
		const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( fCheckExtension( immediateInputFiles[ i ].fCStr( ), ".nut" ) )
			{
				//log_line( 0, immediateInputFiles[ i ] );
				inputOutputsOut.fGrowCount( 1 );
				inputOutputsOut.fBack( ).mOriginalInput = immediateInputFiles[i];
				inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[ i ] );
				inputOutputsOut.fBack( ).mOutput = iAssetGenPlugin::fCreateRelativeOutputPath( immediateInputFiles[ i ], tScriptFile::fGetFileExtension( ) );
			}
		}
	}

	void tScriptFileConverter::fConvertScript( tDynamicBuffer scriptString, const tFilePathPtr& inputFileName, const tFilePathPtr& output, tPlatformId pid, tFilePathPtrList& indirectGenFilesAddTo, const tFilePathPtrList& additionalDependencies )
	{
		tScriptVm svm;
		tScriptFile scriptFile;
		if( !fConvertToScriptFile( scriptFile, inputFileName, scriptString, svm, indirectGenFilesAddTo, pid ) )
			return;

		for( u32 i = 0; i < additionalDependencies.fCount( ); ++i )
			iAssetGenPlugin::fAddDependency( scriptFile, additionalDependencies[ i ] );

		// create output file writer
		tFileWriter ofile( output );
		if( !ofile.fIsOpen( ) )
		{
			log_warning( 0, "Couldn't open output script file [" << output << "], conversion failed for platform [" << fPlatformIdString( pid ) << "]" );
			return;
		}

		// set signature for this platform
		scriptFile.fSetSignature( pid, Rtti::fGetClassId<tScriptFile>( ), tScriptFile::cVersion );

		// serialize to file
		tLoadInPlaceSerializer ser;
		ser.fSave( scriptFile, ofile, pid );
	}

	void tScriptFileConverter::fProcessInputOutputs(
		const tInputOutput& inOut,
		tFilePathPtrList& indirectGenFilesAddTo )
	{
		sigassert( inOut.mInputs.fCount( ) == 1 );

		// store input file name in friendly variable
		const tFilePathPtr inputFileName = inOut.mInputs.fFront( );

		tDynamicBuffer scriptString;
		if( !FileSystem::fReadFileToBuffer( scriptString, inputFileName, "\r\n\r\n" ) )
		{
			log_warning( 0, "Empty script file [" << inputFileName << "] - was this intended?" );
			scriptString = tDynamicBuffer( );
			scriptString.fPushBack( 0 );
		}

		// for each platform
		gFirstPlatform = true;
		for( u32 i = 0; i < inOut.mPlatformsToConvert.fCount( ); ++i, gFirstPlatform = false )
		{
			const tPlatformId pid = inOut.mPlatformsToConvert[ i ];
			const tFilePathPtr outputPath = iAssetGenPlugin::fCreateAbsoluteOutputPath( pid, inOut.mOutput );

			fConvertScript( scriptString, inputFileName, outputPath, pid, indirectGenFilesAddTo, tFilePathPtrList( ) );
		}
	}


	u32 tScriptFileConverter::fReplaceFilePathReferences( std::string& scriptString, const tFilePathPtr& replace, const tFilePathPtr& with )
	{
		// open script file, find all strings, convert to tFilePathPtr, and compare to replace, replace if matched.

		u32 replaceCnt = 0;
		std::string newScript;
		const char* start = scriptString.c_str( );
		const char* lastEnd = start;

		const std::string replacement = StringUtil::fReplaceAllOf( std::string( with.fCStr( ) ), "\\", "/" );

		if( start )
		{
			do 
			{
				start = StringUtil::fReadUntilCharacter( lastEnd, '"' );
				if( *start != 0 )
				{
					newScript += std::string( lastEnd, start );

					//found a string
					const char* end = StringUtil::fReadUntilCharacter( start + 1, '"' );

					if( *end != 0 )
					{
						//string is legit
						lastEnd = end + 1;

						std::string str( start + 1, end );
						tFilePathPtr path( str );
						if( path == replace )
						{
							++replaceCnt;
							log_line( 0, "Replaced " << path << " with " << replacement );
							newScript += "\"" + replacement + "\"";
						}
						else
							newScript += "\"" + str + "\"";
					}
					else
					{
						//string is not legit, wtf!
						log_warning( 0, "End of file found inside of string! Path replacement aborted" );
						return 0;
					}
				}
				

			} while( *start );

			newScript += std::string( lastEnd, scriptString.c_str( ) + scriptString.length( ) );
		}

		scriptString = newScript;

		return replaceCnt;
	}

}
