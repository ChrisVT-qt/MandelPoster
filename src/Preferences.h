// Preferences.h
// Class definition

#ifndef PREFERENCES_H
#define PREFERENCES_H

// Qt includes
#include <QHash>
#include <QObject>
#include <QString>

// Class definition
class Preferences :
    public QObject
{
    Q_OBJECT


    // ============================================================== Lifecycle
private:
    // Constructor
    Preferences();

public:
    // Destructor
    virtual ~Preferences();

    // Singleton instance
    static Preferences * Instance();
private:
    static Preferences * m_Instance;



    // ================================================================= Access
public:
    // Set acceptable preference tag & value
    void SetDefaultTagValue(const QString mcTag,
            const QString mcDefaultValue);

    // Get preference value
    QString GetTagValue(const QString mcTag) const;

    // Set preference value
    void SetTagValue(const QString mcTag, const QString mcValue);

    // Clean up stuff
    void DeleteTagsMatching(const QString mcPattern);

    // Dump preferences
    void DumpTags() const;
};

#endif
