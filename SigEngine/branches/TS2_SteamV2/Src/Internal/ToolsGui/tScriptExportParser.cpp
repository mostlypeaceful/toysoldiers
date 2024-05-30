#include "ToolsGuiPch.hpp"
#include "tScriptExportParser.hpp"
#include "FileSystem.hpp"
#include "tFileWriter.hpp"


namespace Sig { namespace ScriptData
{



	b32 tMemberDescParser::fApply( tMemberDesc& desc, wxRegEx& regEx, const wxString& str )
	{
		if( !regEx.Matches( str ) )
			return false;

		wxString type = StringUtil::fEatWhiteSpace( regEx.GetMatch( str, 1 ) );
		desc.mName = StringUtil::fEatWhiteSpace( regEx.GetMatch( str, 2 ) );

		if( type == "Prop" )
		{
			desc.mType = tMemberDesc::cPropType;
			desc.mGet = StringUtil::fEatWhiteSpace( regEx.GetMatch( str, 3 ) );
			desc.mSet = StringUtil::fEatWhiteSpace( regEx.GetMatch( str, 4 ) );
		}
		else if( type == "Var" )
		{
			desc.mType = tMemberDesc::cVarType;
			desc.mGet = StringUtil::fEatWhiteSpace( regEx.GetMatch( str, 3 ) );
		}
		else if( type == "GlobalFunc" )
		{
			desc.mType = tMemberDesc::cGlobalFuncType;
			desc.mGet = StringUtil::fEatWhiteSpace( regEx.GetMatch( str, 3 ) );
		}
		else if( type == "StaticFunc" )
		{
			desc.mType = tMemberDesc::cStaticFuncType;
			desc.mGet = StringUtil::fEatWhiteSpace( regEx.GetMatch( str, 3 ) );
		}
		else
		{
			desc.mType = tMemberDesc::cFuncType;
			desc.mGet = StringUtil::fEatWhiteSpace( regEx.GetMatch( str, 3 ) );
		}

		return true;
	}

	void tMemberDescParser::fFillTree( tMemberDesc& desc, wxTreeCtrl* tree, wxTreeItemId parent )
	{
		wxTreeItemId m = tree->AppendItem( parent, desc.fGenerateLine( ) );
		tree->SetItemData( m, new tTreeData( &desc, tTreeData::tMemberDesc ) );
	}

	wxString tMemberDescParser::fMakeHppSearchRegex( const wxString& member, u32 type )
	{
		if( member.length( ) == 0 )
			return "";

		tGrowableArray<std::string> bits;
		StringUtil::fSplit( bits, member.c_str( ), ":" );

		wxString realMember = member;
		if( bits.fCount( ) )
			realMember = bits.fBack( );

		realMember = StringUtil::fEatWhiteSpace( realMember );

		if( type == tMemberDesc::cVarType )
		{
			// the double character set here, less the coma in the second, allows for stuff like  f32 x, y, z; 
			return wxString("^([^(,{]*)\\s+?(?:[^({]*?)(?:\\m") + realMember + "\\M).*?[;]";
		}
		else
		{
			//http://www.softlion.com/webTools/RegExpTest/default.aspx (multi-line mode)
			//^([^{;(]*?)(?:\bfSubNameCStr\b[(](.*?)[)]).*?(?:[{;]|$)

			// \b word boundaries dont work with wxRegEx, use \mWord\M
			// C++ needs to double escape \ escape character \\m is a \m in regex

			/* test cases
			void fQuitAsync( ); // defined in platform-specific files
			const char*					fSubNameCStr( ) const;
			inline tStringPtr			fSubName( ) const { return tStringPtr( fSubNameCStr( ) ); }
			inline const char*			fNameCStr( ) const { return mName.fCStr( ); }
			inline tEntity*				fParent( ) const 
			{
				return NULL;
			}
			*/
			return wxString("^([^{;(]*?)(?:\\m") + realMember + "\\M[(](.*?)[)]).*?(?:[{;]|$)";
		}
	}

