#include "Common.h"
#include <QCoreApplication>

QString g_strRootDir;
//static QMutex s_mutex;

void MessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString out;
    switch(type)
    {
        case QtDebugMsg:
            out = QString("[Debug] ");
        break;
        case QtInfoMsg:
            out = QString("[Info] ");
        break;
        case QtWarningMsg:
            out = QString("[Warning] ");
        break;
        case QtCriticalMsg:
            out = QString("[Critical] ");
        break;
        case QtFatalMsg:
            out = QString("[Fatal] ");
    }
    out.append(QString("[%1] ").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
    out.append(QString("[Function: %1] ").arg(QString(context.function)));
    out.append(QString("[%1: Line: %2] ").arg(QString(context.file)).arg(context.line));
    out.append(QString("Message: %1").arg(msg));

    fprintf(stdout, "%s\n", qPrintable(out));
}

void CommonInit()
{
    qInstallMessageHandler(MessageHandler);
    g_strRootDir = QCoreApplication::applicationDirPath();
}
