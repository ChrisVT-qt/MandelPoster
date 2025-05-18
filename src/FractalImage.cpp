// FractalImage.cpp
// Class implementation

// Project includes
#include "CallTracer.h"
#include "FractalImage.h"
#include "FractalWorker.h"
#include "MessageLogger.h"
#include "StringHelper.h"

// Qt includes
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QPainter>
#include <QPixmap>
#include <QThread>

// System include
#include <cmath>

#define TILE_SIZE 100
#define UPDATE_FREQUENCY 500



// ================================================================== Lifecycle



///////////////////////////////////////////////////////////////////////////////
// Constructor
FractalImage::FractalImage()
{
    CALL_IN("");

    // Not running, not stopping
    m_IsWorking = false;
    m_IsStopped = false;

    // Reset statistics
    ResetStatistics();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Destructor
FractalImage::~FractalImage()
{
    CALL_IN("");

    // Nothing to do

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Initilize given object with data from this object
void FractalImage::JustLikeThis(FractalImage * mpFractalImage) const
{
    CALL_IN("mpFractalImage=...");

    Q_UNUSED(mpFractalImage);

    // Nothing to do, really. Will all be initialized upon re-render.

    CALL_OUT("");
}



// ===================================================================== Access



///////////////////////////////////////////////////////////////////////////////
// Get image
QImage FractalImage::GetImage() const
{
    CALL_IN("");

    CALL_OUT("");
    return m_Image.toImage();
}



///////////////////////////////////////////////////////////////////////////////
// Resolution
QPair < int, int > FractalImage::GetImageResolution() const
{
    CALL_IN("");

    CALL_OUT("");
    return QPair < int, int >(m_Image.width(), m_Image.height());
}



// ================================================================== Rendering



///////////////////////////////////////////////////////////////////////////////
// Start to render something
void FractalImage::Render(const QHash < QString, QString > mcParameters)
{
    CALL_IN(QString("mcParameters=%1")
        .arg(CALL_SHOW(mcParameters)));

    // Abbreviation
    const int width = mcParameters["actual resolution width"].toInt();
    const int height = mcParameters["actual resolution height"].toInt();

    // Check if parameter set invalidates storage
    bool invalidate_cache = false;
    if (!m_Parameters.isEmpty())
    {
        // We don't clear out the disk cache if we just opened a fractal
        if (WillParametersInvalidateCache(mcParameters))
        {
            // If parameter changes don't change cached values, don't clear
            // out the disk space!
            invalidate_cache = true;
        }
    }

    // Set new parameters
    m_Parameters = mcParameters;

    // Actual initialization
    if (invalidate_cache)
    {
        InvalidateCache();
        ResetStatistics();
        m_Image = QPixmap();
    }
    if (m_Image.isNull())
    {
        // Recreate image
        m_Image = QPixmap(width, height);
        m_Image.fill(QColor(192, 192, 192));
        emit PeriodicUpdate();
    }

    // Set up cache if necessary
    if (m_TileIDToPointXMin.isEmpty())
    {
        m_NumberOfTiles = 0;
        for (int pixel_y = 0; pixel_y < height; pixel_y += TILE_SIZE)
        {
            const int tile_height = qMin(height - pixel_y, TILE_SIZE);
            for (int pixel_x = 0; pixel_x < width; pixel_x += TILE_SIZE)
            {
                const int tile_width = qMin(width - pixel_x, TILE_SIZE);
                m_TileIDToPointXMin[m_NumberOfTiles] = pixel_x;
                m_TileIDToPointXMax[m_NumberOfTiles] = pixel_x + tile_width;
                m_TileIDToPointYMin[m_NumberOfTiles] = pixel_y;
                m_TileIDToPointYMax[m_NumberOfTiles] = pixel_y + tile_height;
                m_NumberOfTiles++;
            }
        }
    }

    // We're rendering
    m_IsWorking = true;
    m_IsStopped = false;

    // Statistics stuff
    m_Statistics_StartTime =
        QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // We just started
    emit Started();
    m_UpdateTimer.restart();

    // Kick off worker threads
    m_CurrentTile = 0;
    // !!! Should this be used?
    // !!! QHash < QString, QString > this_parameters = m_Parameters;
    int workers_started = 0;
    while (workers_started < QThread::idealThreadCount() - 1)
    {
        LaunchWorker();
        workers_started++;
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Stop rendering
void FractalImage::Stop()
{
    CALL_IN("");

    m_IsStopped = true;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Launch a new worker
void FractalImage::LaunchWorker()
{
    CALL_IN("");

    // Check for no more work
    if (m_CurrentTile >= m_NumberOfTiles ||
        m_IsStopped)
    {
        if (m_TileIDToWorker.isEmpty())
        {
            // We're done!
            m_IsWorking = false;
            m_Statistics_FinishTime =
                QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            if (m_Parameters["storage save picture"] == "yes")
            {
                SavePicture();
            }
            if (m_Parameters["storage save statistics"] == "yes")
            {
                SaveStatistics();
            }
            emit PeriodicUpdate();
            emit Finished();
        }

        CALL_OUT("Done");
        return;
    }

    // There's more work.
    const int tile_id = m_CurrentTile++;

    // Initialize parameter set for this time
    QHash < QString, QString > parameters = m_Parameters;
    parameters["tile id"] = QString("%1").arg(tile_id);
    parameters["total pixel width"] = parameters["actual resolution width"];
    parameters["total pixel height"] = parameters["actual resolution height"];
    parameters["pixel x min"] =
        QString("%1").arg(m_TileIDToPointXMin[tile_id]);
    parameters["pixel x max"] =
        QString("%1").arg(m_TileIDToPointXMax[tile_id]);
    parameters["pixel y min"] =
        QString("%1").arg(m_TileIDToPointYMin[tile_id]);
    parameters["pixel y max"] =
        QString("%1").arg(m_TileIDToPointYMax[tile_id]);

    // Create new worker
    FractalWorker * worker = new FractalWorker();
    m_TileIDToWorker[tile_id] = worker;
    worker -> Prepare(parameters);
    connect (worker, SIGNAL(Finished(const int)),
        this, SLOT(WorkerFinished(const int)));

    // Check if we can read the tile data
    if (m_Parameters["storage save cache data to disk"] == "yes")
    {
        // May or may not work
        ReadCacheData(tile_id);
    }

    // Let's see if we already have cached values for this tile
    if (m_TileIDToColorData.contains(tile_id))
    {
        worker -> SetCacheValues(m_TileIDToColorData[tile_id],
            m_TileIDToBrightnessData[tile_id]);
    }

    // Move worker to its own thread
    QThread * thread = new QThread();
    m_TileIDToWorkerThread[tile_id] = thread;
    connect (thread, SIGNAL(started()),
        worker, SLOT(Start()));
    worker -> moveToThread(thread);

    // Start thread
    thread -> start();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Store results from a finished worker
void FractalImage::WorkerFinished(const int mcTileID)
{
    CALL_IN(QString("mcTileID=%1")
        .arg(CALL_SHOW(mcTileID)));

    // Lock while processing
    m_Mutex.lock();

    // Image portion
    FractalWorker * worker = m_TileIDToWorker[mcTileID];
    {
        QPainter painter(&m_Image);
        QImage tile_image = worker -> GetImage();
        painter.drawImage(m_TileIDToPointXMin[mcTileID],
            m_TileIDToPointYMin[mcTileID], tile_image);
    }

    // Cache data
    if (m_Parameters["storage save cache data to disk"] == "yes")
    {
        SaveCacheData(mcTileID, worker -> GetColorData(),
            worker -> GetBrightnessData());
    }
    if (m_Parameters["storage save cache data to memory"] == "yes")
    {
        m_TileIDToColorData[mcTileID] = worker -> GetColorData();
        m_TileIDToBrightnessData[mcTileID] = worker -> GetBrightnessData();
    } else
    {
        m_TileIDToColorData.remove(mcTileID);
        m_TileIDToBrightnessData.remove(mcTileID);
    }

    // Collect statistics
    AddToStatistics(worker -> GetStatistics());

    // Unlock
    m_Mutex.unlock();

    // Trigger update
    if (m_UpdateTimer.elapsed() > UPDATE_FREQUENCY)
    {
        emit PeriodicUpdate();
        m_UpdateTimer.restart();
    }

    // End thread
    m_TileIDToWorker[mcTileID]-> deleteLater();
    m_TileIDToWorker.remove(mcTileID);

    m_TileIDToWorkerThread[mcTileID] -> quit();
    m_TileIDToWorkerThread[mcTileID] -> wait();
    m_TileIDToWorkerThread[mcTileID] -> deleteLater();
    m_TileIDToWorkerThread.remove(mcTileID);

    // Start a new worker
    LaunchWorker();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Save cache data to a file
void FractalImage::SaveCacheData(const int mcTileID,
    const QVector < double > & mcrColorData,
    const QVector < double > & mcrBrightnessData) const
{
    CALL_IN(QString("mcTileID=%1, mcrColorData=%2, mcrBrightnessData=%3")
        .arg(CALL_SHOW(mcTileID),
             CALL_SHOW(mcrColorData),
             CALL_SHOW(mcrBrightnessData)));

    // Base path for this fractal
    const QString storage_directory = m_Parameters["storage directory"];
    const QString fractal_name = m_Parameters["name"];
    const int width = m_Parameters["actual resolution width"].toInt();
    const int height = m_Parameters["actual resolution height"].toInt();
    const QString directory = QString("%1/%2/%3x%4/cache")
        .arg(storage_directory).arg(fractal_name).arg(width).arg(height);
    const QString filename =
        QString("%1/tile_%2.bin").arg(directory).arg(mcTileID);

    // Don't do anything if file already exists (in which case we just read
    // from it!)
    if (QFile::exists(filename))
    {
        CALL_OUT(tr("Saving data file unnecessary."));
        return;
    }

    // Save cache data
    QDir().mkpath(directory);
    QFile out_file(filename);
    if (!out_file.open(QFile::WriteOnly))
    {
        const QString reason =
            tr("Could not open cache data \"%1\" file for saving.")
                .arg(filename);
        MessageLogger::Error(CALL_METHOD,
            reason);
        CALL_OUT(reason);
        return;
    }
    QDataStream out_stream(&out_file);
    out_stream << mcrColorData;
    out_stream << mcrBrightnessData;
    out_file.close();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Read cache data to a file
void FractalImage::ReadCacheData(const int mcTileID)
{
    CALL_IN(QString("mcTileID=%1")
        .arg(CALL_SHOW(mcTileID)));

    // Base path for this fractal
    const QString storage_directory = m_Parameters["storage directory"];
    const QString fractal_name = m_Parameters["name"];
    const int width = m_Parameters["actual resolution width"].toInt();
    const int height = m_Parameters["actual resolution height"].toInt();
    const QString directory = QString("%1/%2/%3x%4/cache")
        .arg(storage_directory,
             fractal_name,
             QString::number(width),
             QString::number(height));

    // Read cache data
    const QString filename = QString("%1/tile_%2.bin")
        .arg(directory,
             QString::number(mcTileID));
    QFile in_file(filename);
    if (!in_file.open(QFile::ReadOnly))
    {
        CALL_OUT("No cache data for reading.");
        return;
    }
    QDataStream in_stream(&in_file);
    in_stream >> m_TileIDToColorData[mcTileID];
    in_stream >> m_TileIDToBrightnessData[mcTileID];
    in_file.close();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Save picture
void FractalImage::SavePicture() const
{
    CALL_IN("");

    // Base path for this fractal
    const QString storage_directory = m_Parameters["storage directory"];
    const QString fractal_name = m_Parameters["name"];
    const int width = m_Parameters["actual resolution width"].toInt();
    const int height = m_Parameters["actual resolution height"].toInt();
    const QString directory = QString("%1/%2/%3x%4")
        .arg(storage_directory,
             fractal_name,
             QString::number(width),
             QString::number(height));
    QDir().mkpath(directory);

    // Save picture
    const QString filename = QString("%1/%2.png")
        .arg(directory,
             fractal_name);

    // Save it.
    m_Image.save(filename, "png");

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Save statistics
void FractalImage::SaveStatistics() const
{
    CALL_IN("");

    // Base path for this fractal
    const QString storage_directory = m_Parameters["storage directory"];
    const QString fractal_name = m_Parameters["name"];
    const int width = m_Parameters["actual resolution width"].toInt();
    const int height = m_Parameters["actual resolution height"].toInt();
    const QString directory = QString("%1/%2/%3x%4")
        .arg(storage_directory).arg(fractal_name).arg(width).arg(height);
    QDir().mkpath(directory);

    // Save statistics
    // !!!

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Check if new parameters invalidate cache
bool FractalImage::WillParametersInvalidateCache(
    const QHash < QString, QString > mcParameters) const
{
    CALL_IN(QString("mcParameters=%1")
        .arg(CALL_SHOW(mcParameters)));

    // Relevant parameters which will cause invalidation of cache if changed
    QList < QString > relevant_parameters;
    relevant_parameters << "fractal type" << "real min" << "real max" <<
        "imag min" << "imag max" << "depth" << "escape radius" <<
        "oversampling" << "julia real" << "julia imag" << "color base value" <<
        "brightness fold change" << "brightness regularity" <<
        "brightness exponent" << "actual resolution width" <<
        "actual resolution height" << "precision";

    // Check them out
    for (const QString & parameter : relevant_parameters)
    {
        if (mcParameters.contains(parameter) &&
            m_Parameters.contains(parameter) &&
            mcParameters[parameter] != m_Parameters[parameter])
        {
            CALL_OUT("");
            return true;
        }
    }

    // Special case: Switching the brightness value to "flat" will not
    // invalidate the cache
    if (mcParameters["brightness value"] == "flat")
    {
        CALL_OUT("");
        return false;
    }

    // Any other change will
    if (mcParameters["brightness value"] != m_Parameters["brightness value"])
    {
        CALL_OUT("");
        return true;
    }

    // Otherwise, caches are safe!
    CALL_OUT("");
    return false;
}



///////////////////////////////////////////////////////////////////////////////
// Invalidate the cache
void FractalImage::InvalidateCache()
{
    CALL_IN("");

    // Remove from memory
    m_TileIDToPointXMin.clear();
    m_TileIDToPointXMax.clear();
    m_TileIDToPointYMin.clear();
    m_TileIDToPointYMax.clear();
    m_TileIDToColorData.clear();
    m_TileIDToBrightnessData.clear();
    m_NumberOfTiles = 0;
    m_CurrentTile = 0;

    // Remove from disk
    // Base path for this fractal
    const QString storage_directory = m_Parameters["storage directory"];
    const QString fractal_name = m_Parameters["name"];
    const int width = m_Parameters["actual resolution width"].toInt();
    const int height = m_Parameters["actual resolution height"].toInt();
    const QString directory = QString("%1/%2/%3x%4/cache")
        .arg(storage_directory).arg(fractal_name).arg(width).arg(height);
    int tile_id = 0;
    while (true)
    {
        const QString filename =
            QString("%1/tile_%2.bin").arg(directory).arg(tile_id);
        if (!QFile::exists(filename))
        {
            break;
        }
        QFile::remove(filename);
        tile_id++;
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Render status
QString FractalImage::GetRenderStatus() const
{
    CALL_IN("");

    if (m_IsStopped)
    {
        CALL_OUT("");
        return "stopped";
    }
    if (m_IsWorking)
    {
        CALL_OUT("");
        return "working";
    }

    CALL_OUT("");
    return "idle";
}



///////////////////////////////////////////////////////////////////////////////
// Color value at a particular position
double FractalImage::GetColorValueAt(const int mcPixelX, const int mcPixelY)
{
    CALL_IN(QString("mcPixelX=%1, mcPixelY=%2")
        .arg(CALL_SHOW(mcPixelX),
             CALL_SHOW(mcPixelY)));

    // Determine tile index
    int x = int(mcPixelX / TILE_SIZE);
    int loc_x = mcPixelX % TILE_SIZE;
    if (loc_x > 0)
    {
        x++;
    }
    int y = int(mcPixelY / TILE_SIZE);
    int loc_y = mcPixelY % TILE_SIZE;
    if (loc_y > 0)
    {
        y++;
    }
    const int width = m_Parameters["actual resolution width"].toInt();
    int tiles_per_row = int(width / TILE_SIZE);
    if (width % TILE_SIZE > 0)
    {
        tiles_per_row++;
    }
    const int tile_id = y * tiles_per_row + x;

    // Find value
    if (!m_TileIDToColorData.contains(tile_id))
    {
        CALL_OUT("No data for pixel");
        return NAN;
    }

    // !!! Determine value for pixel
    const double value = NAN;

    CALL_OUT("");
    return value;
}



///////////////////////////////////////////////////////////////////////////////
// Brightness value at a particular position
double FractalImage::GetBrightnessValueAt(const int mcPixelX,
    const int mcPixelY)
{
    CALL_IN(QString("mcPixelX=%1, mcPixelY=%2")
        .arg(CALL_SHOW(mcPixelX),
             CALL_SHOW(mcPixelY)));

    // !!! Determine value for pixel
    Q_UNUSED(mcPixelX);
    Q_UNUSED(mcPixelY);
    const double value = NAN;

    CALL_OUT("");
    return value;
}



// ========================================================== Render statistics



///////////////////////////////////////////////////////////////////////////////
// Get all statistics
QHash < QString, QString > FractalImage::GetStatistics() const
{
    CALL_IN("");

    QHash < QString, QString > statistics;
    statistics["start time"] = m_Statistics_StartTime;
    statistics["finish time"] = m_Statistics_FinishTime;
    statistics["processing time ms"] =
        QString("%1").arg(m_Statistics_ProcessingTime_ms);
    statistics["number of threads"] =
        QString("%1").arg(QThread::idealThreadCount() - 1);

    const int width = m_Parameters["actual resolution width"].toInt();
    const int height = m_Parameters["actual resolution height"].toInt();
    const int total_pixels = width * height;
    statistics["total pixels long"] = QString("%1").arg(total_pixels);
    statistics["total pixels short"] =
        StringHelper::ConvertNumber(total_pixels);

    // Calculate total points
    const qint64 oversampling = m_Parameters["oversampling"].toLongLong();
    const qint64 total_points = total_pixels * oversampling * oversampling;
    statistics["total points long"] = QString("%1").arg(total_points);
    statistics["total points short"] =
        StringHelper::ConvertNumber(total_points);

    statistics["points finished long"] =
        QString("%1").arg(m_Statistics_PointsFinished);
    statistics["points finished short"] =
        StringHelper::ConvertNumber(m_Statistics_PointsFinished);

    statistics["points in set long"] =
        QString("%1").arg(m_Statistics_PointsInSet);
    statistics["points in set short"] =
        StringHelper::ConvertNumber(m_Statistics_PointsInSet);

    statistics["points out of bounds long"] =
        QString("%1").arg(m_Statistics_PointsOutOfBounds);
    statistics["points out of bounds short"] =
        StringHelper::ConvertNumber(m_Statistics_PointsOutOfBounds);

    statistics["total iterations long"] =
        QString("%1").arg(m_Statistics_TotalIterations);
    statistics["total iterations short"] =
        StringHelper::ConvertNumber(m_Statistics_TotalIterations);

    statistics["percent complete"] = QString("%1")
        .arg(double(m_Statistics_PointsFinished)/double(total_points) * 100.);

    statistics["min depth"] = QString("%1").arg(m_Statistics_MinDepth);
    statistics["max depth"] = QString("%1").arg(m_Statistics_MaxDepth);

    if (isnan(m_Statistics_MinColorValue) ||
        isnan(m_Statistics_MaxColorValue))
    {
        statistics["min color value"] = "";
        statistics["max color value"] = "";
    } else
    {
        statistics["min color value"] =
            QString("%1").arg(m_Statistics_MinColorValue);
        statistics["max color value"] =
            QString("%1").arg(m_Statistics_MaxColorValue);
    }

    if (isnan(m_Statistics_MinBrightnessValue) ||
        isnan(m_Statistics_MaxBrightnessValue))
    {
        statistics["min brightness value"] = "";
        statistics["max brightness value"] = "";
    } else
    {
        statistics["min brightness value"] =
            QString("%1").arg(m_Statistics_MinBrightnessValue);
        statistics["max brightness value"] =
            QString("%1").arg(m_Statistics_MaxBrightnessValue);
    }

    CALL_OUT("");
    return statistics;
}



///////////////////////////////////////////////////////////////////////////////
// Reset statisics
void FractalImage::ResetStatistics()
{
    CALL_IN("");

    m_Statistics_FirstTile = true;
    m_Statistics_StartTime = "";
    m_Statistics_FinishTime = "";
    m_Statistics_ProcessingTime_ms = 0;
    m_Statistics_PointsFinished = 0;
    m_Statistics_PointsInSet = 0;
    m_Statistics_PointsOutOfBounds = 0;
    m_Statistics_TotalIterations = 0;
    m_Statistics_MinDepth = 0;
    m_Statistics_MaxDepth = 0;
    m_Statistics_MinColorValue = NAN;
    m_Statistics_MaxColorValue = NAN;
    m_Statistics_MinBrightnessValue = NAN;
    m_Statistics_MaxBrightnessValue = NAN;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Add to statistics
void FractalImage::AddToStatistics(
    const QHash < QString, QString > mcTileStatistics)
{
    CALL_IN(QString("mcTileStatistics=%1")
        .arg(CALL_SHOW(mcTileStatistics)));

    m_Statistics_ProcessingTime_ms +=
        mcTileStatistics["processing time ms"].toLongLong();
    m_Statistics_PointsFinished +=
        mcTileStatistics["points finished long"].toLongLong();
    m_Statistics_PointsInSet +=
        mcTileStatistics["points in set long"].toLongLong();
    m_Statistics_PointsOutOfBounds +=
        mcTileStatistics["points out of bounds long"].toLongLong();
    m_Statistics_TotalIterations +=
        mcTileStatistics["total iterations long"].toLongLong();
    if (m_Statistics_FirstTile)
    {
        m_Statistics_MinDepth = mcTileStatistics["min depth"].toInt();
        m_Statistics_MaxDepth = mcTileStatistics["max depth"].toInt();
        m_Statistics_FirstTile = false;
    } else
    {
        m_Statistics_MinDepth =
            qMin(m_Statistics_MinDepth, mcTileStatistics["min depth"].toInt());
        m_Statistics_MaxDepth =
            qMax(m_Statistics_MaxDepth, mcTileStatistics["max depth"].toInt());
    }
    if (mcTileStatistics["min color value"] != "nan")
    {
        if (!isnan(m_Statistics_MinColorValue))
        {
            m_Statistics_MinColorValue = qMin(m_Statistics_MinColorValue,
                mcTileStatistics["min color value"].toDouble());
            m_Statistics_MaxColorValue = qMax(m_Statistics_MaxColorValue,
                mcTileStatistics["max color value"].toDouble());
        } else
        {
            m_Statistics_MinColorValue =
                mcTileStatistics["min color value"].toDouble();
            m_Statistics_MaxColorValue =
                mcTileStatistics["max color value"].toDouble();
        }
    }
    if (mcTileStatistics["min brightness value"] != "nan")
    {
        if (!isnan(m_Statistics_MinBrightnessValue))
        {
            m_Statistics_MinBrightnessValue =
                qMin(m_Statistics_MinBrightnessValue,
                    mcTileStatistics["min brightness value"].toDouble());
            m_Statistics_MaxBrightnessValue =
                qMax(m_Statistics_MaxBrightnessValue,
                    mcTileStatistics["max brightness value"].toDouble());
        } else
        {
            m_Statistics_MinBrightnessValue =
                mcTileStatistics["min brightness value"].toDouble();
            m_Statistics_MaxBrightnessValue =
                mcTileStatistics["max brightness value"].toDouble();
        }
    }

    CALL_OUT("");
}
