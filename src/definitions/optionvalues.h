#ifndef DEF_OPTIONVALUES_H
#define DEF_OPTIONVALUES_H

// AccountManager
#define OPV_ACCOUNT_ROOT                                "accounts"
#define OPV_ACCOUNT_DEFAULTRESOURCE                     "accounts.default-resource"
#define OPV_ACCOUNT_ITEM                                "accounts.account"
#define OPV_ACCOUNT_ORDER                               "accounts.account.order"
#define OPV_ACCOUNT_NAME                                "accounts.account.name"
#define OPV_ACCOUNT_ACTIVE                              "accounts.account.active"
#define OPV_ACCOUNT_STREAMJID                           "accounts.account.streamJid"
#define OPV_ACCOUNT_RESOURCE                            "accounts.account.resource"
#define OPV_ACCOUNT_PASSWORD                            "accounts.account.password"
#define OPV_ACCOUNT_REQUIREENCRYPTION                   "accounts.account.require-encryption"
// ConnectionManager
#define OPV_ACCOUNT_CONNECTION_ITEM                     "accounts.account.connection"
#define OPV_ACCOUNT_CONNECTION_TYPE                     "accounts.account.connection-type"
// DefaultConnection
#define OPV_ACCOUNT_CONNECTION_HOST                     "accounts.account.connection.host"
#define OPV_ACCOUNT_CONNECTION_PORT                     "accounts.account.connection.port"
#define OPV_ACCOUNT_CONNECTION_PROXY                    "accounts.account.connection.proxy"
#define OPV_ACCOUNT_CONNECTION_SSLPROTOCOL              "accounts.account.connection.ssl-protocol"
#define OPV_ACCOUNT_CONNECTION_USELEGACYSSL             "accounts.account.connection.use-legacy-ssl"
#define OPV_ACCOUNT_CONNECTION_CERTVERIFYMODE           "accounts.account.connection.cert-verify-mode"
// *** <<< eyeCU <<< ***
// Registration
#define OPV_ACCOUNT_REGISTER                            "accounts.account.register-on-server"
// *** >>> eyeCU >>> ***
// StatusChanger
#define OPV_ACCOUNT_AUTOCONNECT                         "accounts.account.auto-connect"
#define OPV_ACCOUNT_AUTORECONNECT                       "accounts.account.auto-reconnect"
#define OPV_ACCOUNT_STATUS_ISMAIN                       "accounts.account.status.is-main"
#define OPV_ACCOUNT_STATUS_LASTONLINE                   "accounts.account.status.last-online"
// BookMarks
#define OPV_ACCOUNT_IGNOREAUTOJOIN                      "accounts.account.ignore-autojoin"
// Compress
#define OPV_ACCOUNT_STREAMCOMPRESS                      "accounts.account.stream-compress"
// MessageArchiver
#define OPV_ACCOUNT_HISTORYREPLICATE                    "accounts.account.history-replicate"
#define OPV_ACCOUNT_HISTORYDUPLICATE                    "accounts.account.history-duplicate"
// *** <<< eyeCU <<< ***
#define OPV_ENABLEPOI                                   "enable-poi"
#define OPV_ACCOUNT_ENABLEPOI                           "accounts.account.enable-poi"
#define OPV_PUBLISHUSERTUNE                             "publish-user-tune"
#define OPV_ACCOUNT_PUBLISHUSERTUNE                     "accounts.account.publish-user-tune"
#define OPV_PUBLISHUSERLOCATION                         "publish-user-location"
#define OPV_ACCOUNT_PUBLISHUSERLOCATION                 "accounts.account.publish-user-location"
#define OPV_NICKNAME                                    "nickname"
#define OPV_ACCOUNT_NICKNAME                            "accounts.account.nickname"
#define OPV_NICKNAMECHANGE                              "nickname-change"
#define OPV_ACCOUNT_NICKNAMECHANGE                      "accounts.account.nickname-change"
#define OPV_NICKNAMESET                                 "nickname-set"
#define OPV_ACCOUNT_NICKNAMESET                         "accounts.account.nickname-set"
#define OPV_NICKNAMEBROADCAST                           "nickname-broadcast"
#define OPV_ACCOUNT_NICKNAMEBROADCAST                   "accounts.account.nickname-broadcast"
// *** >>> eyeCU >>> ***

// BirthdayReminder
#define OPV_BIRTHDAYREMINDER_STARTTIME                  "birthdayreminder.start-time"
#define OPV_BIRTHDAYREMINDER_STOPTIME                   "birthdayreminder.stop-time"

// Console
#define OPV_CONSOLE_ROOT                                "console"
#define OPV_CONSOLE_CONTEXT_ITEM                        "console.context"
#define OPV_CONSOLE_CONTEXT_NAME                        "console.context.name"
#define OPV_CONSOLE_CONTEXT_STREAMJID                   "console.context.streamjid"
#define OPV_CONSOLE_CONTEXT_CONDITIONS                  "console.context.conditions"
#define OPV_CONSOLE_CONTEXT_HIGHLIGHTXML                "console.context.highlight-xml"
#define OPV_CONSOLE_CONTEXT_WORDWRAP                    "console.context.word-wrap"

