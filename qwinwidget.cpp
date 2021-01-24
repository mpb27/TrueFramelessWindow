/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Solutions component.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

/* 																										 */
/* 																										 */
/* File is originally from https://github.com/qtproject/qt-solutions/tree/master/qtwinmigrate/src        */
/* 																										 */
/* It has been modified to support borderless window (HTTRANSPARENT) & to remove pre Qt5 cruft          */
/* 																										 */
/* 																										 */


#include "QWinWidget.h"

#include <QApplication>
#include <QEvent>
#include <QFocusEvent>
#include <qt_windows.h>
#include <QWindow>


/*!
    \class QWinWidget qwinwidget.h
    \brief The QWinWidget class is a Qt widget that can be child of a
    native Win32 widget.

    The QWinWidget class is the bridge between an existing application
    user interface developed using native Win32 APIs or toolkits like
    MFC, and Qt based GUI elements.

    Using QWinWidget as the parent of QDialogs will ensure that
    modality, placement and stacking works properly throughout the
    entire application. If the child widget is a top level window that
    uses the \c WDestructiveClose flag, QWinWidget will destroy itself
    when the child window closes down.

    Applications moving to Qt can use QWinWidget to add new
    functionality, and gradually replace the existing interface.
*/


NativeEventFilter::NativeEventFilter(QWinWidget *widget)
    : winWidget(widget)
{

}

bool NativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    if (eventType != "windows_generic_MSG" && eventType != "windows_dispatcher_MSG")
        return false;

    MSG *msg = (MSG *)message;

    if (msg->message == WM_SETFOCUS)
    {
        Qt::FocusReason reason;
        if (::GetKeyState(VK_LBUTTON) < 0 || ::GetKeyState(VK_RBUTTON) < 0)
            reason = Qt::MouseFocusReason;
        else if (::GetKeyState(VK_SHIFT) < 0)
            reason = Qt::BacktabFocusReason;
        else
            reason = Qt::TabFocusReason;
        QFocusEvent e(QEvent::FocusIn, reason);
        QApplication::sendEvent(winWidget, &e);
    }

    //Only close if safeToClose clears()
    if (msg->message == WM_CLOSE)
    {
        //do nothing
    }

    //Double check WM_SIZE messages to see if the parent native window is maximized
    if (msg->message == WM_SIZE)
    {
        if (winWidget->m_widget && winWidget->m_widget->maximizeButton)
        {
            //Get the window state
            WINDOWPLACEMENT wp;
            GetWindowPlacement(winWidget->m_parentNativeWindowHandle, &wp);

            //If we're maximized,
            if (wp.showCmd == SW_MAXIMIZE)
            {
                //Maximize button should show as Restore
                winWidget->m_widget->maximizeButton->setChecked(true);
            }
            else
            {
                //Maximize button should show as Maximize
                winWidget->m_widget->maximizeButton->setChecked(false);
            }
        }
    }

    //Pass NCHITTESTS on the window edges as determined by BORDERWIDTH & TOOLBARHEIGHT through to the parent native window
    if (msg->message == WM_NCHITTEST)
    {
        RECT WindowRect;
        int x, y;

        GetWindowRect(msg->hwnd, &WindowRect);
        x = GET_X_LPARAM(msg->lParam) - WindowRect.left;
        y = GET_Y_LPARAM(msg->lParam) - WindowRect.top;

        if (x >= winWidget->BORDERWIDTH && x <= WindowRect.right - WindowRect.left - winWidget->BORDERWIDTH && y >= winWidget->BORDERWIDTH && y <= winWidget->TOOLBARHEIGHT)
        {
            if (winWidget->m_widget->toolBar)
            {
                //If the mouse is over top of the toolbar area BUT is actually positioned over a child widget of the toolbar,
                //Then we don't want to enable dragging. This allows for buttons in the toolbar, eg, a Maximize button, to keep the mouse interaction
                if (QApplication::widgetAt(QCursor::pos()) != winWidget->m_widget->toolBar)
                    return false;
            }

                //The mouse is over the toolbar area & is NOT over a child of the toolbar, so pass this message
                //through to the native window for HTCAPTION dragging
                *result = HTTRANSPARENT;
                return true;

        }
        else if (x < winWidget->BORDERWIDTH && y < winWidget->BORDERWIDTH)
        {
            *result = HTTRANSPARENT;
            return true;
        }
        else if (x > WindowRect.right - WindowRect.left - winWidget->BORDERWIDTH && y < winWidget->BORDERWIDTH)
        {
            *result = HTTRANSPARENT;
            return true;
        }
        else if (x > WindowRect.right - WindowRect.left - winWidget->BORDERWIDTH && y > WindowRect.bottom - WindowRect.top - winWidget->BORDERWIDTH)
        {
            *result = HTTRANSPARENT;
            return true;
        }
        else if (x < winWidget->BORDERWIDTH && y > WindowRect.bottom - WindowRect.top - winWidget->BORDERWIDTH)
        {
            *result = HTTRANSPARENT;
            return true;
        }
        else if (x < winWidget->BORDERWIDTH)
        {
            *result = HTTRANSPARENT;
            return true;
        }
        else if (y < winWidget->BORDERWIDTH)
        {
            *result = HTTRANSPARENT;
            return true;
        }
        else if (x > WindowRect.right - WindowRect.left - winWidget->BORDERWIDTH)
        {
            *result = HTTRANSPARENT;
            return true;
        }
        else if (y > WindowRect.bottom - WindowRect.top - winWidget->BORDERWIDTH)
        {
            *result = HTTRANSPARENT;
            return true;
        }

        return false;
    }

    return false;
}

