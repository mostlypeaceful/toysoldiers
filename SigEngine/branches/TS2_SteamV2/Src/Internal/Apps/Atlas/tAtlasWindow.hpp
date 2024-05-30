#ifndef __tAtlasWindow__
#define __tAtlasWindow__
#include "tWxSlapOnSpinner.hpp"
#include "Tatml.hpp"

namespace Sig
{


	class tAtlasWindow : public wxFrame
	{
		class tSpinner : public tWxSlapOnSpinner
		{
			tAtlasWindow* mOwner;
		public:
			tSpinner( tAtlasWindow* owner, wxWindow* parent, const std::string& label, f32 min, f32 max, f32 increment, u32 precision ) 
				: tWxSlapOnSpinner( parent, label.c_str( ), min, max, increment, precision ), mOwner( owner ) { }
			virtual void fOnControlUpdated( ) { mOwner->fMarkDirty( ); }
		};

		wxListBox*		mListBox;
		wxMenu*			mEditMenu;
		tSpinner*		mSubWidth;
		tSpinner*		mSubHeight;
		b32				mDirty;
		tFilePathPtr	mCurrentDoc;
	public:

		tAtlasWindow(const wxString& title);
		~tAtlasWindow( );
		void fMarkDirty( b32 dirty = true );
		void fOnAction(wxCommandEvent& event);
		void fOnClose(wxCloseEvent& event);
		void fOnAddTexPressed(wxCommandEvent& event);
		void fOnSubTexPressed(wxCommandEvent& event);
		void fOnListBoxDClick(wxMouseEvent& event);
		void fOnListBoxRUp(wxMouseEvent& event);
		void fOnSelChanged(wxCommandEvent& event);

	private:

		b32 fPathExistsAlready( const tFilePathPtr& path ) const;
		tFilePathPtr fGetPathAt( int index ) const;
		void fUpdateTitle( );
		b32 fClearDoc( );
		b32 fSaveDoc( b32 saveAs );
		void fBuildDoc( );
		void fOpenDoc( );
		void fOpenDoc( const tFilePathPtr& file );
		void fSerialize( const tFilePathPtr& file );
		b32  fDeserialize( const tFilePathPtr& file );
		void fMoveSelection( s32 dir );
		std::string fFigureSemanticAndFormat( Gfx::tTextureFile::tSemantic& semantic, Gfx::tTextureFile::tFormat& format );

	private:
		DECLARE_EVENT_TABLE()
	};

}

#endif//__tAtlasWindow__

