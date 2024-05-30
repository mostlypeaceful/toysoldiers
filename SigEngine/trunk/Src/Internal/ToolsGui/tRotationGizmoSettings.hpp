#ifndef __tRotationGizmoSettings__
#define __tRotationGizmoSettings__
#include "tWxSlapOnDialog.hpp"
#include "tWxSlapOnSpinner.hpp"

namespace Sig
{
	class tWxSlapOnSpinner;
	class tWxSlapOnCheckBoxDataSync;

	///
	/// \brief Dialog box allowing user to manipulate rotation gizmo settings. Saves/loads settings to/from registry.
	/// Used in tWxRenderPanel.
	class toolsgui_export tRotationGizmoSettings : public tWxSlapOnDialog
	{
	public:
		std::string						mRegKeyName;
		tWxSlapOnSpinner *				mAngleSnap;
	public:
		tRotationGizmoSettings( wxWindow* parent, const std::string& regKeyName );
		~tRotationGizmoSettings( );

		u32 fValue( ) { return ( u32 )mAngleSnap->fGetValue( ); }
	protected:
		virtual std::string fRegistryKeyName( ) const { return mRegKeyName; }
		virtual void fSaveInternal( HKEY hKey );
		virtual void fLoadInternal( HKEY hKey );
	};

}

#endif//__tRotationGizmoSettings__