// DataStreamsManager
#define OPV_DATASTREAMS_ROOT                            "datastreams"
#define OPV_DATASTREAMS_SPROFILE_ITEM                   "datastreams.settings-profile"
#define OPV_DATASTREAMS_SPROFILE_NAME                   "datastreams.settings-profile.name"
#define OPV_DATASTREAMS_METHOD_ITEM                     "datastreams.settings-profile.method"
// InBandStream
#define OPV_DATASTREAMS_METHOD_BLOCKSIZE                "datastreams.settings-profile.method.block-size"
#define OPV_DATASTREAMS_METHOD_MAXBLOCKSIZE             "datastreams.settings-profile.method.max-block-size"
#define OPV_DATASTREAMS_METHOD_STANZATYPE               "datastreams.settings-profile.method.stanza-type"
// SocksStreams
#define OPV_DATASTREAMS_SOCKSLISTENPORT                 "datastreams.socks-listen-port"
#define OPV_DATASTREAMS_METHOD_CONNECTTIMEOUT           "datastreams.settings-profile.method.connect-timeout"
#define OPV_DATASTREAMS_METHOD_ENABLEDIRECT             "datastreams.settings-profile.method.enable-direct-connections"
#define OPV_DATASTREAMS_METHOD_ENABLEFORWARD            "datastreams.settings-profile.method.enable-forward-direct"
#define OPV_DATASTREAMS_METHOD_FORWARDADDRESS           "datastreams.settings-profile.method.forward-direct-address"
#define OPV_DATASTREAMS_METHOD_USEACCOUNTSTREAMPROXY    "datastreams.settings-profile.method.use-account-stream-proxy"
#define OPV_DATASTREAMS_METHOD_USEUSERSTREAMPROXY       "datastreams.settings-profile.method.use-user-stream-proxy"
// *** <<< eyeCU <<< ***
#define OPV_DATASTREAMS_METHOD_USERSTREAMPROXYLIST      "datastreams.settings-profile.method.user-stream-proxy-list"
// *** >>> eyeCU >>> ***
#define OPV_DATASTREAMS_METHOD_USEACCOUNTNETPROXY       "datastreams.settings-profile.method.use-account-network-proxy"
#define OPV_DATASTREAMS_METHOD_USERNETWORKPROXY         "datastreams.settings-profile.method.user-network-proxy"

// FileStreamsManager
#define OPV_FILESTREAMS_DEFAULTDIR                      "filestreams.default-dir"
#define OPV_FILESTREAMS_GROUPBYSENDER                   "filestreams.group-by-sender"
#define OPV_FILESTREAMS_DEFAULTMETHOD                   "filestreams.default-method"
#define OPV_FILESTREAMS_ACCEPTABLEMETHODS               "filestreams.acceptable-methods"
// FileTransfer
#define OPV_FILETRANSFER_ROOT                           "filestreams.filetransfer"
#define OPV_FILETRANSFER_AUTORECEIVE                    "filestreams.filetransfer.autoreceive"
#define OPV_FILETRANSFER_HIDEONSTART                    "filestreams.filetransfer.hide-dialog-on-start"

// MainWindow
#define OPV_MAINWINDOW_SHOWONSTART                      "mainwindow.show-on-start"

// MessageWidgets
#define OPV_MESSAGES_ROOT                               "messages"
#define OPV_MESSAGES_LOADHISTORY                        "messages.load-history"
#define OPV_MESSAGES_SHOWSTATUS                         "messages.show-status"
#define OPV_MESSAGES_ARCHIVESTATUS                      "messages.archive-status"
#define OPV_MESSAGES_EDITORAUTORESIZE                   "messages.editor-auto-resize"
#define OPV_MESSAGES_EDITORMINIMUMLINES                 "messages.editor-minimum-lines"
#define OPV_MESSAGES_CLEANCHATTIMEOUT                   "messages.clean-chat-timeout"
#define OPV_MESSAGES_COMBINEWITHROSTER                  "messages.combine-with-roster"
#define OPV_MESSAGES_TABWINDOWS_ROOT                    "messages.tab-windows"
#define OPV_MESSAGES_TABWINDOWS_ENABLE                  "messages.tab-windows.enable"
#define OPV_MESSAGES_TABWINDOWS_DEFAULT                 "messages.tab-windows.default"
#define OPV_MESSAGES_TABWINDOW_ITEM                     "messages.tab-windows.window"
#define OPV_MESSAGES_TABWINDOW_NAME                     "messages.tab-windows.window.name"
#define OPV_MESSAGES_TABWINDOW_TABSCLOSABLE             "messages.tab-windows.window.tabs-closable"
#define OPV_MESSAGES_TABWINDOW_TABSBOTTOM               "messages.tab-windows.window.tabs-bottom"
// Emoticons
#define OPV_MESSAGES_EMOTICONS_ICONSET                  "messages.emoticons.iconset"
#define OPV_MESSAGES_EMOTICONS_MAXINMESSAGE             "messages.emoticons.max-in-message"
// *** <<< eyeCU <<< ***
#define OPV_MESSAGES_EMOTICONS_RECENT                   "messages.emoticons.recent"
#define OPV_MESSAGES_EMOTICONS_RECENT_SET               "messages.emoticons.recent.set"
#define OPV_MESSAGES_EMOTICONS_INSERTIMAGE              "messages.emoticons.insert-image"
// Emoji
#define OPV_MESSAGES_EMOJI_ICONSET                      "messages.emoji.iconset"
#define OPV_MESSAGES_EMOJI_SKINCOLOR                    "messages.emoji.skin-color"
#define OPV_MESSAGES_EMOJI_RECENT                       "messages.emoji.recent"
#define OPV_MESSAGES_EMOJI_RECENT_SET                   "messages.emoji.recent.set"
// *** >>> eyeCU >>> ***
// ChatStates
#define OPV_MESSAGES_CHATSTATESENABLED                  "messages.chatstates-enabled"
// NormalMessageHandler
#define OPV_MESSAGES_UNNOTIFYALLNORMAL                  "messages.unnotify-all-normal-messages"
// MessageStyles
#define OPV_MESSAGES_SHOWDATESEPARATORS                 "messages.show-date-separators"
#define OPV_MESSAGES_MAXMESSAGESINWINDOW                "messages.max-messages-in-window"
// SpellChecker
#define OPV_MESSAGES_SPELL_LANG                         "messages.spell.lang"
#define OPV_MESSAGES_SPELL_ENABLED                      "messages.spell.enabled"

