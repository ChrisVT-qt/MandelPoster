// Fractal.cpp
// Class implementation

// Project includes
#include "CallTracer.h"
#include "Fractal.h"
#include "MessageLogger.h"
#include "Preferences.h"
#include "StringHelper.h"

// Qt includes
#include <QCoreApplication>
#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>
#include <QDebug>
#include <QFile>
#include <QRegularExpression>

// System includes
#include <cmath>



// ================================================================== Lifecycle



///////////////////////////////////////////////////////////////////////////////
// Constructor
Fractal::Fractal()
{
    CALL_IN("");

    // Initialize a few things
    m_FractalType = "mandel";
    m_UseLongDoublePrecision = false;
    m_RealMin = -2.2;
    m_RealMax = 0.7;
    m_ImagMin = -1.3;
    m_ImagMax = 1.3;
    m_RealMin_Long = -2.2L;
    m_RealMax_Long = 0.7L;
    m_ImagMin_Long = -1.3L;
    m_ImagMax_Long = 1.3L;
    m_Depth = 1000;
    m_EscapeRadius = 4000;

    // For Julia set
    m_JuliaReal = 0;
    m_JuliaImag = 0;
    m_JuliaReal_Long = 0L;
    m_JuliaImag_Long = 0L;

    // Resolution-based info
    m_Oversampling = 1;
    m_HasFixedResolution = false;
    m_FixedWidth = 0;
    m_FixedHeight = 0;
    m_AspectRatio = "any";

    // Coloring
    m_ColorBaseValue = "continuous";
    m_ColorMappingMethod = "periodic";

    m_Periodic_ColorScheme = "color";
    m_Periodic_FactorR = 0.1;
    m_Periodic_OffsetR = 1;
    m_Periodic_FactorG = 0.2;
    m_Periodic_OffsetG = 2;
    m_Periodic_FactorB = 0.4;
    m_Periodic_OffsetB = 3;

    m_Periodic_Factor = 0.1;
    m_Periodic_Offset = 1;

    m_Ramp_Offset = 40;
    m_Ramp_Factor = 0.05;

    // Brightness
    m_BrightnessValue = "flat";

    m_StripAverage_FoldChange = 10;

    m_StripAverageAlt_FoldChange = 10;
    m_StripAverageAlt_Regularity = -0.7;
    m_StripAverageAlt_Exponent = 4;

    m_StripAverage_Factor = 10;
    m_StripAverage_Offset = 0;
    m_StripAverage_MinBrightness = 0.3;

    // Storage
    Preferences * p = Preferences::Instance();
    m_StorageDirectory = p -> GetTagValue("Application:SaveDirectory");
    m_IsSavingPicture = false;
    m_IsSavingCacheData = false;
    m_IsSavingCacheDataInMemory = true;
    m_IsSavingStatistics = false;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Destructor
Fractal::~Fractal()
{
    CALL_IN("");

    // Nothing to do.

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Initilize given object with data from this object
void Fractal::JustLikeThis(Fractal * mpFractal) const
{
    CALL_IN("mpFractal=...");

    mpFractal -> m_Name = m_Name;
    mpFractal -> m_FractalType = m_FractalType;
    mpFractal -> m_UseLongDoublePrecision = m_UseLongDoublePrecision;
    mpFractal -> m_RealMin = m_RealMin;
    mpFractal -> m_RealMax = m_RealMax;
    mpFractal -> m_ImagMin = m_ImagMin;
    mpFractal -> m_ImagMax = m_ImagMax;
    mpFractal -> m_RealMin_Long = m_RealMin_Long;
    mpFractal -> m_RealMax_Long = m_RealMax_Long;
    mpFractal -> m_ImagMin_Long = m_ImagMin_Long;
    mpFractal -> m_ImagMax_Long = m_ImagMax_Long;
    mpFractal -> m_Depth = m_Depth;
    mpFractal -> m_EscapeRadius = m_EscapeRadius;
    mpFractal -> m_Oversampling = m_Oversampling;
    mpFractal -> m_JuliaReal = m_JuliaReal;
    mpFractal -> m_JuliaImag = m_JuliaImag;
    mpFractal -> m_JuliaReal_Long = m_JuliaReal_Long;
    mpFractal -> m_JuliaImag_Long = m_JuliaImag_Long;
    mpFractal -> m_HasFixedResolution = m_HasFixedResolution;
    mpFractal -> m_FixedWidth = m_FixedWidth;
    mpFractal -> m_FixedHeight = m_FixedHeight;
    mpFractal -> m_AspectRatio = m_AspectRatio;
    mpFractal -> m_ColorBaseValue = m_ColorBaseValue;
    mpFractal -> m_ColorMappingMethod = m_ColorMappingMethod;
    mpFractal -> m_Periodic_ColorScheme = m_Periodic_ColorScheme;
    mpFractal -> m_Periodic_FactorR = m_Periodic_FactorR;
    mpFractal -> m_Periodic_OffsetR = m_Periodic_OffsetR;
    mpFractal -> m_Periodic_FactorG = m_Periodic_FactorG;
    mpFractal -> m_Periodic_OffsetG = m_Periodic_OffsetG;
    mpFractal -> m_Periodic_FactorB = m_Periodic_FactorB;
    mpFractal -> m_Periodic_OffsetB = m_Periodic_OffsetB;
    mpFractal -> m_Periodic_Factor = m_Periodic_Factor;
    mpFractal -> m_Periodic_Offset = m_Periodic_Offset;
    mpFractal -> m_Ramp_Factor = m_Ramp_Factor;
    mpFractal -> m_Ramp_Offset = m_Ramp_Offset;
    mpFractal -> m_BrightnessValue = m_BrightnessValue;
    mpFractal -> m_StripAverage_FoldChange = m_StripAverage_FoldChange;
    mpFractal -> m_StripAverage_Factor = m_StripAverage_Factor;
    mpFractal -> m_StripAverage_Offset = m_StripAverage_Offset;
    mpFractal -> m_StripAverage_MinBrightness = m_StripAverage_MinBrightness;
    mpFractal -> m_StripAverageAlt_FoldChange = m_StripAverageAlt_FoldChange;
    mpFractal -> m_StripAverageAlt_Regularity = m_StripAverageAlt_Regularity;
    mpFractal -> m_StripAverageAlt_Exponent = m_StripAverageAlt_Exponent;
    mpFractal -> m_StorageDirectory = m_StorageDirectory;
    mpFractal -> m_IsSavingPicture = m_IsSavingPicture;
    mpFractal -> m_IsSavingCacheData = m_IsSavingCacheData;
    mpFractal -> m_IsSavingCacheDataInMemory = m_IsSavingCacheDataInMemory;
    mpFractal -> m_IsSavingStatistics = m_IsSavingStatistics;

    CALL_OUT("");
}



// ============================================================== Serialization



///////////////////////////////////////////////////////////////////////////////
// Save to file
void Fractal::ToFile(const QString mcFilename)
{
    CALL_IN(QString("mcFilename=%1")
        .arg(CALL_SHOW(mcFilename)));

    QFile out_file(mcFilename);
    if (!out_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        const QString reason = tr("Could not open target file \"%1\".")
            .arg(mcFilename);
        MessageLogger::Error(CALL_METHOD,
            reason);
        CALL_OUT(reason);
        return;
    }
    QTextStream out_stream(&out_file);
    out_stream << ToXML();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Save to XML
QString Fractal::ToXML() const
{
    CALL_IN("");

    QDomDocument doc("stuff");

    QDomElement dom_fractal = doc.createElement("fractal");
    doc.appendChild(dom_fractal);

    dom_fractal.setAttribute("name", m_Name);
    dom_fractal.setAttribute("date",
        QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    dom_fractal.setAttribute("time",
        QDateTime::currentDateTime().toString("hh:mm:ss"));

    dom_fractal.setAttribute("type", m_FractalType);
    if (m_FractalType == "julia")
    {
        if (m_UseLongDoublePrecision)
        {
            dom_fractal.setAttribute("julia_real",
                StringHelper::ToString(m_JuliaReal_Long));
            dom_fractal.setAttribute("julia_imag",
                StringHelper::ToString(m_JuliaImag_Long));
        } else
        {
            dom_fractal.setAttribute("julia_real",
                QString::number(m_JuliaReal, 'g', 20));
            dom_fractal.setAttribute("julia_imag",
                QString::number(m_JuliaImag, 'g', 20));
        }
    }

    QDomElement dom_render = doc.createElement("render");
    dom_fractal.appendChild(dom_render);
    dom_render.setAttribute("precision",
        m_UseLongDoublePrecision ? "long double" : "double");
    if (m_UseLongDoublePrecision)
    {
        dom_render.setAttribute("real_min",
            StringHelper::ToString(m_RealMin_Long));
        dom_render.setAttribute("real_max",
            StringHelper::ToString(m_RealMax_Long));
        dom_render.setAttribute("imag_min",
            StringHelper::ToString(m_ImagMin_Long));
        dom_render.setAttribute("imag_max",
            StringHelper::ToString(m_ImagMax_Long));
    } else
    {
        dom_render.setAttribute("real_min",
            QString::number(m_RealMin, 'g', 20));
        dom_render.setAttribute("real_max",
            QString::number(m_RealMax, 'g', 20));
        dom_render.setAttribute("imag_min",
            QString::number(m_ImagMin, 'g', 20));
        dom_render.setAttribute("imag_max",
            QString::number(m_ImagMax, 'g', 20));
    }
    dom_render.setAttribute("depth", m_Depth);
    dom_render.setAttribute("escape_radius", m_EscapeRadius);

    QDomElement dom_picture = doc.createElement("picture");
    dom_fractal.appendChild(dom_picture);
    dom_picture.setAttribute("oversampling", m_Oversampling);
    dom_picture.setAttribute("fixed_resolution",
        (m_HasFixedResolution ? "yes" : "no"));
    if (m_HasFixedResolution)
    {
        dom_picture.setAttribute("width", m_FixedWidth);
        dom_picture.setAttribute("height", m_FixedHeight);
    }
    dom_picture.setAttribute("aspect_ratio", m_AspectRatio);

    QDomElement dom_coloring = doc.createElement("coloring");
    dom_picture.appendChild(dom_coloring);
    dom_coloring.setAttribute("base_value", m_ColorBaseValue);
    dom_coloring.setAttribute("mapping_method", m_ColorMappingMethod);
    if (m_ColorMappingMethod == "periodic")
    {
        dom_coloring.setAttribute("color_scheme", m_Periodic_ColorScheme);
        if (m_Periodic_ColorScheme == "color")
        {
            dom_coloring.setAttribute("factor_r", m_Periodic_FactorR);
            dom_coloring.setAttribute("offset_r", m_Periodic_OffsetR);
            dom_coloring.setAttribute("factor_g", m_Periodic_FactorG);
            dom_coloring.setAttribute("offset_g", m_Periodic_OffsetG);
            dom_coloring.setAttribute("factor_b", m_Periodic_FactorB);
            dom_coloring.setAttribute("offset_b", m_Periodic_OffsetB);
        }
        if (m_Periodic_ColorScheme == "greyscale")
        {
            dom_coloring.setAttribute("factor", m_Periodic_Factor);
            dom_coloring.setAttribute("offset", m_Periodic_Offset);
        }
    }
    if (m_ColorMappingMethod == "ramp")
    {
        dom_coloring.setAttribute("offset", m_Ramp_Offset);
        dom_coloring.setAttribute("factor", m_Ramp_Factor);
    }

    QDomElement dom_brightness = doc.createElement("brightness");
    dom_picture.appendChild(dom_brightness);
    dom_brightness.setAttribute("base_value", m_BrightnessValue);
    if (m_BrightnessValue == "strip average")
    {
        dom_brightness.setAttribute("fold_change", m_StripAverage_FoldChange);
    }
    if (m_BrightnessValue == "strip average alt")
    {
        dom_brightness.setAttribute("fold_change",
            m_StripAverageAlt_FoldChange);
        dom_brightness.setAttribute("regularity",
            m_StripAverageAlt_Regularity);
        dom_brightness.setAttribute("exponent", m_StripAverageAlt_Exponent);
    }
    if (m_BrightnessValue == "strip average" ||
        m_BrightnessValue == "strip average alt")
    {
        dom_brightness.setAttribute("factor", m_StripAverage_Factor);
        dom_brightness.setAttribute("offset", m_StripAverage_Offset);
        dom_brightness.setAttribute("min_brightness",
            m_StripAverage_MinBrightness);
    }

    QDomElement dom_storage = doc.createElement("storage");
    dom_fractal.appendChild(dom_storage);
    dom_storage.setAttribute("directory", m_StorageDirectory);
    dom_storage.setAttribute("save_picture", m_IsSavingPicture ? "yes" : "no");
    dom_storage.setAttribute("save_cache_on_disk",
        m_IsSavingCacheData ? "yes" : "no");
    dom_storage.setAttribute("save_cache_in_memory",
        m_IsSavingCacheDataInMemory ? "yes" : "no");
    dom_storage.setAttribute("save_statistics",
        m_IsSavingStatistics ? "yes" : "no");

    // Convert to text
    QString xml = doc.toString();
    static const QRegularExpression format("<!DOCTYPE [^>]+>\\s*(<.*>\\s*)");
    QRegularExpressionMatch match = format.match(xml);
    if (match.hasMatch())
    {
        xml = match.captured(1);
    }

    CALL_OUT("");
    return xml;
}



///////////////////////////////////////////////////////////////////////////////
// Read from file
void Fractal::FromFile(const QString mcFilename)
{
    CALL_IN(QString("mcFilename=%1")
        .arg(CALL_SHOW(mcFilename)));

    // Read XML from file
    QFile in_file(mcFilename);
    if (!in_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        CALL_OUT(tr("File could not be opened for reading"));
        return;
    }
    QTextStream in_stream(&in_file);
    const QString xml = in_stream.readAll();
    FromXML(xml);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Read from XML
bool Fractal::FromXML(const QString mcXML)
{
    CALL_IN(QString("mcXML=%1")
        .arg(CALL_SHOW(mcXML)));

    QDomDocument doc("stuff");
    doc.setContent(mcXML);

    QDomElement dom_fractal = doc.documentElement();
    if (dom_fractal.tagName() != "fractal")
    {
        const QString reason =
            tr("XML file is missing the root element <fractal>.");
        MessageLogger::Error(CALL_METHOD,
            reason);
        CALL_OUT(reason);
        return false;
    }

    // Also set window title
    SetName(dom_fractal.attribute("name"));

    m_FractalType = dom_fractal.attribute("type");

    QDomElement dom_render = dom_fractal.firstChildElement("render");
    m_UseLongDoublePrecision =
        (dom_render.attribute("precision", "double") == "long double");
    if (m_UseLongDoublePrecision)
    {
        m_RealMin_Long =
            StringHelper::ToLongDouble(dom_render.attribute("real_min"));
        m_RealMax_Long =
            StringHelper::ToLongDouble(dom_render.attribute("real_max"));
        m_ImagMin_Long =
            StringHelper::ToLongDouble(dom_render.attribute("imag_min"));
        m_ImagMax_Long =
            StringHelper::ToLongDouble(dom_render.attribute("imag_max"));
    } else
    {
        m_RealMin = dom_render.attribute("real_min").toDouble();
        m_RealMax = dom_render.attribute("real_max").toDouble();
        m_ImagMin = dom_render.attribute("imag_min").toDouble();
        m_ImagMax = dom_render.attribute("imag_max").toDouble();
    }
    m_Depth = dom_render.attribute("depth").toInt();
    m_EscapeRadius = dom_render.attribute("escape_radius").toDouble();

    if (m_FractalType == "julia")
    {
        if (m_UseLongDoublePrecision)
        {
            m_JuliaReal_Long = StringHelper::ToLongDouble(
                dom_fractal.attribute("julia_real"));
            m_JuliaImag_Long = StringHelper::ToLongDouble(
                dom_fractal.attribute("julia_imag"));
        } else
        {
            m_JuliaReal = dom_fractal.attribute("julia_real").toDouble();
            m_JuliaImag = dom_fractal.attribute("julia_imag").toDouble();
        }
    }

    QDomElement dom_picture = dom_fractal.firstChildElement("picture");
    m_Oversampling = dom_picture.attribute("oversampling").toInt();
    m_HasFixedResolution =
        (dom_picture.attribute("fixed_resolution", "no") == "yes");
    if (m_HasFixedResolution)
    {
        m_FixedWidth = dom_picture.attribute("width").toInt();
        m_FixedHeight = dom_picture.attribute("height").toInt();
    }
    m_AspectRatio = dom_picture.attribute("aspect_ratio", "any");

    QDomElement dom_coloring = dom_picture.firstChildElement("coloring");
    m_ColorBaseValue = dom_coloring.attribute("base_value");
    m_ColorMappingMethod = dom_coloring.attribute("mapping_method");
    if (m_ColorMappingMethod == "periodic")
    {
        m_Periodic_ColorScheme =
            dom_coloring.attribute("color_scheme", "color");
        if (m_Periodic_ColorScheme == "color")
        {
            m_Periodic_FactorR = dom_coloring.attribute("factor_r").toDouble();
            m_Periodic_OffsetR = dom_coloring.attribute("offset_r").toDouble();
            m_Periodic_FactorG = dom_coloring.attribute("factor_g").toDouble();
            m_Periodic_OffsetG = dom_coloring.attribute("offset_g").toDouble();
            m_Periodic_FactorB = dom_coloring.attribute("factor_b").toDouble();
            m_Periodic_OffsetB = dom_coloring.attribute("offset_b").toDouble();
        }
        if (m_Periodic_ColorScheme == "greyscale")
        {
            m_Periodic_Factor = dom_coloring.attribute("factor").toDouble();
            m_Periodic_Offset = dom_coloring.attribute("offset").toDouble();
        }
    }
    if (m_ColorMappingMethod == "ramp")
    {
        m_Ramp_Offset = dom_coloring.attribute("offset").toDouble();
        m_Ramp_Factor = dom_coloring.attribute("factor").toDouble();
    }

    QDomElement dom_brightness = dom_picture.firstChildElement("brightness");
    if (!dom_brightness.isNull())
    {
        m_BrightnessValue = dom_brightness.attribute("base_value");
        if (m_BrightnessValue == "strip average")
        {
            m_StripAverage_FoldChange =
                dom_brightness.attribute("fold_change").toDouble();
        }
        if (m_BrightnessValue == "strip average alt")
        {
            m_StripAverageAlt_FoldChange =
                dom_brightness.attribute("fold_change").toDouble();
            m_StripAverageAlt_Regularity =
                dom_brightness.attribute("regularity").toDouble();
            m_StripAverageAlt_Exponent =
                dom_brightness.attribute("exponent").toDouble();
        }
        if (m_BrightnessValue == "strip average" ||
            m_BrightnessValue == "strip average alt")
        {
            m_StripAverage_Factor =
                dom_brightness.attribute("factor").toDouble();
            m_StripAverage_Offset =
                dom_brightness.attribute("offset").toDouble();
            m_StripAverage_MinBrightness =
                dom_brightness.attribute("min_brightness").toDouble();
        }
    } else
    {
        // Defaults for files that didn't have <brightness>
        m_BrightnessValue = "flat";
        m_StripAverage_FoldChange = 10;
        m_StripAverage_Factor = 10;
        m_StripAverage_Offset = 0;
        m_StripAverage_MinBrightness = 0.2;
    }

    QDomElement dom_storage = dom_fractal.firstChildElement("storage");
    m_StorageDirectory = dom_storage.attribute("directory");
    m_IsSavingPicture = (dom_storage.attribute("save_picture", "no") == "yes");
    m_IsSavingCacheData =
        (dom_storage.attribute("save_cache_on_disk", "no") == "yes");
    m_IsSavingCacheDataInMemory =
        (dom_storage.attribute("save_cache_in_memory", "no") == "yes");
    m_IsSavingStatistics =
        (dom_storage.attribute("save_statistics", "no") == "yes");

    // Storage no longer valid
    emit InvalidateStorage();

    // Done
    CALL_OUT("");
    return true;
}



// ===================================================================== Access



///////////////////////////////////////////////////////////////////////////////
// Set name
void Fractal::SetName(const QString mcNewName)
{
    CALL_IN(QString("mcNewName=%1")
        .arg(CALL_SHOW(mcNewName)));

    // Name cannot be empty
    if (mcNewName.isEmpty())
    {
        const QString reason = tr("Name cannot be empty");
        MessageLogger::Error(CALL_METHOD,
            reason);
        CALL_OUT(reason);
        return;
    }

    // Set new name
    m_Name = mcNewName;

    // Let widget know
    emit NameChanged();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Get name
QString Fractal::GetName() const
{
    CALL_IN("");

    CALL_OUT("");
    return m_Name;
}



///////////////////////////////////////////////////////////////////////////////
// Set fractal type
void Fractal::SetFractalType(const QString mcFractalType)
{
    CALL_IN(QString("mcFractalType=%1")
        .arg(CALL_SHOW(mcFractalType)));

    if (mcFractalType == m_FractalType)
    {
        CALL_OUT("No change");
        return;
    }

    // New fractal type
    m_FractalType = mcFractalType;

    // Set default range
    if (m_FractalType == "mandel")
    {
        m_RealMin = -2.2;
        m_RealMax = 0.7;
        m_ImagMin = -1.3;
        m_ImagMax = 1.3;
    }
    if (m_FractalType == "julia")
    {
        m_RealMin = -2;
        m_RealMax = 2;
        m_ImagMin = -2;
        m_ImagMax = 2;
    }

    // Storage no longer valid
    emit InvalidateStorage();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Set fractal type
QString Fractal::GetFractalType() const
{
    CALL_IN("");

    CALL_OUT("");
    return m_FractalType;
}



///////////////////////////////////////////////////////////////////////////////
// Set precision
void Fractal::SetPrecision(const QString mcNewState)
{
    CALL_IN(QString("mcNewState=%1")
        .arg(CALL_SHOW(mcNewState)));

    const bool new_state = (mcNewState == "long double");
    if (new_state == m_UseLongDoublePrecision)
    {
        // No change.
        CALL_OUT("No change");
        return;
    }

    // Convert some things...
    if (new_state)
    {
        m_UseLongDoublePrecision = true;
        m_RealMin_Long = m_RealMin;
        m_RealMax_Long = m_RealMax;
        m_ImagMin_Long = m_ImagMin;
        m_ImagMax_Long = m_ImagMax;
        m_JuliaReal_Long = m_JuliaReal;
        m_JuliaImag_Long = m_JuliaImag;
    } else
    {
        m_UseLongDoublePrecision = false;
        m_RealMin = m_RealMin_Long;
        m_RealMax = m_RealMax_Long;
        m_ImagMin = m_ImagMin_Long;
        m_ImagMax = m_ImagMax_Long;
        m_JuliaReal = m_JuliaReal_Long;
        m_JuliaImag = m_JuliaImag_Long;
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Precision
QString Fractal::GetPrecision() const
{
    CALL_IN("");

    CALL_OUT("");
    return (m_UseLongDoublePrecision ? "long double" : "double");
}



///////////////////////////////////////////////////////////////////////////////
// Set range
void Fractal::SetRange(const double mcRealMin, const double mcRealMax,
    const double mcImagMin, const double mcImagMax)
{
    CALL_IN(QString("mcRealMin=%1, mcRealMax=%2, mcImagMin=%3, mcImagMax=%4")
        .arg(CALL_SHOW(mcRealMin),
             CALL_SHOW(mcRealMax),
             CALL_SHOW(mcImagMin),
             CALL_SHOW(mcImagMax)));

    // Check for no change
    if (m_RealMin == mcRealMin &&
        m_RealMax == mcRealMax &&
        m_ImagMin == mcImagMin &&
        m_ImagMax == mcImagMax)
    {
        CALL_OUT("No change");
        return;
    }

    m_RealMin = mcRealMin;
    m_RealMax = mcRealMax;
    m_ImagMin = mcImagMin;
    m_ImagMax = mcImagMax;

    // Storage no longer valid
    emit InvalidateStorage();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Set range
void Fractal::SetRange(const long double mcRealMin,
    const long double mcRealMax, const long double mcImagMin,
    const long double mcImagMax)
{
    CALL_IN(QString("mcRealMin=%1, mcRealMax=%2, mcImagMin=%3, mcImagMax=%4")
        .arg(CALL_SHOW(mcRealMin),
             CALL_SHOW(mcRealMax),
             CALL_SHOW(mcImagMin),
             CALL_SHOW(mcImagMax)));

    // Check for no change
    if (m_RealMin_Long == mcRealMin &&
        m_RealMax_Long == mcRealMax &&
        m_ImagMin_Long == mcImagMin &&
        m_ImagMax_Long == mcImagMax)
    {
        CALL_OUT("No change");
        return;
    }

    m_RealMin_Long = mcRealMin;
    m_RealMax_Long = mcRealMax;
    m_ImagMin_Long = mcImagMin;
    m_ImagMax_Long = mcImagMax;

    // Storage no longer valid
    emit InvalidateStorage();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Set range
QHash < QString, QString > Fractal::GetRange() const
{
    CALL_IN("");

    QHash < QString, QString > ret;
    if (m_UseLongDoublePrecision)
    {
        ret["real min"] = StringHelper::ToString(m_RealMin_Long);
        ret["real max"] = StringHelper::ToString(m_RealMax_Long);
        ret["imag min"] = StringHelper::ToString(m_ImagMin_Long);
        ret["imag max"] = StringHelper::ToString(m_ImagMax_Long);
    } else
    {
        ret["real min"] = QString::number(m_RealMin, 'g', 16);
        ret["real max"] = QString::number(m_RealMax, 'g', 16);
        ret["imag min"] = QString::number(m_ImagMin, 'g', 16);
        ret["imag max"] = QString::number(m_ImagMax, 'g', 16);
    }

    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Set depth
void Fractal::SetDepth(const int mcDepth)
{
    CALL_IN(QString("mcDepth=%1")
        .arg(CALL_SHOW(mcDepth)));

    // Check for no change
    if (mcDepth == m_Depth)
    {
        CALL_OUT("No change");
        return;
    }

    m_Depth = mcDepth;

    // Storage no longer valid
    emit InvalidateStorage();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Get depth
int Fractal::GetDepth() const
{
    CALL_IN("");

    CALL_OUT("");
    return m_Depth;
}



///////////////////////////////////////////////////////////////////////////////
// Set escape radius
void Fractal::SetEscapeRadius(const double mcEscapeRadius)
{
    CALL_IN(QString("mcEscapeRadius=%1")
        .arg(CALL_SHOW(mcEscapeRadius)));

    // Check for no change
    if (mcEscapeRadius == m_EscapeRadius)
    {
        CALL_OUT("No change");
        return;
    }

    m_EscapeRadius = mcEscapeRadius;

    // Storage no longer valid
    emit InvalidateStorage();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Get escape radius
double Fractal::GetEscapeRadius() const
{
    CALL_IN("");

    CALL_OUT("");
    return m_EscapeRadius;
}



///////////////////////////////////////////////////////////////////////////////
// Set oversampling level
void Fractal::SetOversampling(const int mcOversampling)
{
    CALL_IN(QString("mcOversampling=%1")
        .arg(CALL_SHOW(mcOversampling)));

    // Check for no change
    if (mcOversampling == m_Oversampling)
    {
        CALL_OUT("No change");
        return;
    }

    m_Oversampling = mcOversampling;

    // Storage no longer valid
    emit InvalidateStorage();

    CALL_OUT("");
}


///////////////////////////////////////////////////////////////////////////////
// Get oversampling level
int Fractal::GetOversampling() const
{
    CALL_IN("");

    CALL_OUT("");
    return m_Oversampling;
}



///////////////////////////////////////////////////////////////////////////////
// Set Julia constant
void Fractal::SetJuliaConstant(const double mcRealJulia,
    const double mcImagJulia)
{
    CALL_IN(QString("mcRealJulia=%1, mcImagJulia=%2")
        .arg(CALL_SHOW(mcRealJulia),
             CALL_SHOW(mcImagJulia)));

    // Check for no change
    if (mcRealJulia == m_JuliaReal &&
        mcImagJulia == m_JuliaImag)
    {
        CALL_OUT("No change");
        return;
    }

    m_JuliaReal = mcRealJulia;
    m_JuliaImag = mcImagJulia;

    // Storage no longer valid
    emit InvalidateStorage();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Set Julia constant
void Fractal::SetJuliaConstant(const long double mcRealJulia,
    const long double mcImagJulia)
{
    CALL_IN(QString("mcRealJulia=%1, mcImagJulia=%2")
        .arg(CALL_SHOW(mcRealJulia),
             CALL_SHOW(mcImagJulia)));

    // Check for no change
    if (mcRealJulia == m_JuliaReal_Long &&
        mcImagJulia == m_JuliaImag_Long)
    {
        CALL_OUT("No change");
        return;
    }

    m_JuliaReal_Long = mcRealJulia;
    m_JuliaImag_Long = mcImagJulia;

    // Storage no longer valid
    emit InvalidateStorage();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Get Julia constant
QHash < QString, QString > Fractal::GetJuliaConstant() const
{
    CALL_IN("");

    QHash < QString, QString > parameters;
    if (m_UseLongDoublePrecision)
    {
        parameters["julia real"] = StringHelper::ToString(m_JuliaReal_Long);
        parameters["julia imag"] = StringHelper::ToString(m_JuliaImag_Long);
    } else
    {
        parameters["julia real"] = QString::number(m_JuliaReal, 'g', 16);
        parameters["julia imag"] = QString::number(m_JuliaImag, 'g', 16);
    }

    CALL_OUT("");
    return parameters;
}



// ================================================================= Resolution



///////////////////////////////////////////////////////////////////////////////
// Fixed resolution
bool Fractal::HasFixedResolution() const
{
    CALL_IN("");

    CALL_OUT("");
    return m_HasFixedResolution;
}



///////////////////////////////////////////////////////////////////////////////
void Fractal::SetFixedResolution(const int mcWidth, const int mcHeight)
{
    CALL_IN(QString("mcWidth=%1, mcHeight=%2")
        .arg(CALL_SHOW(mcWidth),
             CALL_SHOW(mcHeight)));

    m_HasFixedResolution = true;
    m_FixedWidth = mcWidth;
    m_FixedHeight = mcHeight;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
void Fractal::UnsetFixedResolutionFlag()
{
    CALL_IN("");

    m_HasFixedResolution = false;
    m_FixedWidth = 0;
    m_FixedHeight = 0;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
QPair < int, int > Fractal::GetFixedResolution() const
{
    CALL_IN("");

    CALL_OUT("");
    return QPair < int, int >(m_FixedWidth, m_FixedHeight);
}



///////////////////////////////////////////////////////////////////////////////
// Get aspect ratio
QString Fractal::GetAspectRatio() const
{
    CALL_IN("");

    CALL_OUT("");
    return m_AspectRatio;
}



///////////////////////////////////////////////////////////////////////////////
// Get aspect ratio
void Fractal::SetAspectRatio(const QString mcNewAspectRatio)
{
    CALL_IN(QString("mcNewAspectRatio=%1")
        .arg(CALL_SHOW(mcNewAspectRatio)));

    // Check if this is a value value
    if (mcNewAspectRatio != "any" &&
        mcNewAspectRatio != "1:1" &&
        mcNewAspectRatio != "2:1" &&
        mcNewAspectRatio != "3:2" &&
        mcNewAspectRatio != "16:9")
    {
        const QString reason = tr("Unknown aspect ratio setting \"%1\".")
            .arg(mcNewAspectRatio);
        MessageLogger::Error(CALL_METHOD,
            reason);
        CALL_OUT(reason);
        return;
    }

    // Set new value
    m_AspectRatio = mcNewAspectRatio;

    CALL_OUT("");
}



// =================================================================== Coloring



///////////////////////////////////////////////////////////////////////////////
// Set color base value
void Fractal::SetColorBaseValue(const QString mcNewBaseValue)
{
    CALL_IN(QString("mcNewBaseValue=%1")
        .arg(CALL_SHOW(mcNewBaseValue)));

    // Check for no change
    if (mcNewBaseValue == m_ColorBaseValue)
    {
        CALL_OUT("No change.");
        return;
    }

    // Check if base value is accaptable
    if (mcNewBaseValue != "continuous" &&
        mcNewBaseValue != "angle")
    {
        const QString reason = tr("Invalid new base value \"%1\".")
            .arg(mcNewBaseValue);
        MessageLogger::Error(CALL_METHOD,
            reason);
        CALL_OUT(reason);
        return;
    }

    // Set new base value
    m_ColorBaseValue = mcNewBaseValue;

    // Storage invalid
    emit InvalidateStorage();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Get color base value
QString Fractal::GetColorBaseValue() const
{
    CALL_IN("");

    CALL_OUT("");
    return m_ColorBaseValue;
}



///////////////////////////////////////////////////////////////////////////////
// Set color mapper
void Fractal::SetColorMappingMethod(const QString mcMethod)
{
    CALL_IN(QString("mcMethod=%1")
        .arg(CALL_SHOW(mcMethod)));

    // Check if new color mapping method is acceptable
    if (mcMethod != "periodic" &&
        mcMethod != "ramp")
    {
        const QString reason = tr("Invalid new color mapping method \"%1\".")
            .arg(mcMethod);
        MessageLogger::Error(CALL_METHOD,
            reason);
        CALL_OUT(reason);
        return;
    }

    // Set new mapping method
    m_ColorMappingMethod = mcMethod;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Get color mapper
QString Fractal::GetColorMappingMethod() const
{
    CALL_IN("");

    CALL_OUT("");
    return m_ColorMappingMethod;
}



///////////////////////////////////////////////////////////////////////////////
// Set coloring parameters
void Fractal::SetColorParameters(
    const QHash < QString, QString > mcParameters)
{
    CALL_IN(QString("mcParameters=%1")
        .arg(CALL_SHOW(mcParameters)));

    // Check for unused parameters
    QList < QString > parameters = mcParameters.keys();

    // Check color mapping method
    if (m_ColorMappingMethod == "ramp")
    {
        if (mcParameters.contains("color offset"))
        {
            m_Ramp_Offset = mcParameters["color offset"].toDouble();
            parameters.removeAll("color offset");
        }
        if (mcParameters.contains("color factor"))
        {
            m_Ramp_Factor = mcParameters["color factor"].toDouble();
            parameters.removeAll("color factor");
        }
    } else if (m_ColorMappingMethod == "periodic")
    {
        if (mcParameters.contains("color scheme"))
        {
            m_Periodic_ColorScheme = mcParameters["color scheme"];
            parameters.removeAll("color scheme");
        }
        if (m_Periodic_ColorScheme == "color")
        {
            if (mcParameters.contains("color factor red"))
            {
                m_Periodic_FactorR =
                    mcParameters["color factor red"].toDouble();
                parameters.removeAll("color factor red");
            }
            if (mcParameters.contains("color offset red"))
            {
                m_Periodic_OffsetR =
                    mcParameters["color offset red"].toDouble();
                parameters.removeAll("color offset red");
            }
            if (mcParameters.contains("color factor green"))
            {
                m_Periodic_FactorG =
                    mcParameters["color factor green"].toDouble();
                parameters.removeAll("color factor green");
            }
            if (mcParameters.contains("color offset green"))
            {
                m_Periodic_OffsetG =
                    mcParameters["color offset green"].toDouble();
                parameters.removeAll("color offset green");
            }
            if (mcParameters.contains("color factor blue"))
            {
                m_Periodic_FactorB =
                    mcParameters["color factor blue"].toDouble();
                parameters.removeAll("color factor blue");
            }
            if (mcParameters.contains("color offset blue"))
            {
                m_Periodic_OffsetB =
                    mcParameters["color offset blue"].toDouble();
                parameters.removeAll("color offset blue");
            }
        }
        if (m_Periodic_ColorScheme == "greyscale")
        {
            if (mcParameters.contains("color factor"))
            {
                m_Periodic_Factor = mcParameters["color factor"].toDouble();
                parameters.removeAll("color factor");
            }
            if (mcParameters.contains("color offset"))
            {
                m_Periodic_Offset = mcParameters["color offset"].toDouble();
                parameters.removeAll("color offset");
            }
        }
    } else
    {
        // Unknown mapping method
        MessageLogger::Error(CALL_METHOD,
            tr("Unknown mapping method \"%1\".").arg(m_ColorMappingMethod));
    }

    // Check if there are other parameters
    if (!parameters.isEmpty())
    {
        MessageLogger::Error(CALL_METHOD,
            tr("Additional parameters \"%1\" have been ignored.")
                .arg(parameters.join("\", \"")));
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Get coloring parameters
QHash < QString, QString > Fractal::GetColorParameters() const
{
    CALL_IN("");

    // Return value
    QHash < QString, QString > ret;

    // Check color mapping method
    if (m_ColorMappingMethod == "ramp")
    {
        ret["color offset"] = QString("%1").arg(m_Ramp_Offset);
        ret["color factor"] = QString("%1").arg(m_Ramp_Factor);
        CALL_OUT("");
        return ret;
    }
    if (m_ColorMappingMethod == "periodic")
    {
        ret["color scheme"] = m_Periodic_ColorScheme;
        if (m_Periodic_ColorScheme == "color")
        {
            ret["color factor red"] = QString("%1").arg(m_Periodic_FactorR);
            ret["color offset red"] = QString("%1").arg(m_Periodic_OffsetR);
            ret["color factor green"] = QString("%1").arg(m_Periodic_FactorG);
            ret["color offset green"] = QString("%1").arg(m_Periodic_OffsetG);
            ret["color factor blue"] = QString("%1").arg(m_Periodic_FactorB);
            ret["color offset blue"] = QString("%1").arg(m_Periodic_OffsetB);
        }
        if (m_Periodic_ColorScheme == "greyscale")
        {
            ret["color factor"] = QString("%1").arg(m_Periodic_Factor);
            ret["color offset"] = QString("%1").arg(m_Periodic_Offset);
        }
        CALL_OUT("");
        return ret;
    }

    // Unknown mapping method
    const QString reason = tr("Unknown mapping method \"%1\".")
        .arg(m_ColorMappingMethod);
    MessageLogger::Error(CALL_METHOD,
        reason);
    CALL_OUT(reason);
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Set brightness
void Fractal::SetBrightnessValue(const QString mcNewBrightnessValue)
{
    CALL_IN(QString("mcNewBrightnessValue=%1")
        .arg(CALL_SHOW(mcNewBrightnessValue)));

    // Check if brightness value is accaptable
    if (mcNewBrightnessValue != "strip average" &&
        mcNewBrightnessValue != "strip average alt" &&
        mcNewBrightnessValue != "flat")
    {
        const QString reason = tr("Invalid new brightness value \"%1\".")
            .arg(mcNewBrightnessValue);
        MessageLogger::Error(CALL_METHOD,
            reason);
        CALL_OUT(reason);
        return;
    }

    if (mcNewBrightnessValue == m_BrightnessValue)
    {
        // No change
        CALL_OUT(tr("No change."));
        return;
    }

    // Set new brightness value
    m_BrightnessValue = mcNewBrightnessValue;

    // Storage invalid
    emit InvalidateStorage();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Get brightness
QString Fractal::GetBrightnessValue() const
{
    CALL_IN("");

    CALL_OUT("");
    return m_BrightnessValue;
}



///////////////////////////////////////////////////////////////////////////////
// Set brightness parameters
void Fractal::SetBrightnessParameters(
    const QHash < QString, QString > mcParameters)
{
    CALL_IN(QString("mcParameters=%1")
        .arg(CALL_SHOW(mcParameters)));

    // Check for unused parameters
    QList < QString > parameters = mcParameters.keys();

    // Some parameter changes invalidate image
    bool invalidate_storage = false;

    // Check brightness method
    if (m_BrightnessValue == "strip average" ||
        m_BrightnessValue == "strip average alt")
    {
        if (m_BrightnessValue == "strip average")
        {
            if (mcParameters.contains("brightness fold change"))
            {
                if (fabs(mcParameters["brightness fold change"].toDouble() -
                        m_StripAverage_FoldChange) > 1e-10)
                {
                    m_StripAverage_FoldChange =
                        mcParameters["brightness fold change"].toDouble();
                    invalidate_storage = true;
                }
                parameters.removeAll("brightness fold change");
            }
        } else if (m_BrightnessValue == "strip average alt")
        {
            if (mcParameters.contains("brightness fold change"))
            {
                if (fabs(mcParameters["brightness fold change"].toDouble() -
                        m_StripAverageAlt_FoldChange) > 1e-10)
                {
                    m_StripAverageAlt_FoldChange =
                        mcParameters["brightness fold change"].toDouble();
                    invalidate_storage = true;
                }
                parameters.removeAll("brightness fold change");
            }
            if (mcParameters.contains("brightness regularity"))
            {
                if (fabs(mcParameters["brightness regularity"].toDouble() -
                        m_StripAverageAlt_Regularity) > 1e-10)
                {
                    m_StripAverageAlt_Regularity =
                        mcParameters["brightness regularity"].toDouble();
                    invalidate_storage = true;
                }
                parameters.removeAll("brightness regularity");
            }
            if (mcParameters.contains("brightness exponent"))
            {
                if (fabs(mcParameters["brightness exponent"].toDouble() -
                        m_StripAverageAlt_Exponent) > 1e-10)
                {
                    m_StripAverageAlt_Exponent =
                        mcParameters["brightness exponent"].toDouble();
                    invalidate_storage = true;
                }
                parameters.removeAll("brightness exponent");
            }
        }
        if (mcParameters.contains("brightness factor"))
        {
            m_StripAverage_Factor =
                mcParameters["brightness factor"].toDouble();
            parameters.removeAll("brightness factor");
        }
        if (mcParameters.contains("brightness offset"))
        {
            m_StripAverage_Offset =
                mcParameters["brightness offset"].toDouble();
            parameters.removeAll("brightness offset");
        }
        if (mcParameters.contains("brightness min brightness"))
        {
            m_StripAverage_MinBrightness =
                mcParameters["brightness min brightness"].toDouble();
            parameters.removeAll("brightness min brightness");
        }
    } else if (m_BrightnessValue == "flat")
    {
        // No parameters.
    } else
    {
        // Unknown mapping method
        MessageLogger::Error(CALL_METHOD,
            tr("Unknown brightness method \"%1\".")
                .arg(m_BrightnessValue));
    }

    // Check if there are other parameters
    if (!parameters.isEmpty())
    {
        MessageLogger::Error(CALL_METHOD,
            tr("Additional parameters \"%1\" have been ignored.")
                .arg(parameters.join("\", \"")));
    }

    // Check if image has been invalidated
    if (invalidate_storage)
    {
        emit InvalidateStorage();
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Get brightness parameters
QHash < QString, QString > Fractal::GetBrightnessParameters() const
{
    CALL_IN("");

    // Return value
    QHash < QString, QString > ret;

    // Check brightness method
    if (m_BrightnessValue == "strip average" ||
        m_BrightnessValue == "strip average alt")
    {
        if (m_BrightnessValue == "strip average")
        {
            ret["brightness fold change"] =
                QString("%1").arg(m_StripAverage_FoldChange);
        }
        if (m_BrightnessValue == "strip average alt")
        {
            ret["brightness fold change"] =
                QString("%1").arg(m_StripAverageAlt_FoldChange);
            ret["brightness regularity"] =
                QString("%1").arg(m_StripAverageAlt_Regularity);
            ret["brightness exponent"] =
                QString("%1").arg(m_StripAverageAlt_Exponent);
        }
        ret["brightness factor"] = QString("%1").arg(m_StripAverage_Factor);
        ret["brightness offset"] = QString("%1").arg(m_StripAverage_Offset);
        ret["brightness min brightness"] =
            QString("%1").arg(m_StripAverage_MinBrightness);
        CALL_OUT("");
        return ret;
    }
    if (m_BrightnessValue == "flat")
    {
        // No parameters
        CALL_OUT("");
        return ret;
    }

    // Unknown mapping method
    const QString reason = tr("Unknown brightness method \"%1\".")
        .arg(m_BrightnessValue);
    MessageLogger::Error(CALL_METHOD,
        reason);
    CALL_OUT(reason);
    return ret;
}



// ============================================================= Storing things



///////////////////////////////////////////////////////////////////////////////
// Set storage location
void Fractal::SetStorageDirectory(const QString mcDirectory)
{
    CALL_IN(QString("mcDirectory=%1")
        .arg(CALL_SHOW(mcDirectory)));

    m_StorageDirectory = mcDirectory;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Get storage location
QString Fractal::GetStorageDirectory() const
{
    CALL_IN("");

    CALL_OUT("");
    return m_StorageDirectory;
}



///////////////////////////////////////////////////////////////////////////////
// Set save picture flag
void Fractal::SetSavePicture(const bool mcNewState)
{
    CALL_IN(QString("mcNewState=%1")
        .arg(CALL_SHOW(mcNewState)));

    m_IsSavingPicture = mcNewState;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Get save picture flag
bool Fractal::IsSavingPicture() const
{
    CALL_IN("");

    CALL_OUT("");
    return m_IsSavingPicture;
}



///////////////////////////////////////////////////////////////////////////////
// Set save cache data flag
void Fractal::SetSaveCacheData(const bool mcNewState)
{
    CALL_IN(QString("mcNewState=%1")
        .arg(CALL_SHOW(mcNewState)));

    m_IsSavingCacheData = mcNewState;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Get save cache data flag
bool Fractal::IsSavingCacheData() const
{
    CALL_IN("");

    CALL_OUT("");
    return m_IsSavingCacheData;
}



///////////////////////////////////////////////////////////////////////////////
// Set save cache data in memory flag
void Fractal::SetSaveCacheDataInMemory(const bool mcNewState)
{
    CALL_IN(QString("mcNewState=%1")
        .arg(CALL_SHOW(mcNewState)));

    m_IsSavingCacheDataInMemory = mcNewState;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Get save cache data in memory flag
bool Fractal::IsSavingCacheDataInMemory() const
{
    CALL_IN("");

    CALL_OUT("");
    return m_IsSavingCacheDataInMemory;
}



///////////////////////////////////////////////////////////////////////////////
// Set save statistics flag
void Fractal::SetSaveStatistics(const bool mcNewState)
{
    CALL_IN(QString("mcNewState=%1")
        .arg(CALL_SHOW(mcNewState)));

    m_IsSavingStatistics = mcNewState;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Get save statistics flag
bool Fractal::IsSavingStatistics() const
{
    CALL_IN("");

    CALL_OUT("");
    return m_IsSavingStatistics;
}



// ============================================================= Render Support



///////////////////////////////////////////////////////////////////////////////
// Check if everything is okay
QString Fractal::CheckAllParametersValid() const
{
    CALL_IN("");

    // Width and height
    if (m_HasFixedResolution)
    {
        if (m_FixedWidth < 1)
        {
            CALL_OUT("");
            return tr("Invalid width parameter %1").arg(m_FixedWidth);
        }
        if (m_FixedHeight < 1)
        {
            CALL_OUT("");
            return tr("Invalid height parameter %1").arg(m_FixedHeight);
        }
    }

    // Fractal type
    if (m_FractalType != "mandel" &&
        m_FractalType != "julia")
    {
        CALL_OUT("");
        return tr("Invalid fractal type \"%1\"").arg(m_FractalType);
    }

    // Range
    if (m_UseLongDoublePrecision)
    {
        if (m_RealMin_Long >= m_RealMax_Long)
        {
            CALL_OUT("");
            return tr("Real value range is incorrectly oriented: "
                "min=%1, max=%2")
                .arg(StringHelper::ToString(m_RealMin_Long),
                     StringHelper::ToString(m_RealMax_Long));
        }
        if (m_ImagMin_Long >= m_ImagMax_Long)
        {
            CALL_OUT("");
            return tr("Imaginary value range is incorrectly oriented: "
                "min=%1, max=%2")
                .arg(StringHelper::ToString(m_ImagMin_Long),
                     StringHelper::ToString(m_ImagMax_Long));
        }
    } else
    {
        if (m_RealMin >= m_RealMax)
        {
            CALL_OUT("");
            return tr("Real value range is incorrectly oriented: "
                "min=%1, max=%2").arg(m_RealMin).arg(m_RealMax);
        }
        if (m_ImagMin >= m_ImagMax)
        {
            CALL_OUT("");
            return tr("Imaginary value range is incorrectly oriented: "
                "min=%1, max=%2").arg(m_ImagMin).arg(m_ImagMax);
        }
    }

    // Depth
    if (m_Depth < 1)
    {
        CALL_OUT("");
        return tr("Invalid depth parameter %1").arg(m_Depth);
    }

    // Escape radius
    if (m_EscapeRadius < 1)
    {
        CALL_OUT("");
        return tr("Invalid escape radius %1").arg(m_EscapeRadius);
    }

    // Oversampling
    if (m_Oversampling < 1 ||
        m_Oversampling > 5)
    {
        CALL_OUT("");
        return tr("Invalid oversampling parameter %1").arg(m_Oversampling);
    }

    // No checks for Julia constant.

    // !!!
    // If saving anything, check if save location is good.

    // No error, otherwise
    CALL_OUT("");
    return QString();
}



///////////////////////////////////////////////////////////////////////////////
// Get all parameters - simplification to start workers
QHash < QString, QString > Fractal::GetAllParameters() const
{
    CALL_IN("");

    QHash < QString, QString > parameters;

    parameters["name"] = m_Name;
    parameters["fractal type"] = m_FractalType;
    parameters["precision"] =
        (m_UseLongDoublePrecision ? "long double" : "double");

    if (m_UseLongDoublePrecision)
    {
        parameters["real min"] = StringHelper::ToString(m_RealMin_Long);
        parameters["real max"] = StringHelper::ToString(m_RealMax_Long);
        parameters["imag min"] = StringHelper::ToString(m_ImagMin_Long);
        parameters["imag max"] = StringHelper::ToString(m_ImagMax_Long);
        parameters["julia real"] = StringHelper::ToString(m_JuliaReal_Long);
        parameters["julia imag"] = StringHelper::ToString(m_JuliaImag_Long);
    } else
    {
        parameters["real min"] = QString::number(m_RealMin, 'g', 16);
        parameters["real max"] = QString::number(m_RealMax, 'g', 16);
        parameters["imag min"] = QString::number(m_ImagMin, 'g', 16);
        parameters["imag max"] = QString::number(m_ImagMax, 'g', 16);
        parameters["julia real"] = QString::number(m_JuliaReal, 'g', 16);
        parameters["julia imag"] = QString::number(m_JuliaImag, 'g', 16);
    }
    parameters["depth"] = QString("%1").arg(m_Depth);
    parameters["escape radius"] = QString("%1").arg(m_EscapeRadius);

    parameters["oversampling"] = QString("%1").arg(m_Oversampling);
    parameters["use fixed resolution"] = (m_HasFixedResolution ? "yes" : "no");
    if (m_HasFixedResolution)
    {
        parameters["fixed resolution width"] = QString("%1").arg(m_FixedWidth);
        parameters["fixed resolution height"] =
            QString("%1").arg(m_FixedHeight);
    }
    parameters["aspect ratio"] = m_AspectRatio;

    parameters["color base value"] = m_ColorBaseValue;
    parameters["color mapping method"] = m_ColorMappingMethod;
    if (m_ColorMappingMethod == "periodic")
    {
        parameters["color scheme"] = m_Periodic_ColorScheme;
        if (m_Periodic_ColorScheme == "color")
        {
            parameters["color factor red"] =
                QString("%1").arg(m_Periodic_FactorR);
            parameters["color offset red"] =
                QString("%1").arg(m_Periodic_OffsetR);
            parameters["color factor green"] =
                QString("%1").arg(m_Periodic_FactorG);
            parameters["color offset green"] =
                QString("%1").arg(m_Periodic_OffsetG);
            parameters["color factor blue"] =
                QString("%1").arg(m_Periodic_FactorB);
            parameters["color offset blue"] =
                QString("%1").arg(m_Periodic_OffsetB);
        } else
        {
            parameters["color factor"] = QString("%1").arg(m_Periodic_Factor);
            parameters["color offset"] = QString("%1").arg(m_Periodic_Offset);
        }
    }
    if (m_ColorMappingMethod == "ramp")
    {
        parameters["color factor"] = QString("%1").arg(m_Ramp_Factor);
        parameters["color offset"] = QString("%1").arg(m_Ramp_Offset);
    }

    parameters["brightness value"] = m_BrightnessValue;
    if (m_BrightnessValue == "flat")
    {
        // Nothing to do
    }
    if (m_BrightnessValue == "strip average")
    {
        parameters["brightness fold change"] =
            QString("%1").arg(m_StripAverage_FoldChange);
        parameters["brightness factor"] =
            QString("%1").arg(m_StripAverage_Factor);
        parameters["brightness offset"] =
            QString("%1").arg(m_StripAverage_Offset);
        parameters["brightness min brightness"] =
            QString("%1").arg(m_StripAverage_MinBrightness);
    }
    if (m_BrightnessValue == "strip average alt")
    {
        parameters["brightness fold change"] =
            QString("%1").arg(m_StripAverageAlt_FoldChange);
        parameters["brightness regularity"] =
            QString("%1").arg(m_StripAverageAlt_Regularity);
        parameters["brightness exponent"] =
            QString("%1").arg(m_StripAverageAlt_Exponent);
        parameters["brightness factor"] =
            QString("%1").arg(m_StripAverage_Factor);
        parameters["brightness offset"] =
            QString("%1").arg(m_StripAverage_Offset);
        parameters["brightness min brightness"] =
            QString("%1").arg(m_StripAverage_MinBrightness);
    }

    parameters["storage directory"] = m_StorageDirectory;
    parameters["storage save picture"] = (m_IsSavingPicture ? "yes" : "no");
    parameters["storage save cache data to disk"] =
        (m_IsSavingCacheData ? "yes" : "no");
    parameters["storage save cache data to memory"] =
        (m_IsSavingCacheDataInMemory ? "yes" : "no");
    parameters["storage save statistics"] =
        (m_IsSavingStatistics ? "yes" : "no");

    CALL_OUT("");
    return parameters;
}
