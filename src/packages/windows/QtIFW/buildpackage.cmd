echo off
set packagename=eyecu-win
set devpackagename=%packagename%-dev
set version=1.3.0
set packagefilename=%packagename%-%version%
set onlinepackagefilename=%packagename%-online
set devpackagefilename=%devpackagename%-%version%
set packages=packages

echo Creating base package
rem call substver.cmd %packagename% %version%

rem goto copydict

if exist "%qtdir%" goto exists
echo No Qt installation found!
goto end

:exists

if exist "%MSVCREDIST%" goto redistexists
echo Cannot find MSVC Redistributable
goto end

:redistexists
mkdir packages\com.microsoft.vcredist\data\
copy "%MSVCREDIST%" packages\com.microsoft.vcredist\data\vcredist_x86.exe /Y

if not exist "%OPENSSLDIR%\libeay32.dll"  goto noopenssl
if not exist "%OPENSSLDIR%\ssleay32.dll"  goto noopenssl
goto opensslexists
:noopenssl
echo Cannot find OpenSSL libraries
goto end

:opensslexists
mkdir packages\org.openssl.shared\data
for %%f in (libeay32 ssleay32) do copy %OPENSSLDIR%\%%f.dll packages\org.openssl.shared\data\ /Y


for %%f in (phonon4.dll QtCore4.dll QtGui4.dll QtNetwork4.dll QtSvg4.dll QtXml4.dll) do xcopy %qtdir%\bin\%%f packages\org.digia.qt4\data\ /Y

xcopy %qtdir%\plugins\imageformats\q*4.dll packages\org.digia.qt4\data\imageformats\ /Y
del packages\org.digia.qt4\data\imageformats\q*d4.dll /Q
xcopy %qtdir%\plugins\iconengines\q*4.dll packages\org.digia.qt4\data\iconengines\ /Y
del packages\org.digia.qt4\data\iconengines\q*d4.dll /Q

for %%f in (de es pl ja ru uk) do xcopy %qtdir%\translations\qt_%%f.qm packages\org.digia.qt4.%%f\data\translations\ /Y

for %%f in (ar cs da eu fa fr gl he hu ko lt pt sk sl sv zh_CN zh_TW) do xcopy %qtdir%\translations\qt_%%f.qm packages\org.digia.qt4.locales\data\translations\ /Y

if exist %qtdir%\bin\QtMultimedia4.dll (
xcopy %qtdir%\bin\QtMultimedia4.dll packages\org.digia.qt4.multimedia\data\ /Y
) else if exist %qtdir%\bin\QtMultimediaKit1.dll (
xcopy %qtdir%\bin\QtMultimediaKit1.dll packages\org.digia.qt4.multimedia\data\ /Y
) else goto no_multimedia
goto multimedia
:no_multimedia
echo Error! No multimedia framework found!
goto end

:multimedia
xcopy %qtdir%\bin\QtSql4.dll packages\org.digia.qt4.sql\data\ /Y
xcopy %qtdir%\plugins\sqldrivers\qsqlite4.dll packages\org.digia.qt4.sql\data\sqldrivers\* /Y

xcopy %qtdir%\bin\QtScript4.dll packages\org.digia.qt4.script\data\ /Y
xcopy %qtdir%\bin\QtWebKit4.dll packages\org.digia.qt4.webkit\data\ /Y
xcopy %qtdir%\bin\QtSerialPort.dll packages\org.digia.qt4.serialport\data\ /Y

if not exist %qtdir%\mkspecs\features\util.prf goto noqtpurple
if not exist %qtdir%\mkspecs\features\ffmpeg.prf goto noqtpurple
if not exist %qtdir%\mkspecs\features\geo.prf goto noqtpurple
goto qtpurple

:noqtpurple
echo No QtPurple library found!
goto end

:qtpurple
if not exist %ffmpegdir% goto noffmpeg
goto ffmpeg
:noffmpeg
echo No FFMpeg library found! Make sure FFMPEGDIR environment variable set correctly!
goto end