// MultiUserChat
#define OPV_MUC_GROUPCHAT_SHOWENTERS                    "muc.groupchat.show-enters"
#define OPV_MUC_GROUPCHAT_SHOWSTATUS                    "muc.groupchat.show-status"
#define OPV_MUC_GROUPCHAT_ARCHIVESTATUS                 "muc.groupchat.archive-status"
#define OPV_MUC_GROUPCHAT_REJOINAFTERKICK               "muc.groupchat.rejoin-after-kick"
#define OPV_MUC_GROUPCHAT_QUITONWINDOWCLOSE             "muc.groupchat.quit-on-window-close"
#define OPV_MUC_GROUPCHAT_REFERENUMERATION              "muc.groupchat.refer-enumeration"
// *** <<< eyeCU <<< ***
#define OPV_MUC_GROUPCHAT_NICKNAMESUFFIX                "muc.groupchat.nickname-suffix"
#define OPV_MUC_GROUPCHAT_ADDRESSBUTTON                 "muc.groupchat.address-button"
// *** >>> eyeCU >>> ***
// Bookmarks
#define OPV_MUC_GROUPCHAT_SHOWAUTOJOINED                "muc.groupchat.show-auto-joined"

// MessageArchiver
#define OPV_HISTORY_ENGINE_ITEM                         "history.engine"
#define OPV_HISTORY_ENGINE_ENABLED                      "history.engine.enabled"
#define OPV_HISTORY_ENGINE_REPLICATEAPPEND              "history.engine.replicate-append"
#define OPV_HISTORY_ENGINE_REPLICATEREMOVE              "history.engine.replicate-remove"
#define OPV_HISTORY_ARCHIVEVIEW_FONTPOINTSIZE           "history.archiveview.font-point-size"
// FileMessageArchive
#define OPV_FILEARCHIVE_HOMEPATH                        "history.file-archive.home-path"
#define OPV_FILEARCHIVE_DATABASESYNC                    "history.file-archive.database-sync"
#define OPV_FILEARCHIVE_COLLECTION_MINSIZE              "history.file-archive.collection.min-size"
#define OPV_FILEARCHIVE_COLLECTION_MAXSIZE              "history.file-archive.collection.max-size"
#define OPV_FILEARCHIVE_COLLECTION_CRITICALSIZE         "history.file-archive.collection.critical-size"
// ServerMessageArchive
#define OPV_SERVERARCHIVE_MAXUPLOADSIZE                 "history.server-archive.max-upload-size"

// MessageStyleManager
#define OPV_MESSAGESTYLE_ROOT                           "message-styles"
// *** <<< eyeCU <<< ***
#define OPV_MESSAGESTYLE_FONT_MONOSPACED				"message-styles.font.monospaced"
// *** >>> eyeCU >>> ***
#define OPV_MESSAGESTYLE_MTYPE_ITEM                     "message-styles.message-type"
#define OPV_MESSAGESTYLE_CONTEXT_ITEM                   "message-styles.message-type.context"
#define OPV_MESSAGESTYLE_CONTEXT_ENGINEID               "message-styles.message-type.context.engine-id"
#define OPV_MESSAGESTYLE_ENGINE_ITEM                    "message-styles.message-type.context.engine"
#define OPV_MESSAGESTYLE_ENGINE_STYLEID                 "message-styles.message-type.context.engine.style-id"
#define OPV_MESSAGESTYLE_STYLE_ITEM                     "message-styles.message-type.context.engine.style"
// IMessageStyleEngine
#define OPV_MESSAGESTYLE_STYLE_VARIANT                  "message-styles.message-type.context.engine.style.variant"
#define OPV_MESSAGESTYLE_STYLE_FONTFAMILY               "message-styles.message-type.context.engine.style.font-family"
#define OPV_MESSAGESTYLE_STYLE_FONTSIZE                 "message-styles.message-type.context.engine.style.font-size"
#define OPV_MESSAGESTYLE_STYLE_BGCOLOR                  "message-styles.message-type.context.engine.style.bg-color"
#define OPV_MESSAGESTYLE_STYLE_BGIMAGEFILE              "message-styles.message-type.context.engine.style.bg-image-file"
#define OPV_MESSAGESTYLE_STYLE_BGIMAGELAYOUT            "message-styles.message-type.context.engine.style.bg-image-layout"

