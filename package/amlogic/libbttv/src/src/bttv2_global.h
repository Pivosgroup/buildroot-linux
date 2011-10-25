#ifndef BTTV2_GLOBAL_H
#define BTTV2_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(BTTV2_LIBRARY)
#  define BTTV2SHARED_EXPORT Q_DECL_EXPORT
#else
#  define BTTV2SHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // BTTV2_GLOBAL_H
