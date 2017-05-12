#ifndef LIVE_EXPORT_H
#include <qglobal.h>
#if defined (Q_OS_WIN) || defined (Q_OS_OS2)
    #ifdef LIVE_DLL
        #define LIVE_EXPORT __declspec(dllexport)  //Export to dll
    #else
        #define LIVE_EXPORT __declspec(dllimport)  //Import from dll
    #endif
#else
    #define LIVE_EXPORT   //Linux
#endif
#endif //LIVE_EXPORT_H