// OptionsManager
#define OPV_COMMON_ROOT                                 "common"
#define OPV_COMMON_LANGUAGE                             "common.language"
#define OPV_COMMON_ADVANCED                             "common.advanced"
#define OPV_COMMON_AUTOSTART                            "common.autostart"
// ClintInfo
#define OPV_COMMON_SHAREOSVERSION                       "common.share-os-version"
// Statistics
#define OPV_COMMON_STATISTICTS_ENABLED                  "common.statistics-enabled"
// VCardPlugin
#define OPV_COMMON_RESTRICT_VCARD_IMAGES_SIZE           "common.restrict-vcard-images-size"

// Notifications
#define OPV_NOTIFICATIONS_ROOT                          "notifications"
#define OPV_NOTIFICATIONS_SILENTIFDND                   "notifications.silent-if-dnd"
#define OPV_NOTIFICATIONS_SILENTIFAWAY                  "notifications.silent-if-away"
#define OPV_NOTIFICATIONS_NATIVEPOPUPS                  "notifications.native-popups"
#define OPV_NOTIFICATIONS_FORCESOUND                    "notifications.force-sound"
#define OPV_NOTIFICATIONS_HIDEMESSAGE                   "notifications.hide-message"
#define OPV_NOTIFICATIONS_EXPANDGROUPS                  "notifications.expand-groups"
#define OPV_NOTIFICATIONS_SOUNDCOMMAND                  "notifications.sound-command"
#define OPV_NOTIFICATIONS_POPUPTIMEOUT                  "notifications.popup-timeout"
#define OPV_NOTIFICATIONS_TYPEKINDS_ITEM                "notifications.type-kinds.type"
#define OPV_NOTIFICATIONS_KINDENABLED_ITEM              "notifications.kind-enabled.kind"
// *** <<< eyeCU <<< ***
#define OPV_NOTIFICATIONS_ANIMATIONENABLE               "notifications.animation-enable"
// *** >>> eyeCU >>> ***
// ConnectionManager
#define OPV_PROXY_ROOT                                  "proxy"
#define OPV_PROXY_DEFAULT                               "proxy.default"
#define OPV_PROXY_ITEM                                  "proxy.proxy"
#define OPV_PROXY_NAME                                  "proxy.proxy.name"
#define OPV_PROXY_TYPE                                  "proxy.proxy.type"
#define OPV_PROXY_HOST                                  "proxy.proxy.host"
#define OPV_PROXY_PORT                                  "proxy.proxy.port"
#define OPV_PROXY_USER                                  "proxy.proxy.user"
#define OPV_PROXY_PASS                                  "proxy.proxy.pass"

// RostersView
#define OPV_ROSTER_ROOT                                 "roster"
#define OPV_ROSTER_VIEWMODE                             "roster.view-mode"
#define OPV_ROSTER_SORTMODE                             "roster.sort-mode"
#define OPV_ROSTER_SHOWOFFLINE                          "roster.show-offline"
#define OPV_ROSTER_SHOWRESOURCE                         "roster.show-resource"
#define OPV_ROSTER_HIDESCROLLBAR                        "roster.hide-scrollbar"
#define OPV_ROSTER_MERGESTREAMS                         "roster.merge-streams"
// >>> eyeCU >>>
#define OPV_ROSTER_SHOWSELF                             "roster.show-self"
#define OPV_ROSTER_ALTERNATIONHIGHLITE                  "roster.alternation-highlite"
#define OPV_ROSTER_SHOWOFFLINEAGENTS                    "roster.show-offline-transports"
#define OPV_ROSTER_STATUSDISPLAY                        "roster.status-display"
// <<< eyeCU <<<
// RosterChanger
#define OPV_ROSTER_AUTOSUBSCRIBE                        "roster.auto-subscribe"
#define OPV_ROSTER_AUTOUNSUBSCRIBE                      "roster.auto-unsubscribe"

// RosterSearch
#define OPV_ROSTER_SEARCH_ENABLED                       "roster.search.enabled"
#define OPV_ROSTER_SEARCH_FIELDEBANLED                  "roster.search.field-enabled"
// RosterItemExchange
#define OPV_ROSTER_EXCHANGE_AUTOAPPROVEENABLED          "roster.exchange.auto-approve-enabled"
//RecentContact
#define OPV_ROSTER_RECENT_ALWAYSSHOWOFFLINE             "roster.recent.always-show-offline"
#define OPV_ROSTER_RECENT_HIDEINACTIVEITEMS             "roster.recent.hide-inactive-items"
#define OPV_ROSTER_RECENT_SIMPLEITEMSVIEW               "roster.recent.simple-items-view"
#define OPV_ROSTER_RECENT_SORTBYACTIVETIME              "roster.recent.sort-by-active-time"
#define OPV_ROSTER_RECENT_SHOWONLYFAVORITE              "roster.recent.show-only-favorite"
#define OPV_ROSTER_RECENT_MAXVISIBLEITEMS               "roster.recent.max-visible-items"
#define OPV_ROSTER_RECENT_INACTIVEDAYSTIMEOUT           "roster.recent.inactive-days-timeout"

// ShortcutManager
#define OPV_SHORTCUTS                                   "shortcuts"

