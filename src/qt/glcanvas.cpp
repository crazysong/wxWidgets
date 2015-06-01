/////////////////////////////////////////////////////////////////////////////
// Name:        src/qt/glcanvas.cpp
// Author:      Sean D'Epagnier
// Copyright:   (c) Sean D'Epagnier 2014
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#if wxUSE_GLCANVAS

#include "wx/qt/private/winevent.h"
#include "wx/glcanvas.h"

class wxQtGLWidget : public wxQtEventSignalHandler< QGLWidget, wxGLCanvas >
{
public:
    wxQtGLWidget(wxWindow *parent, wxGLCanvas *handler, QGLFormat format)
        : wxQtEventSignalHandler(parent, handler, format)
        {
//            setFormat(format);
            setAutoBufferSwap( false );

            setAttribute(Qt::WA_AcceptTouchEvents);
            grabGesture(Qt::PinchGesture);
            grabGesture(Qt::PanGesture);

        }

 wxQtGLWidget(wxWindow *parent, wxGLCanvas *handler, QGLContext *context)
        : wxQtEventSignalHandler(parent, handler, context)
        {
            setAutoBufferSwap( false );

            setAttribute(Qt::WA_AcceptTouchEvents);
            grabGesture(Qt::PinchGesture);
            grabGesture(Qt::PanGesture);

        }

protected:
    virtual void showEvent ( QShowEvent * event );
    virtual void hideEvent ( QHideEvent * event );
    virtual void resizeEvent ( QResizeEvent * event );
    virtual void paintEvent ( QPaintEvent * event );

    virtual void resizeGL(int w, int h);
    virtual void paintGL();
};

void wxQtGLWidget::showEvent ( QShowEvent * event )
{
    QGLWidget::showEvent( event );
}

void wxQtGLWidget::hideEvent ( QHideEvent * event )
{
    QGLWidget::hideEvent( event );
}

void wxQtGLWidget::resizeEvent ( QResizeEvent * event )
{
    QGLWidget::resizeEvent(event);
}

void wxQtGLWidget::paintEvent ( QPaintEvent * event )
{
    QGLWidget::paintEvent(event);
}

void wxQtGLWidget::resizeGL(int w, int h)
{
    wxSizeEvent event( wxSize(w, h) );
    EmitEvent(event);
}

void wxQtGLWidget::paintGL()
{
    wxPaintEvent event( GetHandler()->GetId() );
    EmitEvent(event);
}

//---------------------------------------------------------------------------
// wxGlContext
//---------------------------------------------------------------------------

#include "qopenglcontext.h"
#include "qwindow.h"

IMPLEMENT_CLASS(wxGLContext, wxWindow)

wxGLContext::wxGLContext(wxGLCanvas *WXUNUSED(win), const wxGLContext* WXUNUSED(other))
{
//    m_glContext = win->GetHandle()->context();
}

bool wxGLContext::SetCurrent(const wxGLCanvas&) const
{
// I think I must destroy and recreate the QGLWidget to change the context?
//    win->GetHandle()->makeCurrent();
    return false;
}

//---------------------------------------------------------------------------
// wxGlCanvas
//---------------------------------------------------------------------------

IMPLEMENT_CLASS(wxGLCanvas, wxWindow)

wxGLCanvas::wxGLCanvas(wxWindow *parent,
                       wxWindowID id,
                       const int *attribList,
                       const wxPoint& pos,
                       const wxSize& size,
                       long style,
                       const wxString& name,
                       const wxPalette& palette)
{
    Create(parent, id, pos, size, style, name, attribList, palette);
}

bool wxGLCanvas::Create(wxWindow *parent,
                        wxWindowID id,
                        const wxPoint& pos,
                        const wxSize& size,
                        long style,
                        const wxString& name,
                        const int *attribList,
                        const wxPalette& palette)
{
#if wxUSE_PALETTE
    wxASSERT_MSG( !palette.IsOk(), wxT("palettes not supported") );
#endif // wxUSE_PALETTE
    wxUnusedVar(palette); // Unused when wxDEBUG_LEVEL==0

    QGLFormat format;
    if (!wxGLCanvas::ConvertWXAttrsToQtGL(attribList, format))
        return false;

    //  This complicated two-step is necessary to get a GLES 1.1 context on some devices.
    //  The default GLES2 context is unsuitable
//    wxQtGLWidget *dummy = new wxQtGLWidget(parent, this, format);

    QOpenGLContext* qcc = new QOpenGLContext(0/*dummy*/);
    QSurfaceFormat fmt = QGLFormat::toSurfaceFormat(format);
    fmt.setRenderableType(QSurfaceFormat::OpenGLES);
//    fmt.setRedBufferSize(8);
//    fmt.setGreenBufferSize(8);
//    fmt.setBlueBufferSize(8);
//    fmt.setAlphaBufferSize(8);
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);

    qcc->setFormat(fmt);
    qcc->create();

    QGLContext *cc = QGLContext::fromOpenGLContext(qcc);
    m_qtWindow = new wxQtGLWidget(parent, this, cc);

    QWindow *window = m_qtWindow->windowHandle();
    window->setSurfaceType(QWindow::OpenGLSurface);

    wxWindow::Create( parent, id, pos, size, style, name );

    return true;
}

