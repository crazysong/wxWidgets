/////////////////////////////////////////////////////////////////////////////
// Name:        include/wx/qt/winevent_qt.h
// Purpose:     QWidget to wxWindow event handler
// Author:      Javier Torres, Peter Most
// Modified by:
// Created:     21.06.10
// Copyright:   (c) Javier Torres
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_EVENTSIGNALFORWARDER_H_
#define _WX_QT_EVENTSIGNALFORWARDER_H_

#include "wx/window.h"
#include "wx/glcanvas.h"
#include "wx/qt/private/converter.h"
#include "wx/qt/private/utils.h"
#include "wx/qt/private/wxQtGesture.h"
#include "wx/log.h"

#include <QtCore/QEvent>
#include <QtGui/QPaintEvent>

#include <QDebug>
#include <QSwipeGesture>
#include <QGLContext>

template< typename Handler >
class wxQtSignalHandler
{
protected:
    wxQtSignalHandler( Handler *handler )
    {
        m_handler = handler;
    }

    void EmitEvent( wxEvent &event ) const
    {
        wxWindow *handler = GetHandler();
        event.SetEventObject( handler );
        handler->HandleWindowEvent( event );
    }

    virtual Handler *GetHandler() const
    {
        return m_handler;
    }

private:
    Handler *m_handler;
};

template < typename Widget, typename Handler >
class wxQtEventSignalHandler : public Widget, public wxQtSignalHandler< Handler >
{
public:
    wxQtEventSignalHandler( wxWindow *parent, Handler *handler )
        : Widget( parent != NULL ? parent->GetHandle() : NULL )
        , wxQtSignalHandler< Handler >( handler )
    {
        // Set immediately as it is used to check if wxWindow is alive
        wxWindow::QtStoreWindowPointer( this, handler );

        // Handle QWidget destruction signal AFTER it gets deleted
        QObject::connect( this, &QObject::destroyed, this,
                          &wxQtEventSignalHandler::HandleDestroyedSignal );

    }

    wxQtEventSignalHandler( wxWindow *parent, Handler *handler, QGLFormat &format )
        : Widget( format, parent != NULL ? parent->GetHandle() : NULL )
        , wxQtSignalHandler< Handler >( handler )
    {
        // Set immediatelly as it is used to check if wxWindow is alive
        wxWindow::QtStoreWindowPointer( this, handler );

        // Handle QWidget destruction signal AFTER it gets deleted
        QObject::connect( this, &QObject::destroyed, this,
                          &wxQtEventSignalHandler::HandleDestroyedSignal );

    }

   wxQtEventSignalHandler( wxWindow *parent, Handler *handler, QGLContext *context )
        : Widget( context, parent != NULL ? parent->GetHandle() : NULL )
        , wxQtSignalHandler< Handler >( handler )
    {
        // Set immediatelly as it is used to check if wxWindow is alive
        wxWindow::QtStoreWindowPointer( this, handler );

        // Handle QWidget destruction signal AFTER it gets deleted
        QObject::connect( this, &QObject::destroyed, this,
                          &wxQtEventSignalHandler::HandleDestroyedSignal );

    }

    void HandleDestroyedSignal()
    {
        wxQtHandleDestroyedSignal(this);
    }

    virtual Handler *GetHandler() const
    {
        // Only process the signal / event if the wxWindow is not destroyed
        if ( !wxWindow::QtRetrieveWindowPointer( this ) )
        {
            wxLogDebug( wxT("%s win pointer is NULL (wxWindow is deleted)!"),
                        Widget::staticMetaObject.className()
                        );
            return NULL;
        }
        else
            return wxQtSignalHandler< Handler >::GetHandler();
    }

protected:


    /* Not implemented here: wxHelpEvent, wxIdleEvent wxJoystickEvent,
     * wxMouseCaptureLostEvent, wxMouseCaptureChangedEvent,
     * wxPowerEvent, wxScrollWinEvent, wxSysColourChangedEvent */