// StatusChanger
#define OPV_STATUSES_ROOT                               "statuses"
#define OPV_STATUSES_MAINSTATUS                         "statuses.main-status"
#define OPV_STATUSES_MODIFY                             "statuses.modify-status"
#define OPV_STATUS_ITEM                                 "statuses.status"
#define OPV_STATUS_NAME                                 "statuses.status.name"
#define OPV_STATUS_SHOW                                 "statuses.status.show"
#define OPV_STATUS_TEXT                                 "statuses.status.text"
#define OPV_STATUS_PRIORITY                             "statuses.status.priority"
// AutoStatus
#define OPV_AUTOSTARTUS_ROOT                            "statuses.autostatus"
#define OPV_AUTOSTARTUS_AWAYRULE                        "statuses.autostatus.away-rule"
#define OPV_AUTOSTARTUS_OFFLINERULE                     "statuses.autostatus.offline-rule"
#define OPV_AUTOSTARTUS_RULE_ITEM                       "statuses.autostatus.rule"
#define OPV_AUTOSTARTUS_RULE_ENABLED                    "statuses.autostatus.rule.enabled"
#define OPV_AUTOSTARTUS_RULE_TIME                       "statuses.autostatus.rule.time"
#define OPV_AUTOSTARTUS_RULE_SHOW                       "statuses.autostatus.rule.show"
#define OPV_AUTOSTARTUS_RULE_TEXT                       "statuses.autostatus.rule.text"
#define OPV_AUTOSTARTUS_RULE_PRIORITY                   "statuses.autostatus.rule.priority"

// StatusIcons
#define OPV_STATUSICONS                                 "statusicons"
#define OPV_STATUSICONS_DEFAULT                         "statusicons.default-iconset"
#define OPV_STATUSICONS_RULES_ROOT                      "statusicons.rules"
#define OPV_STATUSICONS_RULE_ITEM                       "statusicons.rules.rule"
#define OPV_STATUSICONS_RULE_PATTERN                    "statusicons.rules.rule.pattern"
#define OPV_STATUSICONS_RULE_ICONSET                    "statusicons.rules.rule.iconset"

// Statistics
#define OPV_STATISTICS_PROFILEID                        "statistics.profile-id"

// XmppStreams
#define OPV_XMPPSTREAMS_TIMEOUT_HANDSHAKE               "xmppstreams.timeout.handshake"
#define OPV_XMPPSTREAMS_TIMEOUT_KEEPALIVE               "xmppstreams.timeout.keepalive"
#define OPV_XMPPSTREAMS_TIMEOUT_DISCONNECT              "xmppstreams.timeout.disconnect"
// RosterPlugin
#define OPV_XMPPSTREAMS_TIMEOUT_ROSTERREQUEST           "xmppstreams.timeout.roster-request"

// *** <<< eyeCU <<< ***
// Avatars
#define OPV_ROSTER_AVATARS_POSITION                     "roster.avatars.position"
#define OPV_ROSTER_AVATARS_DISPLAY                      "roster.avatars.display"
#define OPV_ROSTER_AVATARS_DISPLAYEMPTY                 "roster.avatars.display-empty"
#define OPV_ROSTER_AVATARS_DISPLAYGRAY                  "roster.avatars.display-gray"

// PEP
#define OPV_PEP_DELETE_RETRACT                          "pep.delete.retract"
#define OPV_PEP_DELETE_PUBLISHEMPTY                     "pep.delete.publish_empty"
#define OPV_PEP_NOTIFY_IGNOREOFFLINE                    "pep.notify.ignore_offline"

// Moods
#define OPV_MOOD_ROOT                                   "mood"
#define OPV_MOOD_COMMENT                                "mood.comment"
#define OPV_ROSTER_MOOD_SHOW                            "roster.mood.show"
#define OPV_MESSAGES_MOOD_DISPLAY                       "messages.mood.display"
#define OPV_MESSAGES_MOOD_NOTIFY                        "messages.mood.notify"

// Activities
#define OPV_ACTIVITY_ROOT                               "activ"
#define OPV_ACTIVITY_COMMENT                            "activ.comment"
#define OPV_ROSTER_ACTIVITY_SHOW                        "roster.activity.show"
#define OPV_MESSAGES_ACTIVITY_DISPLAY                   "messages.activity.display"
#define OPV_MESSAGES_ACTIVITY_NOTIFY                    "messages.activity.notify"

//Tune
#define OPV_ROSTER_TUNE_SHOW                            "roster.tune.show"
#define OPV_MESSAGES_TUNE_DISPLAY                       "messages.tune.display"
#define OPV_MESSAGES_TUNE_NOTIFY                        "messages.tune.notify"
#define OPV_TUNE_PUBLISH								"roster.tune.publish"
#define OPV_TUNE_POLLING_INTERVAL                       "roster.tune.polling.interval"
#define OPV_TUNE_INFOREQUESTER_USED                     "roster.tune.info_requester.used"
#define OPV_TUNE_INFOREQUESTER_PROXY                    "roster.tune.info_requester.proxy"
#define OPV_TUNE_INFOREQUESTER_DISPLAYIMAGE             "roster.tune.info_requester.display_image"
#define OPV_TUNE_INFOREQUESTER_QUERYURL                 "roster.tune.info_requester.query_url"
// Tune listeners
#define OPV_TUNE_LISTENER_FILE_NAME                     "roster.tune.listener.file.name"
#define OPV_TUNE_LISTENER_FILE_FORMAT                   "roster.tune.listener.file.format"
#define OPV_TUNE_LISTENER_PM123_PIPENAME                "roster.tune.listener.pm123.pipe_name"
#define OPV_TUNE_LISTENER_QUPLAYER_PIPENAME             "roster.tune.listener.quplayer.pipe_name"
#define OPV_TUNE_LISTENER_Z_PIPENAME                    "roster.tune.listener.z.pipe_name"
// Tune info requesters
#define OPV_TUNE_INFOREQUESTER_LASTFM_IMAGESIZE         "roster.tune.info_requester.last_fm.image_size"
#define OPV_TUNE_INFOREQUESTER_LASTFM_AUTOCORRECT       "roster.tune.info_requester.last_fm.autocorrect"
// Multimedia Player
#define OPV_MMPLAYER_SHOW                               "mmplayer.show"
#define OPV_MMPLAYER_VOLUME                             "mmplayer.volume"
#define OPV_MMPLAYER_MUTE                               "mmplayer.mute"
#define OPV_MMPLAYER_ASPECTRATIOMODE                    "mmplayer.aspect-ratio-mode"
#define OPV_MMPLAYER_SMOOTHRESIZE                       "mmplayer.smooth-resize"
#define OPV_MMPLAYER_DIRECTORY                          "mmplayer.directory"
#define OPV_MMPLAYER_FILTER								"mmplayer.filter"

