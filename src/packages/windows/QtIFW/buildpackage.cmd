echo off
set packagename=eyecu-win
set devpackagename=%packagename%-dev
set version=1.3.0
set packagefilename=%packagename%-%version%
set devpackagefilename=%devpackagename%-%version%
set packages=packages

echo Creating base package
rem call substver.cmd %packagename% %version%
del %packagefilename%.exe

rem goto build

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
set resources=menuicons statusicons simplemessagestyles sounds
call copyresources ru.rwsoftware.eyecu
copy c:\eyecu\eyecu.exe packages\ru.rwsoftware.eyecu\data /Y
copy c:\eyecu\eyecuutils.dll packages\ru.rwsoftware.eyecu\data /Y

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
call copyplugins ru.rwsoftware.eyecu.compress compress
call copyplugins ru.rwsoftware.eyecu.vcard vcard
call copyplugins ru.rwsoftware.eyecu.birthdayreminder birthdayreminder
call copyplugins ru.rwsoftware.eyecu.avatars avatars
call copyplugins ru.rwsoftware.eyecu.adiummessagestyle adiummessagestyle
call copyresources ru.rwsoftware.eyecu.adiummessagestyle adiummessagestyles\shared
call copyresources ru.rwsoftware.eyecu.adiummessagestyle.renkoo adiummessagestyles\renkoo
call copyresources ru.rwsoftware.eyecu.adiummessagestyle.ymous adiummessagestyles\yMous
call copyplugins ru.rwsoftware.eyecu.gateways gateways
call copyplugins ru.rwsoftware.eyecu.shortcutmanager shortcutmanager
call copyplugins ru.rwsoftware.eyecu.servicediscovery servicediscovery
call copyresources ru.rwsoftware.eyecu.servicediscovery serviceicons\shared
call copyplugins ru.rwsoftware.eyecu.messagecarbons messagecarbons
call copyplugins ru.rwsoftware.eyecu.console console
call copyplugins ru.rwsoftware.eyecu.messagearchiver messagearchiver
call copyplugins ru.rwsoftware.eyecu.messagearchiver.file filemessagearchive
call copyplugins ru.rwsoftware.eyecu.messagearchiver.server servermessagearchive
call copyplugins ru.rwsoftware.eyecu.rostersearch rostersearch
call copyplugins ru.rwsoftware.eyecu.jabbersearch jabbersearch
call copyplugins ru.rwsoftware.eyecu.rosteritemexchange rosteritemexchange
call copyplugins ru.rwsoftware.eyecu.multiuserchat multiuserchat
call copyplugins ru.rwsoftware.eyecu.chatstates chatstates
call copyplugins ru.rwsoftware.eyecu.privatestorage privatestorage
call copyplugins ru.rwsoftware.eyecu.metacontacts metacontacts
call copyplugins ru.rwsoftware.eyecu.bookmarks bookmarks
call copyplugins ru.rwsoftware.eyecu.annotations annotations
call copyplugins ru.rwsoftware.eyecu.recentcontacts recentcontacts
call copyplugins ru.rwsoftware.eyecu.clientinfo clientinfo
call copyplugins ru.rwsoftware.eyecu.urlprocessor urlprocessor
call copyplugins ru.rwsoftware.eyecu.bob bitsofbinary
call copyplugins ru.rwsoftware.eyecu.boburlhandler boburlhandler
call copyplugins ru.rwsoftware.eyecu.xmppuriqueries xmppuriqueries
call copyplugins ru.rwsoftware.eyecu.dataforms dataforms
call copyplugins ru.rwsoftware.eyecu.sessionnegotiation sessionnegotiation
call copyplugins ru.rwsoftware.eyecu.captchaforms captchaforms
call copyplugins ru.rwsoftware.eyecu.registration registration
call copyplugins ru.rwsoftware.eyecu.commands commands
call copyplugins ru.rwsoftware.eyecu.remotecontrol remotecontrol
set pluginlist=filetransfer filestreamsmanager datastreamsmanager
call copyplugins ru.rwsoftware.eyecu.filetransfer
call copyplugins ru.rwsoftware.eyecu.filetransfer.socks5 socksstreams
call copyplugins ru.rwsoftware.eyecu.filetransfer.ibb inbandstreams
call copyplugins ru.rwsoftware.eyecu.autostatus autostatus
call copyplugins ru.rwsoftware.eyecu.iqauth iqauth

call copyplugins ru.rwsoftware.eyecu.pepmanager pepmanager
call copyplugins ru.rwsoftware.eyecu.pepmanager.mood mood
call copyresources ru.rwsoftware.eyecu.pepmanager.mood moodicons\shared
call copyplugins ru.rwsoftware.eyecu.pepmanager.activity activity
call copyresources ru.rwsoftware.eyecu.pepmanager.activity activityicons\shared
call copyplugins ru.rwsoftware.eyecu.pepmanager.tune tune
call copyplugins ru.rwsoftware.eyecu.pepmanager.tune.inforequesters.lastfm tuneinforequesterlastfm
call copyplugins ru.rwsoftware.eyecu.pepmanager.tune.listeners.winamp tunelistenerwinamp
call copyplugins ru.rwsoftware.eyecu.pepmanager.tune.listeners.aimp tunelisteneraimp
call copyplugins ru.rwsoftware.eyecu.pepmanager.tune.listeners.file tunelistenerfile