    virtual bool event(QEvent *event)
    {
        if (event->type() == QEvent::Gesture)
            return gestureEvent(static_cast<QGestureEvent*>(event), event);

        //  If we want scrolled windows to process PanGesture updates,
        //  we must "accept" the TouchBegin event in that window.
        // This seems like flawed logic in Qt, though.....

/*
        if (event->type() == QEvent::TouchBegin){
            wxWindow *win = wxWindow::QtRetrieveWindowPointer( this );
            if(win && win->IsKindOf( CLASSINFO(wxScrolledWindow)) ){
                return true;
            }
            else
                return Widget::event(event);
        }
*/

        return Widget::event(event);
    }

    bool gestureEvent(QGestureEvent *gesture, QEvent *event)
    {


        if (QGesture *swipe = gesture->gesture(Qt::SwipeGesture))
            swipeTriggered(static_cast<QSwipeGesture *>(swipe), event);

        if (QGesture *tah = gesture->gesture(Qt::TapAndHoldGesture)){
            //  Set the policy so that accepted gestures are taken by the first window that gets them
            tah->setGestureCancelPolicy ( QGesture::CancelAllInContext );
            tapandholdTriggered(static_cast<QTapAndHoldGesture *>(tah), event);
        }

        if (QGesture *pan = gesture->gesture(Qt::PanGesture)){
            panTriggered(static_cast<QPanGesture *>(pan), event);
        }

        if (QGesture *pinch = gesture->gesture(Qt::PinchGesture)){
            pinchTriggered(static_cast<QPinchGesture *>(pinch), event);
        }


        return true;
    }

    void tapandholdTriggered(QTapAndHoldGesture *gesture, QEvent *event)
    {
        if (gesture->state() == Qt::GestureFinished) {
//            qDebug() << "TAH_Finished now" << this;
            QMouseEvent e(QEvent::MouseButtonPress, gesture->position(),
                          Qt::RightButton,Qt::RightButton, Qt::NoModifier);

            mousePressEvent ( &e );
/*
            wxMouseEvent ev( wxEVT_RIGHT_DOWN );

            QPoint this_offset = gesture->position().toPoint();
            wxPoint locn = wxQtConvertPoint( this_offset );

            ev.m_x = locn.x;
            ev.m_y = locn.y;

            wxWindow *win = wxWindow::QtRetrieveWindowPointer( this );
            win->ProcessWindowEvent( ev );
*/

        }
        else if (gesture->state() == Qt::GestureStarted) {
//            qDebug() << "TAH_Started now";
        }

        else {
//            qDebug() << "TAH_event" << gesture->state();
        }

        event->accept();

    }

    void swipeTriggered(QSwipeGesture *gesture, QEvent *event)
    {
        QGestureEvent *gev = static_cast<QGestureEvent*>(event);
        (void)gesture;
        wxWindow *win = wxWindow::QtRetrieveWindowPointer( this );
        if(win && win->IsKindOf( CLASSINFO(wxScrolledWindow)) ){
  //          qDebug() << "swipe Scrolled Window trigger" << gesture ;
                qreal angle = gesture->swipeAngle();
//                qDebug() << "angle" << angle;
                int direction = SwipeDirectionHelper(angle);

/*
                if(QSwipeGesture::Up == direction)
                    win->ScrollWindow(0, 40);
                else if(QSwipeGesture::Down == direction)
                    win->ScrollWindow(0, -40);
                else if(QSwipeGesture::Left == direction)
                    win->ScrollWindow(40, 0);
                else if(QSwipeGesture::Right == direction)
                    win->ScrollWindow(-40, 0);
*/
                wxEventType windowEventType = 0;
                int orient = 0;

                if(QSwipeGesture::Up == direction){
                    windowEventType = wxEVT_SCROLLWIN_LINEDOWN; //wxEVT_SCROLLWIN_PAGEDOWN;
            qDebug() << "swipedown";
                    orient = wxVERTICAL;
                }
                else if(QSwipeGesture::Down == direction){
                   windowEventType = wxEVT_SCROLLWIN_LINEUP;    //wxEVT_SCROLLWIN_PAGEUP;
            qDebug() << "swipeup";
                   orient = wxVERTICAL;
                }

                else if(QSwipeGesture::Left == direction){
                    windowEventType = wxEVT_SCROLLWIN_LINEDOWN; //wxEVT_SCROLLWIN_PAGEDOWN;
                    orient = wxHORIZONTAL;
                }
                else {
                   windowEventType = wxEVT_SCROLLWIN_LINEUP;    //wxEVT_SCROLLWIN_PAGEUP;
                   orient = wxHORIZONTAL;
                }

                wxScrollWinEvent swevent( windowEventType, 0,  orient );
                win->ProcessWindowEvent( swevent );

                event->accept();

        }
        else{
//            qDebug() << "swipe ignored" << gesture ;
            gev->ignore();
        }


#if 0
        else if(win){
            wxWindow *parent = win->GetParent();
            if(parent && parent->IsKindOf( CLASSINFO(wxScrolledWindow)) ){
                qDebug() << "swipe parent Scrolled Window trigger" ;
                qreal angle = gesture->swipeAngle();
                qDebug() << "angle" << angle;
                int direction = SwipeDirectionHelper(angle);

                if(QSwipeGesture::Up == direction)
                    parent->ScrollWindow(0, 40);
                else if(QSwipeGesture::Down == direction)
                    parent->ScrollWindow(0, -40);
                else if(QSwipeGesture::Left == direction)
                    parent->ScrollWindow(40, 0);
                else if(QSwipeGesture::Right == direction)
                    parent->ScrollWindow(-40, 0);

                event->accept();
            }
        }
#endif

    }

