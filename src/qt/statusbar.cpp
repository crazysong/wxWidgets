/////////////////////////////////////////////////////////////////////////////
// Name:        src/qt/statusbar.cpp
// Author:      Peter Most, Javier Torres, Mariano Reingart, Sean D'Epagnier
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include "wx/statusbr.h"
#include "wx/qt/private/utils.h"
#include "wx/qt/private/converter.h"
#include "wx/qt/private/winevent.h"


class wxQtStatusBar : public wxQtEventSignalHandler< QStatusBar, wxStatusBar >
{

public:
    wxQtStatusBar( wxWindow *parent, wxStatusBar *handler );
};

wxQtStatusBar::wxQtStatusBar( wxWindow *parent, wxStatusBar *handler )
    : wxQtEventSignalHandler< QStatusBar, wxStatusBar >( parent, handler )
{
}

//==============================================================================

wxStatusBar::wxStatusBar(wxWindow *parent, wxWindowID winid,
            long style,
            const wxString& name)
{
    Create( parent, winid, style, name );
}

bool wxStatusBar::Create(wxWindow *parent, wxWindowID WXUNUSED(winid),
                         long style, const wxString& WXUNUSED(name))
{
    wxMISSING_IMPLEMENTATION( "wxStatusBar::Create parameters" );

    m_qtStatusBar = new wxQtStatusBar( parent, this );

    if(style & wxSTB_SIZEGRIP)
        m_qtStatusBar->setSizeGripEnabled(true);

    PostCreation();

    SetFieldsCount(1);

    return true;
}

bool wxStatusBar::GetFieldRect(int i, wxRect& rect) const
{
    wxCHECK_MSG( (i >= 0) && ((size_t)i < m_panes.GetCount()), false,
                 "invalid statusbar field index" );

    rect = wxQtConvertRect(m_qtPanes[i]->geometry());
    return true;
}

void wxStatusBar::SetMinHeight(int height)
{
    m_qtStatusBar->setMinimumHeight(height);
}

int wxStatusBar::GetBorderX() const
{
    return 0;
}

int wxStatusBar::GetBorderY() const
{
    return 0;
}

void wxStatusBar::DoUpdateStatusText(int number)
{
    m_qtPanes[number]->setText( wxQtConvertString( m_panes[number].GetText() ) );
}

// Called each time number/size of panes changes
void wxStatusBar::Refresh( bool eraseBackground, const wxRect *rect )
{
    UpdateFields();

    wxWindow::Refresh( eraseBackground, rect );
}

void wxStatusBar::UpdateFields()
{
    // is it a good idea to recreate all the panes every update?

    if(m_qtPanes.size() == m_panes.GetCount()){
        for (size_t i = 0; i < m_panes.GetCount(); i++)
        {
        //Set sizes
            int width = m_panes[i].GetWidth();

 //           qDebug() << "Resize" << i << m_panes.GetCount() << width;

            QLabel *pane = m_qtPanes[i];

            if ( width >= 0 )
            {
            //Fixed width field
                pane->setMinimumSize( QSize(width, 0) );
                pane->setMaximumWidth( width );
            }
        }

        return;
    }


    while ( !m_qtPanes.isEmpty() )
    {
        //Remove all panes
        delete m_qtPanes.takeLast();
    }

    for (size_t i = 0; i < m_panes.GetCount(); i++)
    {
        //Set sizes
        int width = m_panes[i].GetWidth();

//        qDebug() << i << m_panes.GetCount() << width;

        QLabel *pane = new QLabel( m_qtStatusBar );
        if(i == m_panes.GetCount()-1)
            pane->setAlignment( Qt::AlignRight | Qt::AlignVCenter );    // prettier

        m_qtPanes.append( pane ); //        AlignVCenter = 0x0080,   AlignBaseline = 0x0100,


        if ( width >= 0 )
        {
            //Fixed width field
            pane->setMinimumSize( QSize(width, 0) );
            pane->setMaximumWidth( width );
            m_qtStatusBar->addWidget( pane );
        }
        else
        {
            m_qtStatusBar->addWidget( pane, -width );
        }
    }
}

QStatusBar *wxStatusBar::GetHandle() const
{
    return m_qtStatusBar;
}