:ffmpeg
for %%f in (avcodec avfilter avformat avutil postproc swresample swscale) do xcopy %ffmpegdir%\bin\%%f-*.dll packages\org.ffmpeg.library\data\ /Y

xcopy %qtdir%\bin\QtFFMpeg.dll packages\ru.purplesoft.qtpurple.ffmpeg\data\ /Y
xcopy %qtdir%\bin\QtUtil.dll packages\ru.purplesoft.qtpurple.util\data\ /Y
xcopy %qtdir%\bin\QtGeo.dll packages\ru.purplesoft.qtpurple.geo\data\ /Y
for %%f in (de es nl pl ja ru uk) do xcopy %qtdir%\translations\qtgeo_%%f.qm packages\ru.purplesoft.qtpurple.geo.%%f\data\translations\ /Y

copy c:\eyecu\COPYING packages\ru.rwsoftware.eyecu\meta\LICENSE.TXT /Y
set pluginlist=accountmanager chatmessagehandler connectionmanager defaultconnection mainwindow messageprocessor messagestyles messagewidgets normalmessagehandler notifications optionsmanager presence roster rosterchanger rostersmodel rostersview saslauth simplemessagestyle stanzaprocessor starttls statuschanger statusicons traymanager xmppstreams
call copyplugins ru.rwsoftware.eyecu
set resources=statusicons simplemessagestyles sounds
call copyresources ru.rwsoftware.eyecu
set files=eyecuicon.def.xml eyecu.svg mainwindow.def.xml mainwindowlogo128.png mainwindowlogo16.png mainwindowlogo20.png mainwindowlogo24.png mainwindowlogo32.png mainwindowlogo40.png mainwindowlogo48.png mainwindowlogo64.png mainwindowlogo96.png mainwindowmenu.png mainwindowquit.png mainwindowshowroster.png pluginmanager.def.xml pluginmanagerabout.png pluginmanageraboutqt.png pluginmanagersetup.png account.png accountchange.png accountlist.png accountmanager.def.xml accountmove.png chatmessagehandler.def.xml chatmessagehandlerclearchat.png chatmessagehandlermessage.png connection.def.xml connectionencrypted.png messagewidgets.def.xml messagewidgetsquote.png messagewidgetsselect.png messagewidgetssend.png messagewidgetstabmenu.png normalmessagehandler.def.xml normalmessagehandlerforward.png normalmessagehandlermessage.png normalmessagehandlernext.png normalmessagehandlerreply.png normalmessagehandlersend.png notifications.def.xml notifications.png notificationsactivateall.png notificationspopupwindow.png notificationsremoveall.png notificationsshowminimized.png notificationssoundoff.png notificationssoundon.png notificationssoundplay.png options.def.xml optionsappearance.png optionsdialog.png optionseditprofiles.png optionsprofile.png optionsprofiles.png rchanger.def.xml rchangeraddcontact.png rchangercopygroup.png rchangercreategroup.png rchangergroup.png rchangermovegroup.png rchangerremovecontact.png rchangerremovecontacts.png rchangerremovefromgroup.png rchangerremovegroup.png rchangerrename.png rchangerrootgroup.png rchangersubscribe.png rchangersubscription.png rchangerthisgroup.png rchangerunsubscribe.png rosterview.def.xml rosterviewclipboard.png rosterviewcontacts.png rosterviewhideoffline.png rosterviewoptions.png rosterviewshowoffline.png schanger.def.xml schangerconnecting.png schangereditstatuses.png schangermodifystatus.png
call copyresources2 ru.rwsoftware.eyecu menuicons\shared

copy c:\eyecu\eyecu.exe packages\ru.rwsoftware.eyecu\data /Y
copy c:\eyecu\eyecuutils.dll packages\ru.rwsoftware.eyecu\data /Y

