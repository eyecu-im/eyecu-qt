#ifndef LIVE_EXPORT_H
#include <qglobal.h>
#if defined (Q_WS_WIN) || defined Q_WS_PM
    #ifdef LIVE_DLL
        #define LIVE_EXPORT __declspec(dllexport)  //Export to dll
    #else
        #define LIVE_EXPORT __declspec(dllimport)  //Import from dll
    #endif
#else
    #define LIVE_EXPORT   //Linux
#endif
#endif //LIVE_EXPORT_H
