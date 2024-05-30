#ifndef __tScriptContextActions__
#define __tScriptContextActions__
#include "tEditorContextAction.hpp"
#include "tScriptNotebook.hpp"
#include "tWxTextEditor.hpp"
#include "WxUtil.hpp"
#include "Win32Util.hpp"

namespace Sig
{
	class tInsertSigVarContextAction : public tEditorContextAction
	{
		tScriptNotebook* mNotebook;
		u32 mInsertNumber, mInsertFile, mInsertEnum, mInsertVec;
	public:
		tInsertSigVarContextAction( tScriptNotebook* notebook )
			: mNotebook( notebook )
			, mInsertNumber( fNextUniqueActionId( ) )
			, mInsertFile( fNextUniqueActionId( ) )
			, mInsertEnum( fNextUniqueActionId( ) )
			, mInsertVec( fNextUniqueActionId( ) )
		{
		}

		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( !mNotebook->fGetCurrent( )->fGetEditor( )->fIsScript( ) )
				return false;

			wxMenu* subMenu = new wxMenu;
			subMenu->Append( mInsertNumber, _T("Number") );
			subMenu->Append( mInsertFile, _T("File") );
			subMenu->Append( mInsertEnum, _T("Enum") );
			subMenu->Append( mInsertVec, _T("Vector") );
			menu.AppendSubMenu( subMenu, _T("Insert SigVar") );
			menu.AppendSeparator( );
			return true;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			tWxTextEditor* ed = mNotebook->fGetCurrent( )->fGetEditor( );
			if( !ed->fIsScript( ) )
				return false;

			const s32 currPos = ed->GetCurrentPos( );

			if( actionId == mInsertNumber )
			{
				ed->BeginUndoAction( );
				ed->InsertText( currPos, "@[TODO_NUMBER] { \"TODO_NUMBER\", 0.5, [ 0.0:1.0 ], \"TODO COMMENT\" }\n" );
				ed->EndUndoAction( );
			}
			else if( actionId == mInsertFile )
			{
				ed->BeginUndoAction( );
				ed->InsertText( currPos, "@[TODO_FILE] { \"TODO_FILE\", \"TODO/PATH/FILE_G.png\", [ *.png|*.png| *.tga|*.tga ], \"TODO COMMENT\" }\n" );
				ed->EndUndoAction( );
			}
			else if( actionId == mInsertEnum )
			{
				ed->BeginUndoAction( );
				ed->InsertText( currPos, "@[TODO_ENUM] { \"TODO_ENUM\", TODO_ENUM_VALUE_1, [ TODO_ENUM_VALUE_1, TODO_ENUM_VALUE_2, TODO_ENUM_VALUE_3 ], \"TODO COMMENT\" }\n" );
				ed->EndUndoAction( );
			}
			else if( actionId == mInsertVec )
			{
				ed->BeginUndoAction( );
				ed->InsertText( currPos, "@[TODO_VECTOR] { \"TODO_VECTOR\", (TODO_ELEMENT_X, TODO_ELEMENT_Y, OPTIONAL_ELEMENT_Z, OPTIONAL_ELEMENT_W), [ TODO_MIN:TODO_MAX ], \"TODO COMMENT\" }\n" );
				ed->EndUndoAction( );
			}
			else
				return false;

			const s32 targetPos = ed->PositionFromLine( ed->LineFromPosition( currPos )+1 );
			ed->SetAnchor( targetPos );
			ed->SetCurrentPos( targetPos );