:copydict
call copydict en.us en_US
call copydict en.gb en_GB
call copydict en.ca en_CA
call copydict en.au en_AU
call copydict en.nz en_NZ
call copydict en.za en_ZA
call copydict de.de_frami de_DE_frami
call copydict de.at_frami de_AT_frami
call copydict de.de_igerman98 de_DE_igerman98
call copydict ru.ru ru_RU
call copydict ru.rk russian-rk-ieyo
call copydict ru.aot russian-aot-ieyo
call copydict pl pl_PL
call copydict es es_ES
call copydict nl nl_NL
call copydict uk uk_UA
pause

call copyplugins ru.rwsoftware.eyecu.spellchecker spellchecker
call copyplugins ru.rwsoftware.eyecu.statistics statistics

call copyplugins ru.rwsoftware.eyecu.privacylists privacylists
set files=privacylists.def.xml privacylists.png	privacylistsadvanced.png privacylistsblock.png 	privacylistsdisable.png privacylistsenable.png privacylistsignore.png privacylistsinvisible.png privacylistslist.png privacylistsvisible.png
call copyresources2 ru.rwsoftware.eyecu.privacylists menuicons\shared

call copyplugins ru.rwsoftware.eyecu.compress compress

call copyplugins ru.rwsoftware.eyecu.vcard vcard
set files=vcard.def.xml vcard.png
call copyresources2 ru.rwsoftware.eyecu.vcard menuicons\shared

call copyplugins ru.rwsoftware.eyecu.birthdayreminder birthdayreminder
set files=birthdayreminder.def.xml birthdayremindernotify.png
call copyresources2 ru.rwsoftware.eyecu.birthdayreminder menuicons\shared

call copyplugins ru.rwsoftware.eyecu.avatars avatars
set files=avatar.def.xml avatarchange.png avatarcustom.png avatarempty.png avatarremove.png avatarset.png
call copyresources2 ru.rwsoftware.eyecu.avatars menuicons\shared

call copyplugins ru.rwsoftware.eyecu.adiummessagestyle adiummessagestyle
call copyresources ru.rwsoftware.eyecu.adiummessagestyle adiummessagestyles\shared
call copyresources ru.rwsoftware.eyecu.adiummessagestyle.renkoo adiummessagestyles\renkoo
call copyresources ru.rwsoftware.eyecu.adiummessagestyle.ymous adiummessagestyles\yMous

call copyplugins ru.rwsoftware.eyecu.gateways gateways
set files=gateways.def.xml gateways.png gatewaysaddcontact.png gatewayschange.png gatewayskeepconnection.png gatewayslogin.png gatewayslogout.png gatewaysremove.png gatewaysreset.png gatewaysresolve.png
call copyresources2 ru.rwsoftware.eyecu.gateways menuicons\shared

call copyplugins ru.rwsoftware.eyecu.shortcutmanager shortcutmanager
set files=shortcuts.def.xml shortcuts.png
call copyresources2 ru.rwsoftware.eyecu.shortcutmanager menuicons\shared

call copyplugins ru.rwsoftware.eyecu.servicediscovery servicediscovery
set files=sdiscovery.def.xml sdiscoverydiscoinfo.png sdiscoverydiscover.png
call copyresources2 ru.rwsoftware.eyecu.servicediscovery menuicons\shared

call copyresources ru.rwsoftware.eyecu.servicediscovery serviceicons\shared

call copyplugins ru.rwsoftware.eyecu.messagecarbons messagecarbons
set files=messagecarbons.def.xml messagecarbons.png
call copyresources2 ru.rwsoftware.eyecu.messagecarbons menuicons\shared

call copyplugins ru.rwsoftware.eyecu.console console
set files=console.def.xml console.png
call copyresources2 ru.rwsoftware.eyecu.console menuicons\shared

call copyplugins ru.rwsoftware.eyecu.messagearchiver messagearchiver
set files=history.def.xml history.png historydate.png
call copyresources2 ru.rwsoftware.eyecu.messagearchiver menuicons\shared

call copyplugins ru.rwsoftware.eyecu.messagearchiver.file filemessagearchive
call copyplugins ru.rwsoftware.eyecu.messagearchiver.server servermessagearchive

