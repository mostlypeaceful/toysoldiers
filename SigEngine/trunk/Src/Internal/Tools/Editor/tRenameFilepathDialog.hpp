#ifndef __tRenameFilepathDialog__
#define __tRenameFilepathDialog__

#include "wx/progdlg.h"

namespace Sig
{


	class tools_export tRenameFilepathDialog : public wxDialog
	{
	public:
		tRenameFilepathDialog( wxWindow* parent, const tFilePathPtr& originalPath, b32 onlySameType );

		struct tEntry : public tRefCounter
		{
			u32 mOccurances;

			tEntry( ) : mOccurances( 0 ) { }

			virtual ~tEntry( ) { }	
			virtual void fReplace( const tFilePathPtr& replace, const tFilePathPtr& with ) = 0;
			b32 fValid( ) const { return mOccurances > 0; }
		};	
		typedef tRefCounterPtr< tEntry > tEntryPtr;
	
	private:
		void fOnAccept( wxCommandEvent& event );

		tFilePathPtr mOriginalPath;
		wxCheckListBox* mListBox;
		tGrowableArray< tEntryPtr > mEntries;
		wxProgressDialog* mProgress;
		wxTextCtrl* mText;
		b32 mOnlySameType;

		b32 fPopulateListBox( );
	};

}

#endif//__tRenameFilepathDialog__
