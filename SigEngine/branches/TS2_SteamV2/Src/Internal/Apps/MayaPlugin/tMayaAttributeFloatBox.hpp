#ifndef __tMayaAttributeFloatBox__
#define __tMayaAttributeFloatBox__
#include "tWxSlapOnSpinner.hpp"
#include "tMayaGuiBase.hpp"

namespace Sig
{

	class tMayaAttributeFloatBox : 
		public tWxSlapOnSpinner, 
		public tMayaAttributeControlBase
	{
		tMayaAttributeValueTracker<f32>	mAttributeTracker;

	public:

		tMayaAttributeFloatBox( wxWindow* parent, const char* label, f32 min, f32 max, f32 increment, u32 precision );

	private:

		virtual void fOnControlUpdated( );
		virtual void fOnMayaSelChanged( );
		b32 fOnMayaSelChangedEachObject( MDagPath& path, MObject& component );
		b32 fSetEachSelectedObject( MDagPath& path, MObject& component );
	};

}

#endif//__tMayaAttributeFloatBox__
