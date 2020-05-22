echo off

set qtdir=h:\qt\5.5\msvc2013
set MSVCREDIST=h:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\1033\
set OPENSSLDIR=h:\openssl
set FFMPEGDIR=h:\ffmpeg

set platform=x86
set qt=5
set packagename=eyecu-win
set devpackagename=%packagename%-dev
set version=2.0.0.20190601
set packagefilename=%packagename%-%platform%-%version%
set devpackagefilename=%devpackagename%-%version%
set packages=packages

echo Creating base package

rem goto build

if exist "c:\eyecu\hunspell\*" goto hunspell
echo No Hunspell found in eyeCU installation directory!
goto end
:hunspell

if exist "%qtdir%" goto exists
echo No Qt installation found!
goto end
:exists

if exist "%MSVCREDIST%\vcredist_%platform%.exe" goto redistexists
echo Cannot find MSVC Redistributable
goto end
:redistexists

mkdir %packages%\com.microsoft.vcredist\data\
mkdir %packages%\com.microsoft.vcredist\meta\

if not exist "%OPENSSLDIR%\%platform%\libeay32.dll"  goto noopenssl
if not exist "%OPENSSLDIR%\%platform%\ssleay32.dll"  goto noopenssl
goto opensslexists
:noopenssl
echo Cannot find OpenSSL libraries
goto end

:opensslexists
mkdir %packages%\org.openssl.shared\data
for %%f in (libeay32 ssleay32) do copy %OPENSSLDIR%\%platform%\%%f.dll %packages%\org.openssl.shared\data\ /Y

if %platform%==x64 goto x64_platform
copy cfg\32\* config\
goto qt_selection
:x64_platform
copy cfg\64\* config\

:qt_selection

if %qt%==5 goto qt5_files
if not exist %qtdir%\mkspecs\features\qputil.prf goto noqtpurple
if not exist %qtdir%\mkspecs\features\qpdns.prf goto noqtpurple
if not exist %qtdir%\mkspecs\features\qpgeo.prf goto noqtpurple
if not exist %qtdir%\mkspecs\features\qpffmpeg.prf goto noqtpurple
if not exist %qtdir%\mkspecs\features\qpice.prf goto noqtpurple

for /d %%p in (packages\org.digia.qt5.*) do rmdir %%p /S /Q

xcopy qt\4\* packages\ /E /Y
xcopy plugins\4\* packages\ /E /Y
xcopy purple\4\* packages\ /E /Y

if exist %qtdir%\bin\QtMultimedia4.dll (
xcopy %qtdir%\bin\QtMultimedia4.dll %packages%\org.digia.qt4.multimedia\data\ /Y
) else if exist %qtdir%\bin\QtMultimediaKit1.dll (
xcopy %qtdir%\bin\QtMultimediaKit1.dll %packages%\org.digia.qt4.multimedia\data\ /Y
) else goto no_multimedia

xcopy %qtdir%\plugins\sqldrivers\qsqlite4.dll %packages%\org.digia.qt4.sql\data\sqldrivers\* /Y
xcopy %qtdir%\bin\QtSql4.dll %packages%\org.digia.qt4.sql\data\ /Y
xcopy %qtdir%\bin\QtScript4.dll %packages%\org.digia.qt4.script\data\ /Y
xcopy %qtdir%\bin\QtWebKit4.dll %packages%\org.digia.qt4.webkit\data\ /Y
xcopy %qtdir%\bin\QtSerialPort.dll %packages%\org.digia.qt4.serialport\data\ /Y
xcopy %qtdir%\bin\QpFFMpeg2.dll %packages%\ru.purplesoft.qtpurple.ffmpeg\data\ /Y
xcopy %qtdir%\bin\QpGeo2.dll %packages%\ru.purplesoft.qtpurple.geo\data\ /Y
xcopy %qtdir%\bin\QpUtil2.dll %packages%\ru.purplesoft.qtpurple.util\data\ /Y
xcopy %qtdir%\bin\QpIce1.dll %packages%\ru.purplesoft.qtpurple.ice\data\ /Y
xcopy %qtdir%\bin\QpDns1.dll %packages%\ru.purplesoft.qtpurple.dns\data\ /Y