	b32 tMemberDescParser::fFindParameters( tMemberDesc& desc, wxRegEx& getRegex, wxRegEx& setRegex, wxRegEx& argRegex, const wxString& line, const std::string& tType )
	{
		if( getRegex.IsValid( ) && getRegex.Matches( line ) )
		{
			wxString retType = StringUtil::fEatWhiteSpace( getRegex.GetMatch( line, 1 ) );
			retType = StringUtil::fReplaceAllOf( std::string( retType ), "inline ", "" );
			retType = StringUtil::fReplaceAllOf( std::string( retType ), "static ", "" );
			desc.mReturnParam.mCppType = retType;

			std::string trimmed = desc.mReturnParam.fTrimmedCPPType( );
			if( trimmed == "t" || trimmed == "T" )
				desc.mReturnParam.mCppType = tType;

			if( desc.mReturnParam.mCppType.length( ) == 0 )
				log_warning( 0, "Parsing error, return params." );

			if( desc.fIsFunction( ) && argRegex.IsValid( ) )
			{
				tGrowableArray<std::string> params;
				StringUtil::fSplit( params, StringUtil::fEatWhiteSpace( getRegex.GetMatch( line, 2 ) ).c_str( ), "," );

				for( u32 i = 0; i < params.fCount( ); ++i )
				{
					if( desc.mType == tMemberDesc::cGlobalFuncType && i == 0 )
						continue; //first parameter is the "this" pointer

					std::string cleaned = params[ i ];

					//strip default value
					wxString defaultVal;
					tGrowableArray<std::string> defaultValue;
					StringUtil::fSplit( defaultValue, cleaned.c_str( ), "=", true );

					if( defaultValue.fCount( ) > 1 )
					{
						cleaned = defaultValue[ 0 ];
						defaultVal = defaultValue[ 1 ];
					}

					cleaned = StringUtil::fEatWhiteSpace( cleaned );

					if( argRegex.Matches( cleaned ) )
					{
						tParameterDesc pDesc;
						pDesc.mCppType = StringUtil::fEatWhiteSpace( argRegex.GetMatch( cleaned, 1 ) );
						pDesc.mName = StringUtil::fEatWhiteSpace( argRegex.GetMatch( cleaned, 2 ) );
						pDesc.mDefaultValue = StringUtil::fEatWhiteSpace( defaultVal );

						std::string trimmed = pDesc.fTrimmedCPPType( );
						if( trimmed == "t" || trimmed == "T" )
							pDesc.mCppType = tType;

						desc.mParams.fPushBack( pDesc );
					}
					else
					{
						desc.mParams.fPushBack( tParameterDesc( ) );
						desc.mParams.fBack( ).mName = "Parsing error";
						log_warning( 0, "Parsing error: " << desc.fScriptSignature( ) );
					}
				}
			}

			return true;
		}

		return false;
	}

	b32 tBindingDescParser::fApply( tBindingDesc& desc, wxRegEx& regEx, const wxString& str )
	{
		if( !regEx.Matches( str ) )
			return false;

		desc.mNameSpace = StringUtil::fEatWhiteSpace( regEx.GetMatch( str, 1 ) );
		desc.mName = StringUtil::fEatWhiteSpace( regEx.GetMatch( str, 2 ) );

		wxString constName = StringUtil::fEatWhiteSpace( regEx.GetMatch( str, 3 ) );

		if( constName.length( ) )
		{
			desc.mName = constName;
			desc.mType = tBindingDesc::cConstType;
		}
		else
			desc.mType = tBindingDesc::cClassType;

		return true;
	}

	void tBindingDescParser::fFillTree( tBindingDesc& desc, wxTreeCtrl* tree, wxTreeItemId parent )
	{
		wxTreeItemId binding = tree->AppendItem( parent, desc.fGenerateLine( ) );
		tree->SetItemData( binding, new tTreeData( &desc, tTreeData::tBindingDesc ) );
	}

