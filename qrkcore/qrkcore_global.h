#ifndef QRKCORE_GLOBAL
#define QRKCORE_GLOBAL

#include <QtCore/qglobal.h>

#ifdef QRK_STATIC
    #define QRK_EXPORT
#else
    #if defined(QRK_BUILD)
        #define QRK_EXPORT Q_DECL_EXPORT
    #else
        #define QRK_EXPORT Q_DECL_IMPORT
    #endif
#endif // QRK_STATIC

#endif // QRKCORE_GLOBAL