set qt_files=phonon4.dll QtCore4.dll QtGui4.dll QtNetwork4.dll QtSvg4.dll QtXml4.dll
set targetqt=qt4
goto copy_qt_files
:qt5_files
if not exist %qtdir%\mkspecs\modules\qt_lib_qputil.pri goto noqtpurple
if not exist %qtdir%\mkspecs\modules\qt_lib_qpdns.pri goto noqtpurple
if not exist %qtdir%\mkspecs\modules\qt_lib_qpgeo.pri goto noqtpurple
if not exist %qtdir%\mkspecs\modules\qt_lib_qpffmpeg.pri goto noqtpurple
if not exist %qtdir%\mkspecs\modules\qt_lib_qpice.pri goto noqtpurple
for /d %%p in (packages\org.digia.qt4.*) do rmdir %%p /S /Q
xcopy qt\5\* packages\ /E /Y
xcopy plugins\5\* packages\ /E /Y
xcopy purple\5\* packages\ /E /Y
if not exist %qtdir%\bin\Qt5Multimedia.dll goto no_multimedia

xcopy %qtdir%\bin\icu*.dll %packages%\org.icuproject.icu\data\ /Y

xcopy %qtdir%\plugins\platforms\qwindows.dll %packages%\org.digia.qt5\data\platforms\ /Y
xcopy %qtdir%\plugins\audio\qtaudio_windows.dll %packages%\org.digia.qt5\data\audio\ /Y
xcopy %qtdir%\plugins\sqldrivers\qsqlite.dll %packages%\org.digia.qt5.sql\data\sqldrivers\* /Y

xcopy %qtdir%\bin\Qt5Sql.dll %packages%\org.digia.qt5.sql\data\ /Y

xcopy %qtdir%\bin\Qt5WebKit.dll %packages%\org.digia.qt5.webkit\data\ /Y
xcopy %qtdir%\bin\Qt5WebKitWidgets.dll %packages%\org.digia.qt5.webkit\data\ /Y
xcopy %qtdir%\bin\Qt5WebChannel.dll %packages%\org.digia.qt5.webkit\data\ /Y

xcopy %qtdir%\bin\Qt5Multimedia.dll %packages%\org.digia.qt5.multimedia\data\ /Y
xcopy %qtdir%\bin\Qt5MultimediaWidgets.dll %packages%\org.digia.qt5.multimedia\data\ /Y

xcopy %qtdir%\bin\Qt5Positioning.dll %packages%\org.digia.qt5.mobility\data\ /Y
xcopy %qtdir%\bin\Qt5Sensors.dll %packages%\org.digia.qt5.mobility\data\ /Y

xcopy %qtdir%\bin\Qt5OpenGL.dll %packages%\org.digia.qt5.opengl\data\ /Y

xcopy %qtdir%\bin\Qt5Quick.dll %packages%\org.digia.qt5.quick\data\ /Y
xcopy %qtdir%\bin\Qt5Qml.dll %packages%\org.digia.qt5.quick\data\ /Y

xcopy %qtdir%\bin\Qt5PrintSupport.dll %packages%\org.digia.qt5.printsupport\data\ /Y

xcopy %qtdir%\bin\Qt5SerialPort.dll %packages%\org.digia.qt5.serialport\data\ /Y

xcopy %qtdir%\bin\QpFFMpeg.dll %packages%\ru.purplesoft.qtpurple.ffmpeg\data\ /Y
xcopy %qtdir%\bin\QpGeo.dll %packages%\ru.purplesoft.qtpurple.geo\data\ /Y
xcopy %qtdir%\bin\QpUtil.dll %packages%\ru.purplesoft.qtpurple.util\data\ /Y
xcopy %qtdir%\bin\QpIce.dll %packages%\ru.purplesoft.qtpurple.ice\data\ /Y
xcopy %qtdir%\bin\QpDns.dll %packages%\ru.purplesoft.qtpurple.dns\data\ /Y

