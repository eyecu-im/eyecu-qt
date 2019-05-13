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

// Avatars
#define OPV_AVATARS_DISPLAYEMPTY						"avatars.display-empty"
#define OPV_AVATARS_ASPECTCROP							"avatars.aspect-crop"
#define OPV_AVATARS_SMALLSIZE                           "avatars.small-size"
#define OPV_AVATARS_LARGESIZE                           "avatars.large-size"
#define OPV_AVATARS_NORMALSIZE                          "avatars.normal-size"

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
#define OPV_MESSAGES_EDITORBASEFONTSIZE                 "messages.editor-base-font-size"
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
#define OPV_MESSAGES_EMOJI_GENDER						"messages.emoji.gender"
#define OPV_MESSAGES_EMOJI_SIZE_MENU                    "messages.emoji.size.menu"
#define OPV_MESSAGES_EMOJI_SIZE_CHAT                    "messages.emoji.size.chat"
#define OPV_MESSAGES_EMOJI_RECENT                       "messages.emoji.recent"
#define OPV_MESSAGES_EMOJI_CATEGORY                     "messages.emoji.category"
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
#define OPV_MUC_SHOWENTERS                              "muc.show-enters"
#define OPV_MUC_SHOWSTATUS                              "muc.show-status"
#define OPV_MUC_ARCHIVESTATUS                           "muc.archive-status"
#define OPV_MUC_REJOINAFTERKICK                         "muc.rejoin-after-kick"
#define OPV_MUC_QUITONWINDOWCLOSE                       "muc.quit-on-window-close"
#define OPV_MUC_REFERENUMERATION                        "muc.refer-enumeration"
#define OPV_MUC_NICKNAMESUFFIX                          "muc.nickname-suffix"
#define OPV_MUC_USERVIEWMODE                            "muc.user-view-mode"
#define OPV_MUC_GROUPCHAT_ITEM                          "muc.groupchat"
#define OPV_MUC_GROUPCHAT_NOTIFYSILENCE                 "muc.groupchat.notify-silence"
// *** <<< eyeCU <<< ***
#define OPV_MUC_ADDRESSBUTTON							"muc.address-button"
#define OPV_MUC_STATUSDISPLAY							"muc.status-display"
#define OPV_MUC_ALTERNATIONHIGHLIGHT					"muc.alternation-highlight"
#define OPV_MUC_CONFIRMLEAVE							"muc.confirm-before-leave"
#define OPV_MUC_LEAVESTATUS								"muc.leave-status-message"
#define OPV_MUC_LEAVESTATUSSTORE						"muc.leave-status-message-store"
#define OPV_MUC_SHOWINITIALJOINS                        "muc.show-initial-joins"
#define OPV_MUC_INVITATIONREASON                        "muc.invitation-reason"
#define OPV_MUC_INVITATIONREASONASK                     "muc.invitation-reason-ask"
#define OPV_MUC_INVITATIONREASONSTORE                   "muc.invitation-reason-store"
#define OPV_MUC_INVITATIONDECLINEREASON                 "muc.invitation-decline-reason"
#define OPV_MUC_INVITATIONDECLINEREASONASK              "muc.invitation-decline-reason-ask"
#define OPV_MUC_INVITATIONDECLINEREASONSTORE            "muc.invitation-decline-reason-store"

// Avatars
#define OPV_MUC_AVATARS									"muc.avatars"
#define OPV_MUC_AVATARS_SIZE							"muc.avatars.size"
#define OPV_MUC_AVATARS_POSITION						"muc.avatars.position"
#define OPV_MUC_AVATARS_DISPLAY							"muc.avatars.display"
#define OPV_MUC_AVATARS_DISPLAYEMPTY					"muc.avatars.display-empty"
#define OPV_MUC_AVATARS_ROUNDED							"muc.avatars.rounded"
// *** >>> eyeCU >>> ***
// Bookmarks
#define OPV_MUC_SHOWAUTOJOINED                          "muc.show-auto-joined"

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
#define OPV_MESSAGESTYLE_STYLE_SELFCOLOR                "message-styles.message-type.context.engine.style.self-color"
#define OPV_MESSAGESTYLE_STYLE_CONTACTCOLOR             "message-styles.message-type.context.engine.style.contact-color"
#define OPV_MESSAGESTYLE_STYLE_BGCOLOR                  "message-styles.message-type.context.engine.style.bg-color"
#define OPV_MESSAGESTYLE_STYLE_BGIMAGEFILE              "message-styles.message-type.context.engine.style.bg-image-file"
#define OPV_MESSAGESTYLE_STYLE_BGIMAGELAYOUT            "message-styles.message-type.context.engine.style.bg-image-layout"

// OptionsManager
#define OPV_COMMON_ROOT                                 "common"
#define OPV_COMMON_LANGUAGE                             "common.language"
// *** <<< eyeCU <<< ***
#define OPV_COMMON_ADVANCED                             "common.advanced"
// *** >>> eyeCU >>> ***
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
#define OPV_ROSTER_ALTERNATIONHIGHLIGHT                  "roster.alternation-highlight"
#define OPV_ROSTER_SHOWOFFLINEAGENTS                    "roster.show-offline-transports"
#define OPV_ROSTER_STATUSDISPLAY                        "roster.status-display"
// Avatars
#define OPV_ROSTER_AVATARS		                        "roster.avatars"
#define OPV_ROSTER_AVATARS_SIZE                         "roster.avatars.size"
#define OPV_ROSTER_AVATARS_ROUNDED                      "roster.avatars.rounded"
#define OPV_ROSTER_AVATARS_POSITION                     "roster.avatars.position"
#define OPV_ROSTER_AVATARS_DISPLAY                      "roster.avatars.display"
#define OPV_ROSTER_AVATARS_DISPLAYGRAY                  "roster.avatars.display-gray"
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