call copyplugins ru.rwsoftware.eyecu.rostersearch rostersearch
set files=rostersearch.def.xml rostersearch.png
call copyresources2 ru.rwsoftware.eyecu.rostersearch menuicons\shared

call copyplugins ru.rwsoftware.eyecu.jabbersearch jabbersearch
set files=jsearch.def.xml jsearch.png
call copyresources2 ru.rwsoftware.eyecu.jabbersearch menuicons\shared

call copyplugins ru.rwsoftware.eyecu.rosteritemexchange rosteritemexchange
set files=rosterexchange.def.xml rosterexchangerequest.png
call copyresources2 ru.rwsoftware.eyecu.rosteritemexchange menuicons\shared

call copyplugins ru.rwsoftware.eyecu.multiuserchat multiuserchat
set files=muc.def.xml mucchangenick.png mucchangetopic.png mucclearchat.png mucconference.png mucconference1.png mucconfigureroom.png mucdatamessage.png mucdestroyroom.png muceditadminslist.png muceditbanlist.png muceditmemberslist.png muceditownerslist.png mucenterroom.png mucexitroom.png mucinvite.png mucjoin.png mucmessage.png mucprivatemessage.png mucrequestvoice.png mucusermenu.png
call copyresources2 ru.rwsoftware.eyecu.multiuserchat menuicons\shared

call copyplugins ru.rwsoftware.eyecu.chatstates chatstates
set files=chatstates.def.xml chatstatesactive.png chatstatescomposing.png chatstatesgone.png chatstatesinactive.png chatstatespaused.png chatstatesunknown.png
call copyresources2 ru.rwsoftware.eyecu.chatstates menuicons\shared

call copyplugins ru.rwsoftware.eyecu.privatestorage privatestorage

call copyplugins ru.rwsoftware.eyecu.metacontacts metacontacts
set files=metacontacts.def.xml metacontactscombine.png metacontactsdestroy.png metacontactsdetach.png
call copyresources2 ru.rwsoftware.eyecu.metacontacts menuicons\shared

call copyplugins ru.rwsoftware.eyecu.bookmarks bookmarks
set files=bookmarks.def.xml bookmarks.png bookmarksadd.png bookmarksedit.png bookmarksremove.png bookmarksroom.png bookmarksurl.png
call copyresources2 ru.rwsoftware.eyecu.bookmarks menuicons\shared

call copyplugins ru.rwsoftware.eyecu.annotations annotations
set files=annotations.def.xml annotations.png
call copyresources2 ru.rwsoftware.eyecu.annotations menuicons\shared

call copyplugins ru.rwsoftware.eyecu.recentcontacts recentcontacts
set files=recentcontacts.def.xml recentcontacts.png recentcontactsfavorite.png recentcontactsinsertfavorite.png recentcontactsremovefavorite.png recentcontactsremoverecent.png
call copyresources2 ru.rwsoftware.eyecu.recentcontacts menuicons\shared

call copyplugins ru.rwsoftware.eyecu.clientinfo clientinfo
set files=clientinfo.def.xml clientinfo.png clientinfoactivity.png clientinfotime.png
call copyresources2 ru.rwsoftware.eyecu.clientinfo menuicons\shared

call copyplugins ru.rwsoftware.eyecu.urlprocessor urlprocessor
call copyplugins ru.rwsoftware.eyecu.bob bitsofbinary
call copyplugins ru.rwsoftware.eyecu.boburlhandler boburlhandler
call copyplugins ru.rwsoftware.eyecu.xmppuriqueries xmppuriqueries
call copyplugins ru.rwsoftware.eyecu.dataforms dataforms

call copyplugins ru.rwsoftware.eyecu.sessionnegotiation sessionnegotiation
set files=snegotiation.def.xml snegotiation.png snegotiationinit.png snegotiationterminate.png
call copyresources2 ru.rwsoftware.eyecu.sessionnegotiation menuicons\shared