set qt_files=Qt5Core.dll Qt5Gui.dll Qt5Widgets.dll Qt5Network.dll Qt5Svg.dll Qt5Xml.dll
set targetqt=qt5
:copy_qt_files
for %%f in (%qt_files%) do xcopy %qtdir%\bin\%%f %packages%\org.digia.%targetqt%\data\ /Y

xcopy %qtdir%\plugins\imageformats\q*.dll %packages%\org.digia.%targetqt%\data\imageformats\ /Y
del %packages%\org.digia.%targetqt%\data\imageformats\q*d.dll /Q
xcopy %qtdir%\plugins\iconengines\q*.dll %packages%\org.digia.%targetqt%\data\iconengines\ /Y
del %packages%\org.digia.%targetqt%\data\iconengines\q*d.dll /Q
for %%f in (ar de es pl ja ru uk) do xcopy %qtdir%\translations\qt_%%f.qm %packages%\org.digia.%targetqt%.%%f\data\translations\ /Y
for %%f in (cs da eu fa fr gl he hu ko lt pt sk sl sv zh_CN zh_TW) do xcopy %qtdir%\translations\qt_%%f.qm %packages%\org.digia.%targetqt%.locales\data\translations\ /Y

xcopy vcredist\%platform%\* packages\com.microsoft.vcredist\meta\ /Y
copy "%MSVCREDIST%\vcredist_%platform%.exe" %packages%\com.microsoft.vcredist\data\ /Y

goto qtpurple
:no_multimedia
echo Error! No multimedia framework found!
goto end

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
for %%f in (avcodec avfilter avformat avutil postproc swresample swscale) do xcopy %ffmpegdir%\bin\%%f-*.dll %packages%\org.ffmpeg.library\data\ /Y

xcopy %qtdir%\bin\QpDns1.dll %packages%\ru.purplesoft.qtpurple.geo\data\ /Y
xcopy %qtdir%\bin\QpUtil2.dll %packages%\ru.purplesoft.qtpurple.util\data\ /Y
xcopy %qtdir%\bin\QpGeo2.dll %packages%\ru.purplesoft.qtpurple.geo\data\ /Y
xcopy %qtdir%\bin\QpFFMpeg2.dll %packages%\ru.purplesoft.qtpurple.ffmpeg\data\ /Y
xcopy %qtdir%\bin\QpIce1.dll %packages%\ru.purplesoft.qtpurple.geo\data\ /Y

for %%f in (de es nl pl ja ru uk) do xcopy %qtdir%\translations\qpgeo_%%f.qm %packages%\ru.purplesoft.qtpurple.geo.%%f\data\translations\ /Y