//Message Delivery Receipts / Chat Markers
#define OPV_MARKERS_SEND_RECEIVED						"markers.send.received"
#define OPV_MARKERS_SEND_DISPLAYED						"markers.send.dsiplayed"
#define OPV_MARKERS_SEND_ACK							"markers.send.ack"

#define OPV_MARKERS_SHOW_LEVEL							"markers.show.level"
#define OPV_MARKERS_SHOW_ACKOWN							"markers.show.ack_own"

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
#define OPV_MAP_FOLLOWMYLOCATION                        "map.follow-my-location"
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

#define OPV_MAP_OSD_CMARKER_COLOR                       "map.osd.cmarker.color"
#define OPV_MAP_OSD_CMARKER_ALPHA                       "map.osd.cmarker.alpha"
#define OPV_MAP_OSD_CMARKER_VISIBLE                     "map.osd.cmarker.visible"

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
#define OPV_MAP_SOURCE_HERE                             "map.source.here"
#define OPV_MAP_SOURCE_HERE_LANG_PRIMARY                "map.source.here.lang.primary"
#define OPV_MAP_SOURCE_HERE_LANG_SECONDARY              "map.source.here.lang.secondary"
#define OPV_MAP_SOURCE_HERE_MODE_NIGHT					"map.source.here.night"
#define OPV_MAP_SOURCE_HERE_POLITICALVIEW				"map.source.political_view"

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
#define OPV_MAP_SEARCH_PROVIDER_2GIS_TYPE               "map.search.provider.2gis.type"
#define OPV_MAP_SEARCH_PROVIDER_HERE_POLITICALVIEW      "map.search.provider.here.political_view"

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

// Jingle
#define OPV_JINGLE                                      "jingle"
//  Transports
#define OPV_JINGLE_TRANSPORT                            "jingle.transport"
//   Raw UDP
#define OPV_JINGLE_TRANSPORT_RAWUDP                     "jingle.transport.raw-udp"
#define OPV_JINGLE_TRANSPORT_RAWUDP_IP                  "jingle.transport.raw-udp.ip"
#define OPV_JINGLE_TRANSPORT_RAWUDP_PORT_FIRST          "jingle.transport.raw-udp.port.first"
#define OPV_JINGLE_TRANSPORT_RAWUDP_PORT_LAST           "jingle.transport.raw-udp.port.last"
#define OPV_JINGLE_TRANSPORT_RAWUDP_TIMEOUT             "jingle.transport.raw-udp.timeout"
//   ICE
#define OPV_JINGLE_TRANSPORT_ICE                        "jingle.transport.ice"
#define OPV_JINGLE_TRANSPORT_ICE_STUN_RTO				"jingle.transport.ice.stun.rto"
#define OPV_JINGLE_TRANSPORT_ICE_NOMINATION_AGGRESSIVE  "jingle.transport.ice.nomination.aggressive"
#define OPV_JINGLE_TRANSPORT_ICE_NOMINATION_DELAY       "jingle.transport.ice.nomination.delay"
#define OPV_JINGLE_TRANSPORT_ICE_NOMINATION_WAIT        "jingle.transport.ice.nomination.wait"
#define OPV_JINGLE_TRANSPORT_ICE_SERVERS_STUN           "jingle.transport.ice.servers.stun"
#define OPV_JINGLE_TRANSPORT_ICE_SERVERS_TURN           "jingle.transport.ice.servers.turn"
//  Applications
//   RTP
#define OPV_JINGLE_RTP                                  "jingle.rtp"
#define OPV_JINGLE_RTP_AUDIO_INPUT                      "jingle.rtp.audio.input"
#define OPV_JINGLE_RTP_AUDIO_INPUT_VOLUME               "jingle.rtp.audio.input.volume"
#define OPV_JINGLE_RTP_AUDIO_OUTPUT                     "jingle.rtp.audio.output"
#define OPV_JINGLE_RTP_AUDIO_OUTPUT_VOLUME              "jingle.rtp.audio.output.volume"
#define OPV_JINGLE_RTP_AUDIO_BITRATE                    "jingle.rtp.audio.bitrate"
#define OPV_JINGLE_RTP_TIMEOUT							"jingle.rtp.timeout"
#define OPV_JINGLE_RTP_RTCP								"jingle.rtp.rtcp"
#define OPV_JINGLE_RTP_RINGING							"jingle.rtp.ringing"
#define OPV_JINGLE_RTP_CODECS_USED                      "jingle.rtp.codecs.used"

// Client Icons
#define OPV_ROSTER_CLIENTICON_SHOW                      "roster.clienticon.show"
#define OPV_MUC_CLIENTICON_SHOW                         "muc.clienticon.show"
#define OPV_MESSAGES_CLIENTICON_DISPLAY                 "messages.clienticon.display"

#define OPV_SCHEDULER_ACTIVE                           "scheduler.active"
#define OPV_SCHEDULER_ITEMS                            "scheduler.items"
// *** >>> eyeCU >>> ***

#endif // DEF_OPTIONVALUES_H