call copyplugins ru.rwsoftware.eyecu.pepmanager.geoloc geoloc
call copyplugins ru.rwsoftware.eyecu.pepmanager.geoloc.positioning positioning
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
call copyplugins ru.rwsoftware.eyecu.xhtmlim xhtmlim
call copyresources ru.rwsoftware.eyecu.xhtmlim xhtml\shared
call copyplugins ru.rwsoftware.eyecu.oob oob
call copyplugins ru.rwsoftware.eyecu.nickname nickname
call copyplugins ru.rwsoftware.eyecu.abbreviations abbreviations
call copyresources ru.rwsoftware.eyecu.abbreviations abbreviations\shared
call copyplugins ru.rwsoftware.eyecu.attention attention
call copyplugins ru.rwsoftware.eyecu.receipts receipts
call copyplugins ru.rwsoftware.eyecu.mmplayer mmplayer
call copyplugins ru.rwsoftware.eyecu.clienticons clienticons
call copyresources ru.rwsoftware.eyecu.clienticons clienticons\shared
call copyplugins ru.rwsoftware.eyecu.contactproximitynotification contactproximitynotification
call copyplugins ru.rwsoftware.eyecu.poi poi
call copyplugins ru.rwsoftware.eyecu.map map
call copyresources ru.rwsoftware.eyecu.map mapicons\shared
call copyplugins ru.rwsoftware.eyecu.map.locationselector maplocationselector
call copyplugins ru.rwsoftware.eyecu.map.magnifier mapmagnifier
call copyplugins ru.rwsoftware.eyecu.map.contacts mapcontacts
call copyplugins ru.rwsoftware.eyecu.map.streetview streetview
call copyplugins ru.rwsoftware.eyecu.map.streetview.google streetviewprovidergoogle
call copyplugins ru.rwsoftware.eyecu.map.placeview placeview
call copyplugins ru.rwsoftware.eyecu.map.placeview.google placeviewprovidergoogle
call copyplugins ru.rwsoftware.eyecu.map.message mapmessage
call copyplugins ru.rwsoftware.eyecu.map.sources.osm mapsourceosm
call copyplugins ru.rwsoftware.eyecu.map.sources.wiki mapsourcewiki
call copyplugins ru.rwsoftware.eyecu.map.sources.google  mapsourcegoogle
call copyplugins ru.rwsoftware.eyecu.map.sources.yandex mapsourceyandex
call copyplugins ru.rwsoftware.eyecu.map.sources.kosmosnimki mapsourcekosmosnimki
call copyplugins ru.rwsoftware.eyecu.map.sources.2gis mapsource2gis
call copyplugins ru.rwsoftware.eyecu.map.sources.yahoo mapsourceyahoo
call copyplugins ru.rwsoftware.eyecu.map.sources.ovi mapsourceovi
call copyplugins ru.rwsoftware.eyecu.map.sources.bing mapsourcebing
call copyplugins ru.rwsoftware.eyecu.map.sources.mailru mapsourcemailru
call copyplugins ru.rwsoftware.eyecu.map.sources.navitel mapsourcenavitel
call copyplugins ru.rwsoftware.eyecu.map.sources.progorod mapsourceprogorod
call copyplugins ru.rwsoftware.eyecu.map.sources.esri mapsourceesri
call copyplugins ru.rwsoftware.eyecu.map.sources.megafon mapsourcemegafon
call copyplugins ru.rwsoftware.eyecu.map.sources.navteq mapsourcenavteq
call copyplugins ru.rwsoftware.eyecu.map.sources.rosreestr mapsourcerosreestr
call copyplugins ru.rwsoftware.eyecu.map.sources.rumap mapsourcerumap
call copyplugins ru.rwsoftware.eyecu.map.sources.vitel mapsourcevitel
call copyplugins ru.rwsoftware.eyecu.mapsearch mapsearch
call copyplugins ru.rwsoftware.eyecu.mapsearch.2gis mapsearchprovider2gis
call copyplugins ru.rwsoftware.eyecu.mapsearch.google mapsearchprovidergoogle
call copyplugins ru.rwsoftware.eyecu.mapsearch.here mapsearchproviderhere
call copyplugins ru.rwsoftware.eyecu.mapsearch.navitel mapsearchprovidernavitel
call copyplugins ru.rwsoftware.eyecu.mapsearch.osm mapsearchproviderosm
call copyplugins ru.rwsoftware.eyecu.mapsearch.yandex mapsearchprovideryandex

set resources=wizards\shared\wizard.def.xml wizards\shared\wizard.png wizards\shared\wiz-banner.png
call copyresources1 ru.rwsoftware.eyecu.wizards

call copyplugins ru.rwsoftware.eyecu.wizards.transport wizardtransport
set resources=wizards\shared\*.html wizards\shared\servers.xml wizards\shared\software.def.xml wizards\shared\ejabberd.png wizards\shared\jabberd.png wizards\shared\openfire.png wizards\shared\prosody.png wizards\shared\gtalk.png wizards\shared\yaonline.png wizards\shared\wiz-acc.png wizards\shared\wiz-acc2.png
call copyresources1 ru.rwsoftware.eyecu.wizards.transport

call copyplugins ru.rwsoftware.eyecu.wizards.account wizardaccount
set resources=wizards\shared\wiz-trans.png wizards\shared\wiz-trans-end.png wizards\shared\gateway.def.xml
call copyresources1 ru.rwsoftware.eyecu.wizards.account

md packages\ru.rwsoftware.eyecu.docs\data
for %%f in (AUTHORS CHANGELOG README TRANSLATORS) do copy c:\eyecu\%%f packages\ru.rwsoftware.eyecu.docs\data\%%f.TXT /Y

:build
binarycreator.exe -c config\config.xml -p %packages% %packagefilename%.exe

:end