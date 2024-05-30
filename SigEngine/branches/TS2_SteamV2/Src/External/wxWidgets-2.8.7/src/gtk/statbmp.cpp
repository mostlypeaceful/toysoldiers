/////////////////////////////////////////////////////////////////////////////
// Name:        statbmp.cpp
// Purpose:
// Author:      Robert Roebling
// Id:          $Id: statbmp.cpp 37056 2006-01-22 20:29:17Z MR $
// Copyright:   (c) 1998 Robert Roebling
// Licence:           wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#if wxUSE_STATBMP

#include "wx/statbmp.h"

#include "gdk/gdk.h"
#include "gtk/gtk.h"

//-----------------------------------------------------------------------------
// wxStaticBitmap
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxStaticBitmap,wxControl)

wxStaticBitmap::wxStaticBitmap(void)
{
}

wxStaticBitmap::wxStaticBitmap( wxWindow *parent, wxWindowID id, const wxBitmap &bitmap,
      const wxPoint &pos, const wxSize &size,
      long style, const wxString &name )
{
    Create( parent, id, bitmap, pos, size, style, name );
}

bool wxStaticBitmap::Create( wxWindow *parent, wxWindowID id, const wxBitmap &bitmap,
                             const wxPoint &pos, const wxSize &size,
                             long style, const wxString &name )
{
    m_needParent = TRUE;

    if (!PreCreation( parent, pos, size ) ||
        !CreateBase( parent, id, pos, size, style, wxDefaultValidator, name ))
    {
        wxFAIL_MSG( wxT("wxStaticBitmap creation failed") );
        return false;
    }

    m_bitmap = bitmap;

    m_widget = gtk_image_new();

    if (bitmap.Ok())
        SetBitmap(bitmap);

    PostCreation(size);
    m_parent->DoAddChild( this );

    return true;
}

void wxStaticBitmap::SetBitmap( const wxBitmap &bitmap )
{
    m_bitmap = bitmap;

    if (m_bitmap.Ok())
    {
        GdkBitmap *mask = (GdkBitmap *) NULL;
        if (m_bitmap.GetMask())
            mask = m_bitmap.GetMask()->GetBitmap();

        if (m_bitmap.HasPixbuf())
        {
            gtk_image_set_from_pixbuf(GTK_IMAGE(m_widget),
                                      m_bitmap.GetPixbuf());
        }
        else
            gtk_image_set_from_pixmap(GTK_IMAGE(m_widget),
                                      m_bitmap.GetPixmap(), mask);

        InvalidateBestSize();
        SetSize(GetBestSize());
    }
}

// static
wxVisualAttributes
wxStaticBitmap::GetClassDefaultAttributes(wxWindowVariant WXUNUSED(variant))
{
    // TODO: overload to allow using gtk_pixmap_new?
    return GetDefaultAttributesFromGTKWidget(gtk_label_new);
}

#endif // wxUSE_STATBMP