//Geoloc
#define OPV_ROSTER_GEOLOC_SHOW                          "roster.geoloc.show"
#define OPV_MESSAGES_GEOLOC_DISPLAY                     "messages.geoloc.display"
#define OPV_GEOLOC_EXTSENDER                            "geoloc.extsender"

//Receipts
#define OPV_RECEIPTS_SHOW                               "receipts.show"
#define OPV_RECEIPTS_SEND                               "receipts.send"

// POI
#define OPV_POI_SHOW                                    "poi.show"
#define OPV_POI_CUR_COUNTRY                             "poi.curcountry"
#define OPV_POI_TYPE                                    "poi.type"
#define OPV_POI_FILTER                                  "poi.filter"

#define OPV_POI_PNT_ICONSIZE                            "poi.pnt.size"
#define OPV_POI_PNT_TEXTCOLOR                           "poi.pnt.textcolor"
#define OPV_POI_PNT_TEMPTEXTCOLOR                       "poi.pnt.temptextcolor"
#define OPV_POI_PNT_SHADOWCOLOR                         "poi.pnt.shadowcolor"
#define OPV_POI_PNT_FONT                                "poi.pnt.font"

// Map
#define OPV_MAP_ZOOM                                    "map.zoom"
#define OPV_MAP_SOURCE                                  "map.source"
#define OPV_MAP_MODE                                    "map.mode"
#define OPV_MAP_COORDS                                  "map.coords"
#define OPV_MAP_PROXY                                   "map.proxy"
#define OPV_MAP_LOADING                                 "map.loading"
#define OPV_MAP_GEOMETRY                                "map.geometry"
#define OPV_MAP_SHOWING                                 "map.showing"
#define OPV_MAP_ATTACH_TO_ROSTER                        "map.attachtoroster"
#define OPV_MAP_ZOOM_WHEEL                              "map.zoom.wheel"
#define OPV_MAP_ZOOM_SLIDERTRACK                        "map.zoom.slidertrack"

#define OPV_MAP_OSD_FONT                                "map.osd.font"
#define OPV_MAP_OSD_TEXT_COLOR                          "map.osd.textcolor"

#define OPV_MAP_OSD_CONTR_FOREGROUND                    "map.osd.contr.foreground"
#define OPV_MAP_OSD_CONTR_BASE                          "map.osd.contr.base"
#define OPV_MAP_OSD_CONTR_BUTTON                        "map.osd.contr.button"
#define OPV_MAP_OSD_CONTR_LIGHT                         "map.osd.contr.light"
#define OPV_MAP_OSD_CONTR_MIDLIGHT                      "map.osd.contr.midlight"
#define OPV_MAP_OSD_CONTR_SHADOW                        "map.osd.contr.shadow"
#define OPV_MAP_OSD_CONTR_DARK                          "map.osd.contr.dark"
#define OPV_MAP_OSD_CONTR_BACKGROUND_COLOR              "map.osd.contr.background.color"
#define OPV_MAP_OSD_CONTR_BACKGROUND_ALPHA              "map.osd.contr.background.alpha"
#define OPV_MAP_OSD_CONTR_BACKGROUND_TRANSPARENT        "map.osd.contr.background.transparent"
#define OPV_MAP_OSD_CONTR_CMARKER_COLOR                 "map.osd.contr.cmarker.color"
#define OPV_MAP_OSD_CONTR_CMARKER_ALPHA                 "map.osd.contr.cmarker.alpha"
#define OPV_MAP_OSD_CONTR_CMARKER_VISIBLE               "map.osd.contr.cmarker.visible"

#define OPV_MAP_OSD_SHADOW_COLOR                        "map.osd.shadowcolor"
#define OPV_MAP_OSD_SHADOW_ALPHA                        "map.osd.shadowalpha"
#define OPV_MAP_OSD_SHADOW_BLUR                         "map.osd.shadowblur"
#define OPV_MAP_OSD_SHADOW_SHIFT                        "map.osd.shadowshift"

#define OPV_MAP_OSD_BOX_FOREGROUND                      "map.osd.box.foreground"
#define OPV_MAP_OSD_BOX_LIGHT                           "map.osd.box.light"
#define OPV_MAP_OSD_BOX_MIDLIGHT                        "map.osd.box.midlight"
#define OPV_MAP_OSD_BOX_DARK                            "map.osd.box.dark"
#define OPV_MAP_OSD_BOX_BACKGROUND_COLOR                "map.osd.box.background.color"
#define OPV_MAP_OSD_BOX_BACKGROUND_ALPHA                "map.osd.box.background.alpha"
#define OPV_MAP_OSD_BOX_BACKGROUND_TRANSPARENT          "map.osd.box.background.transparent"
#define OPV_MAP_OSD_BOX_SHAPE                           "map.osd.box.shape"
#define OPV_MAP_OSD_BOX_SHADOW                          "map.osd.box.shadow"