    int SwipeDirectionHelper(qreal angle)
    {
        if((angle > 45) && (angle < 135))
            return QSwipeGesture::Up;
        else if((angle > 225) && (angle < 315))
            return QSwipeGesture::Down;
        else if((angle > 315) || (angle < 45))
            return QSwipeGesture::Right;
        else if((angle > 135) && (angle < 225))
            return QSwipeGesture::Left;
        else
            return QSwipeGesture::NoDirection;
    }


    void panTriggered(QPanGesture *gesture, QEvent *event)
    {
        int dx = gesture->offset().x() - gesture->lastOffset().x();
        int dy = gesture->offset().y() - gesture->lastOffset().y();

        wxWindow *win = wxWindow::QtRetrieveWindowPointer( this );
        qDebug() << "pan Window trigger" << dx << dy;

#if 1
        if(win && win->IsKindOf( CLASSINFO(wxScrolledWindow)) ){

  //         state = Qt::GestureStarted;
  //        state = Qt::GestureUpdated;
  //         state = Qt::GestureFinished;

                if (gesture->state() == Qt::GestureUpdated) {

               wxEventType windowEventType = 0;
                int orient = 0;

            qDebug() << "pan Scrolled Window trigger" << dx << dy;

                if(dy < 0){
                    windowEventType = wxEVT_SCROLLWIN_LINEDOWN; //wxEVT_SCROLLWIN_PAGEDOWN;
            qDebug() << "down";
                    orient = wxVERTICAL;
                }
                else if(dy > 0){
                   windowEventType = wxEVT_SCROLLWIN_LINEUP;    //wxEVT_SCROLLWIN_PAGEUP;
            qDebug() << "up";
                   orient = wxVERTICAL;
                }

                else if(dx < 0){
                    windowEventType = wxEVT_SCROLLWIN_LINEDOWN; //wxEVT_SCROLLWIN_PAGEDOWN;
                    orient = wxHORIZONTAL;
                }
                else if(dx > 0){
                   windowEventType = wxEVT_SCROLLWIN_LINEUP;    //wxEVT_SCROLLWIN_PAGEUP;
                   orient = wxHORIZONTAL;
                }

                int pos = win->GetScrollPos( orient );
                wxScrollWinEvent swevent( windowEventType, pos,  orient );
                win->ProcessWindowEvent( swevent );

                }
                event->accept();





//             if(gesture->offset().y() > 0)
//                 win->ScrollWindow(0, 5);
//             else if(gesture->offset().y() < 0)
//                 win->ScrollWindow(0, -5);
//
//             event->accept();
        }


        else{
            QGestureEvent *gev = static_cast<QGestureEvent*>(event);
            gev->ignore();
//            qDebug() << "pan ignored" << gesture ;
        }
#endif

#if 0
        if(win){
            wxWindow *parent = win->GetParent();
            if(parent && parent->IsKindOf( CLASSINFO(wxScrolledWindow)) ){
#if 0
                qDebug() << "pan parent Scrolled Window trigger" ;
                if(gesture->offset().y() > 0)
                    parent->ScrollWindow(0, 5);
                else if(gesture->offset().y() < 0)
                    parent->ScrollWindow(0, -5);
#endif
                event->accept();
            }
        }
#endif


        if(win && win->IsKindOf( CLASSINFO(wxGLCanvas)) ){

//            qDebug() << "PanGesture event";

            wxQT_PanGestureEvent ev(wxEVT_QT_PANGESTURE);

            QPoint this_offset = gesture->offset().toPoint();
            ev.SetOffset( wxQtConvertPoint( this_offset ) );

            QPoint last_offset = gesture->lastOffset().toPoint();
            ev.SetLastOffset( wxQtConvertPoint( last_offset ) );

            ev.SetState((wxGestureState)(gesture->state() - 1));

            win->ProcessWindowEvent( ev );
            event->accept();
        }

    }

