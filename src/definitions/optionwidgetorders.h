#ifndef DEF_OPTIONWIDGETORDERS_H
#define DEF_OPTIONWIDGETORDERS_H

//Node = ON_COMMON
#define OHO_COMMON_SETTINGS                       100
// *** <<< eyeCU <<< ***
#  define OWO_COMMON_ADVANCED                     105
// *** <<< eyeCU <<< ***
#  define OWO_COMMON_AUTOSTART                    110
#  define OWO_COMMON_SENDSTATISTICS               120
#  define OWO_COMMON_SENDCLIENTINFO               130
#  define OWO_COMMON_VCARDIMAGE                   140
#define OHO_COMMON_LOCALIZATION                   300
#  define OWO_COMMON_LANGUAGE                     310

//Node = ON_ACCOUNTS
#define OHO_ACCOUNTS_ACCOUNTS                     100
#  define OWO_ACCOUNTS_ACCOUNTS                   110
#define OHO_ACCOUNTS_COMMON                       500
#  define OWO_ACCOUNTS_DEFAULTRESOURCE            530
#  define OWO_ACCOUNTS_DEFAULTPROXY               560

//Node = ON_ACCOUNTS.[id].Params
#define OHO_ACCOUNTS_PARAMS_ACCOUNT               100
#  define OWO_ACCOUNTS_PARAMS_NAME                110
#  define OWO_ACCOUNTS_PARAMS_STREAM_JID          115
#  define OWO_ACCOUNTS_PARAMS_RESOURCE            120
#  define OWO_ACCOUNTS_PARAMS_PASSWORD            130
#define OHO_ACCOUNTS_PARAMS_CONNECTION            300
#  define OWO_ACCOUNTS_PARAMS_CONNECTION          310
#define OHO_ACCOUNT_REGISTRATION                  400
#  define OWO_ACCOUNT_REGISTER                    410

//Node = ON_ACCOUNTS.[id].History
#define OHO_ACCOUNTS_HISTORY_REPLICATION          300
#  define OWO_ACCOUNTS_HISTORY_REPLICATION        310
#  define OWO_ACCOUNTS_HISTORY_DUPLICATION        320
#define OHO_ACCOUNTS_HISTORY_SERVERSETTINGS       500
#  define OWO_ACCOUNTS_HISTORY_SERVERSETTINGS     510

//Node = ON_ACCOUNTS.[id].Additional
#define OHO_ACCOUNTS_ADDITIONAL_SETTINGS          100
#  define OWO_ACCOUNTS_ADDITIONAL_REQUIRESECURE   110
#  define OWO_ACCOUNTS_ADDITIONAL_AUTOCONNECT     170
#  define OWO_ACCOUNTS_ADDITIONAL_AUTORECONNECT   171
#  define OWO_ACCOUNTS_ADDITIONAL_STREAMCOMPRESS  180
#define OHO_ACCOUNTS_ADDITIONAL_CONFERENCES       500
#  define OWO_ACCOUNTS_ADDITIONAL_DISABLEAUTOJOIN 550
// *** <<< eyeCU <<< ***
#  define OWO_ACCOUNTS_ADDITIONAL_POI             200
#define OHO_ACCOUNTS_ADDITIONAL_PEP               300
#  define OWO_ACCOUNTS_ADDITIONAL_USERTUNE        310
#  define OWO_ACCOUNTS_ADDITIONAL_GEOLOC          320
#define OHO_ACCOUNTS_ADDITIONAL_NICKNAME          600
#  define OWO_ACCOUNTS_ADDITIONAL_NICKNAME        610
#  define OWO_ACCOUNTS_ADDITIONAL_NICKNAME_CHANGE 620
#  define OWO_ACCOUNTS_ADDITIONAL_NICKNAME_SET    630
#  define OWO_ACCOUNTS_ADDITIONAL_NICKNAME_BROADCAST 640
// *** >>> eyeCU >>> ***

