/////////////////////////////////////////////////////////////////////////////
// Name:        gtk/timer.cpp
// Purpose:     wxTimer implementation
// Author:      Robert Roebling
// Id:          $Id: timer.cpp 37283 2006-02-03 22:22:27Z MR $
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#if wxUSE_TIMER

#include "wx/timer.h"

#include "gtk/gtk.h"

// ----------------------------------------------------------------------------
// wxTimer
// ----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(wxTimer, wxEvtHandler)

extern "C" {
static gint timeout_callback( gpointer data )
{
    wxTimer *timer = (wxTimer*)data;

    // Don't change the order of anything in this callback!

    if (timer->IsOneShot())
    {
        // This sets m_tag to -1
        timer->Stop();
    }

    // When getting called from GDK's timer handler we
    // are no longer within GDK's grab on the GUI
    // thread so we must lock it here ourselves.
    gdk_threads_enter();

    timer->Notify();

    // Release lock again.
    gdk_threads_leave();

    if (timer->IsOneShot())
        return FALSE;

    return TRUE;
}
}

void wxTimer::Init()
{
    m_tag = -1;
    m_milli = 1000;
}

wxTimer::~wxTimer()
{
    wxTimer::Stop();
}

bool wxTimer::Start( int millisecs, bool oneShot )
{
    (void)wxTimerBase::Start(millisecs, oneShot);

    if (m_tag != -1)
        g_source_remove( m_tag );

    m_tag = g_timeout_add( m_milli, timeout_callback, this );

    return TRUE;
}

void wxTimer::Stop()
{
    if (m_tag != -1)
    {
        g_source_remove( m_tag );
        m_tag = -1;
    }
}

#endif // wxUSE_TIMER

