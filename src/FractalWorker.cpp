// FractalWorker.cpp
// Class implementation

// Project includes
#include "FractalWorker.h"
#include "MessageLogger.h"
#include "StringHelper.h"

// Qt includes
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QTimer>

// System includes
#include <cmath>



// We don't do call tracing here because our way of doing that is not thread
// safe.



// ================================================================== Lifecycle



///////////////////////////////////////////////////////////////////////////////
// Constructor
FractalWorker::FractalWorker()
{
    // Not using long double
    m_UseLongDoublePrecision = false;

    // Cache isn't preset
    m_CacheIsPreset = false;

    // Currently is idle
    m_IsIdle = true;
}



///////////////////////////////////////////////////////////////////////////////
// Destructor
FractalWorker::~FractalWorker()
{
    // Nothing to do.
}


// ============================================================ Everything else



///////////////////////////////////////////////////////////////////////////////
// Perpare rendering a tile
void FractalWorker::Prepare(const QHash < QString, QString > mcParameters)
{
    // Check if worker is still running
    if (!m_IsIdle)
    {
        // Have to wait until it finishes first
        return;
    }

    // Parse parameters
    SetParameters(mcParameters);

    // Initialize caches
    const int tile_width = m_PixelXMax - m_PixelXMin;
    const int tile_height = m_PixelYMax - m_PixelYMin;
    const int cache_size =
        tile_width * tile_height * m_Oversampling * m_Oversampling;
    m_ColorCache.resize(cache_size);
    m_BrightnessCache.resize(cache_size);
    m_CacheIndex = 0;

    // .. image ...
    m_Image = QImage(tile_width, tile_height, QImage::Format_RGB32);
    m_Image.fill(Qt::black);

    // ... and oversampling
    // (In order to get a good sample for the integer coordinate (x,y),
    // oversampling should sample the square [-1/2,1/2]x[-1/2,1/2] instead
    // of the single point at its center. The best way to do this is to sample
    // from an uniform distribution, that is, spread the points such that
    // they all represent 1/oversampling/oversampling area. Below formula
    // calculates these coordinates)
    m_OversamplingValues.clear();
    for (int i = 0; i < m_Oversampling; i++)
    {
        m_OversamplingValues <<
            (2. * i - m_Oversampling + 1.) / 2 / m_Oversampling;
    }
}



///////////////////////////////////////////////////////////////////////////////
// Set cache values
void FractalWorker::SetCacheValues(const QVector < double > mcColorCache,
    const QVector < double > mcBrightnessCache)
{
    m_ColorCache = mcColorCache;
    m_BrightnessCache = mcBrightnessCache;
    m_CacheIsPreset = true;
}



///////////////////////////////////////////////////////////////////////////////
// Precision
void FractalWorker::SetLongDoublePrecision(const bool mcNewState)
{
    // Check if there's any change
    if (mcNewState == m_UseLongDoublePrecision)
    {
        // No.
        return;
    }

    // Set new state
    m_UseLongDoublePrecision = mcNewState;

    // Invalidate caches
    m_ColorCache.clear();
    m_BrightnessCache.clear();
}