	b32 tClassDescParser::fApply( tClassDesc& desc, wxRegEx& regEx, const wxString& str )
	{
		if( !regEx.Matches( str ) )
			return false;

		wxString type = regEx.GetMatch( str, 1 );

		desc.mCppName = StringUtil::fEatWhiteSpace( regEx.GetMatch( str, 2 ) );
		desc.mCppBase = StringUtil::fEatWhiteSpace( regEx.GetMatch( str, 3 ) );

		return true;
	}

	void tClassDescParser::fFillTree( tClassDesc& desc, wxTreeCtrl* tree, wxTreeItemId parent )
	{
		wxTreeItemId c = tree->AppendItem( parent, desc.mBinding.mName + " - ( " + desc.fCPPName( ) + " )" );
		tree->SetItemData( c, new tTreeData( &desc, tTreeData::tClassDesc ) );

		if( desc.mMembers.fCount( ) )
		{
			for( u32 i = 0; i <desc. mMembers.fCount( ); ++i )
				tMemberDescParser::fFillTree( desc.mMembers[ i ], tree, c );
		}
		else
			tree->AppendItem( c, "No members." );
	}

	b32 fLineCommentedOut( const wxRegEx& reg, const wxString& line )
	{
		if( reg.Matches( line ) )
		{
			std::string bitBeforeComment( StringUtil::fEatWhiteSpace( reg.GetMatch( line, 1 ) ) );
			if( bitBeforeComment.length( ) == 0 )
				return true;
		}

		return false;
	}
	
	namespace
	{
		std::string fClassAliases( const std::string& input, std::string& fileOut, std::string& tType )
		{
			if( input == "tVec2f" )
			{
				fileOut = "tVector";
				tType = "f32";
				return "tVector2";
			}
			else if( input == "tVec3f" )
			{
				fileOut = "tVector";
				tType = "f32";
				return "tVector3";
			}
			else if( input == "tVec4f" )
			{
				fileOut = "tVector";
				tType = "f32";
				return "tVector4";
			}
			else if( input == "tEulerAnglesf" )
			{
				fileOut = "tQuaternion";
				tType = "f32";
				return "tEulerAngles";
			}
			else if( input == "tMat3f" )
			{
				fileOut = "tMatrix";
				tType = "f32";
				return "tMatrix3";
			}
			else if( input == "tMat4f" )
			{
				fileOut = "tMatrix";
				tType = "f32";
				return "tMatrix4";
			}
			else if( input == "tRect" )
			{
				fileOut = "tRect";
				tType = "f32";
				return "tRectTemplate";
			}
			else if( input == "tSpheref" )
			{
				fileOut = "tSphere";
				tType = "f32";
				return "tSphere";
			}
			else if( input == "tAabbf" )
			{
				fileOut = "tAabb";
				tType = "f32";
				return "tAabb";
			}
			else if( input == "tObbf" )
			{
				fileOut = "tObb";
				tType = "f32";
				return "tObb";
			}
			else
				return input;
		}
	}

	void tClassDescParser::fFindMemberParameters( tClassDesc& desc )
	{
		std::string aliasFile, tType;

		wxRegEx commentedOutRegex( "(.*)//(.*)" );
		wxRegEx argRegex( "^((?:.*)(?:\\s+|\\*|&))(.*)", wxRE_ADVANCED );
		wxRegEx classRegex( "(?:\\mclass\\M|\\mstruct\\M)\\s+?.*?\\m" + fClassAliases( desc.fTrimmedCPPName( ), aliasFile, tType ) + "\\M", wxRE_ADVANCED );

		tFilePathPtr cpp( desc.mBinding.mFoundInFile.c_str( ) );
		tFilePathPtr hpp( tBindingDesc::fGetHppFile( desc.mBinding.mFoundInFile, aliasFile ) );

		tDynamicBuffer hppBuffer;
		tDynamicBuffer cppBuffer;
		FileSystem::fReadFileToBuffer( hppBuffer, hpp, "\0" );
		FileSystem::fReadFileToBuffer( cppBuffer, cpp, "\0" );

		tGrowableArray< std::string > hppLines;
		tGrowableArray< std::string > cppLines;
		StringUtil::fSplit( hppLines, ( const char* )hppBuffer.fBegin( ), "\n", true );
		StringUtil::fSplit( cppLines, ( const char* )cppBuffer.fBegin( ), "\n", true );


		//find the class
		u32 classStart = 0;
		for( u32 l = 0; l < hppLines.fCount( ); ++l )
		{
			wxString line( hppLines[ l ] );
			if( fLineCommentedOut( commentedOutRegex, line ) )
				continue;

			if( classRegex.Matches( line ) )
			{
				classStart = l + 1;
				break;
			}
		}

		for( u32 i = 0; i < desc.mMembers.fCount( ); ++i )
		{
			wxRegEx getRegex( tMemberDescParser::fMakeHppSearchRegex( desc.mMembers[ i ].mGet, desc.mMembers[ i ].mType ), wxRE_ADVANCED );
			wxRegEx setRegex( tMemberDescParser::fMakeHppSearchRegex( desc.mMembers[ i ].mSet, desc.mMembers[ i ].mType ), wxRE_ADVANCED );

			b32 found = false;

			for( u32 l = classStart; l < hppLines.fCount( ) && !found; ++l )
			{				
				wxString line( hppLines[ l ] );
				if( fLineCommentedOut( commentedOutRegex, line ) )
					continue;

				if( tMemberDescParser::fFindParameters( desc.mMembers[ i ], getRegex, setRegex, argRegex, line, tType ) )
					found = true;
			}

			if( !found )
			{
				//search cpp file
				for( u32 l = 0; l < cppLines.fCount( ) && !found; ++l )
				{				
					wxString line( cppLines[ l ] );
					if( fLineCommentedOut( commentedOutRegex, line ) )
						continue;

					if( tMemberDescParser::fFindParameters( desc.mMembers[ i ], getRegex, setRegex, argRegex, line, tType ) )
						found = true;
				}
			}

			if( !found )
			{
				log_warning( 0, "Could not find member: " << desc.mMembers[ i ].mName << " from " << desc.fCPPName( ) );
			}
		}
	}

	const char* tMemberDescParser::cSearch = "^\\s*\\.\\m(Prop|Func|Var|StaticFunc|GlobalFunc)\\M\\s*\\(\\s*(?:_SC\\s*\\()?\\s*(?:\"(.*?)\").*?&(.*?)[,)]\\s*(?:&(.*?)\\))?.*$";
	const char* tBindingDescParser::cSearch = "^\\s*vm.f(?:RootTable|ConstTable|(?:Namespace.*?(?:\"(.*?)\"))).*(?:.Bind.*?(?:\"(.*?)\")|.Const.*(?:\"(.*?)\"))";  //.*$ //seemd to not be needed here
	const char* tClassDescParser::cSearch = "^\\s*Sqrat::(DerivedClass|Class)\\s*<(.*?),(?:(?:(.*?),.*?>)|(?:(.*?)>))";  //.*$ //seemd to not be needed here

	void tNamespaceDescParser::fFillTree( tNamespaceDesc& desc, wxTreeCtrl* tree, wxTreeItemId parent )
	{
		wxTreeItemId ns = tree->AppendItem( parent, desc.mName.length( ) ? desc.mName : "Unnamed" );
		tree->SetItemData( ns, new tTreeData( &desc, tTreeData::tNamespaceDesc ) );

		for( u32 i = 0; i < desc.mClasses.fCount( ); ++i )
			tClassDescParser::fFillTree( desc.mClasses[ i ], tree, ns );
	}

