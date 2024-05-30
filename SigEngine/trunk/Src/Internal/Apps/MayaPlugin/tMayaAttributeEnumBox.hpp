#ifndef __tMayaAttributeEnumBox__
#define __tMayaAttributeEnumBox__
#include "tWxSlapOnChoice.hpp"
#include "tMayaGuiBase.hpp"

namespace Sig
{
	class tMayaAttributeEnumBox : 
		public tWxSlapOnChoice, 
		public tMayaAttributeControlBase
	{
		tGrowableArray<std::string>		mEnumNames;
		tMayaAttributeValueTracker<u32> mAttributeTracker;
		u32 mDefSlot;

	public:

		tMayaAttributeEnumBox( wxWindow* parent, const char* label, const wxString enumNames[], u32 numEnumNames, u32 defSlot = ~0 );

	private:

		virtual void	fOnControlUpdated( );
		virtual void	fOnMayaSelChanged( );
		b32				fOnMayaSelChangedEachObject( MDagPath& path, MObject& component );
		b32				fSetEachSelectedObject( MDagPath& path, MObject& component );

	};

}

#endif//__tMayaAttributeEnumBox__
