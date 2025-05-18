// MainWindow.h
// Class definition

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QProgressBar>
#include <QPushButton>
#include <QSlider>
#include <QTabWidget>

// Forward declaration
class FractalWidget;

// Class definition
class MainWindow
    : public QMainWindow
{
    Q_OBJECT
    
    
    
    // ============================================================== Lifecycle
private:
    // Constructor
    MainWindow();
    
public:
    // Destructor
    virtual ~MainWindow();
    
    // Instanciator
    static MainWindow * Instance();

private:
    // Instance
    static MainWindow * m_Instance;

    // Set some default preferences
    static bool InitPreferences();
    static bool m_PreferencesInitialized;


    
    // ==================================================================== GUI
private:
    // Initialize GUI
    void InitGUI();
    void InitGUI_Fractal();
    void InitGUI_Visualization();
    void InitGUI_Storage();
    void InitGUI_Optimizer();
    void InitGUI_Statistics();

    // Widgets
    QListWidget * m_OpenWindows;
    QList < int > m_OpenWindowsIDs;
    QLineEdit * m_Name;

    QTabWidget * m_Configuration;

    // === Fractal
    QComboBox * m_FractalType;
    QList < QWidget * > m_Julia_AllWidgets;
    QLineEdit * m_RealJulia;
    QLineEdit * m_ImagJulia;
    QLineEdit * m_RealMin;
    QLineEdit * m_RealMax;
    QLineEdit * m_ImagMin;
    QLineEdit * m_ImagMax;
    QLineEdit * m_MaxDepth;
    QLineEdit * m_EscapeRadius;

    QCheckBox * m_UseHighPrecision;
    QCheckBox * m_NewWindowForJulia;
    QCheckBox * m_NewWindowForZoom;

    // === Visualization
    // Presets
    QComboBox * m_ColorPreset;
    QPushButton * m_ColorPreset_Load;
    QPushButton * m_ColorPreset_Save;
    QPushButton * m_ColorPreset_Delete;

    // Color
    QComboBox * m_ColorBaseValue;
    QComboBox * m_ColorMappingMethod;

    QList < QWidget * > m_Ramp_AllWidgets;
    QLineEdit * m_Ramp_Offset;
    QLineEdit * m_Ramp_Factor;

    QList < QWidget * > m_Periodic_AllWidgets;
    QComboBox * m_Periodic_ColorScheme;
    QList < QWidget * > m_Periodic_Color_AllWidgets;
    QLineEdit * m_Periodic_OffsetR;
    QLineEdit * m_Periodic_FactorR;
    QLineEdit * m_Periodic_OffsetG;
    QLineEdit * m_Periodic_FactorG;
    QLineEdit * m_Periodic_OffsetB;
    QLineEdit * m_Periodic_FactorB;

    QList < QWidget * > m_Periodic_Greyscale_AllWidgets;
    QLineEdit * m_Periodic_GreyscaleOffset;
    QLineEdit * m_Periodic_GreyscaleFactor;

    // Brightness
    QComboBox * m_BrightnessValue;
    QList < QWidget * > m_StripAverage_AllWidgets;
    QLineEdit * m_StripAverage_FoldChange;

    QList < QWidget * > m_StripAverageAlt_AllWidgets;
    QLineEdit * m_StripAverageAlt_FoldChange;
    QLineEdit * m_StripAverageAlt_Regularity;
    QLineEdit * m_StripAverageAlt_Exponent;

    QList < QWidget * > m_StripAverageShared_AllWidgets;
    QLineEdit * m_StripAverage_Factor;
    QLineEdit * m_StripAverage_Offset;
    QLineEdit * m_StripAverage_MinBrightness;
    QSlider * m_StripAverage_MinBrightnessSlider;

    // Resolution
    QComboBox * m_Oversampling;
    QComboBox * m_AspectRatio;
    QCheckBox * m_FitToWindow;
    QLineEdit * m_Width;
    QLineEdit * m_Height;
    QLabel * m_ResolutionWarning;

    // == Storage
    QCheckBox * m_SavePicture;
    QCheckBox * m_SaveCacheData;
    QCheckBox * m_SaveCacheDataInMemory;
    QCheckBox * m_SaveStatistics;
    QPushButton * m_PickTargetDirectory;
    QLabel * m_TargetDirectory;

    // == Optimizer
    QComboBox * m_FirstParameter;
    QLineEdit * m_FirstParameter_Min;
    QLineEdit * m_FirstParameter_Max;
    QComboBox * m_FirstParameter_NumIntervals;
    QComboBox * m_SecondParameter;
    QLineEdit * m_SecondParameter_Min;
    QLineEdit * m_SecondParameter_Max;
    QComboBox * m_SecondParameter_NumIntervals;
    QCheckBox * m_IncludeParameterValues;
    QPushButton * m_StartOptimizer;

    // === Statistics
    QLabel * m_Stats_Status;
    QProgressBar * m_Stats_Progress;
    QLabel* m_Stats_ProgressText;
    QLabel * m_Stats_Started;
    QLabel * m_Stats_Finished;
    QLabel * m_Stats_Duration;
    QLabel * m_Stats_NumberOfThreads;
    QLabel * m_Stats_TotalPoints;
    QLabel * m_Stats_PointsInSet;
    QLabel * m_Stats_TotalIterations;
    QLabel * m_Stats_ColorValueRange;
    QLabel * m_Stats_BrightnessValueRange;

    // Everything else
    QPushButton * m_StartCalulation;
    QPushButton * m_StopCalulation;

private slots:
    // Fractal selected
    void FractalSelected();

    // Refresh values
    void Refresh_Values();

    // Refresh parts of the GUI
    void Refresh_Windows();
    void Refresh_FractalType();
    void Refresh_ColorBaseValue();
    void Refresh_ColorMappingMethod();
    void Refresh_ColorScheme();
    void Refresh_BrightnessValue();
    void Refresh_AspectRatio();
    void Refresh_FitToWindow();
    void NewOversampling();
    void NewWidth();
    void NewHeight();

    // Set values
    void UpdateName();
    void UpdateFractalInfo();
    void UpdateColorParameters();
    void UpdateBrightnessParameters();
    void StripAverageMinBrightnessChanged();

    // Storage
    void SelectTargetDirectory();
    void UpdateStorage();

    // Optimizer
    void Refresh_Optimizer();
    void StartOptimizer();

    // Start/Stop calculation
    void StartCalculation();
    void StopCalculation();

    // Calculate new image width/height when window size changes
    void FractalWindowSizeChanged(const int mcFractalWidgetID);

    // Set new range for a fractal
    void ZoomTo_LowPrecision(const int mcFractalWidgetID,
        double mNewRealMin, double mNewRealMax,
        double mNewImagMin, double mNewImagMax);
    void ZoomTo_HighPrecision(const int mcFractalWidgetID,
        long double mNewRealMin, long double mNewRealMax,
        long double mNewImagMin, long double mNewImagMax);

private:
    void InitActions();

    QAction * m_Action_SaveFractal;
    QAction * m_Action_SavePicture;

private slots:
    // Refresh Actions
    void Refresh_Actions();

    // Update statistics
    void Refresh_Statistics();

    // Periodic update
    void PeriodicUpdate(const int mcFractalWidgetID);

    // Finished
    void Finished(const int mcFractalWidgetID);



    // ================================================================ Presets
private slots:
    // Refresh available coloring presets (and buttons)
    void Refresh_Presets();

    // Load a preset
    void LoadPreset();

    // Save a preset
    void SavePreset();

    // Delete a preset
    void DeletePreset();



    // =========================================================== Menu Actions
public slots:
    // About window
    void About();

    // Quit MandelPoster
    void Quit();

    // Open fractal
    void OpenFractal();

    // Save fractal
    void SaveFractal();

    // Save fractal picture
    void SavePicture();

    // New fractal
    void CreateNewFractal();



    // ======================================================= Register Windows
public slots:
    // Create new default fractal image
    FractalWidget * NewFractal();

    // Create new Julia image
    FractalWidget * NewJuliaSet(const double mcJuliaReal,
        const double JuliaImag);
    FractalWidget * NewJuliaSet(const long double mcJuliaReal,
        const long double JuliaImag);

private:
    // Initialize fractal image widget
    void InitializeFractalWidget(const int mcFractalID);

private slots:
    // Select window
    void SelectWindow(const int mcFractalWidgetID);

    // Close window
    void CloseWindow(const int mcFractalWidgetID);

private:
    // List of open windows
    QHash < int, FractalWidget * > m_FractalWidgets;
    int m_NextFractalWidgetID;

    // Currently selected
    int m_CurrentFractalID;
    FractalWidget * m_CurrentFractalWidget;

    // Fractal count (for naming)
    int m_FractalCount;
};

#endif
