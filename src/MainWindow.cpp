// MainWindow.cpp
// Class definition

// Project includes
#include "Application.h"
#include "CallTracer.h"
#include "Fractal.h"
#include "FractalImage.h"
#include "FractalWidget.h"
#include "MainWindow.h"
#include "MessageLogger.h"
#include "Preferences.h"
#include "StringHelper.h"

// Qt includes
#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QWidget>

// Border in all direction that a fractal window has
#define FRACTAL_WINDOW_BORDER 20



// ================================================================== Lifecycle



///////////////////////////////////////////////////////////////////////////////
// Constructor
MainWindow::MainWindow()
{
    CALL_IN("");

    // We start with fractal 1
    m_FractalCount = 1;

    // Initialize fractal ID
    m_NextFractalWidgetID = 0;

    // Current fractal
    m_CurrentFractalID = -1;
    m_CurrentFractalWidget = nullptr;

    // Initialize Widgets
    InitGUI();
    Refresh_Values();
    Refresh_Presets();

    // Initialize Actions
    InitActions();

    // Show this window
    show();
    move(100,100);

    // Window shall stay on top, always
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    // Small widgets to save some space
    setAttribute(Qt::WA_MacSmallSize);

    // Create a new (default) fractal
    NewFractal();
    StartCalculation();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Destructor
MainWindow::~MainWindow()
{
    CALL_IN("");

    // Nothing to do.

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Instanciator
MainWindow * MainWindow::Instance()
{
    CALL_IN("");

    if (!m_Instance)
    {
        m_Instance = new MainWindow();
    }

    CALL_OUT("");
    return m_Instance;
}
    


///////////////////////////////////////////////////////////////////////////////
// Instance
MainWindow * MainWindow::m_Instance = nullptr;



///////////////////////////////////////////////////////////////////////////////
// Set some default preferences
bool MainWindow::InitPreferences()
{
    CALL_IN("");

    // Set default values
    Preferences * p = Preferences::Instance();
    p -> SetDefaultTagValue("GUI:New Window When Zooming", "no");
    p -> SetDefaultTagValue("GUI:New Window For Julia Set", "yes");

    // Done
    CALL_OUT("");
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// Trigger initialization of preferences
bool MainWindow::m_PreferencesInitialized = MainWindow::InitPreferences();



// ======================================================================== GUI



///////////////////////////////////////////////////////////////////////////////
// All GUI stuff
void MainWindow::InitGUI()
{
    CALL_IN("");

    QWidget * central_widget = new QWidget();
    setCentralWidget(central_widget);
    QHBoxLayout * layout = new QHBoxLayout();
    central_widget -> setLayout(layout);

    // == Left side...
    QVBoxLayout * left_layout = new QVBoxLayout();
    layout -> addLayout(left_layout);

    // Open windows
    QGroupBox * gb_openwindows = new QGroupBox(tr("Open Windows"));
    left_layout -> addWidget(gb_openwindows);
    QVBoxLayout * openwindows_layout = new QVBoxLayout();
    gb_openwindows -> setLayout(openwindows_layout);

    m_OpenWindows = new QListWidget();
    connect (m_OpenWindows, SIGNAL(currentRowChanged(int)),
        this, SLOT(FractalSelected()));
    openwindows_layout -> addWidget(m_OpenWindows);

    // Name
    QHBoxLayout * name_layout = new QHBoxLayout();
    openwindows_layout -> addLayout(name_layout);
    QLabel * l_name = new QLabel(tr("Name:"));
    name_layout -> addWidget(l_name);
    m_Name = new QLineEdit();
    connect (m_Name, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateName()));
    name_layout -> addWidget(m_Name);

    // == Right side...
    QVBoxLayout * right_layout = new QVBoxLayout();
    layout -> addLayout(right_layout);

    // Tab widget
    m_Configuration = new QTabWidget();
    right_layout -> addWidget(m_Configuration);

    // Initialize tab widget pages
    InitGUI_Fractal();
    InitGUI_Visualization();
    InitGUI_Storage();
    InitGUI_Optimizer();
    InitGUI_Statistics();

    // Buttons
    QHBoxLayout * bottom_layout = new QHBoxLayout();
    right_layout -> addLayout(bottom_layout);

    bottom_layout -> addStretch(1);

    m_StartCalulation = new QPushButton(tr("Start"));
    m_StartCalulation -> setFixedWidth(100);
    connect (m_StartCalulation, SIGNAL(clicked()),
        this, SLOT(StartCalculation()));
    bottom_layout -> addWidget(m_StartCalulation);

    m_StopCalulation = new QPushButton(tr("Stop"));
    m_StopCalulation -> setFixedWidth(100);
    connect (m_StopCalulation, SIGNAL(clicked()),
        this, SLOT(StopCalculation()));
    bottom_layout -> addWidget(m_StopCalulation);

    layout -> setStretch(0, 0);
    layout -> setStretch(1, 1);

    setFixedSize(900, 600);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// GUI stuff: Fractal
void MainWindow::InitGUI_Fractal()
{
    CALL_IN("");

    QWidget * main = new QWidget();
    m_Configuration -> addTab(main, tr("Fractal"));

    QGridLayout * main_layout = new QGridLayout();
    main -> setLayout(main_layout);
    int row = 0;

    // Type
    QLabel * l_type = new QLabel(tr("Fractal type"));
    main_layout -> addWidget(l_type, row, 0);

    m_FractalType = new QComboBox();
    m_FractalType -> addItem(tr("Mandelbrot Set"), "mandel");
    m_FractalType -> addItem(tr("Julia Set"), "julia");
    m_FractalType -> setFixedWidth(200);
    connect (m_FractalType, SIGNAL(currentIndexChanged(int)),
        this, SLOT(Refresh_FractalType()));
    connect (m_FractalType, SIGNAL(currentIndexChanged(int)),
        this, SLOT(UpdateFractalInfo()));
    main_layout -> addWidget(m_FractalType, row, 1);
    row++;

    // Julia constant
    QHBoxLayout * layout_julia_real = new QHBoxLayout();
    main_layout -> addLayout(layout_julia_real, row, 1);

    QLabel * l_julia = new QLabel(tr("Julia constant"));
    l_julia -> setFixedWidth(100);
    layout_julia_real -> addWidget(l_julia);

    m_RealJulia = new QLineEdit();
    connect (m_RealJulia, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateFractalInfo()));
    layout_julia_real -> addWidget(m_RealJulia);

    QLabel * l_julia_plus = new QLabel(tr("+"));
    l_julia_plus -> setFixedWidth(20);
    layout_julia_real -> addWidget(l_julia_plus);

    m_ImagJulia = new QLineEdit();
    connect (m_ImagJulia, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateFractalInfo()));
    layout_julia_real -> addWidget(m_ImagJulia);

    QLabel * l_julia_i = new QLabel(tr("i"));
    layout_julia_real -> addWidget(l_julia_i);

    m_Julia_AllWidgets << l_julia << m_RealJulia << l_julia_plus << m_ImagJulia
        << l_julia_i;
    row++;

    // Visible Range
    QLabel * l_range = new QLabel(tr("Range"));
    main_layout -> addWidget(l_range, row, 0);

    QHBoxLayout * layout_range_real = new QHBoxLayout();
    main_layout -> addLayout(layout_range_real, row, 1);
    row++;

    QLabel * l_range_real = new QLabel(tr("Real"));
    l_range_real -> setFixedWidth(100);
    layout_range_real -> addWidget(l_range_real);

    m_RealMin = new QLineEdit();
    m_RealMin -> setFixedWidth(130);
    connect (m_RealMin, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateFractalInfo()));
    layout_range_real -> addWidget(m_RealMin);

    QLabel * l_range_real_to = new QLabel(tr("to"));
    l_range_real_to -> setFixedWidth(20);
    layout_range_real -> addWidget(l_range_real_to);

    m_RealMax = new QLineEdit();
    m_RealMax -> setFixedWidth(130);
    connect (m_RealMax, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateFractalInfo()));
    layout_range_real -> addWidget(m_RealMax);
    layout_range_real -> addStretch(1);
    row++;

    QHBoxLayout * layout_range_imag = new QHBoxLayout();
    main_layout -> addLayout(layout_range_imag, row, 1);

    QLabel * l_range_imag = new QLabel(tr("Imaginary"));
    l_range_imag -> setFixedWidth(100);
    layout_range_imag -> addWidget(l_range_imag);

    m_ImagMin = new QLineEdit();
    m_ImagMin -> setFixedWidth(130);
    connect (m_ImagMin, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateFractalInfo()));
    layout_range_imag -> addWidget(m_ImagMin);

    QLabel * l_range_imag_to = new QLabel(tr("to"));
    l_range_imag_to -> setFixedWidth(20);
    layout_range_imag -> addWidget(l_range_imag_to);

    m_ImagMax = new QLineEdit();
    m_ImagMax -> setFixedWidth(130);
    connect (m_ImagMax, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateFractalInfo()));
    layout_range_imag -> addWidget(m_ImagMax);
    layout_range_imag -> addStretch(1);
    row++;

    // Max Depth
    QLabel * l_depth = new QLabel(tr("Maximum depth"));
    main_layout -> addWidget(l_depth, row, 0);

    m_MaxDepth = new QLineEdit();
    m_MaxDepth -> setFixedWidth(70);
    connect (m_MaxDepth, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateFractalInfo()));
    main_layout -> addWidget(m_MaxDepth, row, 1, Qt::AlignLeft);
    row++;

    // Escape Radius
    QLabel * l_escape = new QLabel(tr("Escape radius"));
    main_layout -> addWidget(l_escape, row, 0);

    m_EscapeRadius = new QLineEdit();
    m_EscapeRadius -> setFixedWidth(70);
    connect (m_EscapeRadius, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateFractalInfo()));
    main_layout -> addWidget(m_EscapeRadius, row, 1, Qt::AlignLeft);
    row++;

    // Stretch
    QWidget * dummy = new QWidget();
    main_layout -> addWidget(dummy, row, 0);
    main_layout -> setRowStretch(row, 1);
    row++;

    // Precision
    m_UseHighPrecision = new QCheckBox(tr("Use high precision (slower)"));
    connect (m_UseHighPrecision, SIGNAL(stateChanged(int)),
        this, SLOT(UpdateFractalInfo()));
    main_layout -> addWidget(m_UseHighPrecision, row, 0, 1, 2);
    row++;

    // New window for Julia set or zoom
    QHBoxLayout * window_layout = new QHBoxLayout();
    main_layout -> addLayout(window_layout, row, 0, 1, 2);

    Preferences * p = Preferences::Instance();
    m_NewWindowForZoom = new QCheckBox(tr("Open new window when zooming"));
    const QString open_when_zooming =
        p -> GetTagValue("GUI:New Window When Zooming");
    m_NewWindowForZoom -> setCheckState(
        open_when_zooming == "yes" ? Qt::Checked : Qt::Unchecked);
    window_layout -> addWidget(m_NewWindowForZoom);

    window_layout -> addSpacing(20);

    m_NewWindowForJulia = new QCheckBox(tr("Open new window for Julia set"));
    const QString open_for_julia =
        p -> GetTagValue("GUI:New Window For Julia Set");
    m_NewWindowForJulia -> setCheckState(
        open_for_julia == "yes" ? Qt::Checked : Qt::Unchecked);
    window_layout -> addWidget(m_NewWindowForJulia);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// GUI stuff: Visualization
