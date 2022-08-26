#ifndef COMMON_H
#define COMMON_H

#include <QtGlobal>
#include <QString>
#include <QDateTime>

/// Release memory allocated
#define RELEASE_ALLOC_MEM(obj)    \
        if (obj != nullptr)    \
        {   \
            delete obj;     \
            obj = nullptr;     \
        }

/// Release memory(array) allocated
#define RELEASE_ALLOC_ARR(obj) \
        if (obj != nullptr)    \
        {   \
            delete[] obj;   \
            obj = nullptr;     \
        }

extern QString g_strRootDir;

void MessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
void CommonInit();

#endif // COMMON_H