copy c:\eyecu\COPYING %packages%\ru.rwsoftware.eyecu\meta\LICENSE.TXT /Y
set pluginlist=accountmanager chatmessagehandler connectionmanager defaultconnection mainwindow messageprocessor messagestyles messagewidgets normalmessagehandler notifications optionsmanager presence roster rosterchanger rostersmodel rostersview saslauth simplemessagestyle stanzaprocessor starttls statuschanger statusicons traymanager xmppstreams
call copyplugins ru.rwsoftware.eyecu
set resources=statusicons simplemessagestyles sounds
call copyresources ru.rwsoftware.eyecu
set files=eyecuicon.def.xml eyecu.svg mainwindow.def.xml mainwindowlogo128.png mainwindowlogo16.png mainwindowlogo20.png mainwindowlogo24.png mainwindowlogo32.png mainwindowlogo40.png mainwindowlogo48.png mainwindowlogo64.png mainwindowlogo96.png mainwindowmenu.png mainwindowquit.png mainwindowshowroster.png pluginmanager.def.xml pluginmanagerabout.png pluginmanageraboutqt.png pluginmanagersetup.png account.png accountchange.png accountlist.png accountmanager.def.xml accountmove.png chatmessagehandler.def.xml chatmessagehandlerclearchat.png chatmessagehandlermessage.png connection.def.xml connectionencrypted.png messagewidgets.def.xml messagewidgetsquote.png messagewidgetsselect.png messagewidgetssend.png messagewidgetstabmenu.png messagewidgetsme.png normalmessagehandler.def.xml normalmessagehandlerforward.png normalmessagehandlermessage.png normalmessagehandlernext.png normalmessagehandlerreply.png normalmessagehandlersend.png notifications.def.xml notifications.png notificationsactivateall.png notificationspopupwindow.png notificationsremoveall.png notificationsshowminimized.png notificationssoundoff.png notificationssoundon.png notificationssoundplay.png options.def.xml optionsappearance.png optionsdialog.png optionseditprofiles.png optionsprofile.png optionsprofiles.png rchanger.def.xml rchangeraddcontact.png rchangercopygroup.png rchangercreategroup.png rchangergroup.png rchangermovegroup.png rchangerremovecontact.png rchangerremovecontacts.png rchangerremovefromgroup.png rchangerremovegroup.png rchangerrename.png rchangerrootgroup.png rchangersubscribe.png rchangersubscription.png rchangerthisgroup.png rchangerunsubscribe.png rosterview.def.xml rosterviewclipboard.png rosterviewcontacts.png rosterviewhideoffline.png rosterviewoptions.png rosterviewshowoffline.png schanger.def.xml schangerconnecting.png schangereditstatuses.png schangermodifystatus.png
call copyresources2 ru.rwsoftware.eyecu menuicons\shared
copy c:\eyecu\eyecuutils.dll %packages%\ru.rwsoftware.eyecu\data /Y
xcopy c:\eyecu\eyecu.exe %packages%\ru.rwsoftware.eyecu.loader\data\ /Y

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
set files=muc.def.xml mucchangenick.png mucchangetopic.png mucclearchat.png mucconference.png mucconference1.png mucconfigureroom.png mucdestroyroom.png muceditadminslist.png muceditmemberslist.png muceditownerslist.png mucenterroom.png mucexitroom.png mucinvite.png mucjoin.png mucmessage.png mucprivatemessage.png mucrequestvoice.png mucchangepassword.png mucediaffiliations.png mucmoderate.png mucnotifysilence.png muctools.png mucusershide.png
call copyresources2 ru.rwsoftware.eyecu.multiuserchat menuicons\shared

call copyplugins ru.rwsoftware.eyecu.chatstates chatstates
set files=chatstates.def.xml chatstatesactive.png chatstatescomposing.png chatstatesgone.png chatstatesinactive.png chatstatespaused.png chatstatesunknown.png
call copyresources2 ru.rwsoftware.eyecu.chatstates menuicons\shared

call copyplugins ru.rwsoftware.eyecu.privatestorage privatestorage

call copyplugins ru.rwsoftware.eyecu.metacontacts metacontacts
set files=metacontacts.def.xml metacontactscombine.png metacontactsdestroy.png metacontactsdetach.png
call copyresources2 ru.rwsoftware.eyecu.metacontacts menuicons\shared

call copyplugins ru.rwsoftware.eyecu.bookmarks bookmarks
set files=bookmarks.def.xml bookmarks.png bookmarksadd.png bookmarksedit.png bookmarksremove.png bookmarksautojoin.png bookmarksempty.png 
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

call copyplugins ru.rwsoftware.eyecu.datastreamspublisher datastreamspublisher

call copyplugins ru.rwsoftware.eyecu.filetransfer.socks5 socksstreams
call copyplugins ru.rwsoftware.eyecu.filetransfer.ibb inbandstreams
call copyplugins ru.rwsoftware.eyecu.autostatus autostatus
call copyplugins ru.rwsoftware.eyecu.iqauth iqauth

call copyplugins ru.rwsoftware.eyecu.pepmanager pepmanager
set files=pepmanager.def.xml pepmanager.png
call copyresources2 ru.rwsoftware.eyecu.pepmanager menuicons\shared

call copyplugins ru.rwsoftware.eyecu.pepmanager.mood mood
call copyresources ru.rwsoftware.eyecu.pepmanager.mood moodicons\shared
set files=moodicon.def.xml mood.png
call copyresources2 ru.rwsoftware.eyecu.pepmanager.mood menuicons\shared