	void tScriptExportDataParser::fAddClass( tScriptExportData& data, const tClassDesc& c, const tFilePathPtr& srcFile )
	{
		if( c.mBinding.mType == tBindingDesc::cConstType )
		{
			data.mConsts.fPushBack( c.mBinding );
			data.mConsts.fBack( ).mFoundInFile = std::string( srcFile.fCStr( ) );
		}
		else
		{
			tNamespaceDesc* ns = NULL;
			for( u32 i = 0; i < data.mNameSpaces.fCount( ); ++i )
			{
				if( data.mNameSpaces[ i ].mName == c.mBinding.mNameSpace )
				{
					ns = &data.mNameSpaces[ i ];
					break;
				}
			}

			if( !ns )
			{
				data.mNameSpaces.fPushBack( tNamespaceDesc( ) );
				ns = &data.mNameSpaces.fBack( );
				ns->mName = c.mBinding.mNameSpace;
			}

			ns->mClasses.fPushBack( c );
			tClassDesc& newClass = ns->mClasses.fBack( );
			
			newClass.mBinding.mFoundInFile = std::string( srcFile.fCStr( ) );
			
			tClassDescParser::fFindMemberParameters( newClass );
			
			if( newClass.mConstructor.mReturnParam.mCppType.length( ) == 0 )
			{
				//not initialized
				newClass.mConstructor.mReturnParam.mCppType = newClass.fTrimmedCPPName( );
			}
		}
	}

	void tScriptExportDataParser::fBuild( tScriptExportData& data, wxDialog* progress )
	{
		tFilePathPtrList src;
		// base: uncomment this to run on the base too. takes a lot longer
		FileSystem::fGetFileNamesInFolder( src, tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineRootFolder( ), tFilePathPtr( "Src/Internal/Base" ) ), 1, true, tFilePathPtr( ".cpp" ) );
		
		// project
		FileSystem::fGetFileNamesInFolder( src, ToolsPaths::fGetCurrentProjectSrcFolder( ), 1, true, tFilePathPtr( ".cpp" ) );
		
		wxRegEx classRegex( tClassDescParser::cSearch, wxRE_ADVANCED );
		wxRegEx bindingRegex( tBindingDescParser::cSearch, wxRE_ADVANCED );
		wxRegEx memberRegex( tMemberDescParser::cSearch, wxRE_ADVANCED );

		u32 srcCnt = src.fCount( );

		//uncomment this to restrict parsing to 50 files
		//srcCnt = fMin( src.fCount( ), 50u );

		if( classRegex.IsValid( ) && bindingRegex.IsValid( ) && memberRegex.IsValid( ) )
		{
			for( u32 i = 0; i < srcCnt; ++i )
			{
				if( progress )
					progress->SetLabel( "Scanning: " + StringUtil::fToString( i ) + "/" + StringUtil::fToString( src.fCount( ) ) );

				tDynamicBuffer buffer;
				FileSystem::fReadFileToBuffer( buffer, src[i], "\0" );

				tGrowableArray< std::string > lines;
				StringUtil::fSplit( lines, ( const char* )buffer.fBegin( ), "\n" );

				tClassDesc desc;
				b32 inClass = false;

				for( u32 l = 0; l < lines.fCount( ); ++l )
				{
					wxString line( lines[ l ] );

					if( !inClass )
					{
						if( tClassDescParser::fApply( desc, classRegex, line ) )
							inClass = true;
						else if( tBindingDescParser::fApply( desc.mBinding, bindingRegex, line ) )
							fAddClass( data, desc, src[i] );
					}
					else
					{
						tMemberDesc mDesc;
						if( tMemberDescParser::fApply( mDesc, memberRegex, line ) )
							desc.mMembers.fPushBack( mDesc );
						else if( tBindingDescParser::fApply( desc.mBinding, bindingRegex, line ) )
						{
							inClass = false;
							fAddClass( data, desc, src[i] );
							desc = tClassDesc( ); //reset
						}
					}
				}
			}
		}

		fAddDefaultData( data );

		data.fSort( );
		data.fSave( );
	}

