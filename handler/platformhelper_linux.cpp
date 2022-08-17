#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <algorithm>

#include "platformhelper.h"
#if defined(Q_OS_LINUX) && defined(USE_BREAKPAD)
#  include "client/linux/handler/exception_handler.h"

namespace {
bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor, void* context, bool succeeded)
{
    auto platform = reinterpret_cast<VW::PlatformHelper*>(context);
    platform->updateDumpPath();
    QString dumpPath(platform->archiveDumpFile(descriptor.path()));
    qDebug("Dump path:%s", qPrintable(dumpPath));

    platform->prepareReportFilePath();
    auto pluginDirList = platform->getPluginDirList();
    QProcess::startDetached(QCoreApplication::applicationDirPath() + "/VW-CrashHandler-cli",
                            {"-t", QCoreApplication::applicationFilePath(),
                             "-p", pluginDirList.join(';'),
                             "-d", dumpPath,
                             "-o", platform->getStackHeapFilePath()});

    return succeeded;
}
}  // namespace

namespace VW {

class PlatformHelperPrivate
{
public:
    PlatformHelperPrivate()  = default;
    ~PlatformHelperPrivate() = default;

    QSharedPointer<google_breakpad::MinidumpDescriptor> _descriptor;
    QSharedPointer<google_breakpad::ExceptionHandler> _excepHandler;
};

PlatformHelper::PlatformHelper()
    : _private(new PlatformHelperPrivate())
    , autoDumpPath(true)
    , dumpPath(QString())
    , stackHeapFilePath(QString())
{
}

PlatformHelper::~PlatformHelper()
{
}

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

    _private->_descriptor.reset(new google_breakpad::MinidumpDescriptor(dumpPath.toStdString()));
    _private->_excepHandler.reset(
        new google_breakpad::ExceptionHandler(*_private->_descriptor, nullptr, dumpCallback, this, true, -1));
}
}  // namespace VW
#endif  // defined(Q_OS_LINUX) && defined(USE_BREAKPAD)
