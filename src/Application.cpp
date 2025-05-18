// Application.cpp
// Class definition

// Project includes
#include "Application.h"
#include "CallTracer.h"
#include "Deploy.h"
#include "MainWindow.h"
#include "MessageLogger.h"
#include "Preferences.h"

// Qt includes
#include <QAction>
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

// Debugging
#define DEBUG false



// ================================================================== Lifecycle



///////////////////////////////////////////////////////////////////////////////
// Constructor
Application::Application(int & argc, char ** argv)
    : QApplication(argc, argv)
{
    QStringList arg_values;
    for (int i = 0; i < argc; i++)
    {
        arg_values << argv[i];
    }
    CALL_IN(QString("argc=%1, argv=%2")
        .arg(CALL_SHOW(argc),
             CALL_SHOW(arg_values)));

    // Initialize some preferences
    Preferences * p = Preferences::Instance();

    // Working directory
    p -> SetDefaultTagValue("Application:SaveDirectory", QDir::home().path());

    // Coloring preferences
    p -> SetDefaultTagValue("Coloring:Number of Presets", "0");

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Destructor
Application::~Application()
{
    CALL_IN("");

    // Nothing to do.

    CALL_OUT("");
}

   

///////////////////////////////////////////////////////////////////////////////
// Instanciator
Application * Application::Instance(int & argc, char ** argv)
{
    QStringList arg_values;
    for (int index = 0; index < argc; index++)
    {
        arg_values << argv[index];
    }
    CALL_IN(QString("argc=%1, argv=%2")
        .arg(CALL_SHOW(argc),
             CALL_SHOW(arg_values)));

    if (!m_Instance)
    {
        m_Instance = new Application(argc, argv);
        
        // Initialize GUI
        m_Instance -> InitGUI();
    }

    CALL_OUT("");
    return m_Instance;
}
    


///////////////////////////////////////////////////////////////////////////////
// Instance
Application * Application::Instance()
{
    CALL_IN("");

    if (!m_Instance)
    {
        // Error.
        MessageLogger::Error("Application::Instance()",
            tr("Trying to access uninitialized instance. Should not happen."));
        CALL_OUT("Trying to access uninitialized instance. "
            "Should not happen.");
        return nullptr;
    }

    CALL_OUT("");
    return m_Instance;
}
    


///////////////////////////////////////////////////////////////////////////////
// Instance
Application * Application::m_Instance = nullptr;



// ======================================================================== GUI



///////////////////////////////////////////////////////////////////////////////
// Initialize GUI
void Application::InitGUI()
{
    CALL_IN("");

    // Create and show main window
    MainWindow * main = MainWindow::Instance();
    main -> show();

    CALL_OUT("");
}
