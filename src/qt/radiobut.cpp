/////////////////////////////////////////////////////////////////////////////
// Name:        src/qt/radiobut.cpp
// Author:      Peter Most
// Copyright:   (c) Peter Most
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include "wx/radiobut.h"
#include "wx/qt/private/converter.h"
#include "wx/qt/private/winevent.h"

class wxQtPushRadioButton : public wxQtEventSignalHandler< QRadioButton, wxRadioButton >
{

public:
    wxQtPushRadioButton( wxWindow *parent, wxRadioButton *handler);

private:
    void clicked(bool);
};

wxQtPushRadioButton::wxQtPushRadioButton(wxWindow *parent, wxRadioButton *handler)
    : wxQtEventSignalHandler< QRadioButton, wxRadioButton >( parent, handler )
{
    connect(this, &QRadioButton::clicked, this, &wxQtPushRadioButton::clicked);
}

void wxQtPushRadioButton::clicked( bool WXUNUSED(checked) )
{
    wxRadioButton *handler = GetHandler();
    if ( handler )
    {
        wxCommandEvent event(  wxEVT_COMMAND_RADIOBUTTON_SELECTED, handler->GetId() );
        EmitEvent( event );
    }
}





wxRadioButton::wxRadioButton()
{
}

wxRadioButton::wxRadioButton( wxWindow *parent,
               wxWindowID id,
               const wxString& label,
               const wxPoint& pos,
               const wxSize& size,
               long style,
               const wxValidator& validator,
               const wxString& name)
{
    Create( parent, id, label, pos, size, style, validator, name );
}

bool wxRadioButton::Create( wxWindow *parent,
             wxWindowID id,
             const wxString& label,
             const wxPoint& pos,
             const wxSize& size,
             long style,
             const wxValidator& validator,
             const wxString& name)
{
    m_qtRadioButton = new wxQtPushRadioButton( parent, this );
    m_qtRadioButton->setText( wxQtConvertString( label ));

    bool ret_val = QtCreateControl( parent, id, pos, size, style, validator, name );

#if 1
    //TODO  Need to implement an array of QButtonGroups in parent, so that they can be deleted on dtor.
    wxWindowQt *Qtparent = (wxWindowQt *)parent;

    if(style == wxRB_GROUP){
        QButtonGroup *bgroup = new QButtonGroup ( parent->GetHandle() );
        bgroup->setExclusive ( true );

        Qtparent->setActiveRBGroup(bgroup);
    }

    QButtonGroup *bgroup = Qtparent->getActiveRBGroup();
    if(bgroup)
        bgroup->addButton( m_qtRadioButton );
#endif

    return ret_val;
}

void wxRadioButton::SetValue(bool value)
{
    m_qtRadioButton->setChecked( value );
}

bool wxRadioButton::GetValue() const
{
    return m_qtRadioButton->isChecked();
}

QRadioButton *wxRadioButton::GetHandle() const
{
    return m_qtRadioButton;
}
