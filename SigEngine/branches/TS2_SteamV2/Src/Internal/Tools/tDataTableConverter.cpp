#include "ToolsPch.hpp"
#include "tDataTableConverter.hpp"
#include "EndianUtil.hpp"
#include "FileSystem.hpp"
#include "tFileWriter.hpp"
#include "tLoadInPlaceSerializer.hpp"

namespace Sig
{
	b32 tDataTableConverter::fIsCsvFile( const tFilePathPtr& csvPath )
	{
		return 
			StringUtil::fCheckExtension( csvPath.fCStr( ), ".csv" ) ||
			StringUtil::fCheckExtension( csvPath.fCStr( ), ".tab" );
	}

	b32 tDataTableConverter::fConvertPlatformCommon( const tFilePathPtr& csvPath )
	{
		std::wstring fileContents, delimiter;

		b32 readSuccess = false;
		if( StringUtil::fCheckExtension( csvPath.fCStr( ), ".csv" ) )
			readSuccess = fReadCsv( csvPath, fileContents, delimiter );
		else if( StringUtil::fCheckExtension( csvPath.fCStr( ), ".tab" ) )
			readSuccess = fReadTxt( csvPath, fileContents, delimiter );

		if( !readSuccess )
			return false;

		if( !fParseFile( fileContents, delimiter ) )
			return false;

		return true;
	}

	b32 tDataTableConverter::fConvertPlatformSpecific( tPlatformId pid )
	{
		return true;
	}

	void tDataTableConverter::fOutput( tFileWriter& ofile, tPlatformId pid )
	{
		sigassert( ofile.fIsOpen( ) );

		// set the binary file signature
		tBinaryFileBase::fSetSignature( pid, Rtti::fGetClassId<tDataTableFile>( ), tDataTableFile::cVersion );

		// output
		tLoadInPlaceSerializer ser;
		ser.fSave( static_cast<tDataTableFile&>( *this ), ofile, pid );
	}

	b32 tDataTableConverter::fReadCsv( const tFilePathPtr& csvPath, std::wstring& fileContents, std::wstring& delimiter )
	{
		tDynamicBuffer rawFileData;
		if( !FileSystem::fReadFileToBuffer( rawFileData, csvPath, "" ) )
			return false;

		fileContents = StringUtil::fStringToWString( ( const char* )rawFileData.fBegin( ) );

		delimiter = L","; // csv uses commas

		return true;
	}

	b32 tDataTableConverter::fReadTxt( const tFilePathPtr& csvPath, std::wstring& fileContents, std::wstring& delimiter )
	{
		tDynamicBuffer rawFileData;
		if( !FileSystem::fReadFileToBuffer( rawFileData, csvPath, "" ) )
			return false;
		if( rawFileData.fCount( ) <= 2 )
			return false;

		if( rawFileData[0]==0xfe && rawFileData[1]==0xff )// check for endian-swapped version of excel's tab-delimited unicode format, and swap to normal
			EndianUtil::fSwap16( rawFileData.fBegin( ), rawFileData.fCount( )/2 );

		if( rawFileData[0]==0xff && rawFileData[1]==0xfe )// check for excel's tab-delimited unicode format
			fileContents = ( const wchar_t* )&rawFileData[2];
		else // we assume it to be excel's tab-delimited ASCII format
			fileContents = StringUtil::fStringToWString( ( const char* )rawFileData.fBegin( ) );

		delimiter = L"\t"; // the text files use tabs

		return true;
	}

	b32 tDataTableConverter::fParseFile( const std::wstring& fileContents, const std::wstring& delimiter )
	{
		std::wstringstream wss; wss << std::endl;
		const std::wstring eol = wss.str( );

		b32 tableStarted = false;
		tGrowableArray< std::wstring > currentTableLines;

		for( size_t i = 0, iend = fileContents.find( eol ); iend != std::wstring::npos; iend = fileContents.find( eol, i ) )
		{
			std::wstring lineText;
			if( iend > i )
				lineText = fileContents.substr( i, iend - i - 1 );

			const b32 newTable = ( lineText.length( ) > 0 && lineText[0] == L'~' );
			const b32 endTable = ( lineText.length( ) == 0 || lineText.find_first_not_of( delimiter ) == std::wstring::npos );

			if( newTable || endTable )
			{
				// pop and process existing table lines
				fParseTable( currentTableLines, delimiter );
				currentTableLines.fSetCount( 0 );
				tableStarted = newTable;
			}

			if( tableStarted && !endTable )
				currentTableLines.fPushBack( lineText );

			i = iend + 1;
			if( i >= fileContents.length( ) )
				break;
		}

		fParseTable( currentTableLines, delimiter );
		return true;
	}