call copyplugins ru.rwsoftware.eyecu.captchaforms captchaforms
set files=captchaforms.def.xml captchaforms.png
call copyresources2 ru.rwsoftware.eyecu.captchaforms menuicons\shared

call copyplugins ru.rwsoftware.eyecu.registration registration
set files=register.def.xml register.png registerchange.png registerremove.png
call copyresources2 ru.rwsoftware.eyecu.registration menuicons\shared

call copyplugins ru.rwsoftware.eyecu.commands commands
set files=commands.def.xml commands.png
call copyresources2 ru.rwsoftware.eyecu.commands menuicons\shared


call copyplugins ru.rwsoftware.eyecu.remotecontrol remotecontrol

set pluginlist=filetransfer filestreamsmanager datastreamsmanager
call copyplugins ru.rwsoftware.eyecu.filetransfer
set files=datastreamsmanager.def.xml datastreamsmanager.png filestreamsmanager.def.xml filestreamsmanager.png filetransfer.def.xml filetransferreceive.png filetransfersend.png
call copyresources2 ru.rwsoftware.eyecu.filetransfer menuicons\shared

call copyplugins ru.rwsoftware.eyecu.filetransfer.socks5 socksstreams
call copyplugins ru.rwsoftware.eyecu.filetransfer.ibb inbandstreams
call copyplugins ru.rwsoftware.eyecu.autostatus autostatus
call copyplugins ru.rwsoftware.eyecu.iqauth iqauth

call copyplugins ru.rwsoftware.eyecu.pepmanager pepmanager
set files=pepmanager.def.xml pepmanager.png
call copyresources2 ru.rwsoftware.eyecu.pepmanager menuicons\shared

call copyplugins ru.rwsoftware.eyecu.pepmanager.mood mood
call copyresources ru.rwsoftware.eyecu.pepmanager.mood moodicons\shared

call copyplugins ru.rwsoftware.eyecu.pepmanager.activity activity
call copyresources ru.rwsoftware.eyecu.pepmanager.activity activityicons\shared

call copyplugins ru.rwsoftware.eyecu.pepmanager.tune tune
set files=tune.def.xml tune.png
call copyresources2 ru.rwsoftware.eyecu.pepmanager.tune menuicons\shared

call copyplugins ru.rwsoftware.eyecu.pepmanager.tune.inforequesters.lastfm tuneinforequesterlastfm
set files=lastfm.def.xml lastfm.png
call copyresources2 ru.rwsoftware.eyecu.pepmanager.tune.inforequesters.lastfm menuicons\shared

call copyplugins ru.rwsoftware.eyecu.pepmanager.tune.listeners.winamp tunelistenerwinamp
call copyplugins ru.rwsoftware.eyecu.pepmanager.tune.listeners.aimp tunelisteneraimp
call copyplugins ru.rwsoftware.eyecu.pepmanager.tune.listeners.file tunelistenerfile

call copyplugins ru.rwsoftware.eyecu.pepmanager.geoloc geoloc
set files=geoloc.def.xml geoloc.png geolocoff.png
call copyresources2 ru.rwsoftware.eyecu.pepmanager.geoloc menuicons\shared

call copyplugins ru.rwsoftware.eyecu.pepmanager.geoloc.positioning positioning
set files=positioning.def.xml positioning.png manual.png location.png serialport.png geoip.gif freegeoip.png
call copyresources2 ru.rwsoftware.eyecu.pepmanager.geoloc.positioning menuicons\shared
call copyplugins ru.rwsoftware.eyecu.pepmanager.geoloc.positioning.manual positioningmethodmanual
call copyplugins ru.rwsoftware.eyecu.pepmanager.geoloc.positioning.ip positioningmethodip
call copyplugins ru.rwsoftware.eyecu.pepmanager.geoloc.positioning.ip.freegeoip positioningmethodipproviderfreegeoip
call copyplugins ru.rwsoftware.eyecu.pepmanager.geoloc.positioning.serialport positioningmethodserialport

