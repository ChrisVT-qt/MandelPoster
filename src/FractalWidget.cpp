// FractalWidget.cpp
// Class implementation

// Project includes
#include "Application.h"
#include "CallTracer.h"
#include "FractalWidget.h"
#include "Fractal.h"
#include "FractalImage.h"
#include "FractalImageWidget.h"
#include "MainWindow.h"
#include "MessageLogger.h"
#include "StringHelper.h"

// Qt includes
#include <QAction>
#include <QCoreApplication>
#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>
#include <QDebug>
#include <QHBoxLayout>
#include <QImage>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QTimer>

// System includes
#include <cmath>

// Border
#define IMAGE_BORDER 40



// ================================================================== Lifecycle



///////////////////////////////////////////////////////////////////////////////
// Constructor
FractalWidget::FractalWidget()
{
    CALL_IN("");

    // Create fractal
    m_Fractal = new Fractal();
    connect (m_Fractal, SIGNAL(NameChanged()),
        this, SLOT(Refresh_Progress()));

    // Create fractal image
    m_FractalImage = new FractalImage();
    connect (m_FractalImage, SIGNAL(PeriodicUpdate()),
        this, SLOT(PerformPeriodicUpdate()));
    connect (m_FractalImage, SIGNAL(Finished()),
        this, SLOT(PerformFinished()));

    // GUI
    InitGUI();

    // GUI is interactive
    SetNonInteractive(false);

    // Is showing image
    m_IsShowingImage = true;

    CALL_OUT("");
}


