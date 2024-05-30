#ifndef __tDataTableConverter__
#define __tDataTableConverter__
#include "tDataTableFile.hpp"

namespace Sig
{
	class tFileWriter;

	class tools_export tDataTableConverter : public tDataTableFile
	{
	public:
		typedef tDynamicArray< std::wstring > tCellMatrixRow;
		typedef tDynamicArray< tCellMatrixRow > tCellMatrix;
		static b32 fIsCsvFile( const tFilePathPtr& csvPath );
	public:
		b32 fConvertPlatformCommon( const tFilePathPtr& csvPath );
		b32 fConvertPlatformSpecific( tPlatformId pid );
		void fOutput( tFileWriter& writer, tPlatformId pid );
	private:
		b32 fReadCsv( const tFilePathPtr& csvPath, std::wstring& fileContents, std::wstring& delimiter );
		b32 fReadTxt( const tFilePathPtr& csvPath, std::wstring& fileContents, std::wstring& delimiter );
		b32 fParseFile( const std::wstring& fileContents, const std::wstring& delimiter );
		void fParseTable( const tGrowableArray<std::wstring>& tableLines, const std::wstring& delimiter );
		tDataTableCellArray* fParseCellArray( const tCellMatrix& cellMatrix, u32 ithCellArray, b32 byColumn );
	};

}

#endif//__tDataTableConverter__

