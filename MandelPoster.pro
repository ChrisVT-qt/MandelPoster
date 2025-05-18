# === Where files can be found
INCLUDEPATH += src/
INCLUDEPATH += shared/

# === Where files go
OBJECTS_DIR = build/
MOC_DIR = build/

# === Frameworks and compiler
TEMPLATE = app
QT += gui
QT += widgets
QT += xml
CONFIG += c++17
CONFIG += release
CONFIG += silent

# Don't allow deprecated versions of methods (before Qt 6.8)
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060800

# Resources
RESOURCES = MandelPoster.qrc

# Application icon
ICON = resources/MandelPoster.icns

HEADERS += shared/CallTracer.h
SOURCES += shared/CallTracer.cpp
HEADERS += shared/MessageLogger.h
SOURCES += shared/MessageLogger.cpp
HEADERS += shared/StringHelper.h
SOURCES += shared/StringHelper.cpp

# Specific classes
HEADERS += src/Application.h
SOURCES += src/Application.cpp
HEADERS += src/Deploy.h
HEADERS += src/Fractal.h
SOURCES += src/Fractal.cpp
HEADERS += src/FractalImage.h
SOURCES += src/FractalImage.cpp
HEADERS += src/FractalImageWidget.h
SOURCES += src/FractalImageWidget.cpp
HEADERS += src/FractalWidget.h
SOURCES += src/FractalWidget.cpp
HEADERS += src/FractalWorker.h
SOURCES += src/FractalWorker.cpp
SOURCES += src/main.cpp
HEADERS += src/MainWindow.h
SOURCES += src/MainWindow.cpp
HEADERS += src/Preferences.h
SOURCES += src/Preferences.cpp

