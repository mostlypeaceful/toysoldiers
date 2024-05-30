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

		tMayaAttributeFloatBox( wxWindow* parent, const char* label, f32 min, f32 max, f32 increment, u32 precision, f32 defValue );

	private:

		virtual void fOnControlUpdated( );
		virtual void fOnMayaSelChanged( );
		b32 fOnMayaSelChangedEachObject( MDagPath& path, MObject& component );
		b32 fSetEachSelectedObject( MDagPath& path, MObject& component );

	private:

		f32 mDefaultValue;
	};

}

#endif//__tMayaAttributeFloatBox__
