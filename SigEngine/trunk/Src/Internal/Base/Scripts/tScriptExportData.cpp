#include "BasePch.hpp"
#include "tScriptExportData.hpp"
#include "tFileWriter.hpp"
#include "FileSystem.hpp"
#include "ToolsPaths.hpp"


namespace Sig { namespace ScriptData
{

	std::string tParameterDesc::fTrimmedCPPType( ) const
	{
		std::string clean = mCppType;
		clean = StringUtil::fReplaceAllOf( clean, "const ", "" );
		clean = StringUtil::fReplaceAllOf( clean, "&", "" );
		clean = StringUtil::fReplaceAllOf( clean, "*", "" );

		tGrowableArray<std::string> bits;
		StringUtil::fSplit( bits, clean.c_str( ), "::" );

		if( bits.fCount( ) )
			clean = bits.fBack( );
		
		return StringUtil::fEatWhiteSpace( clean );
	}

	std::string tParameterDesc::fScriptSignature( ) const
	{
		std::string line = mCppType + " " + mName;

		if( mDefaultValue.length( ) )
			line += " = " + mDefaultValue;

		return line;
	}

	b32 tParameterDesc::fIsVoid( ) const
	{
		std::string typeName = fTrimmedCPPType( );
		if( typeName.length( ) == 0 || typeName == "void" )
			return true;

		return false;
	}

	std::string tMemberDesc::fGenerateLine( ) const
	{
		std::string line;

		if( mType == cPropType )
		{
			line = "Prop: " + mName + "   G: " + mGet;
			if( mSet.length( ) )
				line += "   S: " + mSet;

			if( mReturnParam.mCppType.length( ) )
				line += "   Type: " + mReturnParam.mCppType;
		}
		else if( mType == cVarType )
		{
			line = "Var : " + mName + "   " + mGet;
			if( mReturnParam.mCppType.length( ) )
				line += "   Type: " + mReturnParam.mCppType;
		}
		else
		{
			if( mType == cStaticFuncType )
				line = "StaticFunc: " + mName + "   @: " + mGet;
			else if( mType == cGlobalFuncType )
				line = "GlobalFunc: " + mName + "   @: " + mGet;
			else
				line = "Func: " + mName + "   @: " + mGet;

			if( mReturnParam.mCppType.length( ) )
				line += "   Return: " + mReturnParam.mCppType;

			if( mParams.fCount( ) )
			{
				line += "   Params: ";
				for( u32 i = 0; i < mParams.fCount( ); ++i )
				{
					if( i != 0 )
						line += ", ";
					line += mParams[ i ].mName + " (" + mParams[ i ].mCppType + ")";
				}
			}
		}

		return line;
	}

	std::string tMemberDesc::fScriptSignature( ) const
	{
		std::string line;
		if( mReturnParam.mName.length( ) == 0 )
			line = "void";
		else
			line = mReturnParam.mName;

		line += " " + mName + "( ";

		for( u32 i = 0; i < mParams.fCount( ); ++i )
		{
			const tParameterDesc& param = mParams[ i ];
			if( i > 0 )
				line += ", ";

			line += param.fScriptSignature( );
		}

		if( mParams.fCount( ) )
			line += " ";

		line += ")";

		return line;
	}

	std::string tBindingDesc::fGenerateLine( ) const
	{
		std::string line;

		if( mType == cConstType )
			line = "" + mName;
		else
		{
			line = "Class: ";

			if( mNameSpace.length( ) )
				line += mNameSpace + ".";

			line += mName;
		}

		return line;
	}

	std::string tBindingDesc::fGetHppFile( const std::string& cppfileName, const std::string& aliasFile )
	{
		std::string baseFile = StringUtil::fStripExtension( cppfileName.c_str( ) );

		if( aliasFile.length( ) )
			baseFile = StringUtil::fDirectoryFromPath( baseFile.c_str( ) ) + aliasFile;

		std::string test = baseFile + ".hpp";
		if( FileSystem::fFileExists( tFilePathPtr( test.c_str( ) ) ) )
			return test;
		else
		{
			test = baseFile + ".h";
			if( FileSystem::fFileExists( tFilePathPtr( test.c_str( ) ) ) )
				return test;
			else
			{
				return "";
			}
		}
	}


