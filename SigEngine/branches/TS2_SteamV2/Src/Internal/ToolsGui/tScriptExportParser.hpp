#ifndef __tScriptExportParser__
#define __tScriptExportParser__

#include "wx/regex.h"
#include "wx/treectrl.h"
#include "Scripts/tScriptExportData.hpp"

namespace Sig { namespace ScriptData
{

	struct toolsgui_export tMemberDescParser
	{
		static const char* cSearch;

		static b32      fApply( tMemberDesc& desc, wxRegEx& regEx, const wxString& str );
		static void     fFillTree( tMemberDesc& desc, wxTreeCtrl* tree, wxTreeItemId parent );
		static b32		fFindParameters( tMemberDesc& desc, wxRegEx& getRegex, wxRegEx& setRegex, wxRegEx& argRegex, const wxString& line, const std::string& tType );
		static wxString fMakeHppSearchRegex( const wxString& member, u32 type );
	};

	struct toolsgui_export tBindingDescParser
	{
		static const char* cSearch;

		static b32		fApply( tBindingDesc& desc, wxRegEx& regEx, const wxString& str );
		static void		fFillTree( tBindingDesc& desc, wxTreeCtrl* tree, wxTreeItemId parent );
	};

	struct toolsgui_export tClassDescParser
	{
		static const char* cSearch;

		static b32		fApply( tClassDesc& desc, wxRegEx& regEx, const wxString& str );
		static void		fFillTree( tClassDesc& desc, wxTreeCtrl* tree, wxTreeItemId parent );
		static void		fFindMemberParameters( tClassDesc& desc );
	};

	struct toolsgui_export tNamespaceDescParser
	{
		static void		fFillTree( tNamespaceDesc& desc, wxTreeCtrl* tree, wxTreeItemId parent );
	};

	struct toolsgui_export tScriptExportDataParser
	{
		static void fBuild( tScriptExportData& data, wxDialog* progress = NULL ); //window label updated to show progress
		static void fFillTree( tScriptExportData& data, wxTreeCtrl* tree );

		static void fFindInTree( const char* text, wxTreeCtrl* tree, wxTreeItemId rootNode, tGrowableArray<wxTreeItemId>& searchResults );

	private:
		static void fAddClass( tScriptExportData& data, const tClassDesc& c, const tFilePathPtr& srcFile );
		static void fAddDefaultData( tScriptExportData& data );
	};

	struct toolsgui_export tTreeData : public wxTreeItemData
	{
		enum tType { cUser, cRoot, cNamespaces, cConsts };
		enum tDataType { tParameterDesc, tMemberDesc, tBindingDesc, tClassDesc, tNamespaceDesc };

		tBaseData*	mData;
		tType		mType;
		tDataType	mDataType;


		//tTreeData( tBaseData* data = NULL ) 
		//	: mData( data ), mType( cUser )
		//{ }

		tTreeData( tBaseData* data, tDataType dataType ) 
			: mData( data ), mType( cUser ), mDataType( dataType )
		{ }

		tTreeData( tType type ) 
			: mData( NULL ), mType( type )
		{ }
	};

} }

#endif//____tScriptExportParser____
