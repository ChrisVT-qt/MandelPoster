// Application.h
// Class definition

#ifndef APPLICATION_H
#define APPLICATION_H

// Qt includes
#include <QAction>
#include <QApplication>

// Class definition
class Application
    : public QApplication
{
    Q_OBJECT
    
    
    
    // ============================================================== Lifecycle
private:
    // Constructor
    Application(int & argc, char ** argv);
    
public:
    // Destructor
    virtual ~Application();
   
public:
    // Instanciator
    static Application * Instance(int & argc, char ** argv);
    static Application * Instance();
    
private:
    // Instance
    static Application * m_Instance;
    
    
    
    // ==================================================================== GUI
private:
    // Initialize GUI
    void InitGUI();
};

#endif
