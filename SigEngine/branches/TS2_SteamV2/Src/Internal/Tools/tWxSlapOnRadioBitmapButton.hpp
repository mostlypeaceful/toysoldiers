#ifndef __tWxSlapOnRadioBitmapButton__
#define __tWxSlapOnRadioBitmapButton__
#include "tWxSlapOnGroup.hpp"

namespace Sig
{

	class tWxSlapOnRadioBitmapButton;

	///
	/// \brief Container for a group of radio bitmap buttons. Provides a virtual
	/// method for derived types to receive notification of when the selection
	/// changes (the currently de-pressed/selected bitmap button).
	class tools_export tWxSlapOnRadioBitmapButtonGroup : public tWxSlapOnGroup
	{
		friend class tWxSlapOnRadioBitmapButton;
	protected:
		wxBoxSizer* mMasterContainer;
		wxBoxSizer* mButtonContainer;
		tGrowableArray< tWxSlapOnRadioBitmapButton* > mButtons;
		u32 mMaxButtonsPerRow;
		s32 mSelected;
	public:
		tWxSlapOnRadioBitmapButtonGroup( wxWindow* parent, const char* label, b32 collapsible, u32 maxButtonsPerRow = 6 );
		inline s32 fGetSelected( ) const { return mSelected; }
		void fSetSelected( u32 ithButton, b32 callOnSelChanged = true );
		void fSetSelected( tWxSlapOnRadioBitmapButton* button, b32 callOnSelChanged = true );
		void fClearSelection( s32 ignoreThisIndex = -1 );
		void fDeleteButtons( );

		virtual void fOnSelChanged( ) { }
	};

	///
	/// \brief Represents an individual radio bitmap button (i.e., a button whose
	/// selection state is mutually exclusive with other buttons in the group). Must
	/// be used with tWxSlapOnRadioBitmapButtonGroup.
	class tools_export tWxSlapOnRadioBitmapButton : public wxBitmapButton, public tUncopyable
	{
		friend class tWxSlapOnRadioBitmapButtonGroup;
	protected:
		tWxSlapOnRadioBitmapButtonGroup* mParent;
		wxBitmap mSelected, mDeSelected;
		Math::tVec4f mSolidColor;
	public:
		tWxSlapOnRadioBitmapButton( 
			tWxSlapOnRadioBitmapButtonGroup* parent, 
			const wxBitmap& selected, 
			const wxBitmap& deSelected,
			const char* toolTip=0 );
		u32 fButtonIndex( ) const;

		b32 fIsSelected( ) const { return mParent && mParent->fGetSelected( ) == fButtonIndex( ); }

		wxBitmap& fGetSelectedBitmap( ) { return mSelected; }
		wxBitmap& fGetDeSelectedBitmap( ) { return  mDeSelected; }

		void fSetColor( const Math::tVec4f& newColor )
		{
			mSolidColor = newColor;
			fBuildSolidColor( );
		}
		const Math::tVec4f& fColor( ) const { return mSolidColor; }

	protected:
		virtual void fOnSelected( ) { }
		virtual void fOnDeselected( ) { }
		virtual b32 fReselectable( ) { return false; }
		void fUpdateAllBitmaps( wxBitmap& bmp );
	private:
		void fOnPressed( wxCommandEvent& event );
		void fOnHover( wxMouseEvent& event );

		void fBuildSolidColor( );
	};

}

#endif//__tWxSlapOnRadioBitmapButton__

