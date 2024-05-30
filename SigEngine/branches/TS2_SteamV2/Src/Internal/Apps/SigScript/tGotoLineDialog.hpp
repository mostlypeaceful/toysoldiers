//------------------------------------------------------------------------------
// \file tGotoLineDialog.hpp - 12 Aug 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tGotoLineDialog__
#define __tGotoLineDialog__

namespace Sig
{
	///
	/// \class tGotoLineDialog
	/// \brief A simple dialog that records and returns a line number.
	class tGotoLineDialog : public wxDialog
	{
		wxTextCtrl* mLineNumber;
		s32 mMax;

	public:
		tGotoLineDialog( wxWindow* parent, s32 current, s32 max );

		s32 fGetLineNumber( ) const;
	};
}

#endif//__tGotoLineDialog__
