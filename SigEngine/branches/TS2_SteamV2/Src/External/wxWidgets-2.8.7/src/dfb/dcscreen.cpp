/////////////////////////////////////////////////////////////////////////////
// Name:        src/dfb/dcscreen.cpp
// Purpose:     wxScreenDC implementation
// Author:      Vaclav Slavik
// Created:     2006-08-16
// RCS-ID:      $Id: dcscreen.cpp 43489 2006-11-18 13:17:35Z VS $
// Copyright:   (c) 2006 REA Elektronik GmbH
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ===========================================================================
// declarations
// ===========================================================================

// ---------------------------------------------------------------------------
// headers
// ---------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include "wx/dcscreen.h"

#include "wx/dfb/private.h"

// ===========================================================================
// implementation
// ===========================================================================

//-----------------------------------------------------------------------------
// wxScreenDC
//-----------------------------------------------------------------------------

#warning "FIXME: this doesn't work (neither single app nor multiapp core)
// FIXME: maybe use a subsurface as well?

IMPLEMENT_DYNAMIC_CLASS(wxScreenDC, wxDC)

wxScreenDC::wxScreenDC()
{
    DFBInit(wxIDirectFB::Get()->GetPrimarySurface());
}

#warning "FIXME: does wxScreenDC need Flip call in dtor?"