			return true;
		}
	};

	class tInsertSigImportContextAction : public tEditorContextAction
	{
		tScriptNotebook* mNotebook;
		u32 mInsertSigImport;
	public:
		tInsertSigImportContextAction( tScriptNotebook* notebook )
			: mNotebook( notebook )
			, mInsertSigImport( fNextUniqueActionId( ) )
		{
		}

		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( !mNotebook->fGetCurrent( )->fGetEditor( )->fIsScript( ) )
				return false;

			menu.Append( mInsertSigImport, _T("Insert SigImport") );
			menu.AppendSeparator( );
			return true;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			tWxTextEditor* ed = mNotebook->fGetCurrent( )->fGetEditor( );
			if( !ed->fIsScript( ) )
				return false;

			const s32 currPos = ed->GetCurrentPos( );

			if( actionId == mInsertSigImport )
			{
				std::string pathOut;
				if( WxUtil::fBrowseForFile( pathOut, ed, "sigimport", ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ), 0, "" ) )
				{
					std::string resRelativePath = ToolsPaths::fMakeResRelative( tFilePathPtr( pathOut ) ).fCStr( );
					StringUtil::fReplaceAllOf( resRelativePath, "\\", "/" );

					ed->BeginUndoAction( );
					ed->InsertText( currPos, wxString("sigimport \"") + resRelativePath + "\"\n" );
					ed->EndUndoAction( );
				}
			}
			else
				return false;

			const s32 targetPos = ed->PositionFromLine( ed->LineFromPosition( currPos )+1 );
			ed->SetAnchor( targetPos );
			ed->SetCurrentPos( targetPos );

			return true;
		}
	};


	class tInsertSigExportContextAction : public tEditorContextAction
	{
		tScriptNotebook* mNotebook;
		u32 mInsertEntityOnCreate, mInsertEntityOnChildrenCreate, mInsertEntityOnSiblingsCreate;
	public:
		tInsertSigExportContextAction( tScriptNotebook* notebook )
			: mNotebook( notebook )
			, mInsertEntityOnCreate( fNextUniqueActionId( ) )
			, mInsertEntityOnChildrenCreate( fNextUniqueActionId( ) )
			, mInsertEntityOnSiblingsCreate( fNextUniqueActionId( ) )
		{
		}

		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( !mNotebook->fGetCurrent( )->fGetEditor( )->fIsScript( ) )
				return false;

			wxMenu* subMenu = new wxMenu;
			subMenu->Append( mInsertEntityOnCreate, _T("EntityOnCreate") );
			subMenu->Append( mInsertEntityOnChildrenCreate, _T("EntityOnChildrenCreate") );
			subMenu->Append( mInsertEntityOnSiblingsCreate, _T("EntityOnSiblingsCreate") );
			menu.AppendSubMenu( subMenu, _T("Insert SigExport") );
			menu.AppendSeparator( );
			return true;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			tWxTextEditor* ed = mNotebook->fGetCurrent( )->fGetEditor( );
			if( !ed->fIsScript( ) )
				return false;

			const s32 currPos = ed->GetCurrentPos( );

			if( actionId == mInsertEntityOnCreate )
			{
				ed->BeginUndoAction( );
				ed->InsertText( currPos, "sigexport function EntityOnCreate( entity )\n{\n}\n" );
				ed->EndUndoAction( );
			}
			else if( actionId == mInsertEntityOnChildrenCreate )
			{
				ed->BeginUndoAction( );
				ed->InsertText( currPos, "sigexport function EntityOnChildrenCreate( entity )\n{\n}\n" );
				ed->EndUndoAction( );
			}
			else if( actionId == mInsertEntityOnSiblingsCreate )
			{
				ed->BeginUndoAction( );
				ed->InsertText( currPos, "sigexport function EntityOnSiblingsCreate( entity )\n{\n}\n" );
				ed->EndUndoAction( );
			}
			else
				return false;

			const s32 targetPos = ed->PositionFromLine( ed->LineFromPosition( currPos )+1 );
			ed->SetAnchor( targetPos );
			ed->SetCurrentPos( targetPos );

			return true;
		}
	};

	class tInsertLocTextContextAction : public tEditorContextAction
	{
		tScriptNotebook* mNotebook;
		u32 mInsertString, mInsertComment, mInsertAmpersand, mInsertNewLine;
	public:
		tInsertLocTextContextAction( tScriptNotebook* notebook )
			: mNotebook( notebook )
			, mInsertString( fNextUniqueActionId( ) )
			, mInsertComment( fNextUniqueActionId( ) )
			, mInsertAmpersand( fNextUniqueActionId( ) )
			, mInsertNewLine( fNextUniqueActionId( ) )
		{
		}

		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( !mNotebook->fGetCurrent( )->fGetEditor( )->fIsLocml( ) )
				return false;

			wxMenu* subMenu = new wxMenu;
			subMenu->Append( mInsertString, _T("Loc String") );
			subMenu->Append( mInsertComment, _T("Comment") );
			subMenu->Append( mInsertAmpersand, _T("Ampersand") );
			subMenu->Append( mInsertNewLine, _T("New Line") );
			menu.AppendSubMenu( subMenu, _T("Insert Loc Text") );
			menu.AppendSeparator( );
			return true;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			tWxTextEditor* ed = mNotebook->fGetCurrent( )->fGetEditor( );
			if( !ed->fIsLocml( ) )
				return false;

			const s32 currPos = ed->GetCurrentPos( );

			if( actionId == mInsertString )
			{
				ed->BeginUndoAction( );
				ed->InsertText( currPos, "\t<i name=\"TODO_ID_HERE\">\n\t\t<Text>TODO LOC STRING HERE</Text>\n\t</i>\n" );
				ed->EndUndoAction( );
			}
			else if( actionId == mInsertComment )
			{
				ed->BeginUndoAction( );
				ed->InsertText( currPos, "<!--TODO COMMENT HERE-->\n" );
				ed->EndUndoAction( );
			}
			else if( actionId == mInsertAmpersand )
			{
				ed->BeginUndoAction( );
				ed->InsertText( currPos, " &amp; " );
				ed->EndUndoAction( );
			}
			else if( actionId == mInsertNewLine )
			{
				ed->BeginUndoAction( );
				ed->InsertText( currPos, "&#xA;" );
				ed->EndUndoAction( );
			}
			else
				return false;

			const s32 targetPos = ed->PositionFromLine( ed->LineFromPosition( currPos )+1 );
			ed->SetAnchor( targetPos );
			ed->SetCurrentPos( targetPos );

			return true;
		}
	};

	class tOpenInExplorerContextAction : public tEditorContextAction
	{
		tScriptNotebook* mNotebook;
		u32 mOpenInExplorer;
	public:
		tOpenInExplorerContextAction( tScriptNotebook* notebook )
			: mNotebook( notebook )
			, mOpenInExplorer( fNextUniqueActionId( ) )
		{
		}

		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			menu.Append( mOpenInExplorer, _T("Open containing folder") );
			return true;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			Win32Util::fExploreToDirectoryAndSelectFile( mNotebook->fRightClickFileName( ).fCStr( ) );
			return true;
		}
	};

	class tStandardContextActions : public tEditorContextAction
	{
		tScriptNotebook* mNotebook;
		u32 mCut, mCopy, mPaste;
	public:
		tStandardContextActions( tScriptNotebook* notebook )
			: mNotebook( notebook )
			, mCut( fNextUniqueActionId( ) )
			, mCopy( fNextUniqueActionId( ) )
			, mPaste( fNextUniqueActionId( ) )
		{
		}

		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			tWxTextEditor* ed = mNotebook->fGetCurrent( )->fGetEditor( );
			const b32 textSelected = ed->GetCurrentPos( ) != ed->GetAnchor( );

			menu.Append( mCut, _T("Cut") );
			menu.Append( mCopy, _T("Copy") );
			menu.Append( mPaste, _T("Paste") );

			menu.Enable( mCut, textSelected != false );
			menu.Enable( mCopy, textSelected != false );
			menu.Enable( mPaste, ed->CanPaste( ) );

			menu.AppendSeparator( );

			return true;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId == mCut )
			{
				mNotebook->fGetCurrent( )->fGetEditor( )->Cut( );
			}
			else if( actionId == mCopy )
			{
				mNotebook->fGetCurrent( )->fGetEditor( )->Copy( );
			}
			else if( actionId == mPaste )
			{
				mNotebook->fGetCurrent( )->fGetEditor( )->Paste( );
			}
			else
				return false;

			return true;
		}
	};
}

#endif//__tScriptContextActions__
