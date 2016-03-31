#ifndef DEF_SHORTCUTS_H
#define DEF_SHORTCUTS_H

//PluginManager
#define SCTG_GLOBAL                                                "global"
//MainWindow
#define   SCT_GLOBAL_SHOW                                          "global.show" /*** <<< eyeCU >>> ***/
//ShortcutManager
#define   SCT_GLOBAL_HIDEALLWIDGETS                                "global.hide-all-widgets"
//Notifications
#define   SCT_GLOBAL_TOGGLESOUND                                   "global.toggle-sound"
#define   SCT_GLOBAL_ACTIVATELASTNOTIFICATION                      "global.activate-last-notification"

//PluginManager
#define SCTG_APPLICATION                                           "application"
//MultiUserChat
#define   SCT_APP_MULTIUSERCHAT_WIZARD                             "application.muc-wizard"
//FileStreamsManager
#define   SCT_APP_FILETRANSFERS_SHOW                               "application.show-filetransfers"

//RosterView
#define SCTG_ROSTERVIEW                                            "roster-view"
#define   SCT_ROSTERVIEW_TOGGLESHOWOFFLINE                         "roster-view.toggle-show-offline"
//MainWindow
#define   SCT_ROSTERVIEW_CLOSEWINDOW                               "roster-view.close-window"
//ChatMessageHandler
#define   SCT_ROSTERVIEW_SHOWCHATDIALOG                            "roster-view.show-chat-dialog"
//NormalMessageHandler
#define   SCT_ROSTERVIEW_SHOWNORMALDIALOG                          "roster-view.show-normal-dialog"
//RosterChanger
#define   SCT_ROSTERVIEW_ADDCONTACT                                "roster-view.add-contact"
#define   SCT_ROSTERVIEW_RENAME                                    "roster-view.rename"
#define   SCT_ROSTERVIEW_REMOVEFROMGROUP                           "roster-view.remove-from-group"
#define   SCT_ROSTERVIEW_REMOVEFROMROSTER                          "roster-view.remove-from-roster"
//MessageArchiver
#define   SCT_ROSTERVIEW_SHOWHISTORY                               "roster-view.show-history"
//vCard
#define   SCT_ROSTERVIEW_SHOWVCARD                                 "roster-view.show-vcard"
//RecentContacts
#define   SCT_ROSTERVIEW_INSERTFAVORITE                            "roster-view.insert-favorite"
#define   SCT_ROSTERVIEW_REMOVEFAVORITE                            "roster-view.remove-favorite"
//Metacontacts
#define   SCT_ROSTERVIEW_COMBINECONTACTS                           "roster-view.combine-contacts"
#define   SCT_ROSTERVIEW_DESTROYMETACONTACT                        "roster-view.destroy-metacontact"
#define   SCT_ROSTERVIEW_DETACHFROMMETACONTACT                     "roster-view.detach-from-metacontact"

//MessageWidgets
#define SCTG_MESSAGEWINDOWS                                        "message-windows"
#define   SCT_MESSAGEWINDOWS_QUOTE                                 "message-windows.quote"
#define   SCT_MESSAGEWINDOWS_CLOSEWINDOW                           "message-windows.close-window"
#define   SCT_MESSAGEWINDOWS_EDITPREVMESSAGE                       "message-windows.edit-prev-message"
#define   SCT_MESSAGEWINDOWS_EDITNEXTMESSAGE                       "message-windows.edit-next-message"
#define   SCT_MESSAGEWINDOWS_SENDCHATMESSAGE                       "message-windows.send-chat-message"
#define   SCT_MESSAGEWINDOWS_SENDNORMALMESSAGE                     "message-windows.send-normal-message"
//FileTransfer
#define   SCT_MESSAGEWINDOWS_SENDFILE                              "message-windows.sendfile"
//MessageArchiver
#define   SCT_MESSAGEWINDOWS_SHOWHISTORY                           "message-windows.show-history"
//vCard
#define   SCT_MESSAGEWINDOWS_SHOWVCARD                             "message-windows.show-vcard"

//MultiUserChat
#define   SCT_MESSAGEWINDOWS_SHOWMUCUSERS                          "message-windows.show-muc-users"

//MessageWidgets
#define SCTG_TABWINDOW                                             "tab-window"
#define   SCT_TABWINDOW_NEXTTAB                                    "tab-window.next-tab"
#define   SCT_TABWINDOW_PREVTAB                                    "tab-window.prev-tab"
#define   SCT_TABWINDOW_CLOSETAB                                   "tab-window.close-tab"
#define   SCT_TABWINDOW_CLOSEOTHERTABS                             "tab-window.close-other-tabs"
#define   SCT_TABWINDOW_DETACHTAB                                  "tab-window.detach-tab"
#define   SCT_TABWINDOW_CLOSEWINDOW                                "tab-window.close-window"
#define   SCT_TABWINDOW_QUICKTAB                                   "tab-window.quick-tabs.tab%1"