    void pinchTriggered(QPinchGesture *gesture, QEvent *event)
    {
        wxWindow *win = wxWindow::QtRetrieveWindowPointer( this );
        if(win && win->IsKindOf( CLASSINFO(wxGLCanvas)) ){

            qreal last_sf = gesture->lastScaleFactor();
            qreal this_sf = gesture->scaleFactor();
            qreal total_sf = gesture->totalScaleFactor();

//           qDebug() << " GL PinchGesture event" << last_sf << this_sf << total_sf;

            wxQT_PinchGestureEvent ev(wxEVT_QT_PINCHGESTURE);
            ev.SetLastScaleFactor(last_sf);
            ev.SetScaleFactor(this_sf);
            ev.SetTotalScaleFactor(total_sf);
            ev.SetState((wxGestureState)(gesture->state() - 1));

            win->ProcessWindowEvent( ev );
            event->accept();

        }
    }



    //wxActivateEvent
    virtual void changeEvent ( QEvent * event )
    {
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::changeEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleChangeEvent(this, event) )
            Widget::changeEvent(event);
        else
            event->accept();
    }

    //wxCloseEvent
    virtual void closeEvent ( QCloseEvent * event )
    {
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::closeEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleCloseEvent(this, event) )
            Widget::closeEvent(event);
        else
            event->accept();
    }

    //wxContextMenuEvent
    virtual void contextMenuEvent ( QContextMenuEvent * event )
    {
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::contextMenuEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleContextMenuEvent(this, event) )
            Widget::contextMenuEvent(event);
        else
            event->accept();
    }

    //wxDropFilesEvent
    //virtual void dropEvent ( QDropEvent * event ) { }

    //wxMouseEvent
    virtual void enterEvent ( QEvent * event )
    {
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::enterEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleEnterEvent(this, event) )
            Widget::enterEvent(event);
        else
            event->accept();
    }

    //wxFocusEvent.
    virtual void focusInEvent ( QFocusEvent * event )
    {
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::focusInEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleFocusEvent(this, event) )
            Widget::focusInEvent(event);
        else
            event->accept();
    }

    //wxFocusEvent.
    virtual void focusOutEvent ( QFocusEvent * event )
    {
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::focusOutEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleFocusEvent(this, event) )
            Widget::focusOutEvent(event);
        else
            event->accept();
    }

    //wxShowEvent
    virtual void hideEvent ( QHideEvent * event )
    {
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::hideEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleShowEvent(this, event) )
            Widget::hideEvent(event);
        else
            event->accept();
    }

    //wxKeyEvent
    virtual void keyPressEvent ( QKeyEvent * event )
    {
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::keyPressEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleKeyEvent(this, event) )
            Widget::keyPressEvent(event);
        else
            event->accept();
    }

    //wxKeyEvent
    virtual void keyReleaseEvent ( QKeyEvent * event )
    {
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::keyReleaseEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleKeyEvent(this, event) )
            Widget::keyReleaseEvent(event);
        else
            event->accept();
    }

    //wxMouseEvent
    virtual void leaveEvent ( QEvent * event )
    {
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::leaveEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleEnterEvent(this, event) )
            Widget::leaveEvent(event);
        else
            event->accept();
    }

    //wxMouseEvent
    virtual void mouseDoubleClickEvent ( QMouseEvent * event )
    {
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::mouseDoubleClickEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleMouseEvent(this, event) )
            Widget::mouseDoubleClickEvent(event);
        else
            event->accept();
    }

    //wxMouseEvent
    virtual void mouseMoveEvent ( QMouseEvent * event )
    {
//        qDebug() << "Move";
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::mouseMoveEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleMouseEvent(this, event) )
            Widget::mouseMoveEvent(event);
        else
            event->accept();
    }

    //wxMouseEvent
    virtual void mousePressEvent ( QMouseEvent * event )
    {
/*
        wxWindow *win = wxWindow::QtRetrieveWindowPointer( this );
        if(win && win->IsKindOf( CLASSINFO(wxScrolledWindow)) ){
            event->accept();
            return;
        }

        if(win && win->IsKindOf( CLASSINFO(wxDialog)) ){
            event->accept();
            return;
        }

        qDebug() << "Press";
*/
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::mousePressEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleMouseEvent(this, event) )
            Widget::mousePressEvent(event);
        else
            event->accept();
    }

    //wxMouseEvent
    virtual void mouseReleaseEvent ( QMouseEvent * event )
    {
/*
       wxWindow *win = wxWindow::QtRetrieveWindowPointer( this );
        if(win && win->IsKindOf( CLASSINFO(wxScrolledWindow)) ){
            event->accept();
            return;
        }

        if(win && win->IsKindOf( CLASSINFO(wxDialog)) ){
            event->accept();
            return;
        }

       qDebug() << "Release";
*/
         if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::mouseReleaseEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleMouseEvent(this, event) )
            Widget::mouseReleaseEvent(event);
        else
            event->accept();
    }

    //wxMoveEvent
    virtual void moveEvent ( QMoveEvent * event )
    {
/*
       wxWindow *win = wxWindow::QtRetrieveWindowPointer( this );
        if(win && win->IsKindOf( CLASSINFO(wxScrolledWindow)) ){
            event->accept();
            return;
        }

        if(win && win->IsKindOf( CLASSINFO(wxDialog)) ){
            event->accept();
            return;
        }
*/
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::moveEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleMoveEvent(this, event) )
            Widget::moveEvent(event);
        else
            event->accept();
    }

    //wxEraseEvent then wxPaintEvent
    virtual void paintEvent ( QPaintEvent * event )
    {
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::paintEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandlePaintEvent(this, event) )
            Widget::paintEvent(event);
        else
            event->accept();
    }

    //wxSizeEvent
    virtual void resizeEvent ( QResizeEvent * event )
    {
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::resizeEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleResizeEvent(this, event) )
            Widget::resizeEvent(event);
        else
            event->accept();
    }

    //wxShowEvent
    virtual void showEvent ( QShowEvent * event )
    {
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::showEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleShowEvent(this, event) )
            Widget::showEvent(event);
        else
            event->accept();
    }

    //wxMouseEvent
    virtual void wheelEvent ( QWheelEvent * event )
    {
        if ( !this->GetHandler() )
            wxLogDebug( wxT("%s::wheelEvent for invalid handler!"),
                        Widget::staticMetaObject.className() );
        else if ( !this->GetHandler()->QtHandleWheelEvent(this, event) )
            Widget::wheelEvent(event);
        else
            event->accept();
    }

    /* Unused Qt events
    virtual void actionEvent ( QActionEvent * event ) { }
    virtual void dragEnterEvent ( QDragEnterEvent * event ) { }
    virtual void dragLeaveEvent ( QDragLeaveEvent * event ) { }
    virtual void dragMoveEvent ( QDragMoveEvent * event ) { }
    virtual void inputMethodEvent ( QInputMethodEvent * event ) { }
    virtual bool macEvent ( EventHandlerCallRef caller, EventRef event ) { }
    virtual bool qwsEvent ( QWSEvent * event ) { }
    virtual void tabletEvent ( QTabletEvent * event ) { }
    virtual bool winEvent ( MSG * message, long * result ) { }
    virtual bool x11Event ( XEvent * event ) { } */

};

#endif