	namespace
	{
		static void fSplitCells( tDataTableConverter::tCellMatrixRow& cells, const std::wstring& lineText, const std::wstring& delimiter, u32 maxCells = ~0 )
		{
			u32 cellCount = 0;

			size_t i = 0;
			for( size_t iend = lineText.find( delimiter ); iend != std::wstring::npos; iend = lineText.find( delimiter, i ) )
			{
				std::wstring cellText;
				if( iend > i )
					cellText = lineText.substr( i, iend - i );
				cells.fPushBack( cellText );
				i = iend + 1;

				if( ++cellCount >= maxCells )
					return;
			}

			if( i < lineText.length( ) )
				cells.fPushBack( lineText.substr( i ) );

			if( maxCells != ~0 )
			{
				while( cells.fCount( ) < maxCells )
					cells.fPushBack( std::wstring(L"") );
			}
		}

		static void fCreateCellMatrix( tDataTableConverter::tCellMatrix& cellMatrix, const tGrowableArray<std::wstring>& tableLines, const std::wstring& delimiter )
		{
			if( tableLines.fCount( ) == 0 )
				return;

			// allocate the rows
			cellMatrix.fNewArray( tableLines.fCount( ) );

			// split the cells for the first row so we can determine the number of columns
			fSplitCells( cellMatrix.fFront( ), tableLines.fFront( ), delimiter );

			// trim column cells starting from the first empty one
			for( u32 i = 0; i < cellMatrix.fFront( ).fCount( ); ++i )
			{
				if( cellMatrix.fFront( )[ i ].length( ) == 0 )
				{
					cellMatrix.fFront( ).fResize( i );
					break;
				}
			}

			// now we can split the rest of the rows, given that we have established the number of columns
			sigassert( cellMatrix.fCount( ) == tableLines.fCount( ) );
			for( u32 i = 1; i < cellMatrix.fCount( ); ++i )
			{
				fSplitCells( cellMatrix[ i ], tableLines[ i ], delimiter, cellMatrix.fFront( ).fCount( ) );
				sigassert( cellMatrix[ i ].fCount( ) == cellMatrix.fFront( ).fCount( ) );
			}
		}

		static b32 fExtractNameAndTypeFromCell( const std::wstring& cellText, std::wstring& cellName, std::wstring& cellType )
		{
			size_t colon = cellText.find( L":" );
			if( colon == std::wstring::npos )
			{
				cellName = cellText;
				cellType = L"s"; // default to string
				return false;
			}
			else
			{
				cellName = std::wstring( cellText.begin( ), cellText.begin( ) + colon );
				cellType = cellText.substr( colon + 1 );
				return true;
			}
		}
	}

	void tDataTableConverter::fParseTable( const tGrowableArray<std::wstring>& tableLines, const std::wstring& delimiter )
	{
		if( tableLines.fCount( ) < 1 ) // need at least one line, for table and column names
			return;

		// pre-process each line into cells so we can easily index by row,col
		tCellMatrix cellMatrix;
		fCreateCellMatrix( cellMatrix, tableLines, delimiter );

		if( cellMatrix.fCount( ) < 1 || cellMatrix.fFront( ).fCount( ) < 1 )
			return; // not even a 1x1 matrix - need at least one cell for table name

		const std::wstring tableNameW = std::wstring( cellMatrix[0][0].begin( ) + 1, cellMatrix[0][0].end( ) );
		const std::string tableName = StringUtil::fWStringToString( tableNameW );
		const u32 numRows = cellMatrix.fCount( ) - 1;
		const u32 numCols = cellMatrix.fFront( ).fCount( ) - 1;

		tDataTable* o = new tDataTable( );

		// store table name
		o->mName = fAddLoadInPlaceStringPtr( tableName.c_str( ) );

		o->mByColumn = false;

		if( numCols > 0 )
		{
			sigassert( cellMatrix[0].fCount( ) >= numCols );
			o->mColNames.fNewArray( numCols );
			for( u32 i = 0; i < numCols; ++i )
			{
				std::wstring& cellText = cellMatrix[0][ 1 + i ];

				std::wstring cellName, cellType;
				if( fExtractNameAndTypeFromCell( cellText, cellName, cellType ) )
					o->mByColumn = true;

				const std::string colName = StringUtil::fWStringToString( cellName );
				o->mColNames[ i ] = fAddLoadInPlaceStringPtr( colName.c_str( ) );

				// so that we can quickly refer back to the type of the column, we replace the name with just the type
				cellText = cellType;
			}
		}

		if( numRows > 0 )
		{
			sigassert( cellMatrix.fCount( ) >= numRows );
			o->mRowNames.fNewArray( numRows );
			for( u32 i = 0; i < numRows; ++i )
			{
				std::wstring& cellText = cellMatrix[ 1 + i ][0];

				std::wstring cellName, cellType;
				fExtractNameAndTypeFromCell( cellText, cellName, cellType );

				const std::string rowName = StringUtil::fWStringToString( cellName );
				o->mRowNames[ i ] = fAddLoadInPlaceStringPtr( rowName.c_str( ) );

				// so that we can quickly refer back to the type of the column, we replace the name with just the type
				cellText = cellType;
			}
		}

		if( numCols > 0 && numRows > 0 )
		{
			const u32 cellArrayCount = o->mByColumn ? numCols : numRows;
			o->mCellArrays.fNewArray( cellArrayCount );

			for( u32 i = 0; i < cellArrayCount; ++i )
				o->mCellArrays[ i ] = fParseCellArray( cellMatrix, i, o->mByColumn );
		}

		mTables.fPushBack( o );
	}

