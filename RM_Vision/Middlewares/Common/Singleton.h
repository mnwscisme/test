/* 单例模板 */
#ifndef SINGLETON_H
#define SINGLETON_H

#include <QMutex>
#include <QScopedPointer>

#define DECLARE_SINGLETON(Class) \
Q_DISABLE_COPY(Class) \
public: \
    static Class* Instance() \
    { \
        static QMutex mutex; \
        static QScopedPointer<Class> instance; \
        if (Q_UNLIKELY(!instance)) { \
            mutex.lock(); \
            if (!instance) instance.reset(new Class); \
            mutex.unlock(); \
        } \
        return instance.data(); \
    }

template <class T>
class Singleton
{
public:
    static T* Instance()
    {
        static QMutex mutex;
        static QScopedPointer<T> instance;
        if (Q_UNLIKELY(!instance)) {
            mutex.lock();
            if (!instance) {
                instance.reset(new T);
            }
            mutex.unlock();
        }
        return instance.data();
    }
};

#endif // SINGLETON_H
