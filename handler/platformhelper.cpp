#include "platformhelper.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QSharedPointer>

namespace VW {

#if !defined (USE_BREAKPAD)
class PlatformHelperPrivate
{
public:
    PlatformHelperPrivate() = default;
    ~PlatformHelperPrivate() = default;
};

PlatformHelper::PlatformHelper()
    : _private(new PlatformHelperPrivate())
      , autoDumpPath(true)
    , dumpPath(QString())
    , stackHeapFilePath(QString())
{}

PlatformHelper::~PlatformHelper()
{}

void PlatformHelper::initCrashHandler(const QString &)
{}
#endif

void PlatformHelper::updateDumpPath()
{
  if (!autoDumpPath) {
    return;
  }
    dumpPath += QString("/%1_%2").arg(QCoreApplication::applicationName()).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));
}

void PlatformHelper::setDumpPath(const QString &path)
{
    dumpPath = path;
}

QString PlatformHelper::getDumpPath() const
{
    return dumpPath;
}

void PlatformHelper::setStackHeapFilePath(const QString &filePath)
{
    stackHeapFilePath = filePath;
}

QString PlatformHelper::getStackHeapFilePath() const
{
    return stackHeapFilePath;
}

void PlatformHelper::setPluginDirList(const QStringList &dirList)
{
    pluginDirList = dirList;
}

QStringList PlatformHelper::getPluginDirList() const
{
    return pluginDirList;
}

void PlatformHelper::prepareReportFilePath()
{
    if (stackHeapFilePath.isEmpty()) {
        setStackHeapFilePath(getDumpPath() + QLatin1String("/dump_heap"));
    }
}

QString PlatformHelper::archiveDumpFile(const QString& oldDumpFilePath)
{
    int lastSlashIdx = oldDumpFilePath.lastIndexOf("/");
    QString dumpFileName = oldDumpFilePath.right(oldDumpFilePath.size() - lastSlashIdx - 1);

    QString newDumpFilePath = getDumpPath() + "/" + dumpFileName;

    QDir reportDir(getDumpPath());
    if (!reportDir.exists()) {
        if (!reportDir.mkpath(getDumpPath())) {
            return QString();
        }
    }
    if (!QFile::exists(oldDumpFilePath)) {
        return QString();
    }
    if (!QFile::copy(oldDumpFilePath, newDumpFilePath)) {
        return QString();
    }
    QFile::remove(oldDumpFilePath);
    return newDumpFilePath;
}
} // namespace VW