bool wxGLCanvas::SwapBuffers()
{
    GetHandle()->swapBuffers();
    return true;
}

/* static */
bool wxGLCanvas::ConvertWXAttrsToQtGL(const int *wxattrs, QGLFormat &format)
{
    if (!wxattrs)
        return true;
//    return true;

    // set default parameters to false
    format.setDoubleBuffer(false);
    format.setDepth(false);
    format.setAlpha(false);
    format.setStencil(false);

    //  We don't have the option of setting OpenGLES version defined in the
    //  attribute list Enum.

    // So, just force it here.
    format.setVersion ( 1,1 );

    for ( int arg = 0; wxattrs[arg] != 0; arg++ )
    {
        // indicates whether we have a boolean attribute
        bool isBoolAttr = false;

        int v = wxattrs[arg+1];
        switch ( wxattrs[arg] )
        {
            case WX_GL_BUFFER_SIZE:
                format.setRgba(false);
                // I do not know how to set the buffer size, so fail
                return false;

            case WX_GL_LEVEL:
                format.setPlane(v);
                break;

            case WX_GL_RGBA:
                format.setRgba(true);
                isBoolAttr = true;
                break;

            case WX_GL_DOUBLEBUFFER:
                format.setDoubleBuffer(true);
                isBoolAttr = true;
                break;

            case WX_GL_STEREO:
                format.setStereo(true);
                isBoolAttr = true;
                break;

            case WX_GL_AUX_BUFFERS:
                // don't know how to implement
                return false;

            case WX_GL_MIN_RED:
                format.setRedBufferSize(v*8);
                break;

            case WX_GL_MIN_GREEN:
                format.setGreenBufferSize(v);
                break;

            case WX_GL_MIN_BLUE:
                format.setBlueBufferSize(v);
                break;

            case WX_GL_MIN_ALPHA:
                format.setAlpha(true);
                format.setAlphaBufferSize(v);
                break;

            case WX_GL_DEPTH_SIZE:
                format.setDepth(true);
                format.setDepthBufferSize(v);
                break;

            case WX_GL_STENCIL_SIZE:
                format.setStencil(true);
                format.setStencilBufferSize(v);
                break;

            case WX_GL_MIN_ACCUM_RED:
            case WX_GL_MIN_ACCUM_GREEN:
            case WX_GL_MIN_ACCUM_BLUE:
            case WX_GL_MIN_ACCUM_ALPHA:
                format.setAccumBufferSize(v);
                break;

            case WX_GL_SAMPLE_BUFFERS:
                format.setSampleBuffers(v);
                // can we somehow indicate if it's not supported?
                break;

            case WX_GL_SAMPLES:
                format.setSamples(v);
                // can we somehow indicate if it's not supported?
                break;

            default:
                wxLogDebug(wxT("Unsupported OpenGL attribute %d"),
                           wxattrs[arg]);
                continue;
        }

        if ( !isBoolAttr ) {
            if(!v)
                return false; // zero parameter
            arg++;
        }
    }

    return true;
}

/* static */
bool
wxGLCanvasBase::IsDisplaySupported(const int *attribList)
{
    QGLFormat format;

    if (!wxGLCanvas::ConvertWXAttrsToQtGL(attribList, format))
        return false;

    return QGLWidget(format).isValid();
}

#if 0
// Generic "Slot" to conect destruction signal for debugging purposes:
//  Not sure why we need another instance of this static method, but qtu_gl.so will not load otherwise.
void wxQtHandleDestroyedSignal(QObject *qobj)
{
   wxLogDebug( wxT("%s was destroyed by Qt. pointer=%p"),
               QObject::staticMetaObject.className(), qobj );
}
#endif

#endif // wxUSE_GLCANVAS
