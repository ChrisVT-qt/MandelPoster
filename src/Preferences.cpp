// Preferences.cpp
// Class implementation

// Project includes
#include "CallTracer.h"
#include "Deploy.h"
#include "MessageLogger.h"
#include "Preferences.h"

// Qt includes
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QRegularExpression>
#include <QSettings>



// ================================================================== Lifecycle



///////////////////////////////////////////////////////////////////////////////
// Constructor
Preferences::Preferences()
{
    CALL_IN("");

    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationName(APPLICATION_NAME);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Destructor
Preferences::~Preferences()
{
    CALL_IN("");

    // Nothing to do

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Singleton instance
Preferences * Preferences::Instance()
{
    CALL_IN("");

    if (!m_Instance)
    {
        m_Instance = new Preferences();
    }

    CALL_OUT("");
    return m_Instance;
}



///////////////////////////////////////////////////////////////////////////////
// The actual instance
Preferences * Preferences::m_Instance = nullptr;



// ===================================================================== Access



///////////////////////////////////////////////////////////////////////////////
// Set acceptable preference tag & value
void Preferences::SetDefaultTagValue(const QString mcTag,
    const QString mcDefaultValue)
{
    CALL_IN(QString("mcTag=%1, mcDefaultValue=%2")
        .arg(CALL_SHOW(mcTag),
             CALL_SHOW(mcDefaultValue)));

    // Check if we have a value set already
    QSettings settings(QCoreApplication::organizationName(),
        QCoreApplication::applicationName());
    QStringList all_keys = settings.allKeys();
    if (!all_keys.contains(mcTag))
    {
        settings.setValue(mcTag, mcDefaultValue);
    }

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Get preference value
QString Preferences::GetTagValue(const QString mcTag) const
{
    CALL_IN(QString("mcTag=%1")
        .arg(CALL_SHOW(mcTag)));

    QSettings settings(QCoreApplication::organizationName(),
        QCoreApplication::applicationName());
    QStringList all_keys = settings.allKeys();
    if (all_keys.contains(mcTag))
    {
        CALL_OUT("");
        return settings.value(mcTag).toString();
    }

    // Unknown tag
    const QString reason = tr("Unknown preference tag \"%1\".")
        .arg(mcTag);
    MessageLogger::Error(CALL_METHOD,
        reason);
    CALL_OUT(reason);
    return QString();
}



///////////////////////////////////////////////////////////////////////////////
// Set preference value
void Preferences::SetTagValue(const QString mcTag, const QString mcValue)
{
    CALL_IN(QString("mcTag=%1, mcValue=%2")
        .arg(CALL_SHOW(mcTag),
             CALL_SHOW(mcValue)));

    // Also store in settings
    QSettings settings(QCoreApplication::organizationName(),
        QCoreApplication::applicationName());
    settings.setValue(mcTag, mcValue);

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Clean up stuff
void Preferences::DeleteTagsMatching(const QString mcPattern)
{
    CALL_IN(QString("mcPattern=%1")
        .arg(CALL_SHOW(mcPattern)));

    QSettings settings(QCoreApplication::organizationName(),
        QCoreApplication::applicationName());

    // Loop all tags
    const QRegularExpression format(mcPattern);
    QList < QString > deleted_tags;
    for (const QString & tag : settings.allKeys())
    {
        QRegularExpressionMatch match = format.match(tag);
        if (!match.hasMatch())
        {
            continue;
        }

        // Delete tag
        settings.remove(tag);
        deleted_tags << tag;
    }

    qDebug().noquote() << "Deleted tags:";
    qDebug().noquote() << deleted_tags.join("\n");

    CALL_OUT("");
}



///////////////////////////////////////////////////////////////////////////////
// Dump preferences
void Preferences::DumpTags() const
{
    CALL_IN("");

    QSettings settings(QCoreApplication::organizationName(),
        QCoreApplication::applicationName());
    QStringList all_tags = settings.allKeys();
    std::sort(all_tags.begin(), all_tags.end());
    qDebug().noquote() << "Available tags:";
    for (const QString & tag : all_tags)
    {
        const QString value = settings.value(tag).toString();
        qDebug().noquote().nospace() << "    " << tag << "\"" << value << "\"";
    }

    CALL_OUT("");
}