//Node = ON_ROSTERVIEW
#define OHO_ROSTER_VIEW                           100
#define   OWO_ROSTER_SHOWOFFLINE                  110
#define   OWO_ROSTER_MERGESTREAMS                 120
#define   OWO_ROSTER_SHOWRESOURCE                 130
#define   OWO_ROSTER_HIDESCROLLBAR                140
#define   OWO_ROSTER_VIEWMODE                     150
#define   OWO_ROSTER_SORTMODE                     160
// *** <<< eyeCU <<< ***
#define   OWO_ROSTER_SHOWOFFLIEAGENTS             115
#define   OWO_ROSTER_SHOWSELF                     125
#define   OWO_ROSTER_STATUSDISPLAY                135
#define   OWO_ROSTER_ALTERNATIONHIGHLITE          145
#define OHO_ROSTER_EXTRA_ICONS                    200
#define   OWO_ROSTER_CLIENTICONS                  210
#define   OWO_ROSTER_PEP_MOOD                     220
#define   OWO_ROSTER_PEP_ACTIVITY                 230
#define   OWO_ROSTER_PEP_TUNE                     240
#define   OWO_ROSTER_PEP_LOCATION                 250
#define   OHO_ROSTER_AVATARS                      170
#define   OWO_ROSTER_AVATARS                      175
// *** >>> eyeCU >>> ***
#define OHO_ROSTER_MANAGEMENT                     300
#define   OWO_ROSTER_AUTOSUBSCRIBE                310
#define   OWO_ROSTER_AUTOUNSUBSCRIBE              320
#define   OWO_ROSTER_EXCHANGEAUTO                 330
#define OHO_ROSTER_RECENT                         500
#define   OWO_ROSTER_RECENT_HIDEINACTIVEITEMS     510
#define   OWO_ROSTER_RECENT_SORTBYACTIVETIME      520
#define   OWO_ROSTER_RECENT_ALWAYSSHOWOFFLINE     530
#define   OWO_ROSTER_RECENT_SHOWONLYFAVORITE      540
#define   OWO_ROSTER_RECENT_SIMPLEITEMSVIEW       550
//Node = ON_MESSAGES
#define OHO_MESSAGES_VIEW                         100
#define   OWO_MESSAGES_COMBINEWITHROSTER          110
#define   OWO_MESSAGES_TABWINDOWSENABLE           120
#define   OWO_MESSAGES_EDITORAUTORESIZE           130
#define   OWO_MESSAGES_EDITORMINIMUMLINES         140
#define OHO_MESSAGES_BEHAVIOR                     300
#define   OWO_MESSAGES_LOADHISTORY                310
#define   OWO_MESSAGES_SHOWSTATUS                 320
#define   OWO_MESSAGES_ARCHIVESTATUS              330
#define   OWO_MESSAGES_SHOWDATESEPARATORS         340
#define   OWO_MESSAGES_CHATSTATESENABLED          360
#define   OWO_MESSAGES_UNNOTIFYALLNORMAL          370
// *** <<< eyeCU <<< ***
#define   OWO_MESSAGES_RECEIPTS_SHOW              400
#define   OWO_MESSAGES_RECEIPTS_SEND              410
#define OHO_MESSAGES_INFOBAR_ICONS                450
#define   OWO_MESSAGES_INFOBAR_CLIENTICONS        452
#define   OWO_MESSAGES_INFOBAR_MOOD               454
#define   OWO_MESSAGES_INFOBAR_ACTIVITY           456
#define   OWO_MESSAGES_INFOBAR_TUNE               458
#define   OWO_MESSAGES_INFOBAR_LOCATION           460
#define OHO_MESSAGES_PEP                          470
#define   OWO_MESSAGES_PEP_MOOD                   472
#define   OWO_MESSAGES_PEP_ACTIVITY               474
#define   OWO_MESSAGES_PEP_TUNE                   476
// *** >>> eyeCU >>> ***
#define OHO_MESSAGES_CONFERENCES                  500
#define   OWO_MESSAGES_MUC_SHOWENTERS             510
#define   OWO_MESSAGES_MUC_SHOWSTATUS             520
#define   OWO_MESSAGES_MUC_ARCHIVESTATUS          530
#define   OWO_MESSAGES_MUC_QUITONWINDOWCLOSE      540
#define   OWO_MESSAGES_MUC_REJOINAFTERKICK        550
#define   OWO_MESSAGES_MUC_REFERENUMERATION       560
#define   OWO_MESSAGES_MUC_SHOWAUTOJOINED         570
#define   OWO_MUC_GROUPCHAT_NICKNAMESUFFIX        580 // *** <<< eyeCU >>> ***

// ON_HISTORY
#define OHO_HISORY_ENGINES                        300
#define   OWO_HISORY_ENGINE                       310
#define OHO_HISTORY_ENGINNAME                     500
#define   OWO_HISTORY_ENGINESETTINGS              505

//Node = ON_STATUSITEMS
#define OHO_AUTOSTATUS                            100
#define   OWO_AUTOSTATUS                          150
#define OHO_STATUS_ITEMS                          300
#define   OWO_STATUS_ITEMS                        350

//Node = ON_NOTIFICATIONS
#define OHO_NOTIFICATIONS                         100
#define   OWO_NOTIFICATIONS_DISABLEIFAWAY         110
#define   OWO_NOTIFICATIONS_DISABLEIFDND          120
#define   OWO_NOTIFICATIONS_PEP_IGNOREOFFLINE     125
#define   OWO_NOTIFICATIONS_NATIVEPOPUPS          130
#define   OWO_NOTIFICATIONS_FORCESOUND            140
#define   OWO_NOTIFICATIONS_HIDEMESSAGE           150
#define   OWO_NOTIFICATIONS_EXPANDGROUPS          160
#define   OWO_NOTIFICATIONS_SOUNDCOMMAND          170
#define   OWO_NOTIFICATIONS_POPUPTIMEOUT          180
#define OHO_NOTIFICATIONS_KINDS                   500
#define   OWO_NOTIFICATIONS_ALERTWINDOW           510
#define   OWO_NOTIFICATIONS_KINDS                 590
//Node = ON_SHORTCUTS
#define OHO_SHORTCUTS                             500
#define   OWO_SHORTCUTS                           510

//Node = ON_DATATRANSFER
#define OHO_DATATRANSFER_FILETRANSFER             100
#define   OWO_DATATRANSFER_FILESTREAMS            110
#define   OWO_DATATRANSFER_GROUPBYSENDER          120
#define   OWO_DATATRANSFER_AUTORECEIVE            130
#define   OWO_DATATRANSFER_HIDEONSTART            140
#define   OWO_DATATRANSFER_DEFAULTMETHOD          150
#define OHO_DATATRANSFER_METHODNAME               500
#define   OWO_DATATRANSFER_METHODSETTINGS         505

//Node = OPN_APPEARANCE
#define OHO_APPEARANCE_MESSAGESTYLE               200
#define   OWO_APPEARANCE_MESSAGETYPESTYLE         201
#define OHO_APPEARANCE_ROSTER                     400
#define   OWO_APPEARANCE_STATUSICONS              430
#define OHO_APPEARANCE_MESSAGES                   700
#define   OWO_APPEARANCE_EMOTICONS                730

// *** <<< eyeCU <<< ***
#define OWO_MEDIAFILES_PLAY                       200
#define OHO_MAP_GENERAL                           500
#define   OWO_MAP_LOADING                         510
#define   OWO_MAP_ATTACH_TO_ROSTER                520
#define OHO_MAP_CONTACTS                          600
#define   OWO_MAP_CONTACTS                        610
#define OHO_MAP_MESSAGE                           700
#define   OWO_MAP_MESSAGE                         710
#define OHO_MAP_CONNECTION                        800
#define   OWO_MAP_CONNECTION                      810
#define OHO_MAP_ZOOM                              900
#define   OWO_MAP_ZOOM_WHEEL                      910
#define   OWO_MAP_ZOOM_SLIDERTRACK                920
#define OHO_MAP_OSD                               1000
#define   OWO_MAP_OSD                             1010
#define OHO_MAP_SOURCES                           1100
#define   OWO_MAP_SOURCES                         1110
#define OHO_MAPSEARCH_GENERAL                     500
#define   OWO_MAPSEARCH_GENERAL                   510
#define OHO_MAPSEARCH_CONNECTION                  600
#define   OWO_MAPSEARCH_CONNECTION                610
#define OHO_MAPSEARCH_PROVIDERS                   700
#define   OWO_MAPSEARCH_PROVIDER_2GIS             710
#define OWO_MAPMAGNIFIER                          500
#define OWO_MAPSOURCE                             500
//Node = ON_URLPROCESSOR
#define OWO_URLPROCESSOR_URLPROXY                 500
//Node = ON_CONTACTPROXIMITY
#define OWO_CONTACTPROXIMITYNOTIFICATION          500
//Node = ON_XHTML
#define OHO_XHTML_GENERAL                         500
#define   OWO_XHTML_NORICHTEXT                    520
#define   OWO_XHTML_TABINDENT                     530
#define OHO_XHTML_BOB                             600
#define   OWO_XHTML_BOB                           610
//Node = ON_POSITIONING
#define OWO_POSITIONING                           500
//Node = ON_SERIALPORT
#define OWO_SERIALPORT                            500
//Node = ON_NOPOSITION
#define OWO_MANUAL                                500
#define OHO_GEOIP_GENERAL                         500
#define   OWO_GEOIP_GENERAL                       510
#define OHO_GEOIP_CONNECTION                      600
#define   OWO_GEOIP_CONNECTION                    610
#define OWO_LOCATION                              500
#define OWO_POI                                   500
#define OHO_PEP_DELETE                            500
#define   OWO_PEP_DELETE_RETRACT                  510
#define   OWO_PEP_DELETE_PUBLISHEMPTY             520
#define OHO_PEP_USERTUNE                          600
#define   OWO_PEP_USERTUNE                        610
#define   OWO_PEP_USERTUNE_POLLING_INTERVAL       620
#define   OWO_PEP_USERTUNE_INFOREQUESTER_PROXY    630
#define OHO_PEP_USERTUNE_LISTENERS                700
#define   OWO_PEP_USERTUNE_PM123                  710
#define   OWO_PEP_USERTUNE_QUPLAYER               720
#define   OWO_PEP_USERTUNE_Z                      730
#define   OWO_PEP_USERTUNE_FILE                   740
#define OHO_PEP_USERTUNE_REQUESTERS               800
#define   OWO_PEP_USERTUNE_LASTFM                 810
#define OWO_MMPLAYER                              500
#define OWO_TRACKER                               500
#define OHO_ATTENTION                             600
#define   OWO_ATTENTION_AYWAYSPLAYSOUND           610
#define   OWO_ATTENTION_NOTIFICATIONPOPUP         620
#define   OWO_ATTENTION_MAINWINDOWACTIVATE        630
#define OWO_JINGLERTP                             600
//Node = ON_WIZARDS
#define OWO_WIZARDACCOUNT                         500
#define OWO_WIZARDTRANSPORT                       500
// *** >>> eyeCU >>> ***
#endif // DEF_OPTIONWIDGETORDERS_H
