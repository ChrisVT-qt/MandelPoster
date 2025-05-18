// main.cpp

// Project includes
#include "Application.h"
#include "CallTracer.h"
#include "MainWindow.h"

// Qt includes
#include <QDebug>

// System include
#include <signal.h>



///////////////////////////////////////////////////////////////////////////////
// Main
int main(int mNumParameters, char * mpParameter[])
{
    QStringList arg_values;
    for (int index = 0; index < mNumParameters; index++)
    {
        arg_values << mpParameter[index];
    }
    CALL_IN(QString("mNumParameters=%1, mpParameter={\"%2\"}")
        .arg(mNumParameters).arg(arg_values.join("\", \"")));

    // Handle command line parameters for GUI
    Application * app = Application::Instance(mNumParameters, mpParameter);
    
    // Make sure main window is the active one
    MainWindow * main_window = MainWindow::Instance();
    main_window -> raise();
    main_window -> activateWindow();
    
    // Hand over control to the GUI
    const int result = app -> exec();

    // Done here.
    CALL_OUT("");
    return result;
}

