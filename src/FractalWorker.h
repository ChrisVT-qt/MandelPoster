// FractalWorker.h
// Class definition

#ifndef FRACTALWORKER_H
#define FRACTALWORKER_H

// Qt includes
#include <QElapsedTimer>
#include <QHash>
#include <QImage>
#include <QObject>
#include <QVector>

// Class definition
class FractalWorker
    : public QObject
{
    Q_OBJECT



    // ============================================================== Lifecycle
public:
    // Constructor
    FractalWorker();

    // Destructor
    virtual ~FractalWorker();



    // ======================================================== Everything else
public:
    // Render a tile
    void Prepare(const QHash < QString, QString > mcParameters);

    // Set cache values
    void SetCacheValues(const QVector < double > mcColorCache,
        const QVector < double > mcBrightnessCache);
private:
    // Oversampling values
    QList < double > m_OversamplingValues;

public:
    // Precision
    void SetLongDoublePrecision(const bool mcNewState);
private:
    bool m_UseLongDoublePrecision;

public slots:
    // Start rendering
    void Start();
private:
    // Determine color of a pixel
    QColor CalculatePixelColor(const double mcReal, const double mcImag);
    QColor CalculatePixelColor(const long double mcReal,
        const long double mcImag);

    // Calculate argument (angle) of complex number
    double ComplexArg(const double mcReal, const double mcImag) const;
    long double ComplexArg(const long double mcReal,
        const long double mcImag) const;

    // Calculate color from values
    QColor CalculateColorForIndex(const int mcCacheIndex) const;

signals:
    void Finished(const int mcTileID);

private:
    // Parameters
    void SetParameters(const QHash < QString, QString > mcParameters);
public:
    QHash < QString, QString > GetParameters() const;
private:
    QHash < QString, QString > m_Parameters;

    // Abbreviations
    int m_TileID;

    QString m_FractalType;

    double m_RealMin;
    double m_RealMax;
    double m_ImagMin;
    double m_ImagMax;
    double m_JuliaReal;
    double m_JuliaImag;

    long double m_RealMin_Long;
    long double m_RealMax_Long;
    long double m_ImagMin_Long;
    long double m_ImagMax_Long;
    long double m_JuliaReal_Long;
    long double m_JuliaImag_Long;

    int m_Depth;
    double m_EscapeRadius;
    int m_Oversampling;

    QString m_ColorBaseValue;
    QString m_ColorMappingMethod;
    QString m_Periodic_ColorScheme;
    double m_Periodic_FactorR;
    double m_Periodic_OffsetR;
    double m_Periodic_FactorG;
    double m_Periodic_OffsetG;
    double m_Periodic_FactorB;
    double m_Periodic_OffsetB;
    double m_Ramp_Offset;
    double m_Ramp_Factor;

    QString m_BrightnessValue;
    double m_StripAverage_FoldChange;
    double m_StripAverage_Factor;
    double m_StripAverage_Offset;
    double m_StripAverage_MinBrightness;
    double m_StripAverageAlt_Regularity;
    double m_StripAverageAlt_Exponent;

    int m_PixelTotalWidth;
    int m_PixelTotalHeight;
    int m_PixelXMin;
    int m_PixelXMax;
    int m_PixelYMin;
    int m_PixelYMax;

public:
    // Access to image
    QImage GetImage() const;
private:
    QImage m_Image;

public:
    // Access to cache
    bool HasCachedData() const;
    void ClearCache();
    QVector < double > GetColorData() const;
    QVector < double > GetBrightnessData() const;
private:
    QVector < double > m_ColorCache;
    QVector < double > m_BrightnessCache;
    bool m_CacheIsPreset;
    int m_CacheIndex;

public:
    // Check if worker is idle
    bool IsIdle() const;
private:
    bool m_IsIdle;

public:
    // Get tile statistics
    QHash < QString, QString > GetStatistics() const;
private:
    // Reset statistics
    void ResetStatistics();

    bool m_Statistics_FirstIteration;
    QElapsedTimer m_Timer;
    qint64 m_Statistics_ProcessingTime_ms;
    qint64 m_Statistics_TotalPoints;
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

