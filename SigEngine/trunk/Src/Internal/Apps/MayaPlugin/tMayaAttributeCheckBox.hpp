#ifndef __tMayaAttributeCheckBox__
#define __tMayaAttributeCheckBox__
#include "tWxSlapOnCheckBox.hpp"
#include "tMayaGuiBase.hpp"

namespace Sig
{

	class tMayaAttributeCheckBox : 
		public tWxSlapOnCheckBox,
		public tMayaAttributeControlBase
	{
		tMayaAttributeValueTracker<b32>	mAttributeTracker;

	public:

		tMayaAttributeCheckBox( wxWindow* parent, const char* label );
		virtual void fOnControlUpdated( );

	private:

		virtual void fOnMayaSelChanged( );
		b32 fOnMayaSelChangedEachObject( MDagPath& path, MObject& component );
		b32 fSetEachSelectedObject( MDagPath& path, MObject& component );
	};

}

#endif//__tMayaAttributeCheckBox__
