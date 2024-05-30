#ifndef __tMayaAttributeTextBox__
#define __tMayaAttributeTextBox__
#include "tWxSlapOnTextBox.hpp"
#include "tMayaGuiBase.hpp"

namespace Sig
{

	class tMayaAttributeTextBox : 
		public tWxSlapOnTextBox,
		public tMayaAttributeControlBase
	{
		tMayaAttributeValueTracker<std::string>	mAttributeTracker;

	public:

		tMayaAttributeTextBox( wxWindow* parent, const char* label );
		virtual void fOnControlUpdated( );

	private:

		virtual void fOnMayaSelChanged( );
		b32 fOnMayaSelChangedEachObject( MDagPath& path, MObject& component );
		b32 fSetEachSelectedObject( MDagPath& path, MObject& component );
	};

}

#endif//__tMayaAttributeTextBox__
