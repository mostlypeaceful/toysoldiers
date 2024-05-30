#ifndef __tMayaAttributeMaskBox__
#define __tMayaAttributeMaskBox__
#include "tWxSlapOnMask.hpp"
#include "tMayaGuiBase.hpp"

namespace Sig
{

	class tMayaAttributeMaskBox : 
		public tWxSlapOnMask, 
		public tMayaAttributeControlBase
	{
		tMayaAttributeValueTracker<s32>	mAttributeTracker;
		u32 mDefaultValue;

	public:

		tMayaAttributeMaskBox( wxWindow* parent, const char* label, u32 numBits, u32 defaultVal );

	private:

		virtual void fOnControlUpdated( );
		virtual void fOnMayaSelChanged( );
		b32 fOnMayaSelChangedEachObject( MDagPath& path, MObject& component );
		b32 fSetEachSelectedObject( MDagPath& path, MObject& component );
	};

}

#endif//__tMayaAttributeMaskBox__