call copyplugins ru.rwsoftware.eyecu.pepmanager.activity activity
call copyresources ru.rwsoftware.eyecu.pepmanager.activity activityicons\shared
set files=activity.def.xml activity.png
call copyresources2 ru.rwsoftware.eyecu.pepmanager.activity menuicons\shared

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
set files=positioning.def.xml manual.png location.png serialport.png geoip.gif ipstack.png
call copyresources2 ru.rwsoftware.eyecu.pepmanager.geoloc.positioning menuicons\shared
call copyplugins ru.rwsoftware.eyecu.pepmanager.geoloc.positioning.manual positioningmethodmanual
call copyplugins ru.rwsoftware.eyecu.pepmanager.geoloc.positioning.ip positioningmethodip
call copyplugins ru.rwsoftware.eyecu.pepmanager.geoloc.positioning.ip.stack positioningmethodipprovideripstack
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
call copyresources ru.rwsoftware.eyecu.emoji emoji\category_icons
set files=emoji.json
call copyresources2 ru.rwsoftware.eyecu.emoji emoji
call copyresources ru.rwsoftware.eyecu.emoji.emojione.16 "emoji\assets\Emoji One\png\16"
call copyresources ru.rwsoftware.eyecu.emoji.emojione.32 "emoji\assets\Emoji One\png\32"
call copyresources ru.rwsoftware.eyecu.emoji.emojione.48 "emoji\assets\Emoji One\png\48"

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

set files=chatmarkers.def.xml emptybox.png messagereceived.png
call copyresources2 ru.rwsoftware.eyecu.messagemarkers menuicons\shared
call copyplugins ru.rwsoftware.eyecu.messagemarkers.receipts receipts
set files=messagedisplayed.png messageacknowledged.png messageacknowledge.png
call copyresources2 ru.rwsoftware.eyecu.messagemarkers.chat menuicons\shared
call copyplugins ru.rwsoftware.eyecu.messagemarkers.chat chatmarkers

call copyplugins ru.rwsoftware.eyecu.p2p.otr otr
set files=otr.def.xml otr_unverified.png otr_yes.png otr_no.png
call copyresources2 ru.rwsoftware.eyecu.p2p.otr menuicons\shared

call copyplugins ru.rwsoftware.eyecu.mmplayer mmplayer
set files=mmplayer.def.xml mmplayer.png mmplayereject.png
call copyresources2 ru.rwsoftware.eyecu.mmplayer menuicons\shared

call copyplugins ru.rwsoftware.eyecu.jingle jingle
set files=jingle.def.xml jingle.png
call copyresources2 ru.rwsoftware.eyecu.jingle menuicons\shared

call copyplugins ru.rwsoftware.eyecu.jingle.rtp jinglertp
call copyresources ru.rwsoftware.eyecu.jingle.rtp jingle\shared
set files=jinglertp.png
call copyresources2 ru.rwsoftware.eyecu.jingle menuicons\shared

call copyplugins ru.rwsoftware.eyecu.jingle.transports.iceudp jingletransporticeudp
call copyplugins ru.rwsoftware.eyecu.jingle.transports.rawudp jingletransportrawudp

set files=moodicon.def.xml mood.png
call copyresources2 ru.rwsoftware.eyecu.pepmanager.mood menuicons\shared

call copyplugins ru.rwsoftware.eyecu.clienticons clienticons
call copyresources ru.rwsoftware.eyecu.clienticons clienticons\shared

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

rem call copyplugins ru.rwsoftware.eyecu.map.streetview streetview
rem set files=streetview.def.xml streetview.png
rem call copyresources2 ru.rwsoftware.eyecu.map.streetview menuicons\shared
rem call copyplugins ru.rwsoftware.eyecu.map.streetview.google streetviewprovidergoogle

rem call copyplugins ru.rwsoftware.eyecu.map.placeview placeview
rem set files=placeview.def.xml placeview.png
rem call copyresources2 ru.rwsoftware.eyecu.map.placeview menuicons\shared
rem call copyplugins ru.rwsoftware.eyecu.map.placeview.google placeviewprovidergoogle

