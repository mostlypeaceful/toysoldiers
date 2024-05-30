//------------------------------------------------------------------------------
// \file tFuiLoadCanvas.cpp - 15 Mar 2013
// \author jwittner
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tFuiLoadCanvas.hpp"

namespace Sig { namespace Gui
{
	void tFuiLoadCanvas::fSignalLoadComplete( ) const
	{
		tFuiFunctionCall functionCall( "OnLoadComplete" );
		fMovie( )->fCallFunction( functionCall );
	}

}} // ::Sig::Gui

namespace Sig { namespace Gui
{
	void tFuiLoadCanvas::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tFuiLoadCanvas, tFuiCanvas, Sqrat::NoCopy<tFuiLoadCanvas> > classDesc( vm.fSq( ) );
		classDesc
			.Func( "SignalLoadComplete", &tFuiLoadCanvas::fSignalLoadComplete )
			;
		vm.fNamespace( _SC("Gui") ).Bind( _SC("FuiLoadCanvas"), classDesc );
	}

}} // ::Sig::Gui
