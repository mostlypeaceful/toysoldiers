#ifndef __tExplicitDependenciesDialog__
#define __tExplicitDependenciesDialog__

namespace Sig
{

	class toolsgui_export tExplicitDependenciesDialog : public wxWindow
	{
		wxFrame*				mMyWindow;
		wxScrolledWindow*		mMainPanel;
		wxListBox*				mListBox;
		tFilePathPtrList		mDependencyList;

	protected:
		virtual void fOnChanged( ) { } //override me

	public:
		tExplicitDependenciesDialog( wxFrame* myWindow );

		const tFilePathPtrList& fDependencyList( ) const { return mDependencyList; }
		void fSetDependencyList( const tFilePathPtrList& dl ) { mDependencyList = dl; fRefreshList( ); }

	private:
		void fBuildGui( );
		tFilePathPtr fGetSelectedPath( ) const;
		void fOnAddPressed( wxCommandEvent& event );
		void fOnSubPressed( wxCommandEvent& event );
		void fOnListBoxDClick( wxMouseEvent& event );
		void fAdd( const tFilePathPtr& path );
		void fRemove( const tFilePathPtr& path );
		void fRefreshList( );
	};

}

#endif//__tExplicitDependenciesDialog__
