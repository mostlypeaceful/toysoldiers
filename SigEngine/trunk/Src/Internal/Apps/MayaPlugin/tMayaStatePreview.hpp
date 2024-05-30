#ifndef __tMayaStatePreview__
#define __tMayaStatePreview__
#include "tWxSlapOnSpinner.hpp"
#include "tMayaEvent.hpp"

namespace Sig
{
	class tMayaStatePreview : public tWxSlapOnSpinner
	{
		MSelectionList mShowList;
		MSelectionList mHideList;
	public:
		tMayaStatePreview( wxWindow* parent, const char* label );
	private:
		virtual void fOnControlUpdated( );
		b32 fVisitNode( MDagPath& path, MObject& component );

		void fShowAll( wxCommandEvent& );
	};
}

#endif//__tMayaStatePreview__