// *** <<< eyeCU <<< ***
// Client icons
#define   SCT_MESSAGEWINDOWS_CHAT_SHOWSOFTWAREVERSION              "message-windows.show-software-version"

// Attention
#define   SCT_MESSAGEWINDOWS_CHAT_ATTENTION                        "message-windows.attention"

// Link dialog
#define SCTG_MESSAGEWINDOWS_LINKDIALOG                             "message-windows.link-dialog"
#define   SCT_MESSAGEWINDOWS_LINKDIALOG_OK                         "message-windows.link-dialog.ok"

// Map Contacts
#define   SCT_ROSTERVIEW_SHOWCONTACTONTHEMAP                       "roster-view.show-contact-on-the-map"
#define   SCT_MESSAGEWINDOWS_CHAT_SHOWCONTACTONTHEMAP              "message-windows.show-contact-on-the-map"

// OOB
#define SCTG_MESSAGEWINDOWS_OOB                                    "message-windows.oob"
#define   SCT_MESSAGEWINDOWS_OOB_INSERTLINK                        "message-windows.oob.insert-link"
#define   SCT_MESSAGEWINDOWS_OOB_DELETELINK                        "message-windows.oob.delete-link"
#define   SCT_MESSAGEWINDOWS_OOB_EDITLINK                          "message-windows.oob.edit-link"

// XHTML-IM
#define SCTG_MESSAGEWINDOWS_XHTMLIM				                   "message-windows.xhtml-im"
#define   SCTG_MESSAGEWINDOWS_XHTMLIM_INSERTIMAGEDIALOG            "message-windows.xhtml-im.insert-image-dialog"
#define     SCT_MESSAGEWINDOWS_XHTMLIM_INSERTIMAGEDIALOG_BROWSE    "message-windows.xhtml-im.insert-image-dialog.browse"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_INSERTIMAGE                   "message-windows.xhtml-im.insert-image"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_INSERTLINK                    "message-windows.xhtml-im.insert-link"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_INSERTNBSP                    "message-windows.xhtml-im.insert-nbsp"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_INSERTNEWLINE                 "message-windows.xhtml-im.insert-newline"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_SETTOOLTIP                    "message-windows.xhtml-im.set-title"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_BOLD                          "message-windows.xhtml-im.bold"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_ITALIC                        "message-windows.xhtml-im.italic"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_UNDERLINE                     "message-windows.xhtml-im.underline"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_OVERLINE                      "message-windows.xhtml-im.overline"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_STRIKEOUT                     "message-windows.xhtml-im.strikeout"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_CODE                          "message-windows.xhtml-im.code"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_FONT                          "message-windows.xhtml-im.font"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_CAPSMIXED                     "message-windows.xhtml-im.caps-mixed"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_CAPSSMALL                     "message-windows.xhtml-im.caps-small"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_CAPSCAPITALIZE                "message-windows.xhtml-im.caps-capitalize"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_CAPSALLUPPER                  "message-windows.xhtml-im.caps-all-upper"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_CAPSALLLOWER                  "message-windows.xhtml-im.caps-all-lower"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_FOREGROUNDCOLOR               "message-windows.xhtml-im.foreground-color"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_BACKGROUNDCOLOR               "message-windows.xhtml-im.background-color"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_ALIGNCENTER                   "message-windows.xhtml-im.align-center"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_ALIGNRIGHT                    "message-windows.xhtml-im.align-right"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_ALIGNLEFT                     "message-windows.xhtml-im.align-left"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_ALIGNJUSTIFY                  "message-windows.xhtml-im.align-justify"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_FORMATREMOVE                  "message-windows.xhtml-im.format-remove"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_FORMATAUTORESET               "message-windows.xhtml-im.format-auto-reset"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_INDENTINCREASE                "message-windows.xhtml-im.indent-increase"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_INDENTDECREASE                "message-windows.xhtml-im.indent-decrease"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_HEADING1					   "message-windows.xhtml-im.header-1"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_HEADING2					   "message-windows.xhtml-im.header-2"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_HEADING3					   "message-windows.xhtml-im.header-3"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_HEADING4					   "message-windows.xhtml-im.header-4"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_HEADING5					   "message-windows.xhtml-im.header-5"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_HEADING6					   "message-windows.xhtml-im.header-6"
#define   SCT_MESSAGEWINDOWS_XHTMLIM_PREFORMATTED				   "message-windows.xhtml-im.preformatted"

