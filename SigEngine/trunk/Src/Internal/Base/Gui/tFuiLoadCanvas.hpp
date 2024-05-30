//------------------------------------------------------------------------------
// \file tFuiLoadCanvas.hpp - 15 Mar 2013
// \author jwittner
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tFuiLoadCanvas__
#define __tFuiLoadCanvas__

#include "tFuiCanvas.hpp"

namespace Sig { namespace Gui
{
	///
	/// \class tFuiLoadCanvas
	/// \brief 
	class base_export tFuiLoadCanvas : public tFuiCanvas
	{
		define_dynamic_cast( tFuiLoadCanvas, tFuiCanvas );
	public:

		void fSignalLoadComplete( ) const; // Game is ready to drop the screen

	public:
		
		static void fExportScriptInterface( tScriptVm& vm );
	};

	typedef tRefCounterPtr<tFuiLoadCanvas> tFuiLoadCanvasPtr;

}} // ::Sig::Gui

#endif//__tFuiLoadCanvas__