	void tScriptExportDataParser::fAddDefaultData( tScriptExportData& data )
	{
		tClassDesc* desc = data.fFindClass( "Entity" );
		if( desc )
		{
			tMemberDesc onEntityCreate;
			onEntityCreate.mType = tMemberDesc::cFuncType;
			onEntityCreate.mName = "OnEntityCreate";
			onEntityCreate.mGet = "Entity::OnEntityCreate";
			tParameterDesc param1;
			param1.mName = "entity";
			param1.mCppType = "tEntity";
			onEntityCreate.mParams.fPushBack( param1 );
			desc->mMembers.fPushBack( onEntityCreate );
		}
	}

	void tScriptExportDataParser::fFillTree( tScriptExportData& data, wxTreeCtrl* tree )
	{
		tree->DeleteAllItems( );

		wxTreeItemId root = tree->AddRoot( "Root" );
		tree->SetItemData( root, new tTreeData( tTreeData::cRoot ) );

		wxTreeItemId nameSpaces = tree->AppendItem( root, "Namespaces" );
		tree->SetItemData( nameSpaces, new tTreeData( tTreeData::cNamespaces ) );

		for( u32 i = 0; i < data.mNameSpaces.fCount( ); ++i )
			tNamespaceDescParser::fFillTree( data.mNameSpaces[ i ], tree, nameSpaces );

		wxTreeItemId consts = tree->AppendItem( root, "Consts" );
		tree->SetItemData( consts, new tTreeData( tTreeData::cConsts ) );

		for( u32 i = 0; i < data.mConsts.fCount( ); ++i )
			tBindingDescParser::fFillTree( data.mConsts[ i ], tree, consts );

		tree->Expand( root );
	}

	void tScriptExportDataParser::fFindInTree( const char* text, wxTreeCtrl* tree, wxTreeItemId rootNode, tGrowableArray<wxTreeItemId>& searchResults )
	{
		if( !text || !tree || strlen( text ) <= 0 )
			return;

		if( !rootNode )
			rootNode = tree->GetRootItem( );

		tTreeData* data = static_cast<tTreeData*> ( tree->GetItemData( rootNode ) );
		if( !data )
			return;

		// cUser is the only tree data type we care about
		if( data->mType == tTreeData::cUser )
		{
			tBaseData* baseData = data->mData;
			const char* searchStr = NULL;
			switch( data->mDataType )
			{
			case tTreeData::tBindingDesc:
				{
					tBindingDesc* desc = static_cast<tBindingDesc*>( data->mData );
					if( desc )
						searchStr = desc->mName.c_str( );
				}
				break;

			case tTreeData::tClassDesc:
				{
					tClassDesc* desc = static_cast<tClassDesc*> ( data->mData );
					if( desc )
						searchStr = desc->mBinding.mName.c_str( );
				}
				break;

			case tTreeData::tMemberDesc:
				{
					tMemberDesc* desc = static_cast<tMemberDesc*> ( data->mData );
					if( desc )
						searchStr = desc->mName.c_str( );
				}
				break;

			case tTreeData::tNamespaceDesc:
				{
					tNamespaceDesc* desc = static_cast<tNamespaceDesc*> ( data->mData );
					if( desc )
						searchStr = desc->mName.c_str( );
				}
				break;

			case tTreeData::tParameterDesc:
				{
					tParameterDesc* desc = static_cast<tParameterDesc*> ( data->mData );
					if( desc )
						searchStr = desc->mName.c_str( );
				}
				break;

			default:
				break;
			}

			if( searchStr && strstr( searchStr, text ) )
				searchResults.fPushBack( rootNode );
		}

		wxTreeItemIdValue cookie = NULL;
		wxTreeItemId child = tree->GetFirstChild( rootNode, cookie );
		while( child && child.IsOk( ) )
		{
			fFindInTree( text, tree, child, searchResults );
			child = tree->GetNextChild( child, cookie );
		}
	}
} }