call copyresources ru.rwsoftware.eyecu.statusicons.aim statusicons\aim
call copyresources ru.rwsoftware.eyecu.statusicons.bot statusicons\bot
call copyresources ru.rwsoftware.eyecu.statusicons.car statusicons\car
call copyresources ru.rwsoftware.eyecu.statusicons.conference statusicons\conference
call copyresources ru.rwsoftware.eyecu.statusicons.facebook statusicons\facebook
call copyresources ru.rwsoftware.eyecu.statusicons.gadu statusicons\gadu
call copyresources ru.rwsoftware.eyecu.statusicons.gtalk statusicons\gtalk
call copyresources ru.rwsoftware.eyecu.statusicons.icq statusicons\icq
call copyresources ru.rwsoftware.eyecu.statusicons.livejournal statusicons\livejournal
call copyresources ru.rwsoftware.eyecu.statusicons.mrim statusicons\mrim
call copyresources ru.rwsoftware.eyecu.statusicons.msn statusicons\msn
call copyresources ru.rwsoftware.eyecu.statusicons.odnoklassniki statusicons\odnoklassniki
call copyresources ru.rwsoftware.eyecu.statusicons.rss statusicons\rss
call copyresources ru.rwsoftware.eyecu.statusicons.skype statusicons\skype
call copyresources ru.rwsoftware.eyecu.statusicons.sms statusicons\sms
call copyresources ru.rwsoftware.eyecu.statusicons.smtp statusicons\smtp
call copyresources ru.rwsoftware.eyecu.statusicons.twitter statusicons\twitter
call copyresources ru.rwsoftware.eyecu.statusicons.vkontakte statusicons\vkontakte
call copyresources ru.rwsoftware.eyecu.statusicons.weather statusicons\weather
call copyresources ru.rwsoftware.eyecu.statusicons.yahoo statusicons\yahoo
call copyresources ru.rwsoftware.eyecu.statusicons.yaonline statusicons\yaonline
call copyplugins ru.rwsoftware.eyecu.emoticons emoticons
call copyresources ru.rwsoftware.eyecu.emoticons.default emoticons\default
call copyresources ru.rwsoftware.eyecu.emoticons.blobs_purple emoticons\blobs_purple
call copyplugins ru.rwsoftware.eyecu.emoji emoji
call copyresources ru.rwsoftware.eyecu.emoji emoji\shared
call copyresources ru.rwsoftware.eyecu.emoji.vkontakte.big.basic emoji\vk_big
call copyresources ru.rwsoftware.eyecu.emoji.vkontakte.small.basic emoji\vk_small
call copyresources ru.rwsoftware.eyecu.emoji.vkontakte.big.extra emoji\vk_extra_big
call copyresources ru.rwsoftware.eyecu.emoji.vkontakte.small.extra emoji\vk_extra_small
call copyresources ru.rwsoftware.eyecu.emoji.vkontakte.big.family emoji\vk_family_big
call copyresources ru.rwsoftware.eyecu.emoji.vkontakte.small.family emoji\vk_family_small

call copyplugins ru.rwsoftware.eyecu.xhtmlim xhtmlim
call copyresources ru.rwsoftware.eyecu.xhtmlim xhtml\shared
set files=xhtml.def.xml xhtml.png
call copyresources2 ru.rwsoftware.eyecu.xhtmlim menuicons\shared

call copyplugins ru.rwsoftware.eyecu.oob oob

call copyplugins ru.rwsoftware.eyecu.nickname nickname
set files=nickname.def.xml nickname.png
call copyresources2 ru.rwsoftware.eyecu.nickname menuicons\shared

call copyplugins ru.rwsoftware.eyecu.abbreviations abbreviations
call copyresources ru.rwsoftware.eyecu.abbreviations abbreviations\shared

call copyplugins ru.rwsoftware.eyecu.attention attention
set files=attention.def.xml attention.png bell.gif exclamation.gif
call copyresources2 ru.rwsoftware.eyecu.attention menuicons\shared

call copyplugins ru.rwsoftware.eyecu.receipts receipts
set files=receipts.def.xml receipts.png
call copyresources2 ru.rwsoftware.eyecu.receipts menuicons\shared

call copyplugins ru.rwsoftware.eyecu.mmplayer mmplayer
set files=mmplayer.def.xml mmplayer.png mmplayereject.png
call copyresources2 ru.rwsoftware.eyecu.mmplayer menuicons\shared

call copyplugins ru.rwsoftware.eyecu.clienticons clienticons
call copyresources ru.rwsoftware.eyecu.clienticons clienticons\shared

call copyplugins ru.rwsoftware.eyecu.contactproximitynotification contactproximitynotification
set files=proximity.def.xml proximity.png
call copyresources2 ru.rwsoftware.eyecu.contactproximitynotification menuicons\shared

call copyplugins ru.rwsoftware.eyecu.poi poi
call copyresources ru.rwsoftware.eyecu.poi typepoint\shared
set files=poi.def.xml poi.png poiadd.png poinone.png poinotype.png poitoolbar.png poiview.png description.png flag24.png globus.png connect.png connectend.png connectlong.png bgrmap.png bgrsat.png folder.png folderopen.png
call copyresources2 ru.rwsoftware.eyecu.poi menuicons\shared

call copyplugins ru.rwsoftware.eyecu.map map
set files=map.def.xml map.png viewcenter.png
call copyresources2 ru.rwsoftware.eyecu.map menuicons\shared

call copyresources ru.rwsoftware.eyecu.map mapicons\shared
call copyplugins ru.rwsoftware.eyecu.map.locationselector maplocationselector

call copyplugins ru.rwsoftware.eyecu.map.magnifier mapmagnifier
set files=mapmagnifier.def.xml mapmagnifier.png
call copyresources2 ru.rwsoftware.eyecu.map.magnifier menuicons\shared

call copyplugins ru.rwsoftware.eyecu.map.contacts mapcontacts

call copyplugins ru.rwsoftware.eyecu.map.streetview streetview
set files=streetview.def.xml streetview.png
call copyresources2 ru.rwsoftware.eyecu.map.streetview menuicons\shared
call copyplugins ru.rwsoftware.eyecu.map.streetview.google streetviewprovidergoogle

call copyplugins ru.rwsoftware.eyecu.map.placeview placeview
set files=placeview.def.xml placeview.png
call copyresources2 ru.rwsoftware.eyecu.map.placeview menuicons\shared
call copyplugins ru.rwsoftware.eyecu.map.placeview.google placeviewprovidergoogle

call copyplugins ru.rwsoftware.eyecu.map.message mapmessage
set files=close.def.xml closeactive.png closeinactive.png
call copyresources2 ru.rwsoftware.eyecu.map.message menuicons\shared

call copyplugins ru.rwsoftware.eyecu.map.sources.osm mapsourceosm
call copyplugins ru.rwsoftware.eyecu.map.sources.wiki mapsourcewiki
call copyplugins ru.rwsoftware.eyecu.map.sources.google  mapsourcegoogle
call copyplugins ru.rwsoftware.eyecu.map.sources.yandex mapsourceyandex
call copyplugins ru.rwsoftware.eyecu.map.sources.kosmosnimki mapsourcekosmosnimki
call copyplugins ru.rwsoftware.eyecu.map.sources.2gis mapsource2gis
call copyplugins ru.rwsoftware.eyecu.map.sources.yahoo mapsourceyahoo
call copyplugins ru.rwsoftware.eyecu.map.sources.ovi mapsourceovi
call copyplugins ru.rwsoftware.eyecu.map.sources.bing mapsourcebing
call copyplugins ru.rwsoftware.eyecu.map.sources.navitel mapsourcenavitel
call copyplugins ru.rwsoftware.eyecu.map.sources.progorod mapsourceprogorod
call copyplugins ru.rwsoftware.eyecu.map.sources.esri mapsourceesri
call copyplugins ru.rwsoftware.eyecu.map.sources.megafon mapsourcemegafon
call copyplugins ru.rwsoftware.eyecu.map.sources.navteq mapsourcenavteq
call copyplugins ru.rwsoftware.eyecu.map.sources.rosreestr mapsourcerosreestr
call copyplugins ru.rwsoftware.eyecu.map.sources.rumap mapsourcerumap
call copyplugins ru.rwsoftware.eyecu.map.sources.vitel mapsourcevitel