void MainWindow::InitGUI_Visualization()
{
    CALL_IN("");

    QWidget * main = new QWidget();
    m_Configuration -> addTab(main, tr("Visualization"));

    QVBoxLayout * main_layout = new QVBoxLayout();
    main -> setLayout(main_layout);

    // Presets
    QGroupBox * gb_preset = new QGroupBox(tr("Preset"));
    main_layout -> addWidget(gb_preset);

    QHBoxLayout * preset_layout = new QHBoxLayout();
    gb_preset -> setLayout(preset_layout);

    preset_layout -> addStretch(1);

    m_ColorPreset = new QComboBox();
    m_ColorPreset -> setFixedWidth(200);
    m_ColorPreset -> addItem(tr("New preset"), -1);
    connect (m_ColorPreset, SIGNAL(currentIndexChanged(int)),
        this, SLOT(Refresh_Presets()));
    preset_layout -> addWidget(m_ColorPreset);

    m_ColorPreset_Load = new QPushButton(tr("Load"));
    m_ColorPreset_Load -> setFixedWidth(100);
    connect (m_ColorPreset_Load, SIGNAL(clicked()),
        this, SLOT(LoadPreset()));
    preset_layout -> addWidget(m_ColorPreset_Load);

    m_ColorPreset_Save = new QPushButton(tr("Save"));
    m_ColorPreset_Save -> setFixedWidth(100);
    connect (m_ColorPreset_Save, SIGNAL(clicked()),
        this, SLOT(SavePreset()));
    preset_layout -> addWidget(m_ColorPreset_Save);

    m_ColorPreset_Delete = new QPushButton(tr("Delete"));
    m_ColorPreset_Delete -> setFixedWidth(100);
    connect (m_ColorPreset_Delete, SIGNAL(clicked()),
        this, SLOT(DeletePreset()));
    preset_layout -> addWidget(m_ColorPreset_Delete);

    preset_layout -> addStretch(1);

    QHBoxLayout * box_layout = new QHBoxLayout();
    main_layout -> addLayout(box_layout);

    // == Colors
    QGroupBox * gb_color = new QGroupBox(tr("Colors"));
    gb_color -> setFixedWidth(300);
    box_layout -> addWidget(gb_color);

    QGridLayout * color_layout = new QGridLayout();
    gb_color -> setLayout(color_layout);
    int row = 0;

    QLabel * l_base = new QLabel(tr("Base value"));
    color_layout -> addWidget(l_base, row, 0);

    m_ColorBaseValue = new QComboBox();
    m_ColorBaseValue -> addItem(tr("continuous"), "continuous");
    m_ColorBaseValue -> addItem(tr("angle"), "angle");
    connect (m_ColorBaseValue, SIGNAL(currentIndexChanged(int)),
        this, SLOT(Refresh_ColorBaseValue()));
    color_layout -> addWidget(m_ColorBaseValue, row, 1);
    row++;

    QLabel * l_method = new QLabel(tr("Mapping method"));
    color_layout -> addWidget(l_method, row, 0);

    m_ColorMappingMethod = new QComboBox();
    m_ColorMappingMethod -> addItem(tr("periodic"), "periodic");
    m_ColorMappingMethod -> addItem(tr("ramp"), "ramp");
    connect (m_ColorMappingMethod, SIGNAL(currentIndexChanged(int)),
        this, SLOT(Refresh_ColorMappingMethod()));
    color_layout -> addWidget(m_ColorMappingMethod, row, 1);
    row++;

    // Ramp
    QLabel * l_ramp_offset = new QLabel(tr("Offset"));
    color_layout -> addWidget(l_ramp_offset, row, 0);

    m_Ramp_Offset = new QLineEdit();
    connect (m_Ramp_Offset, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateColorParameters()));
    color_layout -> addWidget(m_Ramp_Offset, row, 1);
    row++;

    QLabel * l_ramp_factor = new QLabel(tr("Factor"));
    color_layout -> addWidget(l_ramp_factor, row, 0);

    m_Ramp_Factor = new QLineEdit();
    connect (m_Ramp_Factor, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateColorParameters()));
    color_layout -> addWidget(m_Ramp_Factor, row, 1);
    row++;

    m_Ramp_AllWidgets << l_ramp_offset << m_Ramp_Offset << l_ramp_factor
        << m_Ramp_Factor;

    // Periodic
    QLabel * l_periodic_scheme = new QLabel(tr("Color Scheme"));
    color_layout -> addWidget(l_periodic_scheme, row, 0);

    m_Periodic_ColorScheme = new QComboBox();
    m_Periodic_ColorScheme -> addItem(tr("color"), "color");
    m_Periodic_ColorScheme -> addItem(tr("greyscale"), "greyscale");
    connect (m_Periodic_ColorScheme, SIGNAL(currentIndexChanged(int)),
        this, SLOT(Refresh_ColorScheme()));

    color_layout -> addWidget(m_Periodic_ColorScheme, row, 1);
    row++;

    m_Periodic_AllWidgets << l_periodic_scheme << m_Periodic_ColorScheme;

    QLabel * l_periodic_gr_offset = new QLabel(tr("Offset"));
    color_layout -> addWidget(l_periodic_gr_offset, row, 0);

    m_Periodic_GreyscaleOffset = new QLineEdit();
    connect (m_Periodic_GreyscaleOffset, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateColorParameters()));
    color_layout -> addWidget(m_Periodic_GreyscaleOffset, row, 1);
    row++;

    QLabel * l_periodic_gr_factor = new QLabel(tr("Factor"));
    color_layout -> addWidget(l_periodic_gr_factor, row, 0);

    m_Periodic_GreyscaleFactor = new QLineEdit();
    connect (m_Periodic_GreyscaleFactor, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateColorParameters()));
    color_layout -> addWidget(m_Periodic_GreyscaleFactor, row, 1);
    row++;

    m_Periodic_Greyscale_AllWidgets
        << l_periodic_gr_offset << m_Periodic_GreyscaleOffset
        << l_periodic_gr_factor << m_Periodic_GreyscaleFactor;
    m_Periodic_AllWidgets << m_Periodic_Greyscale_AllWidgets;

    QLabel * l_periodic_offset_r = new QLabel(tr("Offset - red"));
    color_layout -> addWidget(l_periodic_offset_r, row, 0);

    m_Periodic_OffsetR = new QLineEdit();
    connect (m_Periodic_OffsetR, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateColorParameters()));
    color_layout -> addWidget(m_Periodic_OffsetR, row, 1);
    row++;

    QLabel * l_periodic_offset_g = new QLabel(tr("Offset - green"));
    color_layout -> addWidget(l_periodic_offset_g, row, 0);

    m_Periodic_OffsetG = new QLineEdit();
    connect (m_Periodic_OffsetG, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateColorParameters()));
    color_layout -> addWidget(m_Periodic_OffsetG, row, 1);
    row++;

    QLabel * l_periodic_offset_b = new QLabel(tr("Offset - blue"));
    color_layout -> addWidget(l_periodic_offset_b, row, 0);

    m_Periodic_OffsetB = new QLineEdit();
    connect (m_Periodic_OffsetB, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateColorParameters()));
    color_layout -> addWidget(m_Periodic_OffsetB, row, 1);
    row++;

    QLabel * l_periodic_factor_r = new QLabel(tr("Factor - red"));
    color_layout -> addWidget(l_periodic_factor_r, row, 0);

    m_Periodic_FactorR = new QLineEdit();
    connect (m_Periodic_FactorR, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateColorParameters()));
    color_layout -> addWidget(m_Periodic_FactorR, row, 1);
    row++;

    QLabel * l_periodic_factor_g = new QLabel(tr("Factor - green"));
    color_layout -> addWidget(l_periodic_factor_g, row, 0);

    m_Periodic_FactorG = new QLineEdit();
    connect (m_Periodic_FactorG, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateColorParameters()));
    color_layout -> addWidget(m_Periodic_FactorG, row, 1);
    row++;

    QLabel * l_periodic_factor_b = new QLabel(tr("Factor - blue"));
    color_layout -> addWidget(l_periodic_factor_b, row, 0);

    m_Periodic_FactorB = new QLineEdit();
    connect (m_Periodic_FactorB, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateColorParameters()));
    color_layout -> addWidget(m_Periodic_FactorB, row, 1);
    row++;

    m_Periodic_Color_AllWidgets << l_periodic_offset_r << m_Periodic_OffsetR
        << l_periodic_offset_g << m_Periodic_OffsetG
        << l_periodic_offset_b << m_Periodic_OffsetB
        << l_periodic_factor_r << m_Periodic_FactorR
        << l_periodic_factor_g << m_Periodic_FactorG
        << l_periodic_factor_b << m_Periodic_FactorB;
    m_Periodic_AllWidgets << m_Periodic_Color_AllWidgets;

    QWidget * filler = new QWidget();
    color_layout -> addWidget(filler, row, 0);
    color_layout -> setRowStretch(row, 1);

    // Brightness
    QGroupBox * gb_brightness = new QGroupBox(tr("Brightness"));
    gb_brightness -> setFixedWidth(300);
    box_layout -> addWidget(gb_brightness);

    QGridLayout * brightness_layout = new QGridLayout();
    gb_brightness -> setLayout(brightness_layout);
    row = 0;

    QLabel * l_base_brightness = new QLabel(tr("Base value"));
    brightness_layout -> addWidget(l_base_brightness, row, 0);

    m_BrightnessValue = new QComboBox();
    m_BrightnessValue -> addItem(tr("flat"), "flat");
    m_BrightnessValue -> addItem(tr("strip average"), "strip average");
    m_BrightnessValue -> addItem(tr("alternative strip average"),
        "strip average alt");
    connect (m_BrightnessValue, SIGNAL(currentIndexChanged(int)),
        this, SLOT(Refresh_BrightnessValue()));
    brightness_layout -> addWidget(m_BrightnessValue, row, 1);
    row++;

    // Standard strip average
    QLabel * l_strip_fold = new QLabel(tr("Fold change"));
    brightness_layout -> addWidget(l_strip_fold, row, 0);

    m_StripAverage_FoldChange = new QLineEdit();
    connect (m_StripAverage_FoldChange, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateBrightnessParameters()));
    brightness_layout -> addWidget(m_StripAverage_FoldChange, row, 1);
    row++;

    m_StripAverage_AllWidgets << l_strip_fold << m_StripAverage_FoldChange;

    // Alternate strip average
    l_strip_fold = new QLabel(tr("Fold change"));
    brightness_layout -> addWidget(l_strip_fold, row, 0);

    m_StripAverageAlt_FoldChange = new QLineEdit();
    connect (m_StripAverageAlt_FoldChange,
        SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateBrightnessParameters()));
    brightness_layout -> addWidget(m_StripAverageAlt_FoldChange, row, 1);
    row++;

    QLabel * l_strip_regularity = new QLabel(tr("Regularity"));
    brightness_layout -> addWidget(l_strip_regularity, row, 0);

    m_StripAverageAlt_Regularity = new QLineEdit();
    connect (m_StripAverageAlt_Regularity,
        SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateBrightnessParameters()));
    brightness_layout -> addWidget(m_StripAverageAlt_Regularity, row, 1);
    row++;

    QLabel * l_strip_exponent = new QLabel(tr("Exponent"));
    brightness_layout -> addWidget(l_strip_exponent, row, 0);

    m_StripAverageAlt_Exponent = new QLineEdit();
    connect (m_StripAverageAlt_Exponent, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateBrightnessParameters()));
    brightness_layout -> addWidget(m_StripAverageAlt_Exponent, row, 1);
    row++;

    m_StripAverageAlt_AllWidgets
        << l_strip_fold << m_StripAverageAlt_FoldChange
        << l_strip_regularity << m_StripAverageAlt_Regularity
        << l_strip_exponent << m_StripAverageAlt_Exponent;

    // Shared
    QLabel * l_strip_factor = new QLabel(tr("Factor"));
    brightness_layout -> addWidget(l_strip_factor, row, 0);

    m_StripAverage_Factor = new QLineEdit();
    connect (m_StripAverage_Factor, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateBrightnessParameters()));
    brightness_layout -> addWidget(m_StripAverage_Factor, row, 1);
    row++;

    QLabel * l_strip_offset = new QLabel(tr("Offset"));
    brightness_layout -> addWidget(l_strip_offset, row, 0);

    m_StripAverage_Offset = new QLineEdit();
    connect (m_StripAverage_Offset, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateBrightnessParameters()));
    brightness_layout -> addWidget(m_StripAverage_Offset, row, 1);
    row++;

    QLabel * l_strip_min = new QLabel(tr("Min. brightness"));
    brightness_layout -> addWidget(l_strip_min, row, 0);

    m_StripAverage_MinBrightness = new QLineEdit();
    connect (m_StripAverage_MinBrightness,
        SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdateBrightnessParameters()));
    brightness_layout -> addWidget(m_StripAverage_MinBrightness, row, 1);
    row++;

    m_StripAverage_MinBrightnessSlider = new QSlider(Qt::Horizontal);
    m_StripAverage_MinBrightnessSlider -> setMinimum(0);
    m_StripAverage_MinBrightnessSlider -> setMaximum(100);
    m_StripAverage_MinBrightnessSlider -> setTickInterval(1);
    connect (m_StripAverage_MinBrightnessSlider, SIGNAL(valueChanged(int)),
        this, SLOT(StripAverageMinBrightnessChanged()));
    brightness_layout -> addWidget(m_StripAverage_MinBrightnessSlider, row, 1);
    row++;

    m_StripAverageShared_AllWidgets << l_strip_factor << m_StripAverage_Factor
        << l_strip_offset << m_StripAverage_Offset
        << l_strip_min << m_StripAverage_MinBrightness
        << m_StripAverage_MinBrightnessSlider;

    filler = new QWidget();
    brightness_layout -> addWidget(filler, row, 0);
    brightness_layout -> setRowStretch(row, 1);

    // == Resolution
    QGroupBox * gb_resolution = new QGroupBox(tr("Resolution"));
    main_layout -> addWidget(gb_resolution);

    QGridLayout * resolution_layout = new QGridLayout();
    gb_resolution -> setLayout(resolution_layout);
    row = 0;

    QLabel * l_oversampling = new QLabel(tr("Oversampling"));
    resolution_layout -> addWidget(l_oversampling, row, 0);

    m_Oversampling = new QComboBox();
    m_Oversampling -> addItem(tr("1x (none)"), 1);
    m_Oversampling -> addItem(tr("2"), 2);
    m_Oversampling -> addItem(tr("3"), 3);
    m_Oversampling -> addItem(tr("4"), 4);
    m_Oversampling -> addItem(tr("5"), 5);
    connect (m_Oversampling, SIGNAL(currentIndexChanged(int)),
        this, SLOT(NewOversampling()));
    resolution_layout -> addWidget(m_Oversampling, row, 1);
    row++;

    QLabel * l_resolution = new QLabel(tr("Image resolution"));
    resolution_layout -> addWidget(l_resolution, row, 0);

    m_FitToWindow = new QCheckBox(tr("Fit to window"));
    connect (m_FitToWindow, SIGNAL(stateChanged(int)),
        this, SLOT(Refresh_FitToWindow()));
    resolution_layout -> addWidget(m_FitToWindow, row, 1);
    row++;

    QLabel * l_aspect = new QLabel(tr("Aspect ratio"));
    resolution_layout -> addWidget(l_aspect, row, 0);

    m_AspectRatio = new QComboBox();
    m_AspectRatio -> addItem(tr("any"), "any");
    m_AspectRatio -> addItem(tr("1:1"), "1:1");
    m_AspectRatio -> addItem(tr("2:1"), "2:1");
    m_AspectRatio -> addItem(tr("3:2"), "3:2");
    m_AspectRatio -> addItem(tr("16:9"), "16:9");
    connect (m_AspectRatio, SIGNAL(currentIndexChanged(int)),
        this, SLOT(Refresh_AspectRatio()));
    resolution_layout -> addWidget(m_AspectRatio, row, 1);
    row++;

    QLabel * l_width_height = new QLabel(tr("Width and height"));
    resolution_layout -> addWidget(l_width_height, row, 0);

    m_Width = new QLineEdit();
    connect (m_Width, SIGNAL(textChanged(const QString &)),
        this, SLOT(NewWidth()));
    resolution_layout -> addWidget(m_Width, row, 1);

    QLabel * l_by = new QLabel(tr("by"));
    resolution_layout -> addWidget(l_by, row, 2);

    m_Height = new QLineEdit();
    connect (m_Height, SIGNAL(textChanged(const QString &)),
        this, SLOT(NewHeight()));
    resolution_layout -> addWidget(m_Height, row, 3);
    row++;

    m_ResolutionWarning = new QLabel();
    resolution_layout -> addWidget(m_ResolutionWarning, row, 0, 1, 3);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// GUI stuff: Optimizer
void MainWindow::InitGUI_Optimizer()
{
    CALL_IN("");

    QWidget * main = new QWidget();
    m_Configuration -> addTab(main, tr("Optimizer"));

    QVBoxLayout * main_layout = new QVBoxLayout();
    main -> setLayout(main_layout);

    // First parameter
    QGroupBox * gb_parameter = new QGroupBox(tr("First parameter (x)"));
    main_layout -> addWidget(gb_parameter);

    QGridLayout * layout = new QGridLayout();
    gb_parameter -> setLayout(layout);
    int row = 0;

    QLabel * l_parameter = new QLabel(tr("Parameter"));
    layout -> addWidget(l_parameter, row, 0);
    m_FirstParameter = new QComboBox();
    m_FirstParameter -> setFixedWidth(350);
    layout -> addWidget(m_FirstParameter, row, 1, 1, 3);
    row++;

    QLabel * l_range = new QLabel(tr("Range"));
    layout -> addWidget(l_range, row, 0);
    m_FirstParameter_Min = new QLineEdit();
    layout -> addWidget(m_FirstParameter_Min, row, 1);
    QLabel * l_to = new QLabel(tr("to"));
    layout -> addWidget(l_to, row, 2);
    m_FirstParameter_Max = new QLineEdit();
    layout -> addWidget(m_FirstParameter_Max, row, 3);
    row++;

    QLabel * l_intervals = new QLabel(tr("Intervals"));
    layout -> addWidget(l_intervals, row, 0);
    m_FirstParameter_NumIntervals = new QComboBox();
    m_FirstParameter_NumIntervals -> addItem("1", "1");
    m_FirstParameter_NumIntervals -> addItem("2", "2");
    m_FirstParameter_NumIntervals -> addItem("3", "3");
    m_FirstParameter_NumIntervals -> addItem("4", "4");
    m_FirstParameter_NumIntervals -> addItem("5", "5");
    m_FirstParameter_NumIntervals -> addItem("6", "6");
    m_FirstParameter_NumIntervals -> addItem("7", "7");
    m_FirstParameter_NumIntervals -> addItem("8", "8");
    m_FirstParameter_NumIntervals -> addItem("9", "9");
    m_FirstParameter_NumIntervals -> addItem("10", "10");
    m_FirstParameter_NumIntervals -> setFixedWidth(70);
    layout -> addWidget(m_FirstParameter_NumIntervals, row, 1, 1, 3);

    // Second parameter
    gb_parameter = new QGroupBox(tr("First parameter (x)"));
    main_layout -> addWidget(gb_parameter);

    layout -> setColumnStretch(0,0);
    layout -> setColumnStretch(1,0);
    layout -> setColumnStretch(2,0);
    layout -> setColumnStretch(3,0);
    layout -> setColumnStretch(4,1);

    layout = new QGridLayout();
    gb_parameter -> setLayout(layout);
    row = 0;

    l_parameter = new QLabel(tr("Parameter"));
    layout -> addWidget(l_parameter, row, 0);
    m_SecondParameter = new QComboBox();
    m_SecondParameter -> addItem(tr("none"), "");
    m_SecondParameter -> setFixedWidth(350);
    layout -> addWidget(m_SecondParameter, row, 1, 1, 3);
    row++;

    l_range = new QLabel(tr("Range"));
    layout -> addWidget(l_range, row, 0);
    m_SecondParameter_Min = new QLineEdit();
    layout -> addWidget(m_SecondParameter_Min, row, 1);
    l_to = new QLabel(tr("to"));
    layout -> addWidget(l_to, row, 2);
    m_SecondParameter_Max = new QLineEdit();
    layout -> addWidget(m_SecondParameter_Max, row, 3);
    row++;

    l_intervals = new QLabel(tr("Intervals"));
    layout -> addWidget(l_intervals, row, 0);
    m_SecondParameter_NumIntervals = new QComboBox();
    m_SecondParameter_NumIntervals -> addItem("1", "1");
    m_SecondParameter_NumIntervals -> addItem("2", "2");
    m_SecondParameter_NumIntervals -> addItem("3", "3");
    m_SecondParameter_NumIntervals -> addItem("4", "4");
    m_SecondParameter_NumIntervals -> addItem("5", "5");
    m_SecondParameter_NumIntervals -> addItem("6", "6");
    m_SecondParameter_NumIntervals -> addItem("7", "7");
    m_SecondParameter_NumIntervals -> addItem("8", "8");
    m_SecondParameter_NumIntervals -> addItem("9", "9");
    m_SecondParameter_NumIntervals -> addItem("10", "10");
    m_SecondParameter_NumIntervals -> setFixedWidth(70);

    layout -> addWidget(m_SecondParameter_NumIntervals, row, 1, 1, 3);

    layout -> setColumnStretch(0,0);
    layout -> setColumnStretch(1,0);
    layout -> setColumnStretch(2,0);
    layout -> setColumnStretch(3,0);
    layout -> setColumnStretch(4,1);

    m_IncludeParameterValues = new QCheckBox(tr("Show paramter values"));
    main_layout -> addWidget(m_IncludeParameterValues);

    main_layout -> addStretch(1);

    m_StartOptimizer = new QPushButton(tr("Start"));
    m_StartOptimizer -> setFixedWidth(100);
    connect (m_StartOptimizer, SIGNAL(clicked()),
        this, SLOT(StartOptimizer()));
    main_layout -> addWidget(m_StartOptimizer, 0, Qt::AlignRight);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// GUI stuff: Storage
void MainWindow::InitGUI_Storage()
{
    CALL_IN("");

    QWidget * main = new QWidget();
    m_Configuration -> addTab(main, tr("Storage"));

    QVBoxLayout * main_layout = new QVBoxLayout();
    main_layout -> setSpacing(15);
    main -> setLayout(main_layout);

    QLabel * l_intro = new QLabel(tr("During and after computation..."));
    main_layout -> addWidget(l_intro);

    m_SavePicture = new QCheckBox(tr("Save picture"));
    connect (m_SavePicture, SIGNAL(stateChanged(int)),
        this, SLOT(UpdateStorage()));
    main_layout -> addWidget(m_SavePicture);

    m_SaveCacheData = new QCheckBox(tr("Save cache data on disk"));
    connect (m_SaveCacheData, SIGNAL(stateChanged(int)),
        this, SLOT(UpdateStorage()));
    main_layout -> addWidget(m_SaveCacheData);

    m_SaveCacheDataInMemory = new QCheckBox(tr("Save cache data in memory"));
    connect (m_SaveCacheDataInMemory, SIGNAL(stateChanged(int)),
        this, SLOT(UpdateStorage()));
    main_layout -> addWidget(m_SaveCacheDataInMemory);

    m_SaveStatistics = new QCheckBox(tr("Save statistics"));
    connect (m_SaveStatistics, SIGNAL(stateChanged(int)),
        this, SLOT(UpdateStorage()));
    main_layout -> addWidget(m_SaveStatistics);

    m_PickTargetDirectory = new QPushButton(tr("Select target directory"));
    connect (m_PickTargetDirectory, SIGNAL(clicked()),
        this, SLOT(SelectTargetDirectory()));
    m_PickTargetDirectory -> setFixedWidth(170);
    main_layout -> addWidget(m_PickTargetDirectory);

    m_TargetDirectory = new QLabel();
    Preferences * p = Preferences::Instance();
    const QString directory = p -> GetTagValue("Application:SaveDirectory");
    m_TargetDirectory -> setText(directory);
    main_layout -> addWidget(m_TargetDirectory);

    main_layout -> addStretch(1);

    QLabel * l_explanation = new QLabel(tr("For large resolution, cached "
        "values for color and brightness cannot be stored in memory; storing "
        "them on your hard drive enables later manipulation without "
        "recomputation of the actual fractal. It also enables continuing a "
        "very long calculation.\n"
        "Cached values use up large amounts of space."));
    l_explanation -> setWordWrap(true);
    main_layout -> addWidget(l_explanation);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// GUI stuff: Statistics
void MainWindow::InitGUI_Statistics()
{
    CALL_IN("");

    QWidget * main = new QWidget();
    m_Configuration -> addTab(main, tr("Statistics"));

    QGridLayout * main_layout = new QGridLayout();
    main -> setLayout(main_layout);
    int row = 0;

    QLabel * l_status = new QLabel(tr("Calculation status"));
    main_layout -> addWidget(l_status, row, 0);
    m_Stats_Status = new QLabel();
    main_layout -> addWidget(m_Stats_Status, row, 1);
    main_layout -> setRowStretch(row, 0);
    row++;

    QLabel * l_progress = new QLabel(tr("Progress"));
    main_layout -> addWidget(l_progress, row, 0);
    QHBoxLayout * layout_progress = new QHBoxLayout();
    m_Stats_Progress = new QProgressBar();
    m_Stats_Progress -> setMinimum(0);
    m_Stats_Progress -> setMaximum(100);
    m_Stats_Progress -> setValue(0);
    layout_progress -> addWidget(m_Stats_Progress);
    m_Stats_ProgressText = new QLabel();
    m_Stats_ProgressText -> setText("0%");
    layout_progress -> addWidget(m_Stats_ProgressText);
    main_layout -> addLayout(layout_progress, row, 1);
    main_layout -> setRowStretch(row, 0);
    row++;

    QLabel * l_started = new QLabel(tr("Calculation started"));
    main_layout -> addWidget(l_started, row, 0);
    m_Stats_Started = new QLabel();
    main_layout -> addWidget(m_Stats_Started, row, 1);
    main_layout -> setRowStretch(row, 0);
    row++;

    QLabel * l_finished = new QLabel(tr("Calculation finished"));
    main_layout -> addWidget(l_finished, row, 0);
    m_Stats_Finished = new QLabel();
    main_layout -> addWidget(m_Stats_Finished, row, 1);
    main_layout -> setRowStretch(row, 0);
    row++;

    QLabel * l_duration = new QLabel(tr("CPU Time"));
    main_layout -> addWidget(l_duration, row, 0);
    m_Stats_Duration = new QLabel();
    main_layout -> addWidget(m_Stats_Duration, row, 1);
    main_layout -> setRowStretch(row, 0);
    row++;

    QLabel * l_threads = new QLabel(tr("Threads used"));
    main_layout -> addWidget(l_threads, row, 0);
    m_Stats_NumberOfThreads = new QLabel();
    main_layout -> addWidget(m_Stats_NumberOfThreads, row, 1);
    main_layout -> setRowStretch(row, 0);
    row++;

    QLabel * l_pixels = new QLabel(tr("Total points"));
    main_layout -> addWidget(l_pixels, row, 0);
    m_Stats_TotalPoints = new QLabel();
    main_layout -> addWidget(m_Stats_TotalPoints, row, 1);
    main_layout -> setRowStretch(row, 0);
    row++;

    QLabel * l_inset = new QLabel(tr("Points in set"));
    main_layout -> addWidget(l_inset, row, 0);
    m_Stats_PointsInSet = new QLabel();
    main_layout -> addWidget(m_Stats_PointsInSet, row, 1);
    main_layout -> setRowStretch(row, 0);
    row++;

    QLabel * l_iterations = new QLabel(tr("Total iterations"));
    main_layout -> addWidget(l_iterations, row, 0);
    m_Stats_TotalIterations = new QLabel();
    main_layout -> addWidget(m_Stats_TotalIterations, row, 1);
    main_layout -> setRowStretch(row, 0);
    row++;

    QLabel * l_color = new QLabel(tr("Color value range"));
    main_layout -> addWidget(l_color, row, 0);
    m_Stats_ColorValueRange = new QLabel();
    main_layout -> addWidget(m_Stats_ColorValueRange, row, 1);
    main_layout -> setRowStretch(row, 0);
    row++;

    QLabel * l_brightness = new QLabel(tr("Brightness value range"));
    main_layout -> addWidget(l_brightness, row, 0);
    m_Stats_BrightnessValueRange = new QLabel();
    main_layout -> addWidget(m_Stats_BrightnessValueRange, row, 1);
    main_layout -> setRowStretch(row, 0);
    row++;

    QWidget * filler = new QWidget();
    main_layout -> addWidget(filler, row, 0);
    main_layout -> setRowStretch(row, 1);

    main_layout -> setColumnStretch(0, 0);
    main_layout -> setColumnStretch(1, 1);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// A new fractal has been selected
void MainWindow::FractalSelected()
{
    CALL_IN("");

    const int row = m_OpenWindows -> currentRow();
    if (row == -1)
    {
        m_CurrentFractalID = -1;
        m_CurrentFractalWidget = nullptr;
    } else
    {
        m_CurrentFractalID = m_OpenWindowsIDs[row];
        m_CurrentFractalWidget = m_FractalWidgets[m_CurrentFractalID];
        m_CurrentFractalWidget -> raise();
    }

    Refresh_Values();
    Refresh_Actions();
    Refresh_Optimizer();
    Refresh_Statistics();

    // Update size
    FractalWindowSizeChanged(m_CurrentFractalID);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Refresh values
void MainWindow::Refresh_Values()
{
    CALL_IN("");

    // Get parameters
    QHash < QString, QString > parameters;
    Fractal * fractal = nullptr;
    if (m_CurrentFractalWidget != nullptr)
    {
        fractal = m_CurrentFractalWidget -> GetFractal();
        parameters = fractal -> GetAllParameters();
    } else
    {
        // Nothing selected
        parameters["name"] = "";
        parameters["fractal type"] = "mandel";
        parameters["precision"] = "double";
        parameters["real min"] = "-2.2";
        parameters["real max"] = "0.7";
        parameters["imag min"] = "-1.3";
        parameters["imag max"] = "1.3";
        parameters["depth"] = "1000";
        parameters["escape radius"] = "4000";
        parameters["julia real"] = "0";
        parameters["julia imag"] = "0";

        parameters["oversampling"] = "1";
        parameters["use fixed resolution"] = "no";
        if (parameters["use fixed resolution"] == "yes")
        {
            parameters["fixed resolution width"] = "100";
            parameters["fixed resolution height"] = "100";
        }
        parameters["aspect ratio"] = "any";

        parameters["color base value"] = "continuous";
        parameters["color mapping method"] = "periodic";
        parameters["color scheme"] = "color";
        if (parameters["color mapping method"] == "periodic")
        {
            parameters["color factor red"] = "1";
            parameters["color offset red"] = "0.1";
            parameters["color factor green"] = "2";
            parameters["color offset green"] = "0.2";
            parameters["color factor blue"] = "3";
            parameters["color offset blue"] = "0.4";
        }
        if (parameters["color mapping method"] == "ramp")
        {
            parameters["color factor"] = "0.05";
            parameters["color offset"] = "40";
        }
        parameters["brightness value"] = "flat";
        if (parameters["brightness value"] == "flat")
        {
            // Nothing to do.
        }
        if (parameters["brightness value"] == "strip average")
        {
            parameters["brightness fold change"] = "3";
            parameters["brightness factor"] = "10";
            parameters["brightness offset"] = "0";
            parameters["brightness min brightness"] = "0.2";
        }
        if (parameters["brightness value"] == "strip average alt")
        {
            parameters["brightness fold change"] = "3";
            parameters["brightness regularity"] = "-0.7";
            parameters["brightness exponent"] = "4";
            parameters["brightness factor"] = "10";
            parameters["brightness offset"] = "0";
            parameters["brightness min brightness"] = "0.2";
        }
        parameters["storage directory"] = "";
        parameters["storage save picture"] = "no";
        parameters["storage save cache data to disk"] = "no";
        parameters["storage save cache data to memory"] = "yes";
        parameters["storage save statistics"] = "no";
    }

    // Set parameters in GUI
    m_Name -> blockSignals(true);
    m_Name -> setText(parameters["name"]);
    m_Name -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_Name -> blockSignals(false);

    m_FractalType -> blockSignals(true);
    int idx = m_FractalType -> findData(parameters["fractal type"]);
    m_FractalType -> setCurrentIndex(idx);
    m_FractalType -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_FractalType -> blockSignals(false);

    const bool has_julia = (parameters["fractal type"] == "julia");
    if (has_julia)
    {
        m_RealJulia -> blockSignals(true);
        m_ImagJulia -> blockSignals(true);
        m_RealJulia -> setText(parameters["julia real"]);
        m_ImagJulia -> setText(parameters["julia imag"]);
        m_RealJulia -> blockSignals(false);
        m_ImagJulia -> blockSignals(false);
    }
    for (QWidget * widget : m_Julia_AllWidgets)
    {
        widget -> setVisible(has_julia);
    }
    m_RealJulia -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_ImagJulia -> setEnabled(m_CurrentFractalWidget != nullptr);

    m_RealMin -> blockSignals(true);
    m_RealMax -> blockSignals(true);
    m_ImagMin -> blockSignals(true);
    m_ImagMax -> blockSignals(true);
    m_RealMin -> setText(parameters["real min"]);
    m_RealMin -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_RealMax -> setText(parameters["real max"]);
    m_RealMax -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_ImagMin -> setText(parameters["imag min"]);
    m_ImagMin -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_ImagMax -> setText(parameters["imag max"]);
    m_ImagMax -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_RealMin -> blockSignals(false);
    m_RealMax -> blockSignals(false);
    m_ImagMin -> blockSignals(false);
    m_ImagMax -> blockSignals(false);

    m_MaxDepth -> blockSignals(true);
    m_MaxDepth-> setText(parameters["depth"]);
    m_MaxDepth -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_MaxDepth -> blockSignals(false);

    m_EscapeRadius -> blockSignals(true);
    m_EscapeRadius -> setText(parameters["escape radius"]);
    m_EscapeRadius -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_EscapeRadius -> blockSignals(false);

    m_UseHighPrecision -> blockSignals(true);
    m_UseHighPrecision -> setCheckState(
        parameters["precision"] == "long double" ?
            Qt::Checked : Qt::Unchecked);
    m_UseHighPrecision -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_UseHighPrecision -> blockSignals(false);
    m_NewWindowForJulia -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_NewWindowForZoom -> setEnabled(m_CurrentFractalWidget != nullptr);

    m_ColorBaseValue -> blockSignals(true);
    idx = m_ColorBaseValue -> findData(parameters["color base value"]);
    m_ColorBaseValue -> setCurrentIndex(idx);
    m_ColorBaseValue -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_ColorBaseValue -> blockSignals(false);

    m_ColorMappingMethod -> blockSignals(true);
    idx = m_ColorMappingMethod -> findData(parameters["color mapping method"]);
    m_ColorMappingMethod -> setCurrentIndex(idx);
    m_ColorMappingMethod -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_ColorMappingMethod -> blockSignals(false);

    if (parameters["color mapping method"] == "ramp")
    {
        m_Ramp_Offset -> blockSignals(true);
        m_Ramp_Factor -> blockSignals(true);
        m_Ramp_Offset -> setText(parameters["color offset"]);
        m_Ramp_Offset -> setEnabled(m_CurrentFractalWidget != nullptr);
        m_Ramp_Factor -> setText(parameters["color factor"]);
        m_Ramp_Factor -> setEnabled(m_CurrentFractalWidget != nullptr);
        m_Ramp_Offset -> blockSignals(false);
        m_Ramp_Factor -> blockSignals(false);
        for (QWidget * widget : m_Ramp_AllWidgets)
        {
            widget -> setVisible(true);
        }
        for (QWidget * widget : m_Periodic_AllWidgets)
        {
            widget -> setVisible(false);
        }
    }
    if (parameters["color mapping method"] == "periodic")
    {
        for (QWidget * widget : m_Periodic_AllWidgets)
        {
            widget -> setVisible(true);
        }
        for (QWidget * widget : m_Ramp_AllWidgets)
        {
            widget -> setVisible(false);
        }
        m_Periodic_ColorScheme -> blockSignals(true);
        idx = m_Periodic_ColorScheme -> findData(parameters["color scheme"]);
        m_Periodic_ColorScheme -> setCurrentIndex(idx);
        m_Periodic_ColorScheme -> setEnabled(
            m_CurrentFractalWidget != nullptr);
        m_Periodic_ColorScheme -> blockSignals(false);
        if (parameters["color scheme"] == "color")
        {
            m_Periodic_OffsetR -> blockSignals(true);
            m_Periodic_FactorR -> blockSignals(true);
            m_Periodic_OffsetG -> blockSignals(true);
            m_Periodic_FactorG -> blockSignals(true);
            m_Periodic_OffsetB -> blockSignals(true);
            m_Periodic_FactorB -> blockSignals(true);
            m_Periodic_OffsetR -> setText(parameters["color offset red"]);
            m_Periodic_OffsetR -> setEnabled(
                m_CurrentFractalWidget != nullptr);
            m_Periodic_FactorR -> setText(parameters["color factor red"]);
            m_Periodic_FactorR -> setEnabled(
                m_CurrentFractalWidget != nullptr);
            m_Periodic_OffsetG -> setText(parameters["color offset green"]);
            m_Periodic_OffsetG -> setEnabled(
                m_CurrentFractalWidget != nullptr);
            m_Periodic_FactorG -> setText(parameters["color factor green"]);
            m_Periodic_FactorG -> setEnabled(
                m_CurrentFractalWidget != nullptr);
            m_Periodic_OffsetB -> setText(parameters["color offset blue"]);
            m_Periodic_OffsetB -> setEnabled(
                m_CurrentFractalWidget != nullptr);
            m_Periodic_FactorB -> setText(parameters["color factor blue"]);
            m_Periodic_FactorB -> setEnabled(
                m_CurrentFractalWidget != nullptr);
            m_Periodic_OffsetR -> blockSignals(false);
            m_Periodic_FactorR -> blockSignals(false);
            m_Periodic_OffsetG -> blockSignals(false);
            m_Periodic_FactorG -> blockSignals(false);
            m_Periodic_OffsetB -> blockSignals(false);
            m_Periodic_FactorB -> blockSignals(false);
            for (QWidget * widget : m_Periodic_Greyscale_AllWidgets)
            {
                widget -> setVisible(false);
            }
        }
        if (parameters["color scheme"] == "greyscale")
        {
            m_Periodic_GreyscaleOffset -> blockSignals(true);
            m_Periodic_GreyscaleFactor -> blockSignals(true);
            m_Periodic_GreyscaleOffset -> setText(parameters["color offset"]);
            m_Periodic_GreyscaleOffset -> setEnabled(
                m_CurrentFractalWidget != nullptr);
            m_Periodic_GreyscaleFactor -> setText(parameters["color factor"]);
            m_Periodic_GreyscaleFactor -> setEnabled(
                m_CurrentFractalWidget != nullptr);
            m_Periodic_GreyscaleOffset -> blockSignals(false);
            m_Periodic_GreyscaleFactor -> blockSignals(false);
            for (QWidget * widget : m_Periodic_Color_AllWidgets)
            {
                widget -> setVisible(false);
            }
        }
    }

    m_BrightnessValue -> blockSignals(true);
    idx = m_BrightnessValue -> findData(parameters["brightness value"]);
    m_BrightnessValue -> setCurrentIndex(idx);
    m_BrightnessValue -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_BrightnessValue -> blockSignals(false);

    const bool has_brightness_strip =
        (parameters["brightness value"] == "strip average");
    const bool has_brightness_strip_alt =
        (parameters["brightness value"] == "strip average alt");
    for (QWidget * widget : m_StripAverage_AllWidgets)
    {
        widget -> setVisible(false);
    }
    for (QWidget * widget : m_StripAverageAlt_AllWidgets)
    {
        widget -> setVisible(false);
    }
    for (QWidget * widget : m_StripAverageShared_AllWidgets)
    {
        widget -> setVisible(false);
    }
    if (has_brightness_strip ||
        has_brightness_strip_alt)
    {
        if (has_brightness_strip)
        {
            m_StripAverage_FoldChange -> blockSignals(true);
            m_StripAverage_FoldChange -> setText(
                parameters["brightness fold change"]);
            m_StripAverage_FoldChange -> setEnabled(
                m_CurrentFractalWidget != nullptr);
            m_StripAverage_FoldChange -> blockSignals(false);
        }
        if (has_brightness_strip_alt)
        {
            m_StripAverageAlt_FoldChange -> blockSignals(true);
            m_StripAverageAlt_Regularity -> blockSignals(true);
            m_StripAverageAlt_Exponent -> blockSignals(true);
            m_StripAverageAlt_FoldChange -> setText(
                parameters["brightness fold change"]);
            m_StripAverageAlt_FoldChange -> setEnabled(
                m_CurrentFractalWidget != nullptr);
            m_StripAverageAlt_Regularity -> setText(
                parameters["brightness regularity"]);
            m_StripAverageAlt_Regularity -> setEnabled(
                m_CurrentFractalWidget != nullptr);
            m_StripAverageAlt_Exponent -> setText(
                parameters["brightness exponent"]);
            m_StripAverageAlt_Exponent -> setEnabled(
                m_CurrentFractalWidget != nullptr);
            m_StripAverageAlt_FoldChange -> blockSignals(false);
            m_StripAverageAlt_Regularity -> blockSignals(false);
            m_StripAverageAlt_Exponent -> blockSignals(false);
        }
        m_StripAverage_Factor -> blockSignals(true);
        m_StripAverage_Offset -> blockSignals(true);
        m_StripAverage_MinBrightness -> blockSignals(true);
        m_StripAverage_MinBrightnessSlider -> blockSignals(true);
        m_StripAverage_Factor -> setText(parameters["brightness factor"]);
        m_StripAverage_Factor -> setEnabled(m_CurrentFractalWidget != nullptr);
        m_StripAverage_Offset -> setText(parameters["brightness offset"]);
        m_StripAverage_Offset -> setEnabled(m_CurrentFractalWidget != nullptr);
        m_StripAverage_MinBrightness -> setText(
            parameters["brightness min brightness"]);
        m_StripAverage_MinBrightness -> setEnabled(
            m_CurrentFractalWidget != nullptr);
        m_StripAverage_MinBrightnessSlider -> setValue(
            parameters["brightmess min brightness"].toDouble() * 100);
        m_StripAverage_MinBrightnessSlider -> setEnabled(
            m_CurrentFractalWidget != nullptr);
        m_StripAverage_Factor -> blockSignals(false);
        m_StripAverage_Offset -> blockSignals(false);
        m_StripAverage_MinBrightness -> blockSignals(false);
        m_StripAverage_MinBrightnessSlider -> blockSignals(false);
        for (QWidget * widget : m_StripAverage_AllWidgets)
        {
            widget -> setVisible(has_brightness_strip);
        }
        for (QWidget * widget : m_StripAverageAlt_AllWidgets)
        {
            widget -> setVisible(has_brightness_strip_alt);
        }
        for (QWidget * widget : m_StripAverageShared_AllWidgets)
        {
            widget -> setVisible(true);
        }
    }

    m_Oversampling -> blockSignals(true);
    idx = m_Oversampling -> findData(parameters["oversampling"].toInt());
    m_Oversampling -> setCurrentIndex(idx);
    m_Oversampling -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_Oversampling -> blockSignals(false);

    m_AspectRatio -> blockSignals(true);
    idx = m_AspectRatio -> findData(parameters["aspect ratio"]);
    m_AspectRatio -> setCurrentIndex(idx);
    m_AspectRatio -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_AspectRatio -> blockSignals(false);

    m_FitToWindow -> blockSignals(true);
    if (parameters["use fixed resolution"] == "yes")
    {
        m_FitToWindow -> setCheckState(Qt::Unchecked);
        m_Width -> blockSignals(true);
        m_Height -> blockSignals(true);
        m_Width -> setText(parameters["fixed resolution width"]);
        m_Width -> setEnabled(m_CurrentFractalWidget != nullptr);
        m_Height -> setText(parameters["fixed resolution height"]);
        m_Height -> setEnabled(m_CurrentFractalWidget != nullptr);
        m_Width -> blockSignals(false);
        m_Height -> blockSignals(false);
    } else
    {
        m_FitToWindow -> setCheckState(Qt::Checked);
        m_Width -> setEnabled(false);
        m_Height -> setEnabled(false);
    }
    m_FitToWindow -> blockSignals(false);
    m_FitToWindow -> setEnabled(m_CurrentFractalWidget != nullptr);

    // Storage
    m_TargetDirectory -> setText(parameters["storage directory"]);
    m_SavePicture -> blockSignals(true);
    m_SaveCacheData -> blockSignals(true);
    m_SaveCacheDataInMemory -> blockSignals(true);
    m_SaveStatistics -> blockSignals(true);
    m_SavePicture -> setCheckState(parameters["storage save picture"]
        == "yes" ? Qt::Checked : Qt::Unchecked);
    m_SaveCacheData -> setCheckState(
        parameters["storage save cache data to disk"] == "yes" ?
            Qt::Checked : Qt::Unchecked);
    m_SaveCacheDataInMemory -> setCheckState(
        parameters["storage save cache data to memory"] == "yes" ?
            Qt::Checked : Qt::Unchecked);
    m_SaveStatistics -> setCheckState(parameters["storage save statistics"]
        == "yes" ? Qt::Checked : Qt::Unchecked);
    m_SavePicture -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_SaveCacheData -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_SaveCacheDataInMemory -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_SaveStatistics -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_PickTargetDirectory -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_SavePicture -> blockSignals(false);
    m_SaveCacheData -> blockSignals(false);
    m_SaveCacheDataInMemory -> blockSignals(false);
    m_SaveStatistics -> blockSignals(false);

    // Statistics
    Refresh_Statistics();

    if (m_CurrentFractalWidget)
    {
        FractalImage * fractal_image =
            m_CurrentFractalWidget -> GetFractalImage();
        if (fractal_image -> GetRenderStatus() == "working")
        {
            m_StartCalulation -> setVisible(false);
            m_StopCalulation -> setVisible(true);
        } else
        {
            m_StartCalulation -> setVisible(true);
            m_StopCalulation -> setVisible(false);
        }
    } else
    {
        m_StartCalulation -> setVisible(false);
        m_StopCalulation -> setVisible(true);
    }
    m_StartCalulation -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_StopCalulation -> setEnabled(m_CurrentFractalWidget != nullptr);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Refresh windows
void MainWindow::Refresh_Windows()
{
    CALL_IN("");

    // Preservation of index must happen elsewhere.

    // Sort fractals by name
    QHash < int, QString > by_name;
    for (auto id_iterator = m_FractalWidgets.keyBegin();
         id_iterator != m_FractalWidgets.keyEnd();
         id_iterator++)
    {
        const int id = *id_iterator;
        FractalWidget * fractal_widget = m_FractalWidgets[id];
        const Fractal * fractal = fractal_widget -> GetFractal();
        by_name[id] = fractal -> GetName();
    }
    m_OpenWindowsIDs = StringHelper::SortHash(by_name);

    // Update list
    m_OpenWindows -> blockSignals(true);
    m_OpenWindows -> clear();
    for (int id : m_OpenWindowsIDs)
    {
        if (by_name[id].isEmpty())
        {
            m_OpenWindows -> addItem(tr("(no name)"));
        } else
        {
            m_OpenWindows -> addItem(by_name[id]);
        }
    }
    m_OpenWindows -> blockSignals(false);

    // Set current fractal window
    const int row = m_OpenWindows -> currentRow();
    if (row == -1)
    {
        m_CurrentFractalID = -1;
        m_CurrentFractalWidget = nullptr;
    } else
    {
        m_CurrentFractalID = m_OpenWindowsIDs[row];
        m_CurrentFractalWidget = m_FractalWidgets[m_CurrentFractalID];
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Refresh after fractal type selection changed
void MainWindow::Refresh_FractalType()
{
    CALL_IN("");

    // New status
    int idx = m_FractalType -> currentIndex();
    const QString new_fractal_type = m_FractalType -> itemData(idx).toString();
    const bool has_julia = (new_fractal_type == "julia");

    // Get fractal
    const Fractal * fractal = m_CurrentFractalWidget -> GetFractal();

    // Update a few things
    if (has_julia)
    {
        const QHash < QString, QString> julia_constant =
            fractal -> GetJuliaConstant();
        m_RealJulia -> blockSignals(true);
        m_ImagJulia -> blockSignals(true);
        m_RealJulia -> setText(julia_constant["julia real"]);
        m_ImagJulia -> setText(julia_constant["julia imag"]);
        m_RealJulia -> blockSignals(false);
        m_ImagJulia -> blockSignals(false);
    }
    for (QWidget * widget : m_Julia_AllWidgets)
    {
        widget -> setVisible(has_julia);
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Refresh after change in color base value
void MainWindow::Refresh_ColorBaseValue()
{
    CALL_IN("");

    // New status
    int idx = m_ColorBaseValue-> currentIndex();
    const QString new_value = m_ColorBaseValue -> itemData(idx).toString();

    // Get fractal
    Fractal * fractal = m_CurrentFractalWidget -> GetFractal();
    /// !!!! why is the fractal being updated here?
    fractal -> SetColorBaseValue(new_value);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Refresh after change in color mapping method
void MainWindow::Refresh_ColorMappingMethod()
{
    CALL_IN("");

    // New status
    int idx = m_ColorMappingMethod -> currentIndex();
    const QString new_method =
        m_ColorMappingMethod -> itemData(idx).toString();

    // Get fractal
    Fractal * fractal = m_CurrentFractalWidget -> GetFractal();
    fractal -> SetColorMappingMethod(new_method);
    const QHash < QString, QString > parameters =
        fractal -> GetColorParameters();

    // Update things
    if (new_method == "ramp")
    {
        m_Ramp_Offset -> blockSignals(true);
        m_Ramp_Factor -> blockSignals(true);
        m_Ramp_Offset -> setText(parameters["color offset"]);
        m_Ramp_Factor -> setText(parameters["color factor"]);
        m_Ramp_Offset -> blockSignals(false);
        m_Ramp_Factor -> blockSignals(false);
        for (QWidget * widget : m_Ramp_AllWidgets)
        {
            widget -> setVisible(true);
        }
        for (QWidget * widget : m_Periodic_AllWidgets)
        {
            widget -> setVisible(false);
        }
    }
    if (new_method == "periodic")
    {
        for (QWidget * widget : m_Ramp_AllWidgets)
        {
            widget -> setVisible(false);
        }
        Refresh_ColorScheme();
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Refresh after change in color schenme
void MainWindow::Refresh_ColorScheme()
{
    CALL_IN("");

    // New status
    int idx = m_Periodic_ColorScheme-> currentIndex();
    const QString new_scheme =
        m_Periodic_ColorScheme -> itemData(idx).toString();

    // Get fractal
    Fractal * fractal = m_CurrentFractalWidget -> GetFractal();

    // Set new color scheme
    QHash < QString, QString > parameters;
    parameters["color scheme"] = new_scheme;
    fractal -> SetColorParameters(parameters);

    // Update GUI
    parameters = fractal -> GetColorParameters();
    for (QWidget * widget : m_Periodic_AllWidgets)
    {
        widget -> setVisible(true);
    }
    if (new_scheme == "color")
    {
        m_Periodic_OffsetR -> blockSignals(true);
        m_Periodic_FactorR -> blockSignals(true);
        m_Periodic_OffsetG -> blockSignals(true);
        m_Periodic_FactorG -> blockSignals(true);
        m_Periodic_OffsetB -> blockSignals(true);
        m_Periodic_FactorB -> blockSignals(true);
        m_Periodic_OffsetR -> setText(parameters["color offset red"]);
        m_Periodic_FactorR -> setText(parameters["color factor red"]);
        m_Periodic_OffsetG -> setText(parameters["color offset green"]);
        m_Periodic_FactorG -> setText(parameters["color factor green"]);
        m_Periodic_OffsetB -> setText(parameters["color offset blue"]);
        m_Periodic_FactorB -> setText(parameters["color factor blue"]);
        m_Periodic_OffsetR -> blockSignals(false);
        m_Periodic_FactorR -> blockSignals(false);
        m_Periodic_OffsetG -> blockSignals(false);
        m_Periodic_FactorG -> blockSignals(false);
        m_Periodic_OffsetB -> blockSignals(false);
        m_Periodic_FactorB -> blockSignals(false);
        for (QWidget * widget : m_Periodic_Greyscale_AllWidgets)
        {
            widget -> setVisible(false);
        }
    }
    if (new_scheme == "greyscale")
    {
        m_Periodic_GreyscaleOffset -> blockSignals(true);
        m_Periodic_GreyscaleFactor -> blockSignals(true);
        m_Periodic_GreyscaleOffset -> setText(parameters["color offset"]);
        m_Periodic_GreyscaleFactor -> setText(parameters["color factor"]);
        m_Periodic_GreyscaleOffset -> blockSignals(false);
        m_Periodic_GreyscaleFactor -> blockSignals(false);
        for (QWidget * widget : m_Periodic_Color_AllWidgets)
        {
            widget -> setVisible(false);
        }
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Refresh after change in brightness value
void MainWindow::Refresh_BrightnessValue()
{
    CALL_IN("");

    // New value
    int idx = m_BrightnessValue -> currentIndex();
    const QString new_value =
        m_BrightnessValue -> itemData(idx).toString();

    // Get fractal
    Fractal * fractal = m_CurrentFractalWidget -> GetFractal();
    fractal -> SetBrightnessValue(new_value);
    const QHash < QString, QString > parameters =
        fractal -> GetBrightnessParameters();

    // Update things
    const bool has_strip_average = (new_value == "strip average");
    const bool has_strip_average_alt = (new_value == "strip average alt");
    const bool has_flat = (new_value == "flat");
    if (has_strip_average ||
        has_strip_average_alt)
    {
        if (has_strip_average)
        {
            m_StripAverage_FoldChange -> blockSignals(true);
            m_StripAverage_FoldChange -> setText(
                parameters["brightness fold change"]);
            m_StripAverage_FoldChange -> blockSignals(false);
        }
        if (has_strip_average_alt)
        {
            m_StripAverageAlt_FoldChange -> blockSignals(true);
            m_StripAverageAlt_FoldChange -> setText(
                parameters["brightness fold change"]);
            m_StripAverageAlt_FoldChange -> blockSignals(false);
            m_StripAverageAlt_Regularity -> blockSignals(true);
            m_StripAverageAlt_Regularity -> setText(
                parameters["brightness regularity"]);
            m_StripAverageAlt_Regularity -> blockSignals(false);
            m_StripAverageAlt_Exponent -> blockSignals(true);
            m_StripAverageAlt_Exponent -> setText(
                parameters["brightness exponent"]);
            m_StripAverageAlt_Exponent -> blockSignals(false);
        }
        m_StripAverage_Factor -> blockSignals(true);
        m_StripAverage_Offset -> blockSignals(true);
        m_StripAverage_MinBrightness -> blockSignals(true);
        m_StripAverage_MinBrightnessSlider -> blockSignals(true);
        m_StripAverage_Factor -> setText(parameters["brightness factor"]);
        m_StripAverage_Offset -> setText(parameters["brightness offset"]);
        m_StripAverage_MinBrightness -> setText(
            parameters["brightness min brightness"]);
        m_StripAverage_MinBrightnessSlider -> setValue(
            parameters["brightness min brightness"].toDouble() * 100);
        m_StripAverage_Factor -> blockSignals(false);
        m_StripAverage_Offset -> blockSignals(false);
        m_StripAverage_MinBrightness -> blockSignals(false);
        m_StripAverage_MinBrightnessSlider -> blockSignals(false);
    }
    if (has_flat)
    {
        // Nothing to do
    }
    for (QWidget * widget : m_StripAverage_AllWidgets)
    {
        widget -> setVisible(has_strip_average);
    }
    for (QWidget * widget : m_StripAverageAlt_AllWidgets)
    {
        widget -> setVisible(has_strip_average_alt);
    }
    for (QWidget * widget : m_StripAverageShared_AllWidgets)
    {
        widget -> setVisible(has_strip_average || has_strip_average_alt);
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Refresh after change in aspect ratio
void MainWindow::Refresh_AspectRatio()
{
    CALL_IN("");

    // New value
    const int idx = m_AspectRatio -> currentIndex();
    const QString new_ratio = m_AspectRatio -> itemData(idx).toString();
    Fractal * fractal = m_CurrentFractalWidget -> GetFractal();
    fractal -> SetAspectRatio(new_ratio);

    // Update resolution
    double ratio = 1.;
    if (new_ratio == "any")
    {
        // Nothing to do - leave everything as it is.
        CALL_OUT("no changes");
        return;
    } else if (new_ratio == "1:1")
    {
        ratio = 1./1.;
    } else if (new_ratio == "2:1")
    {
        ratio = 2./1.;
    } else if (new_ratio == "3:2")
    {
        ratio = 3./2.;
    } else if (new_ratio == "16:9")
    {
        ratio = 16./9.;
    }

    // Update height based on width
    const int width = m_Width -> text().toInt();
    const int new_height = width / ratio;
    m_Height -> blockSignals(true);
    m_Height -> setText(QString("%1").arg(new_height));
    m_Height -> blockSignals(false);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Refresh after change in fit to window
void MainWindow::Refresh_FitToWindow()
{
    CALL_IN("");

    // Abbreviation
    Fractal * fractal = m_CurrentFractalWidget -> GetFractal();

    // See if checked
    if (m_FitToWindow -> checkState() == Qt::Checked)
    {
        // Update fractal
        fractal -> UnsetFixedResolutionFlag();

        // Can't edit width/height anymore
        m_Width -> setEnabled(false);
        m_Height -> setEnabled(false);
        m_AspectRatio -> setEnabled(false);
    } else
    {
        // Update fractal
        fractal -> SetFixedResolution(m_Width -> text().toInt(),
            m_Height -> text().toInt());

        // Can edit width/height
        m_Width -> setEnabled(true);
        m_Height -> setEnabled(true);
        m_AspectRatio -> setEnabled(true);
    }

    // Update width & height
    const QPair < int, int > max_size =
        m_CurrentFractalWidget -> GetActualImageSize();
    m_Width -> blockSignals(true);
    m_Height -> blockSignals(true);
    m_Width -> setText(QString("%1").arg(max_size.first));
    m_Height -> setText(QString("%1").arg(max_size.second));
    m_Width -> blockSignals(false);
    m_Height -> blockSignals(false);

    CALL_OUT("");
}




///////////////////////////////////////////////////////////////////////////////
// New oversampling value
void MainWindow::NewOversampling()
{
    CALL_IN("");

    // Set new value
    const int idx = m_Oversampling -> currentIndex();
    const int new_value = m_Oversampling -> itemData(idx).toInt();
    Fractal * fractal = m_CurrentFractalWidget -> GetFractal();
    fractal -> SetOversampling(new_value);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// New width entered
void MainWindow::NewWidth()
{
    CALL_IN("");

    // New value
    const int width = m_Width -> text().toInt();
    int height = m_Height -> text().toInt();

    // Apply aspect ratio
    int idx = m_AspectRatio -> currentIndex();
    const QString ratio_txt = m_AspectRatio -> itemData(idx).toString();

    if (ratio_txt != "any")
    {
        double ratio = 1.;
        if (ratio_txt == "1:1")
        {
            ratio = 1./1.;
        } else if (ratio_txt == "2:1")
        {
            ratio = 2./1.;
        } else if (ratio_txt == "3:2")
        {
            ratio = 3./2.;
        } else if (ratio_txt == "16:9")
        {
            ratio = 16./9.;
        }

        // Update height
        height = width / ratio;
        m_Height -> blockSignals(true);
        m_Height -> setText(QString("%1").arg(height));
        m_Height -> blockSignals(false);
    }

    // No longer fitting to window
    m_FitToWindow -> setCheckState(Qt::Unchecked);

    // Set fixed width/height
    Fractal * fractal = m_CurrentFractalWidget -> GetFractal();
    fractal -> SetFixedResolution(width, height);

    // Check if we need to disable saving cache to ram
    const int oversampling = fractal -> GetOversampling();
    const double cache_size =
        width * oversampling * height * oversampling * 2 * sizeof(double);
    if (cache_size > 8e9)
    {
        // De-activate ram caching
    } else
    {
        // Activate ram-caching
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// New height entered
void MainWindow::NewHeight()
{
    CALL_IN("");

    // New value
    int width = m_Width -> text().toInt();
    const int height = m_Height -> text().toInt();

    // Apply aspect ratio
    int idx = m_AspectRatio -> currentIndex();
    const QString ratio_txt = m_AspectRatio -> itemData(idx).toString();

    if (ratio_txt != "any")
    {
        double ratio = 1.;
        if (ratio_txt == "1:1")
        {
            ratio = 1./1.;
        } else if (ratio_txt == "2:1")
        {
            ratio = 2./1.;
        } else if (ratio_txt == "3:2")
        {
            ratio = 3./2.;
        } else if (ratio_txt == "16:9")
        {
            ratio = 16./9.;
        }

        // Update width
        width = height * ratio;
        m_Width -> blockSignals(true);
        m_Width -> setText(QString("%1").arg(width));
        m_Width -> blockSignals(false);
    }

    // No longer fitting to window
    m_FitToWindow -> setCheckState(Qt::Unchecked);

    // Set fixed width/height
    Fractal * fractal = m_CurrentFractalWidget -> GetFractal();
    fractal -> SetFixedResolution(width, height);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// New name entered
void MainWindow::UpdateName()
{
    CALL_IN("");

    // Get fractal
    Fractal * fractal = m_CurrentFractalWidget -> GetFractal();
    fractal -> SetName(m_Name -> text());

    // Update list of open windows
    const int current_id = m_CurrentFractalID;
    Refresh_Windows();
    m_OpenWindows -> blockSignals(true);
    SelectWindow(current_id);
    m_OpenWindows -> blockSignals(false);
    m_CurrentFractalID = current_id;
    m_CurrentFractalWidget = m_FractalWidgets[m_CurrentFractalID];

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Update fractal info
void MainWindow::UpdateFractalInfo()
{
    CALL_IN("");

    // Set in fractal
    Fractal * fractal = m_CurrentFractalWidget -> GetFractal();

    // Fractal type
    int idx = m_FractalType -> currentIndex();
    const QString fractal_type = m_FractalType -> itemData(idx).toString();
    fractal -> SetFractalType(fractal_type);

    // Precision
    const bool high_precision =
        (m_UseHighPrecision -> checkState() == Qt::Checked);
    fractal -> SetPrecision(high_precision ? "long double" : "double");

    // Julia constant
    if (fractal_type == "julia")
    {
        if (high_precision)
        {
            const long double julia_real =
                StringHelper::ToLongDouble(m_RealJulia -> text());
            const long double julia_imag =
                StringHelper::ToLongDouble(m_ImagJulia -> text());
            fractal -> SetJuliaConstant(julia_real, julia_imag);
        } else
        {
            const double julia_real = m_RealJulia -> text().toDouble();
            const double julia_imag = m_ImagJulia -> text().toDouble();
            fractal -> SetJuliaConstant(julia_real, julia_imag);
        }
    }

    // Area
    if (high_precision)
    {
        const long double real_min =
            StringHelper::ToLongDouble(m_RealMin -> text());
        const long double real_max =
            StringHelper::ToLongDouble(m_RealMax -> text());
        const long double imag_min =
            StringHelper::ToLongDouble(m_ImagMin -> text());
        const long double imag_max =
            StringHelper::ToLongDouble(m_ImagMax -> text());
        fractal -> SetRange(real_min, real_max, imag_min, imag_max);
    } else
    {
        const double real_min = m_RealMin -> text().toDouble();
        const double real_max = m_RealMax -> text().toDouble();
        const double imag_min = m_ImagMin -> text().toDouble();
        const double imag_max = m_ImagMax -> text().toDouble();
        fractal -> SetRange(real_min, real_max, imag_min, imag_max);
    }

    // Depth and escape radius
    const int depth = m_MaxDepth -> text().toInt();
    fractal -> SetDepth(depth);
    const double escape_radius = m_EscapeRadius -> text().toDouble();
    fractal -> SetEscapeRadius(escape_radius);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Color parameters edited
void MainWindow::UpdateColorParameters()
{
    CALL_IN("");

    // Mapping method
    int idx = m_ColorMappingMethod -> currentIndex();
    const QString mapping_method =
        m_ColorMappingMethod -> itemData(idx).toString();

    // Collect parameters
    QHash < QString, QString > parameters;
    if (mapping_method == "ramp")
    {
        parameters["color offset"] = m_Ramp_Offset -> text();
        parameters["color factor"] = m_Ramp_Factor -> text();
    }
    if (mapping_method == "periodic")
    {
        idx = m_Periodic_ColorScheme -> currentIndex();
        const QString color_scheme =
            m_Periodic_ColorScheme -> itemData(idx).toString();
        parameters["color scheme"] = color_scheme;
        if (color_scheme == "color")
        {
            parameters["color offset red"] = m_Periodic_OffsetR -> text();
            parameters["color offset green"] = m_Periodic_OffsetG -> text();
            parameters["color offset blue"] = m_Periodic_OffsetB -> text();
            parameters["color factor red"] = m_Periodic_FactorR -> text();
            parameters["color factor green"] = m_Periodic_FactorG -> text();
            parameters["color factor blue"] = m_Periodic_FactorB -> text();
        }
        if (color_scheme == "greyscale")
        {
            parameters["color offset"] = m_Periodic_GreyscaleOffset -> text();
            parameters["color factor"] = m_Periodic_GreyscaleFactor -> text();
        }
    }

    // Set in fractal
    Fractal * fractal = m_CurrentFractalWidget -> GetFractal();
    fractal -> SetColorParameters(parameters);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Brightness parameters edited
void MainWindow::UpdateBrightnessParameters()
{
    CALL_IN("");

    // Brightness value
    int idx = m_BrightnessValue -> currentIndex();
    const QString brightness_value =
        m_BrightnessValue -> itemData(idx).toString();

    // Make updates only if existing and new brightness value are the same
    Fractal * fractal = m_CurrentFractalWidget -> GetFractal();
    const QString brightness_value_in_fractal =
        fractal -> GetBrightnessValue();
    if (brightness_value != brightness_value_in_fractal)
    {
        CALL_OUT("Inconsistent brightness values in GUI and fractal.");
        return;
    }

    // Collect parameters
    QHash < QString, QString > parameters;
    if (brightness_value == "flat")
    {
        // Nothing to do.
        CALL_OUT("\"flat\" brightness value - no parameters");
        return;
    }
    if (brightness_value == "strip average" ||
        brightness_value == "strip average alt")
    {
        if (brightness_value == "strip average")
        {
            parameters["brightness fold change"] =
                m_StripAverage_FoldChange -> text();
        }
        if (brightness_value == "strip average alt")
        {
            parameters["brightness fold change"] =
                m_StripAverageAlt_FoldChange -> text();
            parameters["brightness regularity"] =
                m_StripAverageAlt_Regularity -> text();
            parameters["brightness exponent"] =
                m_StripAverageAlt_Exponent -> text();
        }
        parameters["brightness factor"] = m_StripAverage_Factor -> text();
        parameters["brightness offset"] = m_StripAverage_Offset -> text();
        parameters["brightness min brightness"] =
            m_StripAverage_MinBrightness -> text();

        // Update slider position
        m_StripAverage_MinBrightnessSlider -> blockSignals(true);
        const double min_brightness =
            parameters["brightness min brightness"].toDouble();
        m_StripAverage_MinBrightnessSlider -> setValue(min_brightness * 100);
        m_StripAverage_MinBrightnessSlider -> blockSignals(false);
    }

    // Set in fractal
    fractal -> SetBrightnessParameters(parameters);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Minimum brightness for strip average changed by slider
void MainWindow::StripAverageMinBrightnessChanged()
{
    CALL_IN("");

    const double value = m_StripAverage_MinBrightnessSlider -> value() * 0.01;
    m_StripAverage_MinBrightness -> setText(QString("%1").arg(value));
    UpdateBrightnessParameters();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Storage: Target directory
void MainWindow::SelectTargetDirectory()
{
    CALL_IN("");

    // Get filename
    Fractal * fractal = m_CurrentFractalWidget -> GetFractal();
    QString old_directory = fractal -> GetStorageDirectory();
    if (!QFile::exists(old_directory))
    {
        // Get default directory
        Preferences * p = Preferences::Instance();
        old_directory = p -> GetTagValue("Application:SaveDirectory");
    }
    const QString new_directory =
        QFileDialog::getExistingDirectory(this, tr("Pick directory"),
           old_directory);

    // Check if directory was selected
    if (new_directory.isEmpty())
    {
        // No.
        CALL_OUT("Canceled.");
        return;
    }

    // Set new storage directory
    fractal -> SetStorageDirectory(new_directory);
    m_TargetDirectory -> setText(new_directory);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Update fractal with storage parameters
void MainWindow::UpdateStorage()
{
    CALL_IN("");

    // Update checkboxes
    Fractal * fractal = m_CurrentFractalWidget -> GetFractal();
    fractal -> SetSavePicture(m_SavePicture -> checkState() == Qt::Checked);
    fractal -> SetSaveCacheData(
        m_SaveCacheData -> checkState() == Qt::Checked);
    fractal -> SetSaveCacheDataInMemory(
        m_SaveCacheDataInMemory -> checkState() == Qt::Checked);
    fractal -> SetSaveStatistics(
        m_SaveStatistics -> checkState() == Qt::Checked);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Optimizer
void MainWindow::Refresh_Optimizer()
{
    CALL_IN("");

    // Get current fractal
    if (!m_CurrentFractalWidget)
    {
        m_FirstParameter -> clear();
        m_FirstParameter -> setEnabled(false);
        m_FirstParameter_Min -> setText("");
        m_FirstParameter_Min -> setEnabled(false);
        m_FirstParameter_Max -> setText("");
        m_FirstParameter_Max -> setEnabled(false);
        m_FirstParameter_NumIntervals -> setEnabled(false);
        m_SecondParameter -> clear();
        m_SecondParameter -> setEnabled(false);
        m_SecondParameter_Min -> setText("");
        m_SecondParameter_Min -> setEnabled(false);
        m_SecondParameter_Max -> setText("");
        m_SecondParameter_Max -> setEnabled(false);
        m_SecondParameter_NumIntervals -> setEnabled(false);
        m_IncludeParameterValues -> setEnabled(false);
        m_StartOptimizer -> setEnabled(false);

        CALL_OUT("no current fractal widget.");
        return;
    }

    // We have a current fractal, so widgets are active
    m_FirstParameter -> setEnabled(true);
    m_FirstParameter_Min -> setEnabled(true);
    m_FirstParameter_Max -> setEnabled(true);
    m_FirstParameter_NumIntervals -> setEnabled(true);
    m_SecondParameter -> setEnabled(true);
    m_SecondParameter_Min -> setEnabled(true);
    m_SecondParameter_Max -> setEnabled(true);
    m_SecondParameter_NumIntervals -> setEnabled(true);
    m_IncludeParameterValues -> setEnabled(true);
    m_StartOptimizer -> setEnabled(true);

    // !!! We only vary parameters of the current visualization
    // Current fractal
    // FractalWidget * widget = m_Windows[row];
    // Fractal * fractal = widget -> GetWorker();

    m_FirstParameter -> clear();
    m_FirstParameter -> addItem(tr("none"), "");
    m_FirstParameter -> addItem(tr("Offset red (periodic coloring)"),
        "color offset red");
    m_FirstParameter -> addItem(tr("Factor red (periodic coloring)"),
        "color factor red");
    m_FirstParameter -> addItem(tr("Offset green (periodic coloring)"),
        "color offset green");
    m_FirstParameter -> addItem(tr("Factor green (periodic coloring)"),
        "color factor green");
    m_FirstParameter -> addItem(tr("Offset blue (periodic coloring)"),
        "color offset blue");
    m_FirstParameter -> addItem(tr("Factor blue (periodic coloring)"),
        "color factor blue");
    m_FirstParameter -> addItem(tr("Offset (periodic coloring/greyscale)"),
        "color offset");
    m_FirstParameter -> addItem(tr("Factor (periodic coloring/greyscale)"),
        "color factor");
    m_FirstParameter -> addItem(tr("Offset (ramp coloring/greyscale)"),
        "color offset");
    m_FirstParameter -> addItem(tr("Factor (ramp coloring/greyscale)"),
        "color factor");
    m_FirstParameter -> addItem(tr("Fold change (brightness/strip average)"),
        "brightness fold change");
    m_FirstParameter -> addItem(tr("Factor (brightness/strip average)"),
        "brightness factor");
    m_FirstParameter -> addItem(tr("Offset (brightness/strip average)"),
        "brightness offset");
    m_FirstParameter -> addItem(
        tr("Minimum brightness (brightness/strip average)"),
        "brightness min brightness");
    m_FirstParameter -> addItem(
        tr("Fold change (brightness/alternative strip average)"),
        "brightness fold change");
    m_FirstParameter -> addItem(
        tr("Regularity (brightness/alternative strip average)"),
        "brightness regularity");
    m_FirstParameter -> addItem(
        tr("Exponent (brightness/alternative strip average)"),
        "brightness exponent");

    m_SecondParameter -> clear();
    m_SecondParameter -> addItem(tr("none"), "");
    m_SecondParameter -> addItem(tr("Offset red (periodic coloring)"),
        "color offset red");
    m_SecondParameter -> addItem(tr("Factor red (periodic coloring)"),
        "color factor red");
    m_SecondParameter -> addItem(tr("Offset green (periodic coloring)"),
        "color offset green");
    m_SecondParameter -> addItem(tr("Factor green (periodic coloring)"),
        "color factor green");
    m_SecondParameter -> addItem(tr("Offset blue (periodic coloring)"),
        "color offset blue");
    m_SecondParameter -> addItem(tr("Factor blue (periodic coloring)"),
        "color factor blue");
    m_SecondParameter -> addItem(tr("Offset (periodic coloring/greyscale)"),
        "color offset");
    m_SecondParameter -> addItem(tr("Factor (periodic coloring/greyscale)"),
        "color factor");
    m_SecondParameter -> addItem(tr("Offset (ramp coloring/greyscale)"),
        "color offset");
    m_SecondParameter -> addItem(tr("Factor (ramp coloring/greyscale)"),
        "color factor");
    m_SecondParameter -> addItem(tr("Fold change (brightness/strip average)"),
        "brightness fold change");
    m_SecondParameter -> addItem(tr("Factor (brightness/strip average)"),
        "brightness factor");
    m_SecondParameter -> addItem(tr("Offset (brightness/strip average)"),
        "brightness offset");
    m_SecondParameter -> addItem(
        tr("Minimum brightness (brightness/strip average)"),
        "brightness min brightness");
    m_SecondParameter -> addItem(
        tr("Fold change (brightness/alternative strip average)"),
        "brightness fold change");
    m_SecondParameter -> addItem(
        tr("Regularity (brightness/alternative strip average)"),
        "brightness regularity");
    m_SecondParameter -> addItem(
        tr("Exponent (brightness/alternative strip average)"),
        "brightness exponent");

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Start optimizer
void MainWindow::StartOptimizer()
{
    CALL_IN("");

    // First parameter (x)
    int idx = m_FirstParameter -> currentIndex();
    const QString first_parameter =
        m_FirstParameter -> itemData(idx).toString();
    const double first_min = m_FirstParameter_Min -> text().toDouble();
    const double first_max = m_FirstParameter_Max -> text().toDouble();
    idx = m_FirstParameter_NumIntervals -> currentIndex();
    const int first_intervals =
        m_FirstParameter_NumIntervals -> itemData(idx).toInt();

    // Second parameter (y)
    idx = m_SecondParameter -> currentIndex();
    const QString second_parameter =
        m_SecondParameter -> itemData(idx).toString();
    const double second_min = m_SecondParameter_Min -> text().toDouble();
    const double second_max = m_SecondParameter_Max -> text().toDouble();
    idx = m_SecondParameter_NumIntervals -> currentIndex();
    const int second_intervals =
        m_SecondParameter_NumIntervals -> itemData(idx).toInt();

    // Current fractal
    FractalImage * fractal_image = m_CurrentFractalWidget -> GetFractalImage();
    Fractal * fractal = m_CurrentFractalWidget -> GetFractal();

    // Calculation
    const QPair < int, int > resolution =
        fractal_image -> GetImageResolution();
    QHash < QString, QString > parameters = fractal -> GetAllParameters();
    const int width = resolution.first;
    const int height = resolution.second;
    for (int area_y = 0; area_y < second_intervals; area_y++)
    {
        // Area
        const int y_min = height * area_y * 1./second_intervals;
        const int y_max = height * (area_y + 1) * 1./second_intervals;
        parameters["pixel y min"] = QString("%1").arg(y_min);
        parameters["pixel y max"] = QString("%1").arg(y_max);

        // Value in that area
        const double value = second_max +
            (second_min - second_max) * area_y * 1./second_intervals;
        parameters[second_parameter] = QString("%1").arg(value);

        for (int area_x = 0; area_x < first_intervals; area_x++)
        {
            // Area
            const int x_min = width * area_x * 1./first_intervals;
            const int x_max = width * (area_x + 1) * 1./first_intervals;
            parameters["pixel x min"] = QString("%1").arg(x_min);
            parameters["pixel x max"] = QString("%1").arg(x_max);

            // Value in that area
            const double value = first_min +
                (first_max - first_min) * area_x * 1./first_intervals;
            // !!! Do we need this somewhere?
            // !!! QHash < QString, QString > this_parameter;
            parameters[first_parameter] = QString("%1").arg(value);

            // Calculate tile
            // !!! fractal -> StartRendering(x_min, x_max, y_min, y_max);
        }
    }

    // Include text on image (if requested)
    // !!!

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Start calculation
void MainWindow::StartCalculation()
{
    CALL_IN("");

    // Only "Stop" button
    m_StartCalulation -> setEnabled(true);
    m_StartCalulation -> hide();
    m_StopCalulation -> setEnabled(true);
    m_StopCalulation -> show();

    // Start calculating
    m_CurrentFractalWidget -> UpdateImage();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Stop calculation
void MainWindow::StopCalculation()
{
    CALL_IN("");

    // Get current fractal
    FractalImage * fractal_image = m_CurrentFractalWidget -> GetFractalImage();

    // Force calculation to stop
    fractal_image -> Stop();

    // Wait for threads to finish
    m_StopCalulation -> setEnabled(false);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Calculate new image width/height when window size changes
void MainWindow::FractalWindowSizeChanged(const int mcFractalWidgetID)
{
    CALL_IN(QString("mcFractalWidgetID=%1")
        .arg(CALL_SHOW(mcFractalWidgetID)));

    // Check if this is the currently selectd fractal
    if (mcFractalWidgetID != m_CurrentFractalID)
    {
        CALL_OUT("Affected fractal widget ID is not the one "
            "that changed size");
        return;
    }

    // Check if we fit resolution to window size
    if (m_FitToWindow -> checkState() != Qt::Checked)
    {
        // Nope. Don't do anything.
        CALL_OUT("Resolution does not change with window size");
        return;
    }

    // Update width/height
    m_Width -> blockSignals(true);
    m_Height -> blockSignals(true);
    if (m_CurrentFractalWidget)
    {
        // Determine actual width and height of the image
        QPair < int, int > actual_resolution =
            m_CurrentFractalWidget -> GetActualImageSize();
        m_Width -> setText(QString("%1").arg(actual_resolution.first));
        m_Height -> setText(QString("%1").arg(actual_resolution.second));
    } else
    {
        m_Width -> setText("");
        m_Height -> setText("");
    }
    m_Width -> blockSignals(false);
    m_Height -> blockSignals(false);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Set new range for a fractal
void MainWindow::ZoomTo_LowPrecision(const int mcFractalWidgetID,
    double mNewRealMin, double mNewRealMax, double mNewImagMin,
    double mNewImagMax)
{
    CALL_IN(QString("mcFractalWidgetID=%1, mNewRealMin=%2, mNewRealMax=%3, "
        "mNewImagMin=%4, mNewImagMax=%5")
            .arg(CALL_SHOW(mcFractalWidgetID),
                 CALL_SHOW(mNewRealMin),
                 CALL_SHOW(mNewRealMax),
                 CALL_SHOW(mNewImagMin),
                 CALL_SHOW(mNewImagMax)));

    // See if we need to create a new fractal widget
    const bool create_new_widget =
        (m_NewWindowForZoom -> checkState() == Qt::Checked);

    // Abbreviation
    FractalWidget * fractal_widget;
    if (create_new_widget)
    {
        fractal_widget = NewFractal();
        m_FractalWidgets[mcFractalWidgetID] -> JustLikeThis(fractal_widget);
        fractal_widget -> show();
    } else
    {
        fractal_widget = m_FractalWidgets[mcFractalWidgetID];
    }
    Fractal * fractal = fractal_widget -> GetFractal();

    // Update in fractal
    fractal -> SetRange(mNewRealMin, mNewRealMax, mNewImagMin, mNewImagMax);

    // Check if this is the currently selected fractal
    if (mcFractalWidgetID == m_CurrentFractalID)
    {
        // Set in GUI
        m_RealMin -> blockSignals(true);
        m_RealMax -> blockSignals(true);
        m_ImagMin -> blockSignals(true);
        m_ImagMax -> blockSignals(true);
        m_RealMin -> setText(QString::number(mNewRealMin, 'g', 16));
        m_RealMax -> setText(QString::number(mNewRealMax, 'g', 16));
        m_ImagMin -> setText(QString::number(mNewImagMin, 'g', 16));
        m_ImagMax -> setText(QString::number(mNewImagMax, 'g', 16));
        m_RealMin -> blockSignals(false);
        m_RealMax -> blockSignals(false);
        m_ImagMin -> blockSignals(false);
        m_ImagMax -> blockSignals(false);

        m_Width -> blockSignals(true);
        m_Height -> blockSignals(true);
        const QPair < int, int > actual_resolution =
            fractal_widget -> GetActualImageSize();
        m_Width -> setText(QString("%1").arg(actual_resolution.first));
        m_Height -> setText(QString("%1").arg(actual_resolution.second));
        m_Width -> blockSignals(false);
        m_Height -> blockSignals(false);
    }

    // Immediately execute in fractal
    StartCalculation();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Set new range for a fractal
void MainWindow::ZoomTo_HighPrecision(const int mcFractalWidgetID,
    long double mNewRealMin, long double mNewRealMax, long double mNewImagMin,
    long double mNewImagMax)
{
    CALL_IN(QString("mcFractalWidgetID=%1, mNewRealMin=%2, mNewRealMax=%3, "
        "mNewImagMin=%4, mNewImagMax=%5")
            .arg(CALL_SHOW(mcFractalWidgetID),
                 CALL_SHOW(mNewRealMin),
                 CALL_SHOW(mNewRealMax),
                 CALL_SHOW(mNewImagMin),
                 CALL_SHOW(mNewImagMax)));

    // See if we need to create a new fractal widget
    const bool create_new_widget =
        (m_NewWindowForZoom -> checkState() == Qt::Checked);

    // Abbreviation
    FractalWidget * fractal_widget;
    if (create_new_widget)
    {
        fractal_widget = NewFractal();
        m_FractalWidgets[mcFractalWidgetID] -> JustLikeThis(fractal_widget);
        fractal_widget -> show();
    } else
    {
        fractal_widget = m_FractalWidgets[mcFractalWidgetID];
    }
    Fractal * fractal = fractal_widget -> GetFractal();

    // Update in fractal
    fractal -> SetRange(mNewRealMin, mNewRealMax, mNewImagMin, mNewImagMax);

    // Check if this is the currently selected fractal
    if (mcFractalWidgetID == m_CurrentFractalID)
    {
        // Set in GUI
        m_RealMin -> blockSignals(true);
        m_RealMax -> blockSignals(true);
        m_ImagMin -> blockSignals(true);
        m_ImagMax -> blockSignals(true);
        m_RealMin -> setText(StringHelper::ToString(mNewRealMin));
        m_RealMax -> setText(StringHelper::ToString(mNewRealMax));
        m_ImagMin -> setText(StringHelper::ToString(mNewImagMin));
        m_ImagMax -> setText(StringHelper::ToString(mNewImagMax));
        m_RealMin -> blockSignals(false);
        m_RealMax -> blockSignals(false);
        m_ImagMin -> blockSignals(false);
        m_ImagMax -> blockSignals(false);

        m_Width -> blockSignals(true);
        m_Height -> blockSignals(true);
        const QPair < int, int > actual_resolution =
            fractal_widget -> GetActualImageSize();
        m_Width -> setText(QString("%1").arg(actual_resolution.first));
        m_Height -> setText(QString("%1").arg(actual_resolution.second));
        m_Width -> blockSignals(false);
        m_Height -> blockSignals(false);
    }

    // Immediately execute in fractal
    StartCalculation();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Initialize keyboard actions
void MainWindow::InitActions()
{
    CALL_IN("");

    // Menu bar
    QMenuBar * menubar = new QMenuBar(nullptr);
    QAction * action = nullptr;

    // File menu
    QMenu * file_menu = menubar -> addMenu(tr("&File"));

    // About
    action = new QAction(tr("About"), this);
    action -> setMenuRole(QAction::AboutRole);
    connect(action, SIGNAL(triggered()),
        this, SLOT(About()));
    file_menu -> addAction(action);

    // Quit
    action = new QAction(tr("Quit MandelPoster"), this);
    action -> setShortcut(tr("Ctrl+Q"));
    action -> setMenuRole(QAction::QuitRole);
    connect(action, SIGNAL(triggered()),
        this, SLOT(Quit()));
    file_menu -> addAction(action);

    // Open fractal
    action = new QAction(tr("Open Fractal"), this);
    action -> setShortcut(tr("Ctrl+O"));
    connect(action, SIGNAL(triggered()),
        this, SLOT(OpenFractal()));
    file_menu -> addAction(action);

    // Save fractal
    m_Action_SaveFractal = new QAction(tr("Save Fractal"), this);
    m_Action_SaveFractal -> setShortcut(tr("Ctrl+S"));
    connect(m_Action_SaveFractal, SIGNAL(triggered()),
        this, SLOT(SaveFractal()));
    file_menu -> addAction(m_Action_SaveFractal);

    // Save fractal picture
    m_Action_SavePicture = new QAction(tr("Save Picture"), this);
    m_Action_SavePicture -> setShortcut(tr("Ctrl+Shift+S"));
    connect(m_Action_SavePicture, SIGNAL(triggered()),
        this, SLOT(SavePicture()));
    file_menu -> addAction(m_Action_SavePicture);

    // Windows menu
    QMenu * window_menu = menubar -> addMenu(tr("&Windows"));

    // New image
    action = new QAction(tr("New Fractal"), this);
    action -> setShortcut(tr("Ctrl+N"));
    connect(action, SIGNAL(triggered()),
        this, SLOT(CreateNewFractal()));
    window_menu -> addAction(action);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Refresh Actions
void MainWindow::Refresh_Actions()
{
    CALL_IN("");

    // See if there's an active fractal
    const int idx = m_OpenWindows -> currentRow();
    const bool has_fractal = (idx != -1);
    m_Action_SaveFractal -> setEnabled(has_fractal);
    m_Action_SavePicture -> setEnabled(has_fractal);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Update statistics
void MainWindow::Refresh_Statistics()
{
    CALL_IN("");

    // Check if currently visible fractal widget is affected
    if (m_CurrentFractalID == -1)
    {
        // No fractal selected
        m_Stats_Status -> setText("n/a");
        m_Stats_Progress -> setValue(0);
        m_Stats_ProgressText -> setText("0%");
        m_Stats_Started -> setText("n/a");
        m_Stats_Finished -> setText("n/a");
        m_Stats_Duration -> setText("n/a");
        m_Stats_NumberOfThreads -> setText("n/a");
        m_Stats_TotalPoints -> setText("n/a");
        m_Stats_PointsInSet -> setText("n/a");
        m_Stats_TotalIterations -> setText("n/a");
        m_Stats_ColorValueRange -> setText("n/a");
        m_Stats_BrightnessValueRange -> setText("n/a");

        CALL_OUT("No current fractal");
        return;
    }

    // Update statistics
    FractalImage * fractal_image = m_CurrentFractalWidget -> GetFractalImage();
    const QHash < QString, QString > statistics =
        fractal_image -> GetStatistics();
    const QString status = fractal_image -> GetRenderStatus();
    if (status == "idle")
    {
        m_Stats_Status -> setText(tr("idle"));
    } else if (status == "stopped")
    {
        m_Stats_Status -> setText(tr("stopped"));
    } else if (status == "working")
    {
        m_Stats_Status -> setText(tr("working"));
    } else
    {
        m_Stats_Status -> setText(tr("unknown status"));
    }

    const double progress = statistics["percent complete"].toDouble();
    m_Stats_Progress -> setValue(progress);
    m_Stats_ProgressText -> setText(QString::number(progress, 'f', 1) + "%");

    m_Stats_Started -> setText(statistics["start time"]);

    m_Stats_Finished -> setText(statistics["finish time"]);

    qint64 duration_ms = statistics["processing time ms"].toLongLong();
    int duration_s = int(duration_ms * 0.001);
    duration_ms = duration_ms % 1000;
    QString duration_text;
    if (duration_s > 3600)
    {
        duration_text = QString("%1:").arg(int(duration_s / 3600));
        duration_s = duration_s % 3600;
    }
    duration_text += QString("%1:%2.%3")
        .arg(int(duration_s / 60), 2, 10, QChar('0'))
        .arg(duration_s % 60, 2, 10, QChar('0'))
        .arg(duration_ms, 3, 10, QChar('9'));
    m_Stats_Duration -> setText(duration_text);

    m_Stats_NumberOfThreads -> setText(statistics["number of threads"]);

    m_Stats_TotalPoints -> setText(statistics["points finished short"]);

    m_Stats_PointsInSet -> setText(statistics["points in set short"]);

    m_Stats_TotalIterations -> setText(statistics["total iterations short"]);

    if (statistics["min color value"].isEmpty() ||
        statistics["max color value"].isEmpty())
    {
        m_Stats_ColorValueRange -> setText(tr("n/a"));
    } else
    {
        m_Stats_ColorValueRange -> setText(tr("%1 to %2")
            .arg(statistics["min color value"],
                 statistics["max color value"]));
    }

    if (statistics["min brightness value"].isEmpty() ||
        statistics["max brightness value"].isEmpty())
    {
        m_Stats_BrightnessValueRange -> setText(tr("n/a"));
    } else
    {
        m_Stats_BrightnessValueRange -> setText(tr("%1 to %2")
            .arg(statistics["min brightness value"],
                 statistics["max brightness value"]));
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Periodic update
void MainWindow::PeriodicUpdate(const int mcFractalWidgetID)
{
    CALL_IN(QString("mcFractalWidgetID=%1")
        .arg(CALL_SHOW(mcFractalWidgetID)));

    // Check if this is the current fractal
    if (mcFractalWidgetID != m_CurrentFractalID)
    {
        // Nope. Nothing to do.
        CALL_OUT("Current fractal is not affected.");
        return;
    }

    // Refresh statistics
    Refresh_Statistics();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Finished
void MainWindow::Finished(const int mcFractalWidgetID)
{
    CALL_IN(QString("mcFractalWidgetID=%1")
        .arg(CALL_SHOW(mcFractalWidgetID)));

    // Check if this is the current fractal
    if (mcFractalWidgetID != m_CurrentFractalID)
    {
        // Nope. Nothing to do.
        CALL_OUT("Current fractal is not affected.");
        return;
    }

    // Refresh statistics
    Refresh_Statistics();

    // Back to "Start" button
    m_StartCalulation -> setEnabled(true);
    m_StartCalulation -> show();
    m_StopCalulation -> setEnabled(true);
    m_StopCalulation -> hide();

    CALL_OUT("");
}



// ==================================================================== Presets



///////////////////////////////////////////////////////////////////////////////
// Refresh available coloring presets (and buttons)
void MainWindow::Refresh_Presets()
{
    CALL_IN("");

    // Clean up
    Preferences * p = Preferences::Instance();

    // Get current preset
    int row = m_ColorPreset -> currentIndex();
    const int active_preset = m_ColorPreset -> itemData(row).toInt();

    // Recreate presets
    m_ColorPreset -> blockSignals(true);
    m_ColorPreset -> clear();

    // Start with "New preset"
    m_ColorPreset -> addItem(tr("New preset"), -1);

    // Add defined presets in alphabetical order
    QHash < int, QString > presets;
    const int num_presets =
        p -> GetTagValue("Coloring:Number of Presets").toInt();
    for (int preset_id = 1; preset_id <= num_presets; preset_id++)
    {
        const QString deleted = p -> GetTagValue(
            QString("Coloring:Preset:%1:deleted").arg(preset_id));
        if (deleted == "yes")
        {
            continue;
        }
        presets[preset_id] = p -> GetTagValue(
            QString("Coloring:Preset:%1:name").arg(preset_id));
    }
    const QList < int > sorted_ids = StringHelper::SortHash(presets);
    for (int preset_id : sorted_ids)
    {
        m_ColorPreset -> addItem(presets[preset_id], preset_id);
    }

    // Restore active preset (if possible)
    row = m_ColorPreset -> findData(active_preset);
    if (row != -1)
    {
        m_ColorPreset -> setCurrentIndex(row);
    }
    m_ColorPreset -> blockSignals(false);

    // Disable all preset widgets if there is no current fractal
    m_ColorPreset -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_ColorPreset_Load -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_ColorPreset_Save -> setEnabled(m_CurrentFractalWidget != nullptr);
    m_ColorPreset_Delete -> setEnabled(m_CurrentFractalWidget != nullptr);

    // For "new preset" (active preset = -1), disable load and delete buttons
    m_ColorPreset_Load -> setEnabled(active_preset != -1);
    m_ColorPreset_Delete -> setEnabled(active_preset != -1);

#if 0
    p -> DeleteTagsMatching("Coloring:Preset:[0-9]+:.*");
    p -> SetTagValue("Coloring:Number of Presets", "0");
#endif

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Load a preset
void MainWindow::LoadPreset()
{
    CALL_IN("");

    // Get current preset
    int row = m_ColorPreset -> currentIndex();
    int active_preset = m_ColorPreset -> itemData(row).toInt();

    // We'll need this
    Preferences * p = Preferences::Instance();

    // Set preset in fractal
    row = m_OpenWindows -> currentRow();
    FractalWidget * widget = m_FractalWidgets[row];
    Fractal * fractal = widget -> GetFractal();
    const QString color_base = p -> GetTagValue(
        QString("Coloring:Preset:%1:color base value").arg(active_preset));
    fractal -> SetColorBaseValue(color_base);
    const QString color_mapping = p -> GetTagValue(
        QString("Coloring:Preset:%1:color mapping method").arg(active_preset));
    fractal -> SetColorMappingMethod(color_mapping);
    QHash < QString, QString > parameters = fractal -> GetColorParameters();
    for (auto key_iterator = parameters.keyBegin();
         key_iterator != parameters.keyEnd();
         key_iterator++)
    {
        const QString key = *key_iterator;
        parameters[key] = p -> GetTagValue(
            QString("Coloring:Preset:%1:%2").arg(active_preset).arg(key));
    }
    fractal -> SetColorParameters(parameters);
    const QString brightness_base = p -> GetTagValue(
        QString("Coloring:Preset:%1:brightness value").arg(active_preset));
    fractal -> SetBrightnessValue(brightness_base);
    parameters = fractal -> GetBrightnessParameters();
    for (auto key_iterator = parameters.keyBegin();
         key_iterator != parameters.keyEnd();
         key_iterator++)
    {
        const QString key = *key_iterator;
        parameters[key] = p -> GetTagValue(
            QString("Coloring:Preset:%1:%2").arg(active_preset).arg(key));
    }

    // Refresh
    Refresh_Values();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Save a preset
void MainWindow::SavePreset()
{
    CALL_IN("");

    // Get current preset
    int row = m_ColorPreset -> currentIndex();
    int active_preset = m_ColorPreset -> itemData(row).toInt();
    QString active_name = m_ColorPreset -> itemText(row);

    // We'll need this
    Preferences * p = Preferences::Instance();

    // Check if "New preset" was selected
    if (active_preset == -1)
    {
        // Ask for name
        QDialog * window = new QDialog();
        window -> setWindowTitle(tr("Preset Name"));

        QHBoxLayout * layout = new QHBoxLayout();
        window -> setLayout(layout);

        QLabel * l_name = new QLabel(tr("Name:"));
        layout -> addWidget(l_name);

        QLineEdit * name = new QLineEdit();
        connect (name, SIGNAL(returnPressed()),
            window, SLOT(accept()));
        layout -> addWidget(name);

        // Get name
        const int result = window -> exec();
        active_name = name -> text();
        delete window;
        if (result == QDialog::Rejected ||
            active_name.isEmpty())
        {
            CALL_OUT("Canceled.");
            return;
        }
        if (active_name == tr("New preset"))
        {
            // Can't use "New preset" as a name. What an odd idea!
            QMessageBox::warning(this, tr("Invalid preset name"),
                tr("You cannot use \"New preset\" as a preset name. Please "
                   "pick a different one."));
            CALL_OUT("\"New preset\" selected.");
            return;
        }
        const int num_presets =
            p -> GetTagValue("Coloring:Number of Presets").toInt();
        for (int preset_id = 1; preset_id <= num_presets; preset_id++)
        {
            const QString existing_name = p -> GetTagValue(
                QString("Coloring:Preset:%1:name").arg(preset_id));
            if (active_name == existing_name)
            {
                // Cannot use an existing name
                QMessageBox::warning(this, tr("Duplicate preset name"),
                    tr("Preset \"%1\" already exists. Please pick a different "
                       "name.").arg(active_name));
                CALL_OUT("Existing name selected.");
                return;
            }
        }

        // Create new preset
        active_preset = num_presets + 1;
        p -> SetTagValue("Coloring:Number of Presets",
            QString("%1").arg(num_presets+1));

        // Add to widget & make current
        m_ColorPreset -> addItem(active_name, active_preset);
        row = m_ColorPreset -> findData(active_preset);
        m_ColorPreset -> setCurrentIndex(row);
    }

    // Get presets
    row = m_OpenWindows -> currentRow();
    FractalWidget * widget = m_FractalWidgets[row];
    Fractal * fractal = widget -> GetFractal();
    QHash < QString, QString > parameters;
    parameters["name"] = active_name;
    parameters["deleted"] = "no";
    parameters["color base value"] = fractal -> GetColorBaseValue();
    parameters["color mapping method"] = fractal -> GetColorMappingMethod();
    QHash < QString, QString > additional_parameters =
        fractal -> GetColorParameters();
    for (auto key_iterator = additional_parameters.keyBegin();
         key_iterator != additional_parameters.keyEnd();
         key_iterator++)
    {
        const QString key = *key_iterator;
        parameters[key] = additional_parameters[key];
    }
    parameters["brightness value"] = fractal -> GetBrightnessValue();
    additional_parameters = fractal -> GetBrightnessParameters();
    for (auto key_iterator = additional_parameters.keyBegin();
         key_iterator != additional_parameters.keyEnd();
         key_iterator++)
    {
        const QString key = *key_iterator;
        parameters[key] = additional_parameters[key];
    }

    // Coloring preferences
    for (auto key_iterator = parameters.keyBegin();
         key_iterator != parameters.keyEnd();
         key_iterator++)
    {
        const QString key = *key_iterator;
        p -> SetTagValue(QString("Coloring:Preset:%1:%2")
            .arg(QString::number(active_preset),
                 key),
            parameters[key]);
    }

    // Refresh list of presets
    Refresh_Presets();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Delete a preset
void MainWindow::DeletePreset()
{
    CALL_IN("");

    // Get current preset
    int row = m_ColorPreset -> currentIndex();
    int active_preset = m_ColorPreset -> itemData(row).toInt();

    // We'll need this
    Preferences * p = Preferences::Instance();

    // Get confirmation
    const int result = QMessageBox::question(this, tr("Delete color preset"),
        tr("Do you really want to delete this preset?"));
    if (result == QMessageBox::Yes)
    {
        // Delete preset
        p -> SetTagValue(
            QString("Coloring:Preset:%1:deleted").arg(active_preset), "yes");
    }

    CALL_OUT("");
}



// =============================================================== Menu Actions



///////////////////////////////////////////////////////////////////////////////
// About window
void MainWindow::About()
{
    CALL_IN("");

    QDialog * dialog = new QDialog();
    dialog -> setWindowTitle(tr("About MandelPoster..."));

    QVBoxLayout * layout = new QVBoxLayout();

    dialog -> setLayout(layout);
    QPixmap splash(":/resources/MandelPoster.png");
    QLabel * splashwidget = new QLabel();
    splashwidget -> setAlignment(Qt::AlignCenter);
    splashwidget -> setPixmap(splash);
    layout -> addWidget(splashwidget);

    QLabel * text = new QLabel(tr("Version %1 (%2) - "
        "&copy; Chris von Toerne").arg(RELEASE_VERSION).arg(RELEASE_DATE));
    text -> setTextFormat(Qt::RichText);
    layout -> addWidget(text, 0, Qt::AlignCenter);

    QPushButton * ok = new QPushButton(tr("Ok"));
    ok -> setFixedWidth(100);
    connect (ok, SIGNAL(clicked()),
        dialog, SLOT(accept()));
    layout -> addWidget(ok, 0, Qt::AlignCenter);

    dialog -> exec();
    delete dialog;

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Quit MandelPoster
void MainWindow::Quit()
{
    CALL_IN("");

    // Ask first
    const int result = QMessageBox::question(this, tr("Quit"),
        tr("Do you really want to quit?"));
    if (result == QMessageBox::No)
    {
        // Not quitting
        CALL_OUT("");
        return;
    }

    // Save some preferences
    Preferences * p = Preferences::Instance();
    p -> SetTagValue("GUI:New Window When Zooming",
        m_NewWindowForZoom -> checkState() == Qt::Checked ? "yes" : "no");
    p -> SetTagValue("GUI:New Window For Julia Set",
        m_NewWindowForJulia -> checkState() == Qt::Checked ? "yes" : "no");

    // Quit
    Application::Instance() -> quit();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Open
void MainWindow::OpenFractal()
{
    CALL_IN("");

    // Get default directory
    Preferences * p = Preferences::Instance();
    QString directory = p -> GetTagValue("Application:SaveDirectory");

    // Get filename
    const QString filename =
        QFileDialog::getOpenFileName(this, tr("Open Fractal Data"),
           directory, tr("Fractal Data (*.xml)"));

    // Check if directory was selected
    if (filename.isEmpty())
    {
        // No.
        CALL_OUT("Canceled.");
        return;
    }

    // Remember directory for the future
    const QPair < QString, QString > split_filename =
        StringHelper::SplitFilename(filename);
    p -> SetTagValue("Application:SaveDirectory", split_filename.first);

    // Create new window
    FractalWidget * ret = new FractalWidget();
    Fractal * fractal = ret -> GetFractal();
    fractal -> FromFile(filename);

    // Show
    ret -> show();

    // Remember this window
    const int fractal_id = m_NextFractalWidgetID++;
    m_FractalWidgets[fractal_id] = ret;

    // Initialize widget
    InitializeFractalWidget(fractal_id);

    // Refresh GUI
    Refresh_Values();
    Refresh_Actions();
    Refresh_Optimizer();
    Refresh_Statistics();

    // For large calculations, ask if we should start
    if (fractal -> IsSavingCacheData())
    {
        const int result = QMessageBox::question(this, tr("Start calculation"),
            tr("Do you want to start/resume the calculation now?"));
        if (result == QMessageBox::No)
        {
            CALL_OUT("Not starting/resuming calculation");
            return;
        }
    }

    // Otherwise, start calculation
    StartCalculation();

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Save fractal
void MainWindow::SaveFractal()
{
    CALL_IN("");

    // Ignore if there's no current widget
    if (!m_CurrentFractalWidget)
    {
        CALL_OUT("No current widget");
        return;
    }

    // Get fractal
    Fractal * fractal = m_CurrentFractalWidget -> GetFractal();

    // Get default directory
    Preferences * p = Preferences::Instance();
    QString directory = p -> GetTagValue("Application:SaveDirectory");

    // Get filename
    const QString name = fractal -> GetName();
    const QString filename =
        QFileDialog::getSaveFileName(this, tr("Save Fractal Data"),
            directory + "/" + name, tr("Fractal Data (*.xml)"));

    // Check if directory was selected
    if (filename.isEmpty())
    {
        // No.
        CALL_OUT("Canceled.");
        return;
    }

    // Save fractal
    fractal -> ToFile(filename);

    // Remember this directory for the future
    const QPair < QString, QString > split_filename =
        StringHelper::SplitFilename(filename);
    p -> SetTagValue("Application:SaveDirectory", split_filename.first);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Save fractal picture
void MainWindow::SavePicture()
{
    CALL_IN("");

    // Get fractal
    const int row = m_OpenWindows -> currentRow();
    FractalWidget * widget = m_FractalWidgets[row];
    Fractal * fractal = widget -> GetFractal();
    FractalImage * fractal_image = widget -> GetFractalImage();

    // Get default directory
    Preferences * p = Preferences::Instance();
    QString directory = p -> GetTagValue("Application:SaveDirectory");

    // Get filename
    const QString name = fractal -> GetName();
    const QString filename =
        QFileDialog::getSaveFileName(this, tr("Save Fractal Image"),
           directory + "/" + name, tr("PNG image (*.png)"));

    // Check if directory was selected
    if (filename.isEmpty())
    {
        // No.
        CALL_OUT("Canceled.");
        return;
    }

    // Get image
    QImage image = fractal_image -> GetImage();

    // Save it.
    QPixmap::fromImage(image).save(filename, "png");

    // Remember directory for the future
    const QPair < QString, QString > split_filename =
        StringHelper::SplitFilename(filename);
    p -> SetTagValue("Application:SaveDirectory", split_filename.first);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// New fractal
void MainWindow::CreateNewFractal()
{
    CALL_IN("");

    NewFractal();

    // Render
    StartCalculation();

    CALL_OUT("");
}



// =========================================================== Register Windows



///////////////////////////////////////////////////////////////////////////////
// Create new fractal image
FractalWidget * MainWindow::NewFractal()
{
    CALL_IN("");

    // Create new window
    FractalWidget * ret = new FractalWidget();
    Fractal * fractal = ret -> GetFractal();
    fractal -> SetName(tr("Fractal %1").arg(m_FractalCount));
    ret -> show();

    // Remember this window
    const int fractal_id = m_NextFractalWidgetID++;
    m_FractalWidgets[fractal_id] = ret;

    // Increase fractal count for next window
    m_FractalCount++;

    // Initialize widget
    InitializeFractalWidget(fractal_id);

    // Done
    CALL_OUT("");
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Create new Julia image
FractalWidget * MainWindow::NewJuliaSet(const double mcJuliaReal,
    const double mcJuliaImag)
{
    CALL_IN(QString("mcJuliaReal=%1. mcJuliaImag=%2")
        .arg(CALL_SHOW(mcJuliaReal),
             CALL_SHOW(mcJuliaImag)));

    // Create new window
    FractalWidget * fractal_widget = nullptr;
    bool create_new_fractal =
        (m_NewWindowForJulia -> checkState() == Qt::Checked);
    if (create_new_fractal)
    {
        fractal_widget = new FractalWidget();
    } else
    {
        fractal_widget = m_CurrentFractalWidget;
    }

    // Set everything
    Fractal * fractal = fractal_widget -> GetFractal();
    fractal -> SetName(tr("Fractal %1").arg(m_FractalCount));
    fractal -> SetFractalType("julia");
    fractal -> SetJuliaConstant(mcJuliaReal, mcJuliaImag);

    // Increase fractal count for next window
    m_FractalCount++;

    // Remember this window
    if (create_new_fractal)
    {
        fractal_widget -> show();

        const int fractal_id = m_NextFractalWidgetID++;
        m_FractalWidgets[fractal_id] = fractal_widget;

        InitializeFractalWidget(fractal_id);
    } else
    {
        const int fractal_id = m_CurrentFractalID;
        Refresh_Windows();
        SelectWindow(fractal_id);
    }

    // Refresh GUI
    Refresh_Values();
    Refresh_Actions();
    Refresh_Optimizer();
    Refresh_Statistics();

    // Re-render
    StartCalculation();

    // Done
    CALL_OUT("");
    return fractal_widget;
}



///////////////////////////////////////////////////////////////////////////////
// Create new Julia image
FractalWidget * MainWindow::NewJuliaSet(const long double mcJuliaReal,
    const long double mcJuliaImag)
{
    CALL_IN(QString("mcJuliaReal=%1. mcJuliaImag=%2")
        .arg(CALL_SHOW(mcJuliaReal),
             CALL_SHOW(mcJuliaImag)));

    // Create new window
    FractalWidget * fractal_widget = nullptr;
    bool create_new_fractal =
        (m_NewWindowForJulia -> checkState() == Qt::Checked);
    if (create_new_fractal)
    {
        fractal_widget = new FractalWidget();
    } else
    {
        fractal_widget = m_CurrentFractalWidget;
    }

    // Set everything
    Fractal * fractal = fractal_widget -> GetFractal();
    fractal -> SetName(tr("Fractal %1").arg(m_FractalCount));
    fractal -> SetFractalType("julia");
    fractal -> SetJuliaConstant(mcJuliaReal, mcJuliaImag);

    // Increase fractal count for next window
    m_FractalCount++;

    // Remember this window
    if (create_new_fractal)
    {
        fractal_widget -> show();

        const int fractal_id = m_NextFractalWidgetID++;
        m_FractalWidgets[fractal_id] = fractal_widget;

        InitializeFractalWidget(fractal_id);
    } else
    {
        const int fractal_id = m_CurrentFractalID;
        Refresh_Windows();
        SelectWindow(fractal_id);
    }

    // Refresh GUI
    Refresh_Values();
    Refresh_Actions();
    Refresh_Optimizer();
    Refresh_Statistics();

    // Re-render
    StartCalculation();

    // Done
    CALL_OUT("");
    return fractal_widget;
}



///////////////////////////////////////////////////////////////////////////////
// Initialize fractal image widget
void MainWindow::InitializeFractalWidget(const int mcFractalID)
{
    CALL_IN(QString("mcFractalID=%1")
        .arg(CALL_SHOW(mcFractalID)));

    // Get fractal widget
    FractalWidget * fractal_widget = m_FractalWidgets[mcFractalID];

    // Move next to the main window
    int x = pos().x() + width() + 5;
    int y = pos().y();
    fractal_widget -> move(x, y);

    // Connect some signals
    connect (fractal_widget, &FractalWidget::WindowActivated,
        [=](){SelectWindow(mcFractalID);});
    connect (fractal_widget, &FractalWidget::WindowSizeChanged,
        [=](){FractalWindowSizeChanged(mcFractalID);});
    connect (fractal_widget, &FractalWidget::ChangeRange_LowPrecision,
        [=](double real_min, double real_max, double imag_min, double imag_max)
            {ZoomTo_LowPrecision(mcFractalID, real_min, real_max, imag_min,
                imag_max);});
    connect (fractal_widget, &FractalWidget::ChangeRange_HighPrecision,
        [=](long double real_min, long double real_max, long double imag_min,
             long double imag_max)
            {ZoomTo_HighPrecision(mcFractalID, real_min, real_max, imag_min,
                imag_max);});
    connect (fractal_widget, &FractalWidget::CloseWindow,
        [=](){CloseWindow(mcFractalID);});

    FractalImage * fractal_image = fractal_widget -> GetFractalImage();
    connect (fractal_image, &FractalImage::Started,
        [=](){PeriodicUpdate(mcFractalID);});
    connect (fractal_image, &FractalImage::PeriodicUpdate,
        [=](){PeriodicUpdate(mcFractalID);});
    connect (fractal_image, &FractalImage::Finished,
        [=](){Finished(mcFractalID);});

    // Make widget the current one
    Refresh_Windows();
    m_OpenWindows -> blockSignals(true);
    SelectWindow(mcFractalID);
    m_OpenWindows -> blockSignals(false);

    // Refresh
    FractalSelected();
    FractalWindowSizeChanged(mcFractalID);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Select window
void MainWindow::SelectWindow(const int mcFractalWindowID)
{
    CALL_IN(QString("mcFractalWindowID=%1")
        .arg(CALL_SHOW(mcFractalWindowID)));

    // Make it the current window
    const int row = m_OpenWindowsIDs.indexOf(mcFractalWindowID);
    m_OpenWindows -> setCurrentRow(row);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Close window
void MainWindow::CloseWindow(const int mcFractalWindowID)
{
    CALL_IN(QString("mcFractalWindowID=%1")
        .arg(CALL_SHOW(mcFractalWindowID)));

    // Preserve selected row as much as possible
    const int row = m_OpenWindowsIDs.indexOf(mcFractalWindowID);

    // Remove window from our list
    m_FractalWidgets.remove(mcFractalWindowID);

    // Refresh list of open windows
    Refresh_Windows();
    m_OpenWindows -> blockSignals(true);
    if (m_OpenWindows -> count() > row)
    {
        m_OpenWindows -> setCurrentRow(row);
    } else if (m_OpenWindows -> count() > 0)
    {
        m_OpenWindows -> setCurrentRow(row-1);
    }
    m_OpenWindows -> blockSignals(false);

    // Update GUI and actions
    FractalSelected();

    CALL_OUT("");
}