	tDataTableCellArray* tDataTableConverter::fParseCellArray( const tCellMatrix& cellMatrix, u32 ithCellArray, b32 byColumn )
	{
		const std::wstring& cellType = byColumn ? cellMatrix[0][ 1 + ithCellArray ] : cellMatrix[ 1 + ithCellArray ][0];
		const u32 numCells = byColumn ? ( cellMatrix.fCount( ) - 1 ) : ( cellMatrix.fFront( ).fCount( ) - 1 );

		if( cellType == L"n" || cellType == L"b" )
		{
			const b32 asBools = ( cellType == L"b" );
			tDynamicArray<f32> vals( numCells );
			for( u32 icell = 0; icell < numCells; ++icell )
			{
				std::string cellValue = StringUtil::fWStringToString( cellMatrix[ 1 + ( byColumn ? icell : ithCellArray ) ][ 1 + ( byColumn ? ithCellArray : icell ) ] );
				vals[ icell ] = atof( cellValue.c_str( ) );
				if( asBools )
				{
					std::transform( cellValue.begin( ), cellValue.end( ), cellValue.begin( ), tolower );
					if( cellValue == "false" )
						vals[ icell ] = 0.0f;
					else if( cellValue == "true" )
						vals[ icell ] = 1.0f;
					else
						vals[ icell ] = vals[ icell ] ? 1.0f : 0.0f;
				}
			}
			return new tDataTableCellArrayNumeric( vals );
		}
		else if( cellType == L"s" || cellType == L"f" )
		{
			tDynamicArray< tLoadInPlacePtrWrapper<tLoadInPlaceStringPtr> > vals( numCells );
			for( u32 icell = 0; icell < numCells; ++icell )
			{
				const std::string cellValue = StringUtil::fWStringToString( cellMatrix[ 1 + ( byColumn ? icell : ithCellArray ) ][ 1 + ( byColumn ? ithCellArray : icell ) ] );
				vals[ icell ] = fAddLoadInPlaceStringPtr( cellValue.c_str( ) );
			}
			return new tDataTableCellArrayStringPtr( vals );
		}
		else if( cellType == L"u" )
		{
			tDynamicArray< tLocalizedString > vals( numCells );
			for( u32 icell = 0; icell < numCells; ++icell )
			{
				const std::wstring& cellValue = cellMatrix[ 1 + ( byColumn ? icell : ithCellArray ) ][ 1 + ( byColumn ? ithCellArray : icell ) ];
				vals[ icell ] = tLocalizedString( cellValue );
			}
			return new tDataTableCellArrayUnicodeString( vals );
		}
		else if( cellType == L"v" )
		{
			tDynamicArray<Math::tVec4f> vals( numCells );
			for( u32 icell = 0; icell < numCells; ++icell )
			{
				const std::string cellValue = StringUtil::fWStringToString( cellMatrix[ 1 + ( byColumn ? icell : ithCellArray ) ][ 1 + ( byColumn ? ithCellArray : icell ) ] );
				Math::tVec4f vecValue( 0 );

				u32 valueIndex = 0;
				size_t prevPos = 1;
				size_t commaPos = cellValue.find_first_of( ',', prevPos );

				while( commaPos != std::string::npos && valueIndex < 4 )
				{
					std::string subStr = cellValue.substr( prevPos, commaPos - prevPos );
					prevPos = commaPos + 1;

					vecValue.fAxis( valueIndex++ ) = atof( subStr.c_str( ) );

					commaPos = cellValue.find_first_of( ',', prevPos );
				}

				const s32 lenRemaining = cellValue.length( ) - prevPos;
				if( valueIndex < 4 && lenRemaining > 0 )
				{
					std::string subStr = cellValue.substr( prevPos, lenRemaining );
					vecValue.fAxis( valueIndex ) = atof( subStr.c_str( ) );
				}

				vals[ icell ] = vecValue;
			}
			return new tDataTableCellArrayVector4( vals );
		}
		else
		{
			log_warning( 0, "unrecognized row type: " << StringUtil::fWStringToString( cellType ) );
		}

		return 0;
	}

}

