/////////////////////////////////////////////////////////////////////////////
// Name:        src/gtk/bmpbuttn.cpp
// Purpose:
// Author:      Robert Roebling
// Id:          $Id: bmpbuttn.cpp 46616 2007-06-22 12:25:11Z RR $
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#if wxUSE_BMPBUTTON

#include "wx/bmpbuttn.h"

#include "wx/gtk/private.h"

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class wxBitmapButton;

//-----------------------------------------------------------------------------
// data
//-----------------------------------------------------------------------------

extern bool   g_blockEventsOnDrag;

//-----------------------------------------------------------------------------
// "clicked"
//-----------------------------------------------------------------------------

extern "C" {
static void gtk_bmpbutton_clicked_callback( GtkWidget *WXUNUSED(widget), wxBitmapButton *button )
{
    if (g_isIdle)
        wxapp_install_idle_handler();

    if (!button->m_hasVMT) return;
    if (g_blockEventsOnDrag) return;

    wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, button->GetId());
    event.SetEventObject(button);
    button->GetEventHandler()->ProcessEvent(event);
}
}

//-----------------------------------------------------------------------------
// "enter"
//-----------------------------------------------------------------------------

extern "C" {
static void gtk_bmpbutton_enter_callback( GtkWidget *WXUNUSED(widget), wxBitmapButton *button )
{
    if (!button->m_hasVMT) return;
    if (g_blockEventsOnDrag) return;

    button->HasFocus();
}
}

//-----------------------------------------------------------------------------
// "leave"
//-----------------------------------------------------------------------------

extern "C" {
static void gtk_bmpbutton_leave_callback( GtkWidget *WXUNUSED(widget), wxBitmapButton *button )
{
    if (!button->m_hasVMT) return;
    if (g_blockEventsOnDrag) return;

    button->NotFocus();
}
}

//-----------------------------------------------------------------------------
// "pressed"
//-----------------------------------------------------------------------------

extern "C" {
static void gtk_bmpbutton_press_callback( GtkWidget *WXUNUSED(widget), wxBitmapButton *button )
{
    if (!button->m_hasVMT) return;
    if (g_blockEventsOnDrag) return;

    button->StartSelect();
}
}

//-----------------------------------------------------------------------------
// "released"
//-----------------------------------------------------------------------------

extern "C" {
static void gtk_bmpbutton_release_callback( GtkWidget *WXUNUSED(widget), wxBitmapButton *button )
{
    if (!button->m_hasVMT) return;
    if (g_blockEventsOnDrag) return;

    button->EndSelect();
}
}

//-----------------------------------------------------------------------------
// wxBitmapButton
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxBitmapButton,wxButton)

void wxBitmapButton::Init()
{
    m_hasFocus =
    m_isSelected = false;
}

bool wxBitmapButton::Create( wxWindow *parent,
                             wxWindowID id,
                             const wxBitmap& bitmap,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style,
                             const wxValidator& validator,
                             const wxString &name )
{
    m_needParent = true;
    m_acceptsFocus = true;

    if (!PreCreation( parent, pos, size ) ||
        !CreateBase( parent, id, pos, size, style, validator, name ))
    {
        wxFAIL_MSG( wxT("wxBitmapButton creation failed") );
        return false;
    }

    m_bmpNormal = bitmap;

    m_widget = gtk_button_new();

    if (style & wxNO_BORDER)
       gtk_button_set_relief( GTK_BUTTON(m_widget), GTK_RELIEF_NONE );

    if (m_bmpNormal.Ok())
    {
        OnSetBitmap();
    }

    g_signal_connect_after (m_widget, "clicked",
                            G_CALLBACK (gtk_bmpbutton_clicked_callback),
                            this);

    g_signal_connect (m_widget, "enter",
                      G_CALLBACK (gtk_bmpbutton_enter_callback), this);
    g_signal_connect (m_widget, "leave",
                      G_CALLBACK (gtk_bmpbutton_leave_callback), this);
    g_signal_connect (m_widget, "pressed",
                      G_CALLBACK (gtk_bmpbutton_press_callback), this);
    g_signal_connect (m_widget, "released",
                      G_CALLBACK (gtk_bmpbutton_release_callback), this);

    m_parent->DoAddChild( this );

    PostCreation(size);

    return true;
}

void wxBitmapButton::SetDefault()
{
    GTK_WIDGET_SET_FLAGS( m_widget, GTK_CAN_DEFAULT );
    gtk_widget_grab_default( m_widget );

    SetSize( m_x, m_y, m_width, m_height );
}

void wxBitmapButton::SetLabel( const wxString &label )
{
    wxCHECK_RET( m_widget != NULL, wxT("invalid button") );

    wxControl::SetLabel( label );
}

void wxBitmapButton::DoApplyWidgetStyle(GtkRcStyle *style)
{
    if (!GTK_BIN(m_widget)->child)
        return;

    wxButton::DoApplyWidgetStyle(style);
}

void wxBitmapButton::OnSetBitmap()
{
    wxCHECK_RET( m_widget != NULL, wxT("invalid bitmap button") );

    InvalidateBestSize();

    wxBitmap the_one;
    if (!m_isEnabled)
        the_one = m_bmpDisabled;
    else if (m_isSelected)
        the_one = m_bmpSelected;
    else if (m_hasFocus)
        the_one = m_bmpFocus;
    else
        the_one = m_bmpNormal;

    if (!the_one.Ok()) the_one = m_bmpNormal;
    if (!the_one.Ok()) return;

    GtkWidget *child = GTK_BIN(m_widget)->child;
    if (child == NULL)
    {
        // initial bitmap
        GtkWidget *pixmap = 
            gtk_image_new_from_pixbuf(the_one.GetPixbuf());

        gtk_widget_show(pixmap);
        gtk_container_add(GTK_CONTAINER(m_widget), pixmap);
    }
    else
    {   // subsequent bitmaps
        GtkImage *pixmap = GTK_IMAGE(child);
        gtk_image_set_from_pixbuf(pixmap, the_one.GetPixbuf());
    }
}

wxSize wxBitmapButton::DoGetBestSize() const
{
    return wxControl::DoGetBestSize();
}

bool wxBitmapButton::Enable( bool enable )
{
    if ( !wxWindow::Enable(enable) )
        return false;

    OnSetBitmap();

    return true;
}

void wxBitmapButton::HasFocus()
{
    m_hasFocus = true;
    OnSetBitmap();
}

void wxBitmapButton::NotFocus()
{
    m_hasFocus = false;
    OnSetBitmap();
}

void wxBitmapButton::StartSelect()
{
    m_isSelected = true;
    OnSetBitmap();
}

void wxBitmapButton::EndSelect()
{
    m_isSelected = false;
    OnSetBitmap();
}

#endif // wxUSE_BMPBUTTON
