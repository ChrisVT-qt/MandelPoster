// FractalImageWidget.h
// Class definition

#ifndef FRACTALIMAGEWIDGET_H
#define FRACTALIMAGEWIDGET_H

// Qt includes
#include <QEvent>
#include <QImage>
#include <QMouseEvent>
#include <QWidget>

// Forward declaration

// Class definition
class FractalImageWidget
    : public QWidget
{
    Q_OBJECT



    // ============================================================== Lifecycle
public:
    // Constructor
    FractalImageWidget();

    // Destructor
    virtual ~FractalImageWidget();



    // ================================================================= Access
public:
    // Image
    void SetImage(const QImage mcNewImage);
private:
    QImage m_Image;

public:
    // Not reacting to mouse activity
    void SetNonInteractive(const bool mcNewState);
private:
    bool m_IsNonInteractive;



    // ============================================================== GUI stuff
protected slots:
    // Redraw
    void paintEvent(QPaintEvent * mpEvent);

    // Mouse events
    void mousePressEvent(QMouseEvent * mpEvent);
    void mouseMoveEvent(QMouseEvent * mpEvent);
    void mouseReleaseEvent(QMouseEvent * mpEvent);
    virtual void enterEvent(QEnterEvent * mpEvent);
    virtual void leaveEvent(QEvent * mpEvent);

private:
    bool m_MouseInWidget;
    bool m_ButtonPressed;

    // Button pressed: selection
    int m_SelectionXStart;
    int m_SelectionYStart;
    int m_SelectionXStop;
    int m_SelectionYStop;

    // Button not pressed: auxillary lines
    int m_LineX;
    int m_LineY;

signals:
    // Hovering
    void HoveringAt(const int mcX, const int mcY);

    // Area selected
    void AreaSelected(const int mcXMin, const int mcXMax, const int mcYMin,
        const int mcYMax);

    // Mouse pointer left widget
    void MouseLeftWidget();
};

#endif