	std::string tClassDesc::fCPPName( ) const
	{
		std::string value = mCppName;
		if( mCppBase.length( ) )
			value += " : " + mCppBase;
		return value;
	}

	std::string tClassDesc::fTrimmedCPPName( ) const
	{
		tGrowableArray<std::string> bits;
		StringUtil::fSplit( bits, mCppName.c_str( ), "::" );

		std::string value;
		if( bits.fCount( ) )
			value = bits.fBack( );
		else
			value = mCppName;

		return StringUtil::fEatWhiteSpace( value );
	}

	std::string tClassDesc::fTrimmedBaseName( ) const
	{
		tGrowableArray<std::string> bits;
		StringUtil::fSplit( bits, mCppBase.c_str( ), "::" );

		std::string value;
		if( bits.fCount( ) )
			value = bits.fBack( );
		else
			value = mCppBase;

		return StringUtil::fEatWhiteSpace( value );
	}

	void tClassDesc::fSort( )
	{
		std::sort( mMembers.fBegin( ), mMembers.fEnd( ) );
	}

	tMemberDesc* tClassDesc::fFindMember( const std::string& scriptName )
	{
		for( u32 i = 0; i < mMembers.fCount( ); ++i )
			if( mMembers[ i ].mName == scriptName )
				return &mMembers[ i ];
		return NULL;
	}

	void tNamespaceDesc::fSort( )
	{
		std::sort( mClasses.fBegin( ), mClasses.fEnd( ) );
		for( u32 i = 0; i < mClasses.fCount( ); ++i )
			mClasses[ i ].fSort( );
	}

	void tScriptExportData::fSort( )
	{
		std::sort( mConsts.fBegin( ), mConsts.fEnd( ) );
		std::sort( mNameSpaces.fBegin( ), mNameSpaces.fEnd( ) );

		for( u32 i = 0; i < mNameSpaces.fCount( ); ++i )
			mNameSpaces[ i ].fSort( );
	}

	tFilePathPtr tScriptExportData::fGetDataPath( )
	{
#ifdef __ToolsPaths__
		return ToolsPaths::fCreateTempEngineFilePath( ".bin", tFilePathPtr("SigScript"), "ScriptExportData" );
#else
		return tFilePathPtr( ); //:(
#endif
	}

	void tScriptExportData::fSave( )
	{
		tFileWriter o( fGetDataPath( ) );

		tGameArchiveSave save;
		save.fSave( *this );
		o( save.fBuffer( ).fBegin( ), save.fBuffer( ).fCount( ) );
	}

	b32 tScriptExportData::fLoad( )
	{
		tDynamicBuffer dataBuffer;

		if( !FileSystem::fReadFileToBuffer( dataBuffer, fGetDataPath( ) ) )
			return false;

		tGameArchiveLoad load( dataBuffer.fBegin( ), dataBuffer.fCount( ) );
		load.fLoad( *this );

		return !load.fFailed( );
	}

	tNamespaceDesc* tScriptExportData::fFindNameSpace( const std::string& ns )
	{
		for( u32 i = 0; i < mNameSpaces.fCount( ); ++i )
			if( mNameSpaces[ i ].mName == ns )
				return &mNameSpaces[ i ];

		return NULL;
	}

	tClassDesc* tScriptExportData::fFindClass( const std::string& scriptName )
	{
		for( u32 i = 0; i < mNameSpaces.fCount( ); ++i )
			for( u32 c = 0; c < mNameSpaces[ i ].mClasses.fCount( ); ++c )
				if( mNameSpaces[ i ].mClasses[ c ].mBinding.mName == scriptName )
					return &mNameSpaces[ i ].mClasses[ c ];

		return NULL;
	}

	tClassDesc* tScriptExportData::fFindClassByCPPName( const std::string& cppName )
	{
		for( u32 i = 0; i < mNameSpaces.fCount( ); ++i )
			for( u32 c = 0; c < mNameSpaces[ i ].mClasses.fCount( ); ++c )
				if( mNameSpaces[ i ].mClasses[ c ].fTrimmedCPPName( ) == cppName )
					return &mNameSpaces[ i ].mClasses[ c ];

		return NULL;
	}

	tClassDesc* tScriptExportData::fClassFromParameter(	tParameterDesc* param )
	{
		std::string typeName = param->fTrimmedCPPType( );
		return fFindClassByCPPName( typeName );
	}


} }