///////////////////////////////////////////////////////////////////////////////
// Start rendering
void FractalWorker::Start()
{
    // Reset statistics
    ResetStatistics();

    // Start timer
    m_Timer.restart();

    // Not idle anymore!
    m_IsIdle = false;

    // Generate image
    double normalizer_oversampling = 1./m_Oversampling/m_Oversampling;
    for (int pixel_y = m_PixelYMin; pixel_y < m_PixelYMax; pixel_y++)
    {
        for (int pixel_x = m_PixelXMin; pixel_x < m_PixelXMax; pixel_x++)
        {
            int color_r = 0;
            int color_g = 0;
            int color_b = 0;
            for (double delta_x : m_OversamplingValues)
            {
                for (double delta_y : m_OversamplingValues)
                {
                    QColor color;
                    if (m_UseLongDoublePrecision)
                    {
                        const long double real = m_RealMin_Long +
                            (m_RealMax_Long - m_RealMin_Long) *
                                (pixel_x + delta_x) / (m_PixelTotalWidth - 1.L);
                        const long double imag = m_ImagMax_Long -
                            (m_ImagMax_Long - m_ImagMin_Long) *
                            (pixel_y + delta_y) / (m_PixelTotalHeight - 1.L);
                        color = CalculatePixelColor(real, imag);
                    } else
                    {
                        const double real = m_RealMin +
                            (m_RealMax - m_RealMin) *
                            (pixel_x + delta_x) / (m_PixelTotalWidth - 1.);
                        const double imag = m_ImagMax -
                            (m_ImagMax - m_ImagMin) *
                            (pixel_y + delta_y) / (m_PixelTotalHeight - 1.);
                        color = CalculatePixelColor(real, imag);
                    }
                    color_r += color.red();
                    color_g += color.green();
                    color_b += color.blue();
                }
            }
            color_r *= normalizer_oversampling;
            color_g *= normalizer_oversampling;
            color_b *= normalizer_oversampling;
            m_Image.setPixelColor(pixel_x - m_PixelXMin, pixel_y - m_PixelYMin,
                QColor(color_r, color_g, color_b));
        }
    }

    // Record elapsed time
    m_Statistics_ProcessingTime_ms = m_Timer.elapsed();

    // Idle again!
    m_IsIdle = true;

    // Let outside world know we're done for now.
    emit Finished(m_TileID);
}



///////////////////////////////////////////////////////////////////////////////
// Determine color of a pixel
QColor FractalWorker::CalculatePixelColor(const double mcReal,
    const double mcImag)
{
    // Just in case we have cached values
    if (m_CacheIsPreset)
    {
        m_Statistics_PointsFinished++;
        const int index = m_CacheIndex++;
        return CalculateColorForIndex(index);
    }

    // Speed-ups
    const bool is_mandel = (m_FractalType == "mandel");
    const double r_squared = m_EscapeRadius * m_EscapeRadius;

    // Prep visualization method "strip average"
    // If strip average is selected as value
    const bool use_strip = (m_BrightnessValue == "strip average");
    const bool use_strip_alt = (m_BrightnessValue == "strip average alt");
    const bool use_sac = use_strip || use_strip_alt;
    double sac_avg = 0;
    double sac_previous_avg = 0;
    int sac_skip = 1;

    // Initiatize iteration
    int current_depth = 0;
    double real = 0;
    double imag = 0;
    if (!is_mandel)
    {
        real = mcReal;
        imag = mcImag;
    }

    // Iteration
    double new_real;
    while (current_depth < m_Depth &&
        real * real + imag * imag < r_squared)
    {
        if (is_mandel)
        {
            // Mandelbrot set
            new_real = real * real - imag * imag + mcReal;
            imag = 2 * real * imag + mcImag;
            real = new_real;
        } else
        {
            // Julia set
            new_real = real * real - imag * imag + m_JuliaReal;
            imag = 2 * real * imag + m_JuliaImag;
            real = new_real;
        }
        if (use_sac && current_depth > sac_skip)
        {
            sac_previous_avg = sac_avg;
            const double arg = ComplexArg(real, imag);
            if (use_strip)
            {
                sac_avg += (1. + sin(m_StripAverage_FoldChange * arg)) / 2.;
            }
            if (use_strip_alt)
            {
                sac_avg += 1./(1. + m_StripAverageAlt_Regularity *
                    pow(sin(m_StripAverage_FoldChange * arg),
                        m_StripAverageAlt_Exponent));
            }
        }
        current_depth++;
    }

    // Update statistics
    m_Statistics_PointsFinished++;
    if (m_Statistics_FirstIteration)
    {
        m_Statistics_MinDepth = current_depth;
        m_Statistics_MaxDepth = current_depth;
    } else
    {
        m_Statistics_MinDepth = qMin(m_Statistics_MinDepth, current_depth);
        m_Statistics_MaxDepth = qMax(m_Statistics_MaxDepth, current_depth);
    }
    m_Statistics_TotalIterations += current_depth;

    // Check if inside the set
    const bool inside_set = (current_depth == m_Depth);

    // ... or out of bounds (start value exceeded escape radius)
    const bool out_of_bounds = (current_depth == 0);

    // Determine color value
    double color_value = 0;
    if (inside_set)
    {
        m_Statistics_PointsInSet++;
        color_value = INFINITY;
    } else if (out_of_bounds)
    {
        m_Statistics_PointsOutOfBounds++;
        color_value = -INFINITY;
    } else
    {
        // Actual value
        if (m_ColorBaseValue == "continuous")
        {
            // Continuous coloring
            // (see http://linas.org/art-gallery/escape/escape.html)
            // (see https://math.stackexchange.com/questions/4035/
            //      continuous-coloring-of-a-mandelbrot-fractal)
            color_value =
                current_depth - log2(log2(real * real + imag * imag) / 2);
        }
        if (m_ColorBaseValue == "angle")
        {
            color_value = ComplexArg(real, imag);
        }

        // Update statistics
        if (m_Statistics_FirstIteration)
        {
            m_Statistics_MinColorValue = color_value;
            m_Statistics_MaxColorValue = color_value;
        } else
        {
            m_Statistics_MinColorValue =
                qMin(m_Statistics_MinColorValue, color_value);
            m_Statistics_MaxColorValue =
                qMax(m_Statistics_MaxColorValue, color_value);
        }
    }

    double brightness = 0;
    if (inside_set)
    {
        m_Statistics_PointsInSet++;
        brightness = 1.;
    } else
    {
        // Actual value
        if (m_BrightnessValue == "flat")
        {
            brightness = 1.;
        }
        if (use_strip ||
            use_strip_alt)
        {
            if (current_depth <= sac_skip + 1)
            {
                // Both averages need to exist
                brightness = 0;
            } else
            {
                // Strip Average Coloring (SAC) per
                // (see https://en.wikibooks.org/wiki/Fractals/
                //   Iterations_in_the_complex_plane/stripeAC)
                sac_avg /= (current_depth - sac_skip);
                sac_previous_avg /= (current_depth - sac_skip - 1);
                const double log_r = 0.5 * log(real * real + imag * imag);
                double lambda = 1. + log2(log(m_EscapeRadius) / log_r);
                brightness =
                    lambda * sac_avg + (1. - lambda) * sac_previous_avg;

                // Update statistics
                if (m_Statistics_FirstIteration)
                {
                    m_Statistics_MinBrightnessValue = brightness;
                    m_Statistics_MaxBrightnessValue = brightness;
                } else
                {
                    m_Statistics_MinBrightnessValue =
                        qMin(m_Statistics_MinBrightnessValue, brightness);
                    m_Statistics_MaxBrightnessValue =
                        qMax(m_Statistics_MaxBrightnessValue, brightness);
                }
            }
        }
    }

    // Save value in storage
    const int index = m_CacheIndex++;
    m_ColorCache[index] = color_value;
    m_BrightnessCache[index] = brightness;

    // No more first iteration
    m_Statistics_FirstIteration = false;

    // Get color
    return CalculateColorForIndex(index);
}



///////////////////////////////////////////////////////////////////////////////
// Determine color of a pixel
QColor FractalWorker::CalculatePixelColor(const long double mcReal,
    const long double mcImag)
{
    // Just in case we have cached values
    if (m_CacheIsPreset)
    {
        m_Statistics_PointsFinished++;
        const int index = m_CacheIndex++;
        return CalculateColorForIndex(index);
    }

    // Speed-ups
    const bool is_mandel = (m_FractalType == "mandel");
    const long double r_squared = m_EscapeRadius * m_EscapeRadius;

    // Prep visualization method "strip average"
    // If strip average is selected as value
    const bool use_strip = (m_BrightnessValue == "strip average");
    const bool use_strip_alt = (m_BrightnessValue == "strip average alt");
    const bool use_sac = use_strip || use_strip_alt;
    double sac_avg = 0;
    double sac_previous_avg = 0;
    int sac_skip = 1;

    // Initiatize iteration
    int current_depth = 0;
    long double real = 0;
    long double imag = 0;
    if (!is_mandel)
    {
        real = mcReal;
        imag = mcImag;
    }

    // Iteration
    long double new_real;
    while (current_depth < m_Depth &&
        real * real + imag * imag < r_squared)
    {
        if (is_mandel)
        {
            // Mandelbrot set
            new_real = real * real - imag * imag + mcReal;
            imag = 2 * real * imag + mcImag;
            real = new_real;
        } else
        {
            // Julia set
            new_real = real * real - imag * imag + m_JuliaReal_Long;
            imag = 2.0L * real * imag + m_JuliaImag_Long;
            real = new_real;
        }
        if (use_sac && current_depth > sac_skip)
        {
            sac_previous_avg = sac_avg;
            const double arg = ComplexArg(real, imag);
            if (use_strip)
            {
                sac_avg += (1. + sin(m_StripAverage_FoldChange * arg)) / 2.;
            }
            if (use_strip_alt)
            {
                sac_avg += 1./(1. + m_StripAverageAlt_Regularity *
                    pow(sin(m_StripAverage_FoldChange * arg),
                        m_StripAverageAlt_Exponent));
            }
        }
        current_depth++;
    }

    // Update statistics
    m_Statistics_PointsFinished++;
    if (m_Statistics_FirstIteration)
    {
        m_Statistics_MinDepth = current_depth;
        m_Statistics_MaxDepth = current_depth;
    } else
    {
        m_Statistics_MinDepth = qMin(m_Statistics_MinDepth, current_depth);
        m_Statistics_MaxDepth = qMax(m_Statistics_MaxDepth, current_depth);
    }
    m_Statistics_TotalIterations += current_depth;

    // Check if inside the set
    const bool inside_set = (current_depth == m_Depth);

    // ... or out of bounds (start value exceeded escape radius)
    const bool out_of_bounds = (current_depth == 0);

    // Determine color value
    double color_value = 0;
    if (inside_set)
    {
        m_Statistics_PointsInSet++;
        color_value = INFINITY;
    } else if (out_of_bounds)
    {
        m_Statistics_PointsOutOfBounds++;
        color_value = -INFINITY;
    } else
    {
        // Actual value
        if (m_ColorBaseValue == "continuous")
        {
            // Continuous coloring
            // (see http://linas.org/art-gallery/escape/escape.html)
            // (see https://math.stackexchange.com/questions/4035/
            //      continuous-coloring-of-a-mandelbrot-fractal)
            color_value =
                current_depth - log2l(log2l(real * real + imag * imag) / 2);
        }
        if (m_ColorBaseValue == "angle")
        {
            color_value = ComplexArg(real, imag);
        }

        // Update statistics
        if (m_Statistics_FirstIteration)
        {
            m_Statistics_MinColorValue = color_value;
            m_Statistics_MaxColorValue = color_value;
        } else
        {
            m_Statistics_MinColorValue =
                qMin(m_Statistics_MinColorValue, color_value);
            m_Statistics_MaxColorValue =
                qMax(m_Statistics_MaxColorValue, color_value);
        }
    }

    double brightness = 0;
    if (inside_set)
    {
        m_Statistics_PointsInSet++;
        brightness = 1.;
    } else
    {
        // Actual value
        if (m_BrightnessValue == "flat")
        {
            brightness = 1.;
        }
        if (use_strip ||
            use_strip_alt)
        {
            if (current_depth <= sac_skip + 1)
            {
                // Both averages need to exist
                brightness = 0;
            } else
            {
                // Strip Average Coloring (SAC) per
                // (see https://en.wikibooks.org/wiki/Fractals/
                //   Iterations_in_the_complex_plane/stripeAC)
                sac_avg /= (current_depth - sac_skip);
                sac_previous_avg /= (current_depth - sac_skip - 1);
                const double log_r = 0.5 * log(real * real + imag * imag);
                double lambda = 1. + log2(log(m_EscapeRadius) / log_r);
                brightness =
                    lambda * sac_avg + (1. - lambda) * sac_previous_avg;

                // Update statistics
                if (m_Statistics_FirstIteration)
                {
                    m_Statistics_MinBrightnessValue = brightness;
                    m_Statistics_MaxBrightnessValue = brightness;
                } else
                {
                    m_Statistics_MinBrightnessValue =
                        qMin(m_Statistics_MinBrightnessValue, brightness);
                    m_Statistics_MaxBrightnessValue =
                        qMax(m_Statistics_MaxBrightnessValue, brightness);
                }
            }
        }
    }

    // Save value in storage
    const int index = m_CacheIndex++;
    m_ColorCache[index] = color_value;
    m_BrightnessCache[index] = brightness;

    // No more first iteration
    m_Statistics_FirstIteration = false;

    // Get color
    return CalculateColorForIndex(index);
}



///////////////////////////////////////////////////////////////////////////////
// Calculate argument (angle) of complex number
double FractalWorker::ComplexArg(const double mcReal,
    const double mcImag) const
{
    if (mcReal > 0)
    {
        return atan(mcImag/mcReal);
    } else if (mcReal < 0)
    {
        if (mcImag >= 0)
        {
            return atan(mcImag/mcReal) + M_PI;
        } else
        {
            return atan(mcImag/mcReal) - M_PI;
        }
    } else
    {
        if (mcImag > 0)
        {
            return M_PI/2;
        } else
        {
            return -M_PI/2;
        }
    }

    return NAN;
}



///////////////////////////////////////////////////////////////////////////////
// Calculate argument (angle) of complex number
long double FractalWorker::ComplexArg(const long double mcReal,
    const long double mcImag) const
{
    if (mcReal > 0)
    {
        return atan(mcImag/mcReal);
    } else if (mcReal < 0)
    {
        if (mcImag >= 0)
        {
            return atanl(mcImag/mcReal) + M_PI;
        } else
        {
            return atanl(mcImag/mcReal) - M_PI;
        }
    } else
    {
        if (mcImag > 0)
        {
            return M_PI/2;
        } else
        {
            return -M_PI/2;
        }
    }

    return NAN;
}



///////////////////////////////////////////////////////////////////////////////
// Calculate color from values
QColor FractalWorker::CalculateColorForIndex(const int mcCacheIndex) const
{
    // Get value (abbreviation)
    const double color_value = m_ColorCache[mcCacheIndex];

    // Special cases
    if (isinf(color_value))
    {
        if (color_value > 0)
        {
            // Inside the set
            return Qt::black;
        } else
        {
            // Out of bounds
            return Qt::red;
        }
    }

    // Coloring
    double color_r = 0;
    double color_g = 0;
    double color_b = 0;
    if (m_ColorMappingMethod == "ramp")
    {
        double norm = tanh((color_value - m_Ramp_Offset) * m_Ramp_Factor);
        norm = qMax(qMin(norm, 1.), 0.);
        color_r = norm;
        color_g = color_r;
        color_b = color_r;
    }
    if (m_ColorMappingMethod == "periodic")
    {
        color_r = (sin(color_value * m_Periodic_FactorR +
            m_Periodic_OffsetR) + 1.) / 2;
        color_g = (sin(color_value * m_Periodic_FactorG +
            m_Periodic_OffsetG) + 1.) / 2;
        color_b = (sin(color_value * m_Periodic_FactorB +
            m_Periodic_OffsetB) + 1.) / 2;
    }

    // Brightness
    const double brightness_value = m_BrightnessCache[mcCacheIndex];
    double brightness = 0;
    if (m_BrightnessValue == "flat")
    {
        brightness = 1.;
    }
    if (m_BrightnessValue == "strip average" ||
        m_BrightnessValue == "strip average alt")
    {
        brightness = (sin(brightness_value * m_StripAverage_Factor +
            m_StripAverage_Offset) + 1.) / 2;
        brightness = m_StripAverage_MinBrightness +
            (1. - m_StripAverage_MinBrightness) * brightness;
    }

    // Color
    color_r *= brightness * 255;
    color_g *= brightness * 255;
    color_b *= brightness * 255;
    return QColor(color_r, color_g, color_b);
}



///////////////////////////////////////////////////////////////////////////////
// Set parameters
void FractalWorker::SetParameters(
    const QHash < QString, QString > mcParameters)
{
    // Store parameters
    m_Parameters = mcParameters;

    // For faster access
    m_TileID = m_Parameters["tile id"].toInt();

    m_FractalType = m_Parameters["fractal type"];
    m_UseLongDoublePrecision = (m_Parameters["precision"] == "long double");
    if (m_UseLongDoublePrecision)
    {
        m_RealMin_Long = StringHelper::ToLongDouble(m_Parameters["real min"]);
        m_RealMax_Long = StringHelper::ToLongDouble(m_Parameters["real max"]);
        m_ImagMin_Long = StringHelper::ToLongDouble(m_Parameters["imag min"]);
        m_ImagMax_Long = StringHelper::ToLongDouble(m_Parameters["imag max"]);
        m_JuliaReal_Long =
            StringHelper::ToLongDouble(m_Parameters["julia real"]);
        m_JuliaImag_Long =
            StringHelper::ToLongDouble(m_Parameters["julia imag"]);
    } else
    {
        m_RealMin = m_Parameters["real min"].toDouble();
        m_RealMax = m_Parameters["real max"].toDouble();
        m_ImagMin = m_Parameters["imag min"].toDouble();
        m_ImagMax = m_Parameters["imag max"].toDouble();
        m_JuliaReal = m_Parameters["julia real"].toDouble();
        m_JuliaImag = m_Parameters["julia imag"].toDouble();
    }
    m_Depth = m_Parameters["depth"].toInt();
    m_EscapeRadius = m_Parameters["escape radius"].toDouble();
    m_Oversampling = m_Parameters["oversampling"].toInt();

    m_ColorBaseValue = m_Parameters["color base value"];
    m_ColorMappingMethod = m_Parameters["color mapping method"];
    if (m_ColorMappingMethod == "periodic")
    {
        m_Periodic_ColorScheme = m_Parameters["color scheme"];
        if (m_Periodic_ColorScheme == "color")
        {
            m_Periodic_FactorR = m_Parameters["color factor red"].toDouble();
            m_Periodic_OffsetR = m_Parameters["color offset red"].toDouble();
            m_Periodic_FactorG = m_Parameters["color factor green"].toDouble();
            m_Periodic_OffsetG = m_Parameters["color offset green"].toDouble();
            m_Periodic_FactorB = m_Parameters["color factor blue"].toDouble();
            m_Periodic_OffsetB = m_Parameters["color offset blue"].toDouble();
        } else
        {
            m_Periodic_FactorR = m_Parameters["color factor"].toDouble();
            m_Periodic_OffsetR = m_Parameters["color offset"].toDouble();
            m_Periodic_FactorG = m_Periodic_FactorR;
            m_Periodic_OffsetG = m_Periodic_OffsetR;
            m_Periodic_FactorB = m_Periodic_FactorR;
            m_Periodic_OffsetB = m_Periodic_OffsetR;
        }
    }
    if (m_ColorMappingMethod == "ramp")
    {
        m_Ramp_Factor = m_Parameters["color factor"].toDouble();
        m_Ramp_Offset = m_Parameters["color offset"].toDouble();
    }

    m_BrightnessValue = m_Parameters["brightness value"];
    if (m_BrightnessValue == "flat")
    {
        // Nothing to do
    }
    if (m_BrightnessValue == "strip average")
    {
        m_StripAverage_FoldChange =
            m_Parameters["brightness fold change"].toDouble();
        m_StripAverage_Factor = m_Parameters["brightness factor"].toDouble();
        m_StripAverage_Offset = m_Parameters["brightness offset"].toDouble();
        m_StripAverage_MinBrightness =
            m_Parameters["brightness min brightness"].toDouble();
    }
    if (m_BrightnessValue == "strip average alt")
    {
        m_StripAverage_FoldChange =
            m_Parameters["brightness fold change"].toDouble();
        m_StripAverage_Factor = m_Parameters["brightness factor"].toDouble();
        m_StripAverage_Offset = m_Parameters["brightness offset"].toDouble();
        m_StripAverage_MinBrightness =
            m_Parameters["brightness min brightness"].toDouble();
        m_StripAverageAlt_Regularity =
            m_Parameters["brightness regularity"].toDouble();
        m_StripAverageAlt_Exponent =
            m_Parameters["brightness exponent"].toDouble();
    }

    m_PixelTotalWidth = m_Parameters["total pixel width"].toInt();
    m_PixelTotalHeight = m_Parameters["total pixel height"].toInt();
    m_PixelXMin = m_Parameters["pixel x min"].toInt();
    m_PixelXMax = m_Parameters["pixel x max"].toInt();
    m_PixelYMin = m_Parameters["pixel y min"].toInt();
    m_PixelYMax = m_Parameters["pixel y max"].toInt();
}



///////////////////////////////////////////////////////////////////////////////
// Get parameters
QHash < QString, QString > FractalWorker::GetParameters() const
{
    return m_Parameters;
}



///////////////////////////////////////////////////////////////////////////////
// Access to image
QImage FractalWorker::GetImage() const
{
    return m_Image;
}



///////////////////////////////////////////////////////////////////////////////
// Check if there are chached data
bool FractalWorker::HasCachedData() const
{
    // Both caches are created at the same time, sufficient to check one of
    // them
    return !m_ColorCache.isEmpty();
}



///////////////////////////////////////////////////////////////////////////////
// Clear cache
void FractalWorker::ClearCache()
{
    m_ColorCache.resize(0);
    m_BrightnessCache.resize(0);
}



///////////////////////////////////////////////////////////////////////////////
// Access to cache: color data
QVector < double > FractalWorker::GetColorData() const
{
    return m_ColorCache;
}



///////////////////////////////////////////////////////////////////////////////
// Access to cache: brightness data
QVector < double > FractalWorker::GetBrightnessData() const
{
    return m_BrightnessCache;
}



///////////////////////////////////////////////////////////////////////////////
// Check if worker is idle
bool FractalWorker::IsIdle() const
{
    return m_IsIdle;
}



///////////////////////////////////////////////////////////////////////////////
// Get tile statistics
QHash < QString, QString > FractalWorker::GetStatistics() const
{
    QHash < QString, QString > statistics;
    statistics["processing time ms"] =
        QString("%1").arg(m_Statistics_ProcessingTime_ms);
    statistics["points finished long"] =
        QString("%1").arg(m_Statistics_PointsFinished);
    statistics["points in set long"] =
        QString("%1").arg(m_Statistics_PointsInSet);
    statistics["points out of bounds long"] =
        QString("%1").arg(m_Statistics_PointsOutOfBounds);
    statistics["total iterations long"] =
        QString("%1").arg(m_Statistics_TotalIterations);
    statistics["min depth"] = QString("%1").arg(m_Statistics_MinDepth);
    statistics["max depth"] = QString("%1").arg(m_Statistics_MaxDepth);
    statistics["min color value"] =
        QString("%1").arg(m_Statistics_MinColorValue);
    statistics["max color value"] =
        QString("%1").arg(m_Statistics_MaxColorValue);
    statistics["min brightness value"] =
        QString("%1").arg(m_Statistics_MinBrightnessValue);
    statistics["max brightness value"] =
        QString("%1").arg(m_Statistics_MaxBrightnessValue);
    return statistics;
}



///////////////////////////////////////////////////////////////////////////////
// Reset statistics
void FractalWorker::ResetStatistics()
{
    m_Statistics_FirstIteration = true;
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
}