///////////////////////////////////////////////////////////////////////////////
// Destructor
FractalWidget::~FractalWidget()
{
    CALL_IN("");

    // Nothing to do.

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Initilize given object with data from this object
void FractalWidget::JustLikeThis(FractalWidget * mpFractalWidget) const
{
    CALL_IN("mpFractalWidget=...");

    m_Fractal -> JustLikeThis(mpFractalWidget -> m_Fractal);
    m_FractalImage -> JustLikeThis(mpFractalWidget -> m_FractalImage);
    mpFractalWidget -> m_IsShowingImage = m_IsShowingImage;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Initialize GUI
void FractalWidget::InitGUI()
{
    CALL_IN("");

    // Layout
    QVBoxLayout * layout = new QVBoxLayout();
    setLayout(layout);

    layout -> addStretch(1);

    // Image
    m_FractalImageWidget = new FractalImageWidget();
    m_FractalImageWidget -> setMinimumSize(600,600);
    connect (m_FractalImageWidget, SIGNAL(HoveringAt(const int, const int)),
        this, SLOT(HoveringAtPixel(const int, const int)));
    connect (m_FractalImageWidget,
        SIGNAL(AreaSelected(const int, const int, const int, const int)),
        this,
        SLOT(AreaSelected(const int, const int, const int, const int)));
    connect (m_FractalImageWidget, SIGNAL(MouseLeftWidget()),
        this, SLOT(MouseLeftFractalImage()));
    layout -> addWidget(m_FractalImageWidget, 0, Qt::AlignHCenter);

    // Message
    m_Message = new QLabel();
    layout -> addWidget(m_Message);

    // If not showing image
    m_NoImage = new QLabel(tr("No image preview"));
    QFont huge_font = m_NoImage -> font();
    huge_font.setPointSize(40);
    huge_font.setBold(true);
    m_NoImage -> setFont(huge_font);
    layout -> addWidget(m_NoImage, 0, Qt::AlignHCenter);
    m_NoImage -> hide();

    m_Progress = new QLabel();
    m_Progress -> setFont(huge_font);
    layout -> addWidget(m_Progress, 0, Qt::AlignHCenter);
    m_Progress -> hide();

    layout -> addStretch(1);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Refresh window title
void FractalWidget::Refresh_Progress()
{
    CALL_IN("");

    QString title = m_Fractal -> GetName();
    if (m_FractalImage -> GetRenderStatus() == "working")
    {
        const QHash < QString, QString > statistics =
            m_FractalImage -> GetStatistics();
        const double percent_complete =
            statistics["percent complete"].toDouble();
        const QString completion_txt =
            QString("%1%").arg(QString::number(percent_complete, 'f', 1));

        title += QString(" - %1").arg(completion_txt);
        m_Progress -> setText(completion_txt);
    } else
    {
        m_Progress -> setText("");
    }
    setWindowTitle(title);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Refresh image being shown
void FractalWidget::Refresh_Image()
{
    CALL_IN("");

    // Update image
    if (m_IsShowingImage)
    {
        QImage image = m_FractalImage -> GetImage();
        m_FractalImageWidget -> SetImage(image);
        repaint();
    }

    CALL_OUT("");
}



// ===================================================================== Access



///////////////////////////////////////////////////////////////////////////////
// Get access to worker
Fractal * FractalWidget::GetFractal()
{
    CALL_IN("");

    CALL_OUT("");
    return m_Fractal;
}



///////////////////////////////////////////////////////////////////////////////
// Get access to fractal image
FractalImage * FractalWidget::GetFractalImage()
{
    CALL_IN("");

    CALL_OUT("");
    return m_FractalImage;
}



///////////////////////////////////////////////////////////////////////////////
// Are we showing the image?
bool FractalWidget::IsShowingImage() const
{
    CALL_IN("");

    CALL_OUT("");
    return m_IsShowingImage;
}



///////////////////////////////////////////////////////////////////////////////
// Se if we are showing the image
void FractalWidget::SetShowingImage(const bool mcNewState)
{
    CALL_IN(QString("mcNewState=%1")
        .arg(CALL_SHOW(mcNewState)));

    m_IsShowingImage = mcNewState;
    if (m_IsShowingImage)
    {
        m_FractalImageWidget -> setVisible(true);
        m_Message -> setVisible(true);
        m_NoImage -> setVisible(false);
        m_Progress -> setVisible(false);
    } else
    {
        m_FractalImageWidget -> setVisible(false);
        m_Message -> setVisible(false);
        m_NoImage -> setVisible(true);
        m_Progress -> setVisible(true);
    }

    CALL_OUT("");
}



// ================================================================== Rendering



///////////////////////////////////////////////////////////////////////////////
// Update image
void FractalWidget::UpdateImage()
{
    CALL_IN("");

    // GUI is non-interactive while calculation is running
    SetNonInteractive(true);

    // Get parameters
    QHash < QString, QString > parameters =
        m_Fractal -> GetAllParameters();

    // Save fractal
    if (parameters["storage saving cache data"] == "yes")
    {
        const QString storage_directory = parameters["storage directory"];
        const QString fractal_name = parameters["name"];
        const int width = parameters["actual resolution width"].toInt();
        const int height = parameters["actual resolution height"].toInt();
        const QString directory = QString("%1/%2/%3x%4")
            .arg(storage_directory,
                 fractal_name,
                 QString::number(width),
                 QString::number(height));
        const QString filename = QString("%1/%2.xml")
            .arg(directory,
                 fractal_name);
        m_Fractal -> ToFile(filename);
    }

    // Calculate resolution
    int actual_width;
    int actual_height;
    if (parameters["use fixed resolution"] == "yes")
    {
        actual_width = parameters["fixed resolution width"].toInt();
        actual_height = parameters["fixed resolution height"].toInt();
    } else
    {
        const QPair < int, int > actual_resolution = GetActualImageSize();
        actual_width = actual_resolution.first;
        actual_height = actual_resolution.second;
    }
    parameters["actual resolution width"] =
        QString("%1").arg(actual_width);
    parameters["actual resolution height"] =
        QString("%1").arg(actual_height);

    // Update range if necessary
    const QHash < QString, QString > new_range =
        GetRangeForResolution(actual_width, actual_height);
    for (auto key_iterator = new_range.keyBegin();
         key_iterator != new_range.keyEnd();
         key_iterator++)
    {
        const QString key = *key_iterator;
        parameters[key] = new_range[key];
    }

    // Update title
    Refresh_Progress();
    Refresh_Image();

    // For large pictures, don't show image
    if (parameters["use fixed resolution"] == "yes")
    {
        if (parameters["actual resolution width"].toInt() > 2000 ||
            parameters["actual resolution height"].toInt() > 2000)
        {
            SetShowingImage(false);
        } else
        {
            SetShowingImage(true);
        }
    }

    // Do the work
    m_FractalImage -> Render(parameters);

    // Worker threads will disconnect and return to the main loop immediately,
    // before calculation is acually completed

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Periodic Update
void FractalWidget::PerformPeriodicUpdate()
{
    CALL_IN("");

    // Update title and image
    Refresh_Progress();
    Refresh_Image();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Fractal calculation finished
void FractalWidget::PerformFinished()
{
    CALL_IN("");

    // Final update of image, title and such
    Refresh_Progress();
    Refresh_Image();

    // GUI is interactive again
    SetNonInteractive(false);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Calculate range based on resolution
QHash < QString, QString > FractalWidget::GetRangeForResolution(
    const int mcWidth, const int mcHeight) const
{
    CALL_IN(QString("mcWidth=%1, mcHeight=%2")
        .arg(CALL_SHOW(mcWidth),
             CALL_SHOW(mcHeight)));

    // We'll fit the entire range into an existing resolution with square
    // pixels, and will expand the range to fit all pixels.

    // When we get here, the resolution already has been set, without any
    // or a pre-defined aspect ratio, as this step has nothing to do with
    // the complex range.
    const QHash < QString, QString > parameters =
        m_Fractal -> GetAllParameters();

    // Area in the complex plane
    QHash < QString, QString > parameters_ret;
    if (parameters["precision"] == "long double")
    {
        long double real_min =
            StringHelper::ToLongDouble(parameters["real min"]);
        long double real_max =
            StringHelper::ToLongDouble(parameters["real max"]);
        long double imag_min =
            StringHelper::ToLongDouble(parameters["imag min"]);
        long double imag_max =
            StringHelper::ToLongDouble(parameters["imag max"]);
        const long double area_width = real_max - real_min;
        const long double area_height = imag_max - imag_min;

        const long double res_x = area_width / mcWidth;
        const long double res_y = area_height / mcHeight;
        if (res_x > res_y)
        {
            const long double imag_center = 0.5 * (imag_min + imag_max);
            imag_min = imag_center - 0.5 * res_x * mcHeight;
            imag_max = imag_center + 0.5 * res_x * mcHeight;
        } else
        {
            const long double real_center = 0.5 * (real_min + real_max);
            real_min = real_center - 0.5 * res_y * mcWidth;
            real_max = real_center + 0.5 * res_y * mcWidth;
        }

        // Return area
        parameters_ret["real min"] = StringHelper::ToString(real_min);
        parameters_ret["real max"] = StringHelper::ToString(real_max);
        parameters_ret["imag min"] = StringHelper::ToString(imag_min);
        parameters_ret["imag max"] = StringHelper::ToString(imag_max);
    } else
    {
        double real_min = parameters["real min"].toDouble();
        double real_max = parameters["real max"].toDouble();
        double imag_min = parameters["imag min"].toDouble();
        double imag_max = parameters["imag max"].toDouble();
        const double area_width = real_max - real_min;
        const double area_height = imag_max - imag_min;

        const double res_x = area_width / mcWidth;
        const double res_y = area_height / mcHeight;
        if (res_x > res_y)
        {
            const double imag_center = 0.5 * (imag_min + imag_max);
            imag_min = imag_center - 0.5 * res_x * mcHeight;
            imag_max = imag_center + 0.5 * res_x * mcHeight;
        } else
        {
            const double real_center = 0.5 * (real_min + real_max);
            real_min = real_center - 0.5 * res_y * mcWidth;
            real_max = real_center + 0.5 * res_y * mcWidth;
        }

        // Return area
        parameters_ret["real min"] = QString::number(real_min, 'g', 16);
        parameters_ret["real max"] = QString::number(real_max, 'g', 16);
        parameters_ret["imag min"] = QString::number(imag_min, 'g', 16);
        parameters_ret["imag max"] = QString::number(imag_max, 'g', 16);
    }

    CALL_OUT("");
    return parameters_ret;
}



// ================================================================== GUI stuff



///////////////////////////////////////////////////////////////////////////////
// Make GUI interactive or non-interactive
void FractalWidget::SetNonInteractive(const bool mcNewState)
{
    CALL_IN(QString("mcNewState=%1")
        .arg(CALL_SHOW(mcNewState)));

    m_IsNonInteractive = mcNewState;
    m_FractalImageWidget -> SetNonInteractive(mcNewState);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Hovering above a pixel
void FractalWidget::HoveringAtPixel(const int mcX, const int mcY)
{
    CALL_IN(QString("mcX=%1, mcY=%2")
        .arg(CALL_SHOW(mcX),
             CALL_SHOW(mcY)));

    // Information about this position
    const QHash < QString, QString > range = m_Fractal -> GetRange();
    const QPair < int, int > resolution =
        m_FractalImage -> GetImageResolution();
    QString message;
    if (m_Fractal -> GetPrecision() == "long double")
    {
        const long double real =
            StringHelper::ToLongDouble(range["real min"]) +
            (StringHelper::ToLongDouble(range["real max"]) -
            StringHelper::ToLongDouble(range["real min"])) *
            mcX / (resolution.first - 1.);
        const long double imag =
            StringHelper::ToLongDouble(range["imag max"]) -
            (StringHelper::ToLongDouble(range["imag max"]) -
            StringHelper::ToLongDouble(range["imag min"])) *
            mcY / (resolution.second - 1.);

        message = QString("z = %1 %2 %3i")
            .arg(StringHelper::ToString(real),
                 imag >= 0 ? "+" : "-",
                 StringHelper::ToString(fabs(imag)));
    } else
    {
        const double real = range["real min"].toDouble() +
            (range["real max"].toDouble() - range["real min"].toDouble()) *
            mcX / (resolution.first - 1.);
        const double imag = range["imag max"].toDouble() -
            (range["imag max"].toDouble() - range["imag min"].toDouble()) *
            mcY / (resolution.second - 1.);

        message = QString("z = %1 %2 %3i")
            .arg(QString::number(real, 'g', 20),
                 imag >= 0 ? "+" : "-",
                 QString::number(fabs(imag), 'g', 20));
    }
    const double value = m_FractalImage -> GetColorValueAt(mcX, mcY);
    if (!isnan(value))
    {
        message += tr(", color value = %1").arg(value);
    }
    m_Message -> setText(message);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Area selected
void FractalWidget::AreaSelected(const int mcXMin, const int mcXMax,
    const int mcYMin, const int mcYMax)
{
    CALL_IN(QString("mcXMin=%1, mcXMax=%2, mcYMin=%3, mcYMax=%4")
        .arg(CALL_SHOW(mcXMin),
             CALL_SHOW(mcXMax),
             CALL_SHOW(mcYMin),
             CALL_SHOW(mcYMax)));

    const QHash < QString, QString > range = m_Fractal -> GetRange();
    const QPair < int, int > resolution =
        m_FractalImage -> GetImageResolution();
    if (m_Fractal -> GetPrecision() == "long double")
    {
        const long double real_min =
            StringHelper::ToLongDouble(range["real min"]) +
            (StringHelper::ToLongDouble(range["real max"]) -
            StringHelper::ToLongDouble(range["real min"])) *
            (long double)(mcXMin) / (long double)(resolution.first - 1.);
        const long double real_max =
            StringHelper::ToLongDouble(range["real min"]) +
            (StringHelper::ToLongDouble(range["real max"]) -
            StringHelper::ToLongDouble(range["real min"])) *
            (long double)(mcXMax) / (long double)(resolution.first - 1.);
        const long double imag_min =
            StringHelper::ToLongDouble(range["imag max"]) -
            (StringHelper::ToLongDouble(range["imag max"]) -
            StringHelper::ToLongDouble(range["imag min"])) *
            (long double)(mcYMax) / (long double)(resolution.second - 1.);
        const long double imag_max =
            StringHelper::ToLongDouble(range["imag max"]) -
            (StringHelper::ToLongDouble(range["imag max"]) -
            StringHelper::ToLongDouble(range["imag min"])) *
            (long double)(mcYMin) / (long double)(resolution.second - 1.);

        // Set new range
        emit ChangeRange_HighPrecision(real_min, real_max, imag_min, imag_max);
    } else
    {
        const double real_min = range["real min"].toDouble() +
            (range["real max"].toDouble() - range["real min"].toDouble()) *
            mcXMin / (resolution.first - 1.);
        const double imag_min = range["imag max"].toDouble() -
            (range["imag max"].toDouble() - range["imag min"].toDouble()) *
            mcYMax / (resolution.second - 1.);
        const double real_max = range["real min"].toDouble() +
            (range["real max"].toDouble() - range["real min"].toDouble()) *
            mcXMax / (resolution.first - 1.);
        const double imag_max = range["imag max"].toDouble() -
            (range["imag max"].toDouble() - range["imag min"].toDouble()) *
            mcYMin / (resolution.second - 1.);

        // Set new range
        emit ChangeRange_LowPrecision(real_min, real_max, imag_min, imag_max);
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Mouse left fractal image
void FractalWidget::MouseLeftFractalImage()
{
    CALL_IN("");

    m_Message -> setText("");

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// We use this to detect when window was activated
bool FractalWidget::event(QEvent * mpEvent)
{
    CALL_IN("mpEvent=...");

    if (mpEvent -> type() == QEvent::WindowActivate ||
        mpEvent -> type() == QEvent::ContextMenu)
    {
        emit WindowActivated();
    }

    // Deal with any event the default (base class) way
    CALL_OUT("");
    return QWidget::event(mpEvent);
}



///////////////////////////////////////////////////////////////////////////////
// Resizing the window
void FractalWidget::resizeEvent(QResizeEvent * mpEvent)
{
    CALL_IN("mpEvent=...");

    // Accept event
    mpEvent -> accept();

    // Let everybody know size changed
    emit WindowSizeChanged();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Closing the window
void FractalWidget::closeEvent(QCloseEvent * mpEvent)
{
    CALL_IN("mpEvent=...");

    // Check if we really want to close it.
    const int result = QMessageBox::question(this, tr("Close fractal"),
        tr("Do you really want to close this fractal?"));
    if (result == QMessageBox::Yes)
    {
        emit CloseWindow();
        mpEvent -> accept();
    } else
    {
        mpEvent -> ignore();
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Calculate image size
QPair < int, int > FractalWidget::GetActualImageSize() const
{
    CALL_IN("");

    // Get parameters
    QHash < QString, QString > parameters =
        m_Fractal -> GetAllParameters();

    if (parameters["aspect ratio"] != "any" &&
        parameters["use fixed resolution"] == "no")
    {
        // Resolution will adjust to defined aspect ratio

        double target_ratio = 1.;
        if (parameters["aspect ratio"] == "1:1")
        {
            target_ratio = 1./1.;
        } else if (parameters["aspect ratio"] == "2:1")
        {
            target_ratio = 2./1.;
        } else if (parameters["aspect ratio"] == "3:2")
        {
            target_ratio = 3./2.;
        } else if (parameters["aspect ratio"] == "16:9")
        {
            target_ratio = 16./9.;
        }

        // Update width and height
        const QPair < int, int > resolution = GetAvailableWindowSize();
        int width = resolution.first;
        int height = resolution.second;
        const double res_ratio = width * 1./height;
        if (res_ratio > target_ratio)
        {
            width = height * target_ratio;
        } else
        {
            height = width / target_ratio;
        }

        CALL_OUT("");
        return QPair < int, int >(width, height);
    }

    if (parameters["aspect ratio"] == "any")
    {
        // Resolution will adjust to aspect ration given by range
        double aspect_ratio = 0;
        if (parameters["precision"] == "long double")
        {
            const long double real_min =
                StringHelper::ToLongDouble(parameters["real min"]);
            const long double real_max =
                StringHelper::ToLongDouble(parameters["real max"]);
            const long double imag_min =
                StringHelper::ToLongDouble(parameters["imag min"]);
            const long double imag_max =
                StringHelper::ToLongDouble(parameters["imag max"]);
            aspect_ratio = (real_max - real_min) / (imag_max - imag_min);
        } else
        {
            const double real_min = parameters["real min"].toDouble();
            const double real_max = parameters["real max"].toDouble();
            const double imag_min = parameters["imag min"].toDouble();
            const double imag_max = parameters["imag max"].toDouble();
            aspect_ratio = (real_max - real_min) / (imag_max - imag_min);
        }

        // Available resolution
        const QPair < int, int > resolution = GetAvailableWindowSize();
        int width = resolution.first;
        int height = resolution.second;
        if (width < aspect_ratio * height)
        {
            height = width / aspect_ratio;
        } else
        {
            width = height * aspect_ratio;
        }

        CALL_OUT("");
        return QPair < int, int >(width, height);
    }

    if (parameters["use fixed resolution"] == "yes")
    {
        // Resolution is fixed.
        const int width = parameters["fixed resolution width"].toInt();
        const int height = parameters["fixed resolution height"].toInt();

        CALL_OUT("");
        return QPair < int, int >(width, height);
    }

    // We shouldn't get here
    CALL_OUT("");
    return QPair < int, int >(0,0);
}



///////////////////////////////////////////////////////////////////////////////
// Maximum size of the image according to window size
QPair < int, int > FractalWidget::GetAvailableWindowSize() const
{
    CALL_IN("");

    const int available_width = width() - 2 * IMAGE_BORDER
        - m_Message -> height();
    const int available_height = height() - 2 * IMAGE_BORDER;

    CALL_OUT("");
    return QPair < int, int >(available_width, available_height);
}



///////////////////////////////////////////////////////////////////////////////
// Context menu
void FractalWidget::contextMenuEvent(QContextMenuEvent * mpEvent)
{
    CALL_IN("mpEvent=...");

    // Accept event
    mpEvent -> accept();

    // Not possible if non-interactive or working
    if (m_IsNonInteractive)
    {
        CALL_OUT("Non-interactive");
        return;
    }

    // Position
    QPoint menu_pos = mpEvent -> pos();
    m_Context_PixelX = menu_pos.x() - m_FractalImageWidget -> x();
    m_Context_PixelY = menu_pos.y() - m_FractalImageWidget -> y();

    // Create menu
    QMenu * menu = new QMenu();
    QAction * action;

    action = new QAction(tr("Zoom out"), this);
    connect (action, SIGNAL(triggered()),
        this, SLOT(Context_ZoomOut()));
    menu -> addAction(action);

    action = new QAction(tr("Zoom out all the way"), this);
    connect (action, SIGNAL(triggered()),
        this, SLOT(Context_ZoomOutAllTheWay()));
    menu -> addAction(action);

    action = new QAction(tr("Center here"), this);
    connect (action, SIGNAL(triggered()),
        this, SLOT(Context_CenterHere()));
    menu -> addAction(action);

    action = new QAction(tr("Create Julia"), this);
    const QString fractal_type = m_Fractal -> GetFractalType();
    action -> setEnabled(fractal_type == "mandel");
    connect (action, SIGNAL(triggered()),
        this, SLOT(Context_CreateJuliaSet()));
    menu -> addAction(action);

    menu_pos = mapToGlobal(menu_pos);
    menu -> exec(menu_pos);

    // Delete it.
    delete menu;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Zoom out
void FractalWidget::Context_ZoomOut()
{
    CALL_IN("");

    // New Range
    const QHash < QString, QString > range = m_Fractal -> GetRange();
    if (m_Fractal -> GetPrecision() == "long double")
    {
        const long double real_range =
            StringHelper::ToLongDouble(range["real max"]) -
            StringHelper::ToLongDouble(range["real min"]);
        const long double real_min =
            StringHelper::ToLongDouble(range["real min"]) - real_range/2;
        const long double real_max =
            StringHelper::ToLongDouble(range["real max"]) + real_range/2;
        const long double imag_range =
            StringHelper::ToLongDouble(range["imag max"]) -
            StringHelper::ToLongDouble(range["imag min"]);
        const long double imag_min =
            StringHelper::ToLongDouble(range["imag min"]) - imag_range/2;
        const long double imag_max =
            StringHelper::ToLongDouble(range["imag max"]) + imag_range/2;
        emit ChangeRange_HighPrecision(real_min, real_max, imag_min, imag_max);
    } else
    {
        const double real_range =
            range["real max"].toDouble() - range["real min"].toDouble();
        const double real_min = range["real min"].toDouble() - real_range/2;
        const double real_max = range["real max"].toDouble() + real_range/2;
        const double imag_range =
            range["imag max"].toDouble() - range["imag min"].toDouble();
        const double imag_min = range["imag min"].toDouble() - imag_range/2;
        const double imag_max = range["imag max"].toDouble() + imag_range/2;
        emit ChangeRange_LowPrecision(real_min, real_max, imag_min, imag_max);
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Zoom out all the way
void FractalWidget::Context_ZoomOutAllTheWay()
{
    CALL_IN("");

    // New Range
    const QString fractal_type = m_Fractal -> GetFractalType();
    if (fractal_type == "mandel")
    {
        if (m_Fractal -> GetPrecision() == "long double")
        {
            emit ChangeRange_HighPrecision(-2.2L, 0.7L, -1.3L, 1.3L);
        } else
        {
            emit ChangeRange_LowPrecision(-2.2, 0.7, -1.3, 1.3);
        }
    }
    if (fractal_type == "julia")
    {
        if (m_Fractal -> GetPrecision() == "long double")
        {
            emit ChangeRange_HighPrecision(-2.L, 2.L, -2.L, 2.L);
        } else
        {
            emit ChangeRange_LowPrecision(-2., 2., -2., 2.);
        }
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Center here
void FractalWidget::Context_CenterHere()
{
    CALL_IN("");

    // New center
    const QHash < QString, QString > range = m_Fractal -> GetRange();
    const QPair < int, int > resolution =
        m_FractalImage -> GetImageResolution();
    if (m_Fractal -> GetPrecision() == "long double")
    {
        const long double center_real =
            StringHelper::ToLongDouble(range["real min"]) +
            (StringHelper::ToLongDouble(range["real max"]) -
            StringHelper::ToLongDouble(range["real min"])) *
            m_Context_PixelX / (resolution.first - 1.L);
        const long double center_imag =
            StringHelper::ToLongDouble(range["imag max"]) -
            (StringHelper::ToLongDouble(range["imag max"]) -
            StringHelper::ToLongDouble(range["imag min"])) *
            m_Context_PixelY / (resolution.second - 1.L);

        // New range
        const long double real_range =
            StringHelper::ToLongDouble(range["real max"]) -
            StringHelper::ToLongDouble(range["real min"]);
        const long double real_min = center_real - real_range/2;
        const long double real_max = center_real + real_range/2;
        const long double imag_range =
            StringHelper::ToLongDouble(range["imag max"]) -
            StringHelper::ToLongDouble(range["imag min"]);
        const long double imag_min = center_imag - imag_range/2;
        const long double imag_max = center_imag + imag_range/2;
        emit ChangeRange_HighPrecision(real_min, real_max, imag_min, imag_max);
    } else
    {
        const double center_real = range["real min"].toDouble() +
            (range["real max"].toDouble() - range["real min"].toDouble()) *
            m_Context_PixelX / (resolution.first - 1.);
        const double center_imag = range["imag max"].toDouble() -
            (range["imag max"].toDouble() - range["imag min"].toDouble()) *
            m_Context_PixelY / (resolution.second - 1.);

        // New range
        const double real_range =
            range["real max"].toDouble() - range["real min"].toDouble();
        const double real_min = center_real - real_range/2;
        const double real_max = center_real + real_range/2;
        const double imag_range =
            range["imag max"].toDouble() - range["imag min"].toDouble();
        const double imag_min = center_imag - imag_range/2;
        const double imag_max = center_imag + imag_range/2;
        emit ChangeRange_LowPrecision(real_min, real_max, imag_min, imag_max);
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Create Julia set
void FractalWidget::Context_CreateJuliaSet()
{
    CALL_IN("");

    // Julia constant
    const QHash < QString, QString > range = m_Fractal -> GetRange();
    const QPair < int, int > resolution =
        m_FractalImage -> GetImageResolution();
    if (m_Fractal -> GetPrecision() == "long double")
    {
        const long double julia_real =
            StringHelper::ToLongDouble(range["real min"]) +
            (StringHelper::ToLongDouble(range["real max"]) -
            StringHelper::ToLongDouble(range["real min"])) *
            m_Context_PixelX / (resolution.first - 1.);
        const long double julia_imag =
            StringHelper::ToLongDouble(range["imag max"]) -
            (StringHelper::ToLongDouble(range["imag max"]) -
            StringHelper::ToLongDouble(range["imag min"])) *
            m_Context_PixelY / (resolution.second - 1.);

        // Open Julia set in a new instance
        MainWindow * mw = MainWindow::Instance();
        mw -> NewJuliaSet(julia_real, julia_imag);
    } else
    {
        const double julia_real = range["real min"].toDouble() +
            (range["real max"].toDouble() - range["real min"].toDouble()) *
            m_Context_PixelX / (resolution.first - 1.);
        const double julia_imag = range["imag max"].toDouble() -
            (range["imag max"].toDouble() - range["imag min"].toDouble()) *
            m_Context_PixelY / (resolution.second - 1.);

        // Open Julia set in a new instance
        MainWindow * mw = MainWindow::Instance();
        mw -> NewJuliaSet(julia_real, julia_imag);
    }

    CALL_OUT("");
}
