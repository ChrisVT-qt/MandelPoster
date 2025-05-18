// FractalImageWidget.cpp
// Class implementation

// Project includes
#include "CallTracer.h"
#include "FractalImageWidget.h"

// Qt includes
#include <QDebug>
#include <QPainter>



// ================================================================== Lifecycle



///////////////////////////////////////////////////////////////////////////////
// Constructor
FractalImageWidget::FractalImageWidget()
{
    CALL_IN("");

    // Not marking a new range
    m_ButtonPressed = false;

    // Not in widget
    m_MouseInWidget = false;

    // We do track mouse position
    setMouseTracking(true);

    CALL_OUT("");
}


///////////////////////////////////////////////////////////////////////////////
// Destructor
FractalImageWidget::~FractalImageWidget()
{
    CALL_IN("");

    // Nothing to do.

    CALL_OUT("");
}



// ===================================================================== Access



///////////////////////////////////////////////////////////////////////////////
// Image
void FractalImageWidget::SetImage(const QImage mcNewImage)
{
    CALL_IN("mcNewImage=...");

    m_Image = mcNewImage;
    setFixedSize(m_Image.width(), m_Image.height());

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Not reacting to mouse activity
void FractalImageWidget::SetNonInteractive(const bool mcNewState)
{
    CALL_IN(QString("mcNewState=%1")
        .arg(CALL_SHOW(mcNewState)));

    m_IsNonInteractive = mcNewState;
    setMouseTracking(!m_IsNonInteractive);

    CALL_OUT("");
}



// ================================================================== GUI stuff



///////////////////////////////////////////////////////////////////////////////
// Redraw
void FractalImageWidget::paintEvent(QPaintEvent * mpEvent)
{
    CALL_IN("mpEvent=...");

    // We're not using the acutal event
    Q_UNUSED(mpEvent);

    // Paint picture first
    QPixmap pixmap = QPixmap::fromImage(m_Image);
    QPainter mypainter(this);
    mypainter.setRenderHint(QPainter::Antialiasing);
    mypainter.setClipRect(rect());
    mypainter.drawPixmap(0, 0, pixmap.width(), pixmap.height(), pixmap);

    // Don't draw anything else if we're rendering
    if (m_IsNonInteractive)
    {
        CALL_OUT("Non-interactive");
        return;
    }

    if (m_ButtonPressed)
    {
        // Show selected area
        mypainter.setOpacity(.4);
        QBrush brush(Qt::SolidPattern);
        brush.setColor(Qt::red);
        const int x0 = qMin(m_SelectionXStart, m_SelectionXStop);
        const int x1 = qMax(m_SelectionXStart, m_SelectionXStop);
        const int y0 = qMin(m_SelectionYStart, m_SelectionYStop);
        const int y1 = qMax(m_SelectionYStart, m_SelectionYStop);
        mypainter.fillRect(x0, y0, x1 - x0, y1 - y0, brush);
    } else
    {
        // Show auxillary lines only if mouse is in widget
        if (m_MouseInWidget)
        {
            mypainter.setOpacity(.4);
            QBrush brush(Qt::SolidPattern);
            brush.setColor(Qt::red);
            mypainter.setBrush(brush);
            mypainter.drawLine(0, m_LineY, pixmap.width(), m_LineY);
            mypainter.drawLine(m_LineX, 0, m_LineX, pixmap.height());
        }
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Mouse events/drag & drop
void FractalImageWidget::mousePressEvent(QMouseEvent * mpEvent)
{
    CALL_IN("mpEvent=...");

    // Not possible if non-interactive or working
    if (m_IsNonInteractive)
    {
        CALL_OUT("Non-interaxctive");
        return;
    }

    // Has to be left mouse button.
    if (mpEvent -> button() != Qt::LeftButton)
    {
        CALL_OUT("Not the left mouse button");
        return;
    }

    // Start marking an area
    m_ButtonPressed = true;
    m_SelectionXStart = mpEvent -> pos().x();
    m_SelectionYStart = mpEvent -> pos().y();
    m_SelectionXStop = m_SelectionXStart;
    m_SelectionYStop = m_SelectionYStart;

    // Refresh
    repaint();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Mouse events/drag & drop
void FractalImageWidget::mouseMoveEvent(QMouseEvent * mpEvent)
{
    CALL_IN("mpEvent=...");

    // Not possible if non-interactive or working
    if (m_IsNonInteractive ||
        m_Image.isNull())
    {
        CALL_OUT("Non-interactive, or no image");
        return;
    }

    // Pixel position
    int x = mpEvent -> pos().x();
    int y = mpEvent -> pos().y();

    // Ignore if mouse button isn't down
    if (m_ButtonPressed)
    {
        // Set new corner
        m_SelectionXStart = qMax(0, qMin(x, width()));
        m_SelectionYStart = qMax(0, qMin(y, height()));
    } else
    {
        m_LineX = x;
        m_LineY = y;
    }

    // Let people know
    emit HoveringAt(x, y);

    // Refresh
    repaint();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Mouse events/drag & drop
void FractalImageWidget::mouseReleaseEvent(QMouseEvent * mpEvent)
{
    CALL_IN("mpEvent=...");

    Q_UNUSED(mpEvent);

    // Not possible if non-interactive or working
    if (m_IsNonInteractive)
    {
        CALL_OUT("Non-interactive");
        return;
    }

    // No more dragging
    m_ButtonPressed = false;
    repaint();

    // New zoom
    const double xmin = qMin(m_SelectionXStart, m_SelectionXStop);
    const double xmax = qMax(m_SelectionXStart, m_SelectionXStop);
    const double ymin = qMin(m_SelectionYStart, m_SelectionYStop);
    const double ymax = qMax(m_SelectionYStart, m_SelectionYStop);

    // Check if we're acutally zooming
    if (xmax - xmin < 10 || ymax - ymin < 10)
    {
        // Do nothing
        CALL_OUT("Area too small");
        return;
    }

    // Zoom in
    emit AreaSelected(xmin, xmax, ymin, ymax);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Mouse entering widget
void FractalImageWidget::enterEvent(QEnterEvent * mpEvent)
{
    CALL_IN("mpEvent=...");

    mpEvent -> accept();
    m_MouseInWidget = true;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Mouse leaving widget
void FractalImageWidget::leaveEvent(QEvent * mpEvent)
{
    CALL_IN("mpEvent=...");

    mpEvent -> accept();
    m_MouseInWidget = false;
    repaint();

    emit MouseLeftWidget();

    CALL_OUT("");
}
