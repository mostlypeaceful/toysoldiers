#ifndef __tLocmlControlPanel__
#define __tLocmlControlPanel__

#include "tLocmlConvertDictionary.hpp"

namespace Sig
{
	class tScriptNotebook;

	class tLocmlControlPanel : public wxScrolledWindow
	{
	public:
		tLocmlControlPanel( wxWindow* parent, tScriptNotebook* notebook );
		~tLocmlControlPanel();

	private:

		class tLocmlTargetUI : wxEvtHandler
		{
		public:
			tLocmlTargetUI( wxWindow* parent, wxString lang, tFilePathPtr defaultPath );

			void			fSetSource( );

			b32				fActive() const;
			tFilePathPtr	fGetFullPath() const;
			std::string		fGetLang() const { return std::string( mLang.c_str() ); }

		private:
			wxWindow* mParent;
			wxCheckBox* mActive;
			wxTextCtrl* mFilePath;
			wxString mLang;

			tLocmlTargetUI() { }

			void fOnBrowse( wxCommandEvent& event );
		};

		tGrowableArray< tLocmlTargetUI* >	mLangTargets;
		tLocmlConvertDictionary				mDict;
		tScriptNotebook*					mNotebook;

		void fBuildDict( );

		// wxEvents
		void fOnPush( wxCommandEvent& event );
		void fOnFindErrors( wxCommandEvent& event );
		void fOnRebuild( wxCommandEvent& event );
	};
}

#endif // __tLocmlControlPanel__
