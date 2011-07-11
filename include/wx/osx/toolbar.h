/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/toolbar.h
// Purpose:     wxToolBar class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id$
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TOOLBAR_H_
#define _WX_TOOLBAR_H_

#if wxUSE_TOOLBAR

#include "wx/tbarbase.h"
#include "wx/dynarray.h"

#ifdef wxOSX_USE_IPHONE

// wxToolBar iOS styles
enum {
    wxTB_NORMAL_BG =            0x00000000,
    wxTB_BLACK_OPAQUE_BG =      0x00010000,
    wxTB_BLACK_TRANSLUCENT_BG = 0x00020000
};

// Bar button item styles
enum {    
    wxID_DONE = wxID_OK,
    wxID_SEARCH = wxID_FIND,
    
    wxID_FLEXIBLESPACE = wxID_HIGHEST+1,
    wxID_FIXEDSPACE,
    wxID_BACK,
    wxID_COMPOSE,
    wxID_REPLY,
    wxID_ACTION,
    wxID_ORGANIZE,
    wxID_BOOKMARKS,
    wxID_CAMERA,
    wxID_TRASH,
    wxID_PLAY,
    wxID_PAUSE,
    wxID_REWIND,
    wxID_FASTFORWARD,
    wxID_PAGECURL,
    
    /*
     Recognised as bar button ids but already defined by wxWidgets

     wxID_CANCEL
     wxID_SAVE
     wxID_EDIT
     wxID_ADD
     wxID_REFRESH
     wxID_STOP
     */
};

#endif  // wxOSX_USE_IPHONE


class WXDLLIMPEXP_CORE wxToolBar: public wxToolBarBase
{
  DECLARE_DYNAMIC_CLASS(wxToolBar)
 public:
  /*
   * Public interface
   */

   wxToolBar() { Init(); }

  inline wxToolBar(wxWindow *parent, wxWindowID id,
                   const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
                   long style = wxNO_BORDER|wxTB_HORIZONTAL,
                   const wxString& name = wxToolBarNameStr)
  {
    Init();
    Create(parent, id, pos, size, style, name);
  }
  virtual ~wxToolBar();

  bool Create(wxWindow *parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
            long style = wxNO_BORDER|wxTB_HORIZONTAL,
            const wxString& name = wxToolBarNameStr);

    virtual void SetWindowStyleFlag(long style);

    virtual bool Destroy();

    // override/implement base class virtuals
    virtual wxToolBarToolBase *FindToolForPosition(wxCoord x, wxCoord y) const;

#ifndef __WXOSX_IPHONE__
    virtual bool Show(bool show = true);
    virtual bool IsShown() const;
#endif
    virtual bool Realize();

    virtual void SetToolBitmapSize(const wxSize& size);
    virtual wxSize GetToolSize() const;

    virtual void SetRows(int nRows);

    virtual void SetToolNormalBitmap(int id, const wxBitmap& bitmap);
    virtual void SetToolDisabledBitmap(int id, const wxBitmap& bitmap);

#ifndef __WXOSX_IPHONE__
    // Add all the buttons

    virtual wxString MacGetToolTipString( wxPoint &where ) ;
    void OnPaint(wxPaintEvent& event) ;
    void OnMouse(wxMouseEvent& event) ;
    virtual void MacSuperChangedPosition() ;
#endif

#if wxOSX_USE_NATIVE_TOOLBAR
    bool MacInstallNativeToolbar(bool usesNative);
    void MacUninstallNativeToolbar();
    bool MacWantsNativeToolbar();
    bool MacTopLevelHasNativeToolbar(bool *ownToolbarInstalled) const;
#endif
protected:
    // common part of all ctors
    void Init();
    
    void DoLayout();
    
    void DoSetSize(int x, int y, int width, int height, int sizeFlags);

#ifndef __WXOSX_IPHONE__
   virtual void DoGetSize(int *width, int *height) const;
    virtual wxSize DoGetBestSize() const;
#endif
    virtual bool DoInsertTool(size_t pos, wxToolBarToolBase *tool);
    virtual bool DoDeleteTool(size_t pos, wxToolBarToolBase *tool);

    virtual void DoEnableTool(wxToolBarToolBase *tool, bool enable);
    virtual void DoToggleTool(wxToolBarToolBase *tool, bool toggle);
    virtual void DoSetToggle(wxToolBarToolBase *tool, bool toggle);

    virtual wxToolBarToolBase *CreateTool(int id,
                                          const wxString& label,
                                          const wxBitmap& bmpNormal,
                                          const wxBitmap& bmpDisabled,
                                          wxItemKind kind,
                                          wxObject *clientData,
                                          const wxString& shortHelp,
                                          const wxString& longHelp);
    virtual wxToolBarToolBase *CreateTool(wxControl *control,
                                          const wxString& label);

    DECLARE_EVENT_TABLE()
#if wxOSX_USE_NATIVE_TOOLBAR
    bool m_macUsesNativeToolbar ;
    void* m_macToolbar ;
#endif
#ifdef __WXOSX_IPHONE__
    WX_UIView m_macToolbar;
#endif
};

#endif // wxUSE_TOOLBAR

#endif
    // _WX_TOOLBAR_H_