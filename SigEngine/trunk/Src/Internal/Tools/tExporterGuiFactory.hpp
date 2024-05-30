#ifndef __tExporterGuiFactory__
#define __tExporterGuiFactory__
#include "tWxSlapOnDialog.hpp"
#include "tWxSlapOnTabSet.hpp"
#include "tWxSlapOnPanel.hpp"
#include "tWxSlapOnGroup.hpp"
#include "tWxSlapOnCheckBox.hpp"
#include "tWxSlapOnTextBox.hpp"
#include "tWxSlapOnChoice.hpp"
#include "tWxSlapOnSpinner.hpp"
#include "tWxSlapOnQuickExport.hpp"
#include "tWxSlapOnButton.hpp"
#include "tWxSlapOnListBox.hpp"
#include "tWxSlapOnMask.hpp"

namespace Sig
{

	///
	/// \brief Allows creation code for a GUI system to remain identical,
	/// while simply swapping out the factory type will mean different
	/// object types get created. Useful for implementing the same gui
	/// between two different applications, for example.
	class tools_export tExporterGuiFactory
	{
	public:

		virtual ~tExporterGuiFactory( ) { }

		// containers

		virtual tWxSlapOnDialog*		fCreateDialog( const char* title, wxWindow* parent=0 ) = 0;
		virtual tWxSlapOnTabSet*		fCreateTabSet( wxWindow* parent ) = 0;
		virtual tWxSlapOnPanel*			fCreatePanel( wxWindow* parent, const char* title ) = 0;
		virtual tWxSlapOnPanel*			fCreatePanel( wxNotebook* parent, const char* title ) = 0;
		virtual tWxSlapOnGroup*			fCreateGroup( wxWindow* parent, const char* title, b32 collapsible ) = 0;

		// controls

		virtual tWxSlapOnCheckBox*		fCreateCheckBoxAttribute( wxWindow* parent, const char* label ) = 0;
		virtual tWxSlapOnTextBox*		fCreateTextBoxAttribute( wxWindow* parent, const char* label ) = 0;
		virtual tWxSlapOnChoice*		fCreateChoiceAttribute( wxWindow* parent, const char* label, const wxString enumNames[], u32 numEnumNames, u32 defSlot = ~0 ) = 0;
		virtual tWxSlapOnControl*		fCreateMultiChoiceAttribute( wxWindow* parent, const char * label, const wxString enumNames[], u32 numEnumNames, u32 defSlot = ~0 ) = 0;
		virtual tWxSlapOnSpinner*		fCreateSpinnerAttribute( wxWindow* parent, const char* label, f32 min, f32 max, f32 increment, u32 precision, f32 defValue ) = 0;
		virtual tWxSlapOnMask*			fCreateMaskAttribute( wxWindow* parent, const char* label, u32 numBits, u32 defaultValue ) = 0;
		virtual tWxSlapOnQuickExport*	fCreateQuickExport( wxWindow* parent, const char* label ) = 0;
		virtual tWxSlapOnSpinner*		fCreateStatePreview( wxWindow* parent, const char* label ) = 0;
		virtual tWxSlapOnButton*		fCreateTexturePathFixerButton( wxWindow* parent, const char* label ) = 0;
		virtual tWxSlapOnButton*		fCreateCheckOutButton( wxWindow* parent, const char* label ) = 0;
		virtual tWxSlapOnListBox*		fCreateAnimSourceSkeletonsListBox( wxWindow * parent, const char * label) = 0;

	};


}

#endif//__tExporterGuiFactory__