call copyplugins ru.rwsoftware.eyecu.map.message mapmessage
set files=close.def.xml closeactive.png closeinactive.png
call copyresources2 ru.rwsoftware.eyecu.map.message menuicons\shared

call copyplugins ru.rwsoftware.eyecu.map.sources.osm mapsourceosm
call copyplugins ru.rwsoftware.eyecu.map.sources.wiki mapsourcewiki
call copyplugins ru.rwsoftware.eyecu.map.sources.google  mapsourcegoogle
call copyplugins ru.rwsoftware.eyecu.map.sources.yandex mapsourceyandex
call copyplugins ru.rwsoftware.eyecu.map.sources.kosmosnimki mapsourcekosmosnimki
call copyplugins ru.rwsoftware.eyecu.map.sources.2gis mapsource2gis
call copyplugins ru.rwsoftware.eyecu.map.sources.ovi mapsourceovi
call copyplugins ru.rwsoftware.eyecu.map.sources.bing mapsourcebing
call copyplugins ru.rwsoftware.eyecu.map.sources.navitel mapsourcenavitel
call copyplugins ru.rwsoftware.eyecu.map.sources.progorod mapsourceprogorod
call copyplugins ru.rwsoftware.eyecu.map.sources.esri mapsourceesri
call copyplugins ru.rwsoftware.eyecu.map.sources.megafon mapsourcemegafon
call copyplugins ru.rwsoftware.eyecu.map.sources.rumap mapsourcerumap
call copyplugins ru.rwsoftware.eyecu.map.sources.vitel mapsourcevitel

call copyplugins ru.rwsoftware.eyecu.mapsearch mapsearch
set files=mapsearch.def.xml mapsearch.png
call copyresources2 ru.rwsoftware.eyecu.mapsearch menuicons\shared

rem call copyplugins ru.rwsoftware.eyecu.mapsearch.2gis mapsearchprovider2gis
rem call copyplugins ru.rwsoftware.eyecu.mapsearch.google mapsearchprovidergoogle
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
xcopy c:\eyecu\resources\wizards\shared\*.en.html %packages%\ru.rwsoftware.eyecu.wizards.account.en\data\resources\wizards\shared\* /S /Y
xcopy c:\eyecu\resources\wizards\shared\*.ru.html %packages%\ru.rwsoftware.eyecu.wizards.account.ru\data\resources\wizards\shared\* /S /Y
xcopy c:\eyecu\resources\wizards\shared\*.nl.html %packages%\ru.rwsoftware.eyecu.wizards.account.nl\data\resources\wizards\shared\* /S /Y

rem *** Resources ***
rem   Country
call copyresources ru.rwsoftware.eyecu.resources.country country\shared

rem   Menu Icons
set files=mapsources.def.xml 2gis.png bing.png esri.png geocon.png google.png here.png kosmosnimki.png megafon.png navitel.png osm.png progorod.png vitel.png wiki.png yandex.png
call copyresources2 ru.rwsoftware.eyecu.resources.menuicons.mapsources menuicons\shared

set files=edit.def.xml edit.png editadd.png editcopy.png editdelete.png
call copyresources2 ru.rwsoftware.eyecu.resources.menuicons.edit menuicons\shared

set files=link.def.xml link.png linkadd.png
call copyresources2 ru.rwsoftware.eyecu.resources.menuicons.link menuicons\shared

rem *** Documentaion ***
md %packages%\ru.rwsoftware.eyecu.docs\data
for %%f in (AUTHORS CHANGELOG README TRANSLATORS) do copy c:\eyecu\%%f %packages%\ru.rwsoftware.eyecu.docs\data\%%f.TXT /Y

:build
del %packagefilename%.exe
binarycreator.exe --offline-only -c config\config.xml -p %packages% %packagefilename%.exe

:repo
set repository=repository
if %platform%==x64 set repository=%repository%.x64

repogen.exe -p %packages% %repository%
:end

pause