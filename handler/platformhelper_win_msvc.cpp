#include "platformhelper.h"
#include <QCoreApplication>
#include <QProcess>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QSharedPointer>
#include <functional>

#if defined (Q_OS_WIN32) && defined (Q_CC_MSVC) && defined(USE_BREAKPAD)
#include "client/windows/handler/exception_handler.h"
namespace {

bool callback(const wchar_t *dump_path, const wchar_t *id, void *context, EXCEPTION_POINTERS *exinfo, MDRawAssertionInfo *assertion, bool succeeded)
{
    auto loop = QSharedPointer<QEventLoop>::create();
    auto platform = reinterpret_cast<ValWell::PlatformHelper *>(context);
    platform->getInfoManager()->openWidget(loop);
    platform->updateBreakpadPath();
    platform->prepareReportFilePath();

    QString newDumpFilePath(platform->archiveDumpFile(QString::fromWCharArray(dump_path)));
    if (newDumpFilePath.isEmpty()) {
        qDebug("move dump file path error");
        return succeeded;
    }

    QtConcurrent::run([=]() {
        if (!platform->archiveSym(QCoreApplication::applicationName(), QCoreApplication::applicationDirPath())) {
            qDebug("build sym failed!");
            return ;
        }

        auto sharedNameList = platform->prepareSharedLib(QCoreApplication::applicationDirPath(), QString("dll"));
        std::function<bool(const QString&)> functor = [=](const QString& sharedName) {
            return platform->archiveSym(sharedName, QCoreApplication::applicationDirPath());
        };
        auto mappedResult = QtConcurrent::mapped(sharedNameList, functor);
        mappedResult.waitForFinished();

        QString pluginDirPath = QCoreApplication::applicationDirPath() + "/plugin";
        auto pluginNameList = platform->prepareSharedLib(pluginDirPath, QString("dll"));
        auto pluginResult = QtConcurrent::mapped(pluginNameList, functor);
        pluginResult.waitForFinished();

        if (!platform->reportCrash(newDumpFilePath)) {
            qDebug("report crash failed!");
            return ;
        }
        platform->removeSymbolDir();
        platform->infoFinished();
    }, &loop);
    
    loop->exec();

    return succeeded;
}
}

namespace ValWell {
class PlatformHelperPrivate final
{
public:
    PlatformHelperPrivate();
    ~PlatformHelperPrivate() = default;

    QSharedPointer<google_breakpad::ExceptionHandler> _excepHandler;
};

PlatformHelperPrivate::PlatformHelperPrivate()
{}

PlatformHelper::PlatformHelper()
    : _private(new PlatformHelperPrivate())
    , outputPath(QString())
    , dumpSymsPath(QString())
    , minidumpStackwalkPath(QString())
    , crashReportFilePath(QString())
{
    dumpSymsPath = QString("%1/breakpad/dump_syms.exe").arg(QCoreApplication::applicationDirPath());
    minidumpStackwalkPath = QString("%1/breakpad/minidump_stackwalk.exe").arg(QCoreApplication::applicationDirPath());

}
PlatformHelper::~PlatformHelper()
{}

void PlatformHelper::initCrashHandler(const QString& dumpDirPath)
{
    if (dumpDirPath.isEmpty()) {
        dumpPath = QCoreApplication::applicationDirPath() + QLatin1String("/crash");
    } else {
        dumpPath = dumpDirPath;
        autoDumpPath = false;
    }
    QDir dir(dumpPath);
    if (!dir.exists()) {
        dir.mkpath(dumpPath);
    }

//    auto eh = new google_breakpad::ExceptionHandler(dumpPath.toStdWString(), nullptr, callback, nullptr, google_breakpad::ExceptionHandler::HANDLER_ALL);
    _private->_excepHandler.reset(new google_breakpad::ExceptionHandler(/*outputPath.toStdWString()*/L".", nullptr, callback, nullptr, google_breakpad::ExceptionHandler::HANDLER_ALL));
}

//QStringList PlatformHelper::queryTargetSharedLib(const QString& targetPath)
//{
//    QString dirPath = targetPath.left(targetPath.lastIndexOf("/"));
//    return prepareSharedLib(dirPath, QString("dll"));
//}
//QStringList PlatformHelper::queryTargetPluginLib(const QString& pluginPath)
//{
//    // TODO:
//    return QStringList();
//}

//QStringList PlatformHelper::prepareSharedLib(const QString &dirPath, const QString& suffix)
//{
//    QDir sharedLibDir(dirPath);
//    QStringList filenameList;
//    QFileInfoList fileinfoList = sharedLibDir.entryInfoList(QDir::Files);

//    for (auto& fileinfo : fileinfoList) {
//        if (fileinfo.completeSuffix().contains(suffix) && !fileinfo.fileName().contains("Qt5")
//                && !fileinfo.fileName().contains("api-ms-win", Qt::CaseInsensitive)) {
//            filenameList.append(fileinfo.fileName());
//        }
//    }
//    return filenameList;
//}

//bool PlatformHelper::archiveSym(const QString &appName, const QString &appDirPath)
//{
//    QFileInfo appInfo(appDirPath, appName);
//    QString newAppName = appName;
//    if (QFile::exists(QString("%1/%2.pdb").arg(appDirPath).arg(appInfo.completeBaseName()))) {
//        newAppName = appInfo.completeBaseName() + ".pdb";
//    }
//    QSharedPointer<QProcess> dumpSymProcess(new QProcess());
//    QSharedPointer<QByteArray> readData(new QByteArray());

//    QObject::connect(dumpSymProcess.data(), &QProcess::readyReadStandardOutput, [=](){
//        readData->append(dumpSymProcess->readAllStandardOutput());
//    });

//    QString appNamePath = appDirPath + "/" + newAppName;
//    dumpSymProcess->start(dumpSymsPath, {appNamePath});
//    dumpSymProcess->waitForFinished();
//    dumpSymProcess->close();
//    if (dumpSymProcess->exitCode() != 0) {
//        return false;
//    }
//    if (readData->isEmpty()) {
//        return false;
//    }

//    QString symId(readFirstLineId(readData));
//    if (symId.isEmpty()) {
//        return false;
//    }
//    QString symDirPath = QString("%1/symbols/%2/%3").arg(breakpadPath()).arg(newAppName).arg(symId);
//    QDir symDir(symDirPath);
//    if (!symDir.mkpath(symDirPath)) {
//        return false;
//    }
//    // xxx.sym，不需要.pdb或.exe
//    QString newSymFilePath = QString("%1/%2.sym").arg(symDirPath).arg(appInfo.completeBaseName());
//    QFile newSymFile(newSymFilePath);
//    if (!newSymFile.open(QFile::WriteOnly)) {
//        return false;
//    }
//    if (newSymFile.write(readData->data()) <= 0) {
//        newSymFile.close();
//        return false;
//    }
//    newSymFile.close();
//    return true;
//}
} // namespace ValWell
#endif // defined (Q_OS_WIN32) && defined (Q_CC_MSVC) && defined(USE_BREAKPAD)
