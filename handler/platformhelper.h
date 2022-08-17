#ifndef PLATFORMHELPER_H
#define PLATFORMHELPER_H
#include <QString>
#include <QSharedPointer>
#include <QScopedPointer>

namespace VW {
class PlatformHelperPrivate;
class PlatformHelper
{
public:
    explicit PlatformHelper();

    ~PlatformHelper();

    void initCrashHandler(const QString& dumpDirPath = QString());

    void updateDumpPath();
    void setDumpPath(const QString& path);
    QString getDumpPath() const;
    
    void setStackHeapFilePath(const QString& filePath);
    QString getStackHeapFilePath() const;
    
    void setPluginDirList(const QStringList& dirList);
    QStringList getPluginDirList() const;

    void prepareReportFilePath();
    
    QString archiveDumpFile(const QString &oldDumpFilePath);

private:
    QScopedPointer<PlatformHelperPrivate> _private;
    bool autoDumpPath;
    QString dumpPath;
    QString stackHeapFilePath;
    QStringList pluginDirList;
};


} // namespace VW
#endif // PLATFORMHELPER_H