#define OPV_MAP_SOURCE_GOOGLE                           "map.source.google"
#define OPV_MAP_SOURCE_GOOGLE_VERSION_SATELLITE         "map.source.google.version.satellite"
#define OPV_MAP_SOURCE_GOOGLE_VERSION_MAP               "map.source.google.version.map"
#define OPV_MAP_SOURCE_GOOGLE_VERSION_TERRAIN_R         "map.source.google.version.terrain.r"
#define OPV_MAP_SOURCE_GOOGLE_VERSION_TERRAIN_T         "map.source.google.version.terrain.t"
#define OPV_MAP_SOURCE_YANDEX                           "map.source.yandex"
#define OPV_MAP_SOURCE_YANDEX_VERSION_SCHEME            "map.source.yandex.version.scheme"
#define OPV_MAP_SOURCE_YANDEX_VERSION_SATELLITE         "map.source.yandex.version.satellite"

#define OPV_MAP_MAGNIFIER_SIZE                          "map.magnifier.size"
#define OPV_MAP_MAGNIFIER_ZOOM                          "map.magnifier.zoom"
#define OPV_MAP_MAGNIFIER_SCALE                         "map.magnifier.scale"
#define OPV_MAP_MAGNIFIER_RULERS                        "map.magnifier.rulers"
#define OPV_MAP_MAGNIFIER_OBJECTS                       "map.magnifier.objects"
#define OPV_MAP_MAGNIFIER_HIGHPRECISION                 "map.magnifier.highprecision"
#define OPV_MAP_MAGNIFIER_SHADOW_COLOR                  "map.magnifier.shadow.color"
#define OPV_MAP_MAGNIFIER_SHADOW_OPACITY                "map.magnifier.shadow.opacity"
#define OPV_MAP_MAGNIFIER_SHADOW_SHIFT                  "map.magnifier.shadow.shift"
#define OPV_MAP_MAGNIFIER_SHADOW_BLUR                   "map.magnifier.shadow.blur"
#define OPV_MAP_MAGNIFIER_ZOOMFACTOR                    "map.magnifier.zoomfactor"
#define OPV_MAP_MAGNIFIER_ZOOMFACTOR_COLOR              "map.magnifier.zoomfactor.color"
#define OPV_MAP_MAGNIFIER_ZOOMFACTOR_OPACITY            "map.magnifier.zoomfactor.opacity"
#define OPV_MAP_MAGNIFIER_ZOOMFACTOR_FONT               "map.magnifier.zoomfactor.font"

// Map search
#define OPV_MAP_SEARCH_LABEL_COLOR                      "map.search.label.color"
#define OPV_MAP_SEARCH_SHOW                             "map.search.show"
#define OPV_MAP_SEARCH_LIMITRANGE                       "map.search.limit-range"
#define OPV_MAP_SEARCH_PAGESIZE                         "map.search.page-size"
#define OPV_MAP_SEARCH_PAGESIZE_DEFAULT                 "map.search.page-size.default"
#define OPV_MAP_SEARCH_PROVIDER                         "map.search.provider"
#define OPV_MAP_SEARCH_PROXY                            "map.search.proxy"
#define OPV_MAP_SEARCH_PROVIDER_2GIS_TYPE               "map.search.provider.doublegis.type"

// Street View
#define OPV_MAP_STREETVIEW                              "map.streetview"
#define OPV_MAP_STREETVIEW_FOV                          "map.streetview.fov"
#define OPV_MAP_STREETVIEW_SIZE                         "map.streetview.size"
#define OPV_MAP_STREETVIEW_PROVIDER                     "map.streetview.provider"
#define OPV_MAP_STREETVIEW_IMAGEDIRECTORY               "map.streetview.image_directory"
// Place View
#define OPV_MAP_PLACEVIEW                               "map.placeview"
#define OPV_MAP_PLACEVIEW_PROVIDER                      "map.placeview.provider"
#define OPV_MAP_PLACEVIEW_RADIUS                        "map.placeview.radius"
#define OPV_MAP_PLACEVIEW_TYPE                          "map.placeview.type"
#define OPV_MAP_PLACEVIEW_RANKBY                        "map.placeview.rankby"
#define OPV_MAP_PLACEVIEW_WAY                           "map.placeview.way"
//Weather
#define OPV_MAP_WEATHER                                 "map.weather"
#define OPV_MAP_WEATHER_PROVIDER                        "map.weather.provider"

// Map place
#define OPV_PLACE_GOOGLE_KEY                            "map.place.google.key"
#define OPV_PLACE_GOOGLE_USERKEY                        "map.place.google.userkey"
#define OPV_PLACE_GOOGLE_KEY_STATUS                     "map.place.google.keystatus"
// Map contacts
#define OPV_MAP_CONTACTS_VIEW                           "map.contacts.view"
#define OPV_MAP_CONTACTS_FOLLOW                         "map.contacts.follow"