call copyplugins ru.rwsoftware.eyecu.mapsearch mapsearch
set files=mapsearch.def.xml mapsearch.png
call copyresources2 ru.rwsoftware.eyecu.mapsearch menuicons\shared

call copyplugins ru.rwsoftware.eyecu.mapsearch.2gis mapsearchprovider2gis
call copyplugins ru.rwsoftware.eyecu.mapsearch.google mapsearchprovidergoogle
call copyplugins ru.rwsoftware.eyecu.mapsearch.here mapsearchproviderhere
call copyplugins ru.rwsoftware.eyecu.mapsearch.osm mapsearchproviderosm
call copyplugins ru.rwsoftware.eyecu.mapsearch.yandex mapsearchprovideryandex
call copyplugins ru.rwsoftware.eyecu.mapsearch.navitel mapsearchprovidernavitel
call copyresources ru.rwsoftware.eyecu.mapsearch.navitel navitel\shared

set resources=wizard.def.xml wizard.png banner.png
call copyresources1 ru.rwsoftware.eyecu.wizards wizards\shared

call copyplugins ru.rwsoftware.eyecu.wizards.transport wizardtransport
set resources=transports.xml transport.png transportend.png yes.png no.png
call copyresources1 ru.rwsoftware.eyecu.wizards.transport wizards\shared

call copyplugins ru.rwsoftware.eyecu.wizards.account wizardaccount
set resources=servers.xml software.def.xml networks.def.xml ejabberd.png jabberd.png openfire.png prosody.png tigase.png gtalk.png livejournal.png odnoklassniki.png qip.png xmpp.png yaonline.png account.png accountend.png
call copyresources1 ru.rwsoftware.eyecu.wizards.account wizards\shared
xcopy c:\eyecu\resources\wizards\shared\*.html %packages%\ru.rwsoftware.eyecu.wizards.account\data\resources\wizards\shared\* /S /Y

rem *** Resources ***
rem   Country
call copyresources ru.rwsoftware.eyecu.resources.country country\shared

rem   Menu Icons
set files=mapsources.def.xml 2gis.png bing.png esri.png geocon.png google.png here.png kosmosnimki.png megafon.png navitel.png navteq.png osm.png progorod.png rosreestr.png vitel.png wiki.png yahoo.png yandex.png
call copyresources2 ru.rwsoftware.eyecu.resources.menuicons.mapsources menuicons\shared

set files=edit.def.xml edit.png editadd.png editcopy.png editdelete.png
call copyresources2 ru.rwsoftware.eyecu.resources.menuicons.edit menuicons\shared

set files=link.def.xml link.png linkadd.png
call copyresources2 ru.rwsoftware.eyecu.resources.menuicons.link menuicons\shared

rem *** Documentaion ***
md packages\ru.rwsoftware.eyecu.docs\data
for %%f in (AUTHORS CHANGELOG README TRANSLATORS) do copy c:\eyecu\%%f packages\ru.rwsoftware.eyecu.docs\data\%%f.TXT /Y

:build
del %packagefilename%.exe
copy installscript.offline packages\ru.rwsoftware.eyecu\meta\*.js
binarycreator.exe -c config\config.xml -p %packages% %packagefilename%.exe

:online
del %onlinepackagefilename%.exe
copy installscript.online packages\ru.rwsoftware.eyecu\meta\*.js
binarycreator.exe -n -c config\config-repo.xml -p %packages% %onlinepackagefilename%.exe

:repo
repogen.exe -p %packages% repository

:end