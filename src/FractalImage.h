// FractalImage.h
// Class definition

#ifndef FRACTALIMAGE_H
#define FRACTALIMAGE_H

// Qt includes
#include <QElapsedTimer>
#include <QColor>
#include <QHash>
#include <QImage>
#include <QMutex>
#include <QObject>
#include <QPixmap>
#include <QThread>
#include <QVector>

// Forward declaration
class FractalWorker;

// Class definition
class FractalImage
    : public QObject
{
    Q_OBJECT



    // ============================================================== Lifecycle
public:
    // Constructor
    FractalImage();

    // Destructor
    virtual ~FractalImage();

    // Initilize given object with data from this object
    void JustLikeThis(FractalImage * mpFractalImage) const;



    // ================================================================= Access
public:
    // Get image
    QImage GetImage() const;
private:
    QPixmap m_Image;

public:
    // Resolution
    QPair < int, int > GetImageResolution() const;



    // ============================================================== Rendering
public:
    // Render image
    void Render(const QHash < QString, QString > mcParameters);
private:
    QHash < QString, QString > m_Parameters;

private slots:
    // Launch a new worker
    void LaunchWorker();

    // Store results from a finished worker
    void WorkerFinished(const int mcTileID);

    // Save cache data to a file
    void SaveCacheData(const int mcTileID,
        const QVector < double > & mcrColorData,
        const QVector < double > & mcrBrightnessData) const;

    // Read cache data to a file
    void ReadCacheData(const int mcTileID);

    // Save picture
    void SavePicture() const;

    // Save statistics
    void SaveStatistics() const;

private:
    QHash < int, FractalWorker * > m_TileIDToWorker;
    QHash < int, QThread * > m_TileIDToWorkerThread;
    QMutex m_Mutex;

public:
    // Stop rendering
    void Stop();

    // Render status
    QString GetRenderStatus() const;
private:
    bool m_IsWorking;
    bool m_IsStopped;
    QElapsedTimer m_UpdateTimer;

signals:
    // Started to render image
    void Started();

    // Image has been updated
    void PeriodicUpdate();

    // Image done
    void Finished();

public:
    // Check if new parameters invalidate cache
    bool WillParametersInvalidateCache(
        const QHash < QString, QString > mcParameters) const;

private:
    // Invalidate the cache
    void InvalidateCache();

public:
    // Color value at a particular position
    double GetColorValueAt(const int mcPixelX, const int mcPixelY);

    // Brightness value at a particular position
    double GetBrightnessValueAt(const int mcPixelX, const int mcPixelY);

private:
    // Storage
    QHash < int, int > m_TileIDToPointXMin;
    QHash < int, int > m_TileIDToPointXMax;
    QHash < int, int > m_TileIDToPointYMin;
    QHash < int, int > m_TileIDToPointYMax;
    QHash < int, QVector < double > > m_TileIDToColorData;
    QHash < int, QVector < double > > m_TileIDToBrightnessData;
    int m_NumberOfTiles;
    int m_CurrentTile;



    // ====================================================== Render statistics
public:
    // Get all statistics
    QHash < QString, QString > GetStatistics() const;

    // Reset statisics
    void ResetStatistics();

    // Add to statistics
    void AddToStatistics(const QHash < QString, QString > mTileStatistics);
private:
    bool m_Statistics_FirstTile;
    QString m_Statistics_StartTime;
    QString m_Statistics_FinishTime;
    qint64 m_Statistics_ProcessingTime_ms;
    qint64 m_Statistics_PointsFinished;
    qint64 m_Statistics_PointsInSet;
    qint64 m_Statistics_PointsOutOfBounds;
    qint64 m_Statistics_TotalIterations;
    int m_Statistics_MinDepth;
    int m_Statistics_MaxDepth;
    double m_Statistics_MinColorValue;
    double m_Statistics_MaxColorValue;
    double m_Statistics_MinBrightnessValue;
    double m_Statistics_MaxBrightnessValue;
};

#endif
