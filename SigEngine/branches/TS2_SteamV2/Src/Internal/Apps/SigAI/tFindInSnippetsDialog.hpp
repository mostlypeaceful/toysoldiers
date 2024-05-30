#ifndef __tFindInSnippetsDialog__
#define __tFindInSnippetsDialog__
#include "tFindInDialogBase.hpp"

namespace Sig
{
	class tSigAIMainWindow;

	/// 
	/// \brief
	/// Used to search through all snippets in the goal editor
	class tFindInSnippetsDialog : public tFindInDialogBase
	{
		tSigAIMainWindow* mParent;

	public:
		tFindInSnippetsDialog( 
			tSigAIMainWindow* parent );
	

		virtual void fFindNext( b32 searchUp = false );

	private:
		virtual void fFindInFiles( const wxString& searchText, tGrowableArray< tOccurence >& occurences, b32 findOneOccurencePerFile = false );

		virtual void fOnSearchNextPressed( wxCommandEvent& event );
		virtual void fOnReplacePressed( wxCommandEvent& event );
		virtual void fOnReplaceAllPressed( wxCommandEvent& event );

		const char* fFindStr( const char* searchIn, const char* searchFor );
		virtual void fSelectItem( );

		void fGetNodeAndHandler( const tOccurence& occ, s32& node, s32& handler );
	};
}

#endif//__tFindInSnippetsDialog__