// Contact proximity notifications
#define OPV_CONTACTPROXIMITYNOTIFICATIONS_DISTANCE      "contact-proximity-notifications.distance"
#define OPV_CONTACTPROXIMITYNOTIFICATIONS_TRESHOLD      "contact-proximity-notifications.treshold"
#define OPV_CONTACTPROXIMITYNOTIFICATIONS_IGNOREOWN     "contact-proximity-notifications.ignore-own-resources"

// Map message
#define OPV_MAP_MESSAGE_AUTOFOCUS                       "map.message.autofocus"
#define OPV_MAP_MESSAGE_SHOW                            "map.message.show"
#define OPV_MAP_MESSAGE_ANIMATE                         "map.message.animate"

// Positioning
#define OPV_POSITIONING_ROOT                            "positioning"
#define OPV_POSITIONING_METHOD                          "positioning.method"
// Positioning methods
// Serial port
#define OPV_POSITIONING_METHOD_SERIALPORT               "positioning.method.serialport"
#define OPV_POSITIONING_METHOD_SERIALPORT_NAME          "positioning.method.serialport.name"
#define OPV_POSITIONING_METHOD_SERIALPORT_BUFFERSIZE    "positioning.method.serialport.buffer-size"
#define OPV_POSITIONING_METHOD_SERIALPORT_TIMEOUT       "positioning.method.serialport.timeout"

#define OPV_POSITIONING_METHOD_SERIALPORT_BAUDRATE      "positioning.method.serialport.baud-rate"
#define OPV_POSITIONING_METHOD_SERIALPORT_DATABITS      "positioning.method.serialport.data-bits"
#define OPV_POSITIONING_METHOD_SERIALPORT_PARITY        "positioning.method.serialport.parity"
#define OPV_POSITIONING_METHOD_SERIALPORT_FLOWCONTROL   "positioning.method.serialport.flow-control"
#define OPV_POSITIONING_METHOD_SERIALPORT_STOPBITS      "positioning.method.serialport.stop-bits"

#define OPV_POSITIONING_METHOD_SERIALPORT_TIMETRESHOLD  "positioning.method.serialport.time-treshold"
#define OPV_POSITIONING_METHOD_SERIALPORT_DISTANCETRESHOLD "positioning.method.serialport.distance-treshold"
// Manual
#define OPV_POSITIONING_METHOD_MANUAL_COORDINATES       "positioning.method.manual.coordinates"
#define OPV_POSITIONING_METHOD_MANUAL_POI               "positioning.method.manual.poi"
#define OPV_POSITIONING_METHOD_MANUAL_TIMESTAMP         "positioning.method.manual.timestamp"
#define OPV_POSITIONING_METHOD_MANUAL_INTERVAL          "positioning.method.manual.interval"
// Location
#define OPV_POSITIONING_METHOD_LOCATION_INTERVAL        "positioning.method.location.interval"
// IP
#define OPV_POSITIONING_METHOD_IP_UPDATERATE            "positioning.method.ip.update-rate"
#define OPV_POSITIONING_METHOD_IP_PROVIDER              "positioning.method.ip.provider"
#define OPV_POSITIONING_METHOD_IP_PROXY                 "positioning.method.ip.proxy"

// Tracker
#define OPV_TRACKER                                     "tracker"
#define OPV_TRC_LINECOLOR                               "tracker.linecolor"
#define OPV_TRC_TEXTCOLOR                               "tracker.textcolor"
#define OPV_TRC_SHADOWCOLOR                             "tracker.shadowcolor"
#define OPV_TRC_FONT                                    "tracker.font"
#define OPV_TRC_LINE_TYPE                               "tracker.type"
#define OPV_TRC_LINE_SIZE                               "tracker.size"

// XHTML-IM
#define OPV_XHTML                                       "xhtml"
#define OPV_XHTML_MAXAGE                                "xhtml.maxage"
#define OPV_XHTML_EMBEDSIZE                             "xhtml.embed-size"
#define OPV_XHTML_DEFAULTIMAGEFORMAT                    "xhtml.default-image-format"
#define OPV_XHTML_TABINDENT                             "xhtml.tab-indent"
#define OPV_XHTML_NORICHTEXT                            "xhtml.no-rich-text"
#define OPV_XHTML_IMAGESAVEDIRECTORY                    "xhtml.image-save-directory"
#define OPV_XHTML_IMAGEOPENDIRECTORY                    "xhtml.image-open-directory"
#define OPV_XHTML_EDITORTOOLBAR                         "xhtml.editor-toolbar"
#define OPV_XHTML_EDITORMENU                            "xhtml.editor-menu"
#define OPV_XHTML_FORMATAUTORESET                       "xhtml.format-auto-reset"

// Attention
#define OPV_ATTENTION_ROOT                              "attention"
#define OPV_ATTENTION_NOTIFICATIONPOPUP                 "attention.notification-popup"
#define OPV_ATTENTION_AYWAYSPLAYSOUND                   "attention.always-play-sound"

// Jingle RTP
#define OPV_JINGLERTP                                   "jinglertp"
#define OPV_JINGLERTP_NOTIFYINTERVAL                    "jinglertp.notify-interval"
#define OPV_JINGLERTP_USERTCP                           "jinglertp.use-rtcp"
// Client Icons
#define OPV_ROSTER_CLIENTICON_SHOW                     "roster.clienticon.show"
#define OPV_MESSAGES_CLIENTICON_DISPLAY                "messages.clienticon.display"
// *** >>> eyeCU >>> ***

#endif // DEF_OPTIONVALUES_H
