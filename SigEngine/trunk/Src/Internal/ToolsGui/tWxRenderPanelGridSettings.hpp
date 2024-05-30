#ifndef __tWxRenderPanelGridSettings__
#define __tWxRenderPanelGridSettings__
#include "tWxSlapOnDialog.hpp"
#include "Gfx/tSolidColorGrid.hpp"

namespace Sig
{
	class tWxSlapOnSpinner;
	class tWxSlapOnCheckBoxDataSync;

	///
	/// \brief Dialog box allowing user to manipulate grid settings. Saves/loads settings to/from registry.
	/// Used in tWxRenderPanel.
	class toolsgui_export tWxRenderPanelGridSettings : public tWxSlapOnDialog
	{
	public:
		std::string						mRegKeyName;
		Gfx::tSolidColorGrid&			mGrid;
		tWxSlapOnCheckBoxDataSync*		mDisplayGrid;
		tWxSlapOnCheckBoxDataSync*		mSnapToGrid;
		tWxSlapOnSpinner*				mGridLineCount;
		tWxSlapOnSpinner*				mMinorGridLines;
		tWxSlapOnSpinner*				mMajorGridLines;
		tWxSlapOnSpinner*				mCenterX;
		tWxSlapOnSpinner*				mCenterY;
		tWxSlapOnSpinner*				mCenterZ;
		Math::tVec3f					mGridXAxis;
		Math::tVec3f					mGridZAxis;
	public:
		tWxRenderPanelGridSettings( wxWindow* parent, const std::string& regKeyName, Gfx::tSolidColorGrid& grid, b32& showGrid, b32& snapToGrid );
		~tWxRenderPanelGridSettings( );
		void fUpdateGrid( );
		void fSnapVertex( Math::tVec3f& vert );
		void fSetGridAxes( const Math::tVec3f& xAxis, const Math::tVec3f& zAxis ) { mGridXAxis = xAxis; mGridZAxis = zAxis; }
	protected:
		virtual std::string fRegistryKeyName( ) const { return mRegKeyName; }
		virtual void fSaveInternal( HKEY hKey );
		virtual void fLoadInternal( HKEY hKey );
	};

}

#endif//__tWxRenderPanelGridSettings__

