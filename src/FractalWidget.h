// FractalWidget.h
// Class definition

#ifndef FRACTALWIDGET_H
#define FRACTALWIDGET_H

// Qt includes
#include <QElapsedTimer>
#include <QMouseEvent>
#include <QLabel>
#include <QThread>
#include <QVector>
#include <QWidget>

// Forward declaration
class Fractal;
class FractalImage;
class FractalImageWidget;

// Class definition
class FractalWidget
    : public QWidget
{
    Q_OBJECT



    // ============================================================== Lifecycle
public:
    // Constructor
    FractalWidget();

    // Destructor
    virtual ~FractalWidget();

    // Initilize given object with data from this object
    void JustLikeThis(FractalWidget * mpFractalImage) const;

private:
    // GUI
    void InitGUI();

    FractalImageWidget * m_FractalImageWidget;
    QLabel * m_Message;

    QLabel * m_NoImage;
    QLabel * m_Progress;

private slots:
    //  Refresh
    void Refresh_Progress();
    void Refresh_Image();

signals:
    // Update statistics
    void ShouldPerformUpdate(FractalWidget * mpFactalWidget);



    // ========================================================== Serialization
public:
    // Save to XML
    QString ToXML() const;

    // Read from XML
    bool FromXML(const QString mcXML);



    // ================================================================= Access
public:
    // Get access to fractal
    Fractal * GetFractal();
private:
    Fractal * m_Fractal;
    QThread * m_WorkerThread;

public:
    // Get access to fractal image
    FractalImage * GetFractalImage();
private:
    FractalImage * m_FractalImage;

public:
    // No image
    bool IsShowingImage() const;
    void SetShowingImage(const bool mcNewState);
private:
    bool m_IsShowingImage;



    // ============================================================== Rendering
public:
    // Update image
    void UpdateImage();

private slots:
    // Periodic Update
    void PerformPeriodicUpdate();

    // Fractal calculation finished
    void PerformFinished();

public:
    // Calculate range based on resolution
    QHash < QString, QString > GetRangeForResolution(const int mcWidth,
        const int mcHeight) const;



    // ============================================================== GUI stuff
public:
    // Make GUI non-interactive
    void SetNonInteractive(const bool mcNewState);
private:
    bool m_IsNonInteractive;

private slots:
    // Hovering above a pixel
    void HoveringAtPixel(const int mcX, const int mcY);

    // Area selected
    void AreaSelected(const int mcXMin, const int mcXMax,
        const int mcYMin, const int mcYMax);

    // Mouse left fractal image
    void MouseLeftFractalImage();

protected:
    // We use this to detect when window was activated
    bool event(QEvent * mpEvent);

    // Resizing the window
    void resizeEvent(QResizeEvent * mpEvent);

    // Closing the window
    void closeEvent(QCloseEvent * mpEvent);

signals:
    // Window was activated
    void WindowActivated();

    // New image width/height
    void WindowSizeChanged();

    // Change range
    void ChangeRange_LowPrecision(const double mcNewRealMin,
        const double mcNewRealMax, const double mcNewImagMin,
        const double mcNewImagMax);
    void ChangeRange_HighPrecision(const long double mcNewRealMin,
        const long double mcNewRealMax, const long double mcNewImagMin,
        const long double mcNewImagMax);

    // Close window
    void CloseWindow();

public:
    // Calculate image size
    QPair < int, int > GetActualImageSize() const;

private:
   // Get available window size
    QPair < int, int > GetAvailableWindowSize() const;

private slots:
    // Context menu
    virtual void contextMenuEvent(QContextMenuEvent * mpEvent);
private:
    int m_Context_PixelX;
    int m_Context_PixelY;

private slots:
    // Zoom out
    void Context_ZoomOut();

    // Zoom out all the way
    void Context_ZoomOutAllTheWay();

    // Center here
    void Context_CenterHere();

    // Create Julia set
    void Context_CreateJuliaSet();
};

#endif