// Map
#define SCTG_MAP                                                   "map"
#define   SCT_MAP_SHOW                                             "map.show"
#define   SCT_MAP_NEWCENTER                                        "map.new-center"
#define   SCT_MAP_MYLOCATION                                       "map.my-location"
#define   SCT_MAP_REFRESH                                          "map.reload"
#define   SCT_MAP_OPTIONS                                          "map.options"
#define   SCTG_MAP_MAGNIFIER                                       "map.magnifier"
#define     SCT_MAP_MAGNIFIER_TOGGLE                               "map.magnifier.toggle"
#define     SCT_MAP_MAGNIFIER_ZOOMIN                               "map.magnifier.zoom-in"
#define     SCT_MAP_MAGNIFIER_ZOOMOUT                              "map.magnifier.zoom-out"
#define   SCTG_MAP_MOVE                                            "map.move"
#define     SCT_MAP_MOVE_RIGHT                                     "map.move.right"
#define     SCT_MAP_MOVE_LEFT                                      "map.move.left"
#define     SCT_MAP_MOVE_UP                                        "map.move.up"
#define     SCT_MAP_MOVE_DOWN                                      "map.move.down"
#define   SCTG_MAP_ZOOM                                            "map.zoom"
#define     SCT_MAP_ZOOM_IN                                        "map.zoom.in"
#define     SCT_MAP_ZOOM_OUT                                       "map.zoom.out"
#define   SCT_MAP_STOPLOCATIONPUBLICATION                          "map.stop-location-publication"

// POI
#define SCTG_POI                                                   "poi"
#define   SCT_POI_VIEW                                             "poi.view"
#define   SCT_POI_LIST                                             "poi.list"
#define   SCT_POI_LISTACCOUNT                                      "poi.list-account"
#define   SCT_POI_SHOW                                             "poi.show"
#define   SCT_POI_DELETE                                           "poi.delete"
#define   SCT_POI_REMOVE                                           "poi.remove"
#define   SCT_POI_EDIT                                             "poi.edit"
#define   SCT_POI_SAVE                                             "poi.save"
#define   SCT_POI_HEREIAM                                          "poi.here-i-am"
#define   SCT_POI_OPENURL                                          "poi.open-url"
#define SCTG_MESSAGEWINDOWS_POI                                    "message-windows.poi"
#define   SCT_MESSAGEWINDOWS_POI_INSERT                            "message-windows.poi.insert"
#define   SCT_MESSAGEWINDOWS_POI_EDIT                              "message-windows.poi.edit"
#define   SCT_MESSAGEWINDOWS_POI_DELETE                            "message-windows.poi.delete"
#define   SCT_MESSAGEWINDOWS_POI_INSERTLOCATION                    "message-windows.poi.insert-location"

// Mood
#define   SCT_APP_SETMOOD											"application.set-mood"
#define   SCT_ROSTERVIEW_SETMOOD									"roster-view.set-mood"
#define   SCT_MESSAGEWINDOWS_SETMOOD								"message-windows.set-mood"

// Activity
#define   SCT_APP_SETACTIVITY										"application.set-activity"
#define   SCT_ROSTERVIEW_SETACTIVITY								"roster-view.set-activity"

// Tune
#define   SCT_APP_PUBLISHTUNE										"application.publish-tune"

// Map Search
#define SCTG_MAPSEARCH												"map-search"
#define   SCT_MAPSEARCH_SEARCHDIALOG								"map-search.search-dialog"
#define   SCT_MAPSEARCH_CLEARLIST									"map-search.clear-list"
#define   SCT_MAPSEARCH_SHOW										"map-search.show"
#define   SCT_MAPSEARCH_LIMITRANGE									"map-search.limit-range"
#define   SCT_MAPSEARCH_SELECTPROVIDER								"map-search.select-provider"
#define   SCT_MESSAGEWINDOWS_POI_INSERTSEARCHRESULT					"message-windows.poi.insert-search-result"

// Multimedia Player
#define SCTG_MMPLAYER												"multimedia-player"
#define	  SCT_MMPLAYER_SHOW											"multimedia-player.show"
#define   SCT_MMPLAYER_OPEN											"multimedia-player.open"
#define   SCT_MMPLAYER_PLAY											"multimedia-player.play"
#define   SCT_MMPLAYER_VOLUMEUP										"multimedia-player.volume-up"
#define   SCT_MMPLAYER_VOLUMEDOWN									"multimedia-player.volume-down"
#define   SCT_MMPLAYER_MUTE											"multimedia-player.mute"
#define   SCT_MMPLAYER_OPTIONS										"multimedia-player.options"

// *** >>> eyeCU >>> ***
#endif // DEF_SHORTCUTS_H