QWinWidget::QWinWidget()
    : QWidget(nullptr),
      m_layout(nullptr),
      m_widget(nullptr),
      m_parentNativeWindowHandle(nullptr),
      m_prevFocusHandle(nullptr),
      m_reenableParent(false)
{
    qApp->installNativeEventFilter(new NativeEventFilter(this));

    //Create a native window and give it geometry values * devicePixelRatio for HiDPI support
    auto pixel_ratio = window()->devicePixelRatio();
    m_parentWinNativeWindow = new WinNativeWindow(1 * pixel_ratio
        , 1 * pixel_ratio
        , 1 * pixel_ratio
        , 1 * pixel_ratio);

	//If you want to set a minimize size for your app, do so here
    //m_parentWinNativeWindow->setMinimumSize(1024 * window()->devicePixelRatio(), 768 * window()->devicePixelRatio());

	//If you want to set a maximum size for your app, do so here
	//m_parentWinNativeWindow->setMaximumSize(1024 * window()->devicePixelRatio(), 768 * window()->devicePixelRatio());


    //Save the native window handle for shorthand use
    m_parentNativeWindowHandle = m_parentWinNativeWindow->hWnd;
    Q_ASSERT(m_parentNativeWindowHandle);


    //Create the child window & embed it into the native one
    if (m_parentNativeWindowHandle)
    {
        setWindowFlags(Qt::FramelessWindowHint);
        setProperty("_q_embedded_native_parent_handle", (WId)m_parentNativeWindowHandle);
        ::SetWindowLong((HWND)winId(), GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        ::SetParent((HWND)winId(), m_parentNativeWindowHandle);
        QEvent e(QEvent::EmbeddingControl);
        QApplication::sendEvent(this, &e);
    }

    //Pass along our window handle & widget pointer to WinFramelessWidget so we can exchange messages
    m_parentWinNativeWindow->childWindow = (HWND)winId();
    m_parentWinNativeWindow->childWidget = this;

    //Clear margins & spacing & add the layout to prepare for the MainAppWidget
    setContentsMargins(0, 0, 0, 0);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    //setLayout(m_layout); //fixme: is this needed?

    //Create the true app widget 
    m_widget = new Widget(this);
    m_widget->setParent(this, Qt::Widget);
    m_widget->setVisible(true);
    m_layout->addWidget(m_widget);

    //Update the BORDERWIDTH value if needed for HiDPI displays
    BORDERWIDTH = BORDERWIDTH * window()->devicePixelRatio();

	//Update the TOOLBARHEIGHT value to match the height of toolBar * if needed, the HiDPI display
	if (m_widget->toolBar)
	{
		TOOLBARHEIGHT = m_widget->toolBar->height() * window()->devicePixelRatio();
	}


    //You need to keep the native window in sync with the Qt window & children, so wire min/max/close buttons to 
	//slots inside of QWinWidget. QWinWidget can then talk with the native window as needed 
	if (m_widget->minimizeButton)
	{
		connect(m_widget->minimizeButton, &QToolButton::clicked, this, &QWinWidget::onMinimizeButtonClicked);
	}
	if (m_widget->maximizeButton)
	{
		connect(m_widget->maximizeButton, &QToolButton::clicked, this, &QWinWidget::onMaximizeButtonClicked);

	}
	if (m_widget->closeButton)
	{
		connect(m_widget->closeButton, &QToolButton::clicked, this, &QWinWidget::onCloseButtonClicked);
	}

	

    //Send the parent native window a WM_SIZE message to update the widget size 
    ::SendMessage(m_parentNativeWindowHandle, WM_SIZE, 0, 0);
}


/*!
    Destroys this object, freeing all allocated resources.
*/
QWinWidget::~QWinWidget()
{

}

/*!
    Returns the handle of the native Win32 parent window.
*/
HWND QWinWidget::getParentWindow() const
{
    return m_parentNativeWindowHandle;
}

/*!
    \reimp
*/
void QWinWidget::childEvent(QChildEvent *e)
{
    QObject *obj = e->child();
    if (obj->isWidgetType())
    {
        if (e->added())
        {
            if (obj->isWidgetType())
            {
                obj->installEventFilter(this);
            }
        }
        else if (e->removed() && m_reenableParent)
        {
            m_reenableParent = false;
            ::EnableWindow(m_parentNativeWindowHandle, true);
            obj->removeEventFilter(this);
        }
    }
    QWidget::childEvent(e);
}

/*! \internal */
void QWinWidget::saveFocus()
{
    if (!m_prevFocusHandle) m_prevFocusHandle = ::GetFocus();
    if (!m_prevFocusHandle) m_prevFocusHandle = getParentWindow();
}

/*!
    Shows this widget. Overrides QWidget::show().

    \sa showCentered()
*/
void QWinWidget::show()
{
    ::ShowWindow(m_parentNativeWindowHandle, true);
    saveFocus();
    QWidget::show();
}

/*!
    Centers this widget over the native parent window. Use this
    function to have Qt toplevel windows (i.e. dialogs) positioned
    correctly over their native parent windows.

    \code
    QWinWidget qwin(hParent);
    qwin.center();

    QMessageBox::information(&qwin, "Caption", "Information Text");
    \endcode

    This will center the message box over the client area of hParent.
*/
void QWinWidget::center()
{
    const QWidget *child = findChild<QWidget*>();
    if (child && !child->isWindow()) {
        qWarning("QWinWidget::center: Call this function only for QWinWidgets with toplevel children");
    }
    RECT r;
    ::GetWindowRect(m_parentNativeWindowHandle, &r);
    setGeometry((r.right-r.left)/2+r.left, (r.bottom-r.top)/2+r.top,0,0);
    // todo: Check geometry math, what happens near rounding?
}

/*!
    \obsolete

    Call center() instead.
*/
void QWinWidget::showCentered()
{
    center();
    show();
}

void QWinWidget::setGeometry(int x, int y, int w, int h)
{
    auto dpr = window()->devicePixelRatio();
    m_parentWinNativeWindow->setGeometry(x*dpr, y*dpr, w*dpr, h*dpr);
}

/*!
    Sets the focus to the window that had the focus before this widget
    was shown, or if there was no previous window, sets the focus to
    the parent window.
*/
void QWinWidget::resetFocus()
{
    if (m_prevFocusHandle)
        ::SetFocus(m_prevFocusHandle);
    else
        ::SetFocus(getParentWindow());
}

//Tell the parent native window to minimize
void QWinWidget::onMinimizeButtonClicked()
{
    ::SendMessage(m_parentNativeWindowHandle, WM_SYSCOMMAND, SC_MINIMIZE, 0);
}

//Tell the parent native window to maximize or restore as appropriate
void QWinWidget::onMaximizeButtonClicked()
{
    if (m_widget->maximizeButton->isChecked())  // todo: check this, versus "!::IsZoomed(m_parentNativeWidowHandle)" condition
    {
        ::SendMessage(m_parentNativeWindowHandle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    }
    else
    {
        ::SendMessage(m_parentNativeWindowHandle, WM_SYSCOMMAND, SC_RESTORE, 0);
    }
}

void QWinWidget::onCloseButtonClicked()
{
    close();
}

/*!
    \reimp
*/

void QWinWidget::closeEvent(QCloseEvent *event)
{
    //use closeEvent to check for it whether it is safe to close your app here or not
    ::ShowWindow(m_parentNativeWindowHandle, false);
}

bool QWinWidget::eventFilter(QObject *o, QEvent *e)
{
    auto* w = (QWidget*) o;

    switch (e->type())
    {
        case QEvent::WindowDeactivate:
            if (w->isModal() && w->isHidden()) {
                ::BringWindowToTop(m_parentNativeWindowHandle);
            }
            break;

        case QEvent::Hide:
            if (m_reenableParent) {
                ::EnableWindow(m_parentNativeWindowHandle, true);
                m_reenableParent = false;
            }
            resetFocus();

            if (w->testAttribute(Qt::WA_DeleteOnClose) && w->isWindow()) {
                deleteLater();
            }
            break;

        case QEvent::Show:
            if (w->isWindow()) {
                saveFocus();
                hide();
                if (w->isModal() && !m_reenableParent) {
                    ::EnableWindow(m_parentNativeWindowHandle, false);
                    m_reenableParent = true;
                }
            }
            break;

        case QEvent::Close:
            ::SetActiveWindow(m_parentNativeWindowHandle);
            if (w->testAttribute(Qt::WA_DeleteOnClose)) {
                deleteLater();
            }
            break;

        default:
            break;
    }

    return QWidget::eventFilter(o, e);
}

/*! \reimp
*/
void QWinWidget::focusInEvent(QFocusEvent *e)
{
    QWidget *candidate = this;

    switch (e->reason())
    {
        case Qt::TabFocusReason:
        case Qt::BacktabFocusReason:
            while (!(candidate->focusPolicy() & Qt::TabFocus))
            {
                candidate = candidate->nextInFocusChain();
                if (candidate == this) {
                    candidate = nullptr;
                    break;
                }
            }
            if (candidate) {
                candidate->setFocus(e->reason());
                if (e->reason() == Qt::BacktabFocusReason || e->reason() == Qt::TabFocusReason) {
                    candidate->setAttribute(Qt::WA_KeyboardFocusChange);
                    candidate->window()->setAttribute(Qt::WA_KeyboardFocusChange);
                }
                if (e->reason() == Qt::BacktabFocusReason) {
                    QWidget::focusNextPrevChild(false);
                }
            }
            break;

        default:
            break;
    }
}

/*! \reimp
*/
bool QWinWidget::focusNextPrevChild(bool next)
{
    QWidget *curFocus = focusWidget();
    if (!next) {
        if (!curFocus->isWindow()) {
            QWidget *nextFocus = curFocus->nextInFocusChain();
            QWidget *prevFocus = nullptr;
            QWidget *topLevel = nullptr;
            while (nextFocus != curFocus)
            {
                if (nextFocus->focusPolicy() & Qt::TabFocus) {
                    prevFocus = nextFocus;
                    topLevel = nullptr;
                }
                nextFocus = nextFocus->nextInFocusChain();
            }

            if (!topLevel) {
                return QWidget::focusNextPrevChild(false);
            }
        }
    }
    else {
        QWidget *nextFocus = curFocus;
        while (1 && nextFocus != 0) {
            nextFocus = nextFocus->nextInFocusChain();
            if (nextFocus->focusPolicy() & Qt::TabFocus) {
                return QWidget::focusNextPrevChild(true);
            }
        }
    }

    ::SetFocus(m_parentNativeWindowHandle);

    return true;
}
