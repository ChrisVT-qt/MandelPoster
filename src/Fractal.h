// Fractal.h
// Class definition

// Has all information about the fractal and the way it's being rendered.
// Can open and save the fractal

#ifndef FRACTAL_H
#define FRACTAL_H

// Qt includes
#include <QObject>

// Class definition
class Fractal
    : public QObject
{
    Q_OBJECT



    // ============================================================== Lifecycle
public:
    // Constructor
    Fractal();

    // Destructor
    virtual ~Fractal();

    // Initilize given object with data from this object
    void JustLikeThis(Fractal * mpFractal) const;



    // ========================================================== Serialization
public:
    // Save to file
    void ToFile(const QString mcFilename);

    // Serialize
    QString ToXML() const;

    // Read from file
    void FromFile(const QString mcFilename);

    // Deserialize
    bool FromXML(const QString mcXML);



    // ================================================================= Access
public:
    // Name
    void SetName(const QString mcNewName);
    QString GetName() const;
private:
    QString m_Name;
signals:
    void NameChanged();

public:
    // Fractal type
    void SetFractalType(const QString mcFractalType);
    QString GetFractalType() const;
private:
    QString m_FractalType;

public:
    // Precision
    void SetPrecision(const QString mcNewState);
    QString GetPrecision() const;
private:
    bool m_UseLongDoublePrecision;

public:
    // Ranges
    void SetRange(const double mcRealMin, const double mcRealMax,
        const double mcImagMin, const double mcImagMax);
    void SetRange(const long double mcRealMin, const long double mcRealMax,
        const long double mcImagMin, const long double mcImagMax);
    QHash < QString, QString > GetRange() const;
private:
    double m_RealMin;
    double m_RealMax;
    double m_ImagMin;
    double m_ImagMax;

    long double m_RealMin_Long;
    long double m_RealMax_Long;
    long double m_ImagMin_Long;
    long double m_ImagMax_Long;

public:
    // Depth
    void SetDepth(const int mcDepth);
    int GetDepth() const;
private:
    int m_Depth;

public:
    // Escape radius
    void SetEscapeRadius(const double mcEscapeRadius);
    double GetEscapeRadius() const;
private:
    double m_EscapeRadius;

public:
    // Oversampling level
    void SetOversampling(const int mcOversampling);
    int GetOversampling() const;
private:
    int m_Oversampling;
    QList < double > m_OversamplingValues;

public:
    // Julia constant
    void SetJuliaConstant(const double mcRealJulia, const double mcImagJulia);
    void SetJuliaConstant(const long double mcRealJulia,
        const long double mcImagJulia);
    QHash < QString, QString > GetJuliaConstant() const;
private:
    double m_JuliaReal;
    double m_JuliaImag;

    long double m_JuliaReal_Long;
    long double m_JuliaImag_Long;



    // ============================================================= Resolution
public:
    // Fixed resolution
    bool HasFixedResolution() const;
    void SetFixedResolution(const int mcWidth, const int mcHeight);
    void UnsetFixedResolutionFlag();
    QPair < int, int > GetFixedResolution() const;
private:
    bool m_HasFixedResolution;
    int m_FixedWidth;
    int m_FixedHeight;

public:
    // Aspect ratio
    QString GetAspectRatio() const;
    void SetAspectRatio(const QString mcNewAspectRatio);
private:
    QString m_AspectRatio;



    // =============================================================== Coloring
public:
    // Color base value
    void SetColorBaseValue(const QString mcNewBaseValue);
    QString GetColorBaseValue() const;

    // Color mapper
    void SetColorMappingMethod(const QString mcMethod);
    QString GetColorMappingMethod() const;

    // Coloring parameters
    void SetColorParameters(const QHash < QString, QString > mcParameters);
    QHash < QString, QString > GetColorParameters() const;

private:
    QString m_ColorBaseValue;
    QString m_ColorMappingMethod;

    // Periodic
    QString m_Periodic_ColorScheme;

    double m_Periodic_FactorR;
    double m_Periodic_OffsetR;
    double m_Periodic_FactorG;
    double m_Periodic_OffsetG;
    double m_Periodic_FactorB;
    double m_Periodic_OffsetB;

    double m_Periodic_Factor;
    double m_Periodic_Offset;

    // Ramp
    double m_Ramp_Factor;
    double m_Ramp_Offset;

public:
    // Brightness value
    void SetBrightnessValue(const QString mcNewBrightnessValue);
    QString GetBrightnessValue() const;

    // Brightness parameters
    void SetBrightnessParameters(
        const QHash < QString, QString > mcParameters);
    QHash < QString, QString > GetBrightnessParameters() const;

private:
    QString m_BrightnessValue;

    // Strip average
    double m_StripAverage_FoldChange;
    double m_StripAverage_Factor;
    double m_StripAverage_Offset;
    double m_StripAverage_MinBrightness;

    // Strip average, alternate version
    double m_StripAverageAlt_FoldChange;
    double m_StripAverageAlt_Regularity;
    double m_StripAverageAlt_Exponent;



    // ========================================================= Storing things
public:
    // Storage location
    void SetStorageDirectory(const QString mcDirectory);
    QString GetStorageDirectory() const;
private:
    QString m_StorageDirectory;

public:
    // Save picture?
    void SetSavePicture(const bool mcNewState);
    bool IsSavingPicture() const;
private:
    bool m_IsSavingPicture;

public:
    // Save cache data?
    void SetSaveCacheData(const bool mcNewState);
    bool IsSavingCacheData() const;
private:
    bool m_IsSavingCacheData;

public:
    // Save cache data in memory?
    void SetSaveCacheDataInMemory(const bool mcNewState);
    bool IsSavingCacheDataInMemory() const;
private:
    bool m_IsSavingCacheDataInMemory;

public:
    // Saving statistics?
    void SetSaveStatistics(const bool mcNewState);
    bool IsSavingStatistics() const;
private:
    bool m_IsSavingStatistics;



    // ========================================================= Render Support
public:
    // Check if everything is okay
    QString CheckAllParametersValid() const;

    // Get all parameters
    QHash < QString, QString > GetAllParameters() const;

signals:
    // Invalidate Storage
    void InvalidateStorage();
};

#endif
