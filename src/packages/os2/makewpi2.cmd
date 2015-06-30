@echo off
set packagename=eyecu2
set devpackagename=%packagename%-dev
set version=1.3.0.1201
set extrapackagename=%packagename%-extra
set extradevpackagename=%extrapackagename%-dev
set packagefilename=%packagename%-%version%
set devpackagefilename=%devpackagename%-%version%
set extrapackagefilename=%extrapackagename%-%version%
set extradevpackagefilename=%extradevpackagename%-%version%

echo Creating base package
call substver.cmd %packagename% %version%
if exist %packagefilename%.wpi del %packagefilename%.wpi

set pluginlist=accntmgr chatmsgh connectm dfltconn mainwndw msgprcsr msgstyle msgwdgts nrmlmsgh notifics optsmngr presence roster rstrchng rstrmodl rstrview saslauth smpmsgst stanzapr starttls statusch sttsicns traymngr xmppstrm
call copyplugins 1
set resources=menuicons statusicons simplemessagestyles sounds
call copyresources 1
copy c:\eyecu\eyecu.exe 1\
copy c:\eyecu\eyecutls.dll 1\

call copyplugins 3 spellchk
call copyplugins 4 statstcs
call copyplugins 5 privlsts
call copyplugins 6 compress
call copyplugins 7 vcard
call copyplugins 8 birthday
call copyplugins 9 avatars
call copyplugins 10 adiummsg
call copyresources 10 adiummessagestyles\shared
call copyresources 11 adiummessagestyles\renkoo
call copyresources 12 adiummessagestyles\yMous
call copyplugins 15 gateways
call copyplugins 16 shcutmgr
call copyplugins 17 srvdisco
call copyresources 17 serviceicons\shared
call copyplugins 18 msgcrbns
call copyplugins 19 console
call copyplugins 20 msgarchv
call copyplugins 21 fmsgarch
call copyplugins 22 smsgarch
call copyplugins 25 rstrsrch
call copyplugins 26 jbbrsrch
call copyplugins 27 ritxchng
call copyplugins 28 muchat
call copyplugins 29 chatstat

call copyplugins 30 privstor
call copyplugins 31 metacnts
call copyplugins 32 bookmark
call copyplugins 33 annotats
call copyplugins 34 rccntcts
call copyplugins 35 clntinfo
call copyplugins 36 urlproc
call copyplugins 37 bitsobin
call copyplugins 38 boburlhd
call copyplugins 39 xmppuriq
call copyplugins 40 datafrms
call copyplugins 41 commands
call copyplugins 42 captchaf
call copyplugins 43 remtctrl
call copyplugins 44 registrn
call copyplugins 45 sessnego

set pluginlist=filetran fstrmmgr dstrmmgr
call copyplugins 46
call copyplugins 47 sockstrm
call copyplugins 48 ibbstrms

call copyplugins 50 pepmangr
call copyplugins 51 geoloc
call copyplugins 52 position
call copyplugins 53 pmmanual
call copyplugins 54 pmip
call copyplugins 55 pmipfree
call copyplugins 58 autostts
call copyplugins 59 iqauth

call copyresource 60 statusicons\aim
call copyresource 61 statusicons\bot
call copyresource 62 statusicons\car
call copyresource 63 statusicons\conference
call copyresource 64 statusicons\facebook
call copyresource 65 statusicons\gadu
call copyresource 66 statusicons\gtalk
call copyresource 67 statusicons\icq
call copyresource 68 statusicons\livejournal
call copyresource 69 statusicons\mrim
call copyresource 70 statusicons\msn
call copyresource 71 statusicons\odnoklassniki
call copyresource 72 statusicons\rss
call copyresource 73 statusicons\skype
call copyresource 74 statusicons\sms
call copyresource 75 statusicons\smtp
call copyresource 76 statusicons\twitter
call copyresource 77 statusicons\vkontakte
call copyresource 78 statusicons\weather
call copyresource 79 statusicons\yahoo
call copyresource 80 statusicons\yaonline
xcopy c:\eyecu\doc\* 81\
call copyplugins 85 emoticon
call copyresource 86 emoticons\default
call copyresource 87 emoticons\blobs_purple
call copyplugins 89 xhtmlim
call copyresources 89 xhtml\shared

wic -a %packagefilename%.wpi 1 -r -c1 * 2 shadow 3 -r -c3 * 4 -r -c4 * 5 -r -c5 * 6 -r -c6 * 7 -r -c7  * 8 -r -c8 * 9 -r -c9 * 10 -r -c10 * 11 -r -c11 * 12 -r -c12 * 15 -r -c15 * 16 -r -c16 * 17 -r -c17 * 18 -r -c18 * 19 -r -c19 * 20 -r -c20 * 21 -r -c21 * 22 -r -c22 * 25 -r -c25 * 26 -r -c26 * 27 -r -c27 * 28 -r -c28 * 29 -r -c29 * 30 -r -c30 -r * 31 -c31 -r * 32 -r -c32 * 33 -r -c33 * 34 -r -c34 * 35 -r -c35 * 36 -r -c36 -r * 37 -r -c37 * 38 -r -c38 * 39 -r -c39 * 40 -r -c40 * 41 -r -c41 * 42 -r -c42 * 43 -r -c43 * 44 -r -c44 * 45 -r -c45 * 46 -r -c46 * 47 -r -c47 * 48 -r -c48 * 50 -r -c50 * 51 -r -c51 * 52 -r -c52 * 53 -r -c53 * 54 -r -c54 * 55 -r -c55 * 58 -r -c58 * 59 -r -c59 * 60 -c60 * 61 -c61 * 62 -c62 * 63 -c63 * 64 -c64 * 65 -c65 * 66 -c66 * 67 -c67 * 68 -c68 * 69 -c69 * 70 -c70 * 71 -c71 * 72 -c72 * 73 -c73 * 74 -c74 * 75 -c75 * 76 -c76 * 77 -c77 * 78 -c78 * 79 -c79 * 80 -c80 * 81 -c81 * 85 -r -c85 * 86 -c86 * 87 -c87 * 89 -r -c89 * -s %packagename%.wis 

call substver.cmd %devpackagename% %version%
del %devpackagefilename%.wpi
wic -a %devpackagefilename%.wpi 99 -cc:\eyecu eyecutls.lib
wic -a %devpackagefilename%.wpi 99 -cc:\eyecu\sdk definitions\* utils\* * interfaces\iaccountmanager.h interfaces\iannotations.h interfaces\iautostatus.h interfaces\iavatars.h interfaces\ibirthdayreminder.h interfaces\ibitsofbinary.h interfaces\ibookmarks.h interfaces\icaptchaforms.h interfaces\ichatstates.h interfaces\iclientinfo.h interfaces\icommands.h interfaces\iconnectionmanager.h interfaces\idataforms.h interfaces\idatastreamsmanager.h interfaces\idefaultconnection.h interfaces\iemoticons.h interfaces\ifilemessagearchive.h interfaces\ifilestreamsmanager.h interfaces\ifiletransfer.h interfaces\igateways.h interfaces\iinbandstreams.h interfaces\ijabbersearch.h interfaces\imainwindow.h interfaces\imessagearchiver.h interfaces\imessagecarbons.h
wic -a %devpackagefilename%.wpi 99 -cc:\eyecu\sdk interfaces\imessageprocessor.h interfaces\imessagestyles.h interfaces\imessagewidgets.h interfaces\imetacontacts.h interfaces\imultiuserchat.h interfaces\inotifications.h interfaces\ioptionsmanager.h interfaces\ipepmanager.h interfaces\ipluginmanager.h interfaces\ipositioning.h interfaces\ipresence.h interfaces\iprivacylists.h interfaces\iprivatestorage.h interfaces\ireceipts.h interfaces\irecentcontacts.h interfaces\iregistraton.h interfaces\iroster.h interfaces\irosterchanger.h interfaces\irosteritemexchange.h interfaces\irostersearch.h interfaces\irostersmodel.h interfaces\irostersview.h interfaces\iservermessagearchive.h interfaces\iservicediscovery.h interfaces\isessionnegotiation.h interfaces\isocksstreams.h
wic -a %devpackagefilename%.wpi 99 -cc:\eyecu\sdk interfaces\ispellchecker.h interfaces\istanzaprocessor.h interfaces\istatistics.h interfaces\istatuschanger.h interfaces\istatusicons.h interfaces\itraymanager.h interfaces\iurlprocessor.h interfaces\ivcard.h interfaces\ixmppstreams.h interfaces\ixmppuriqueries.h ixhtmlim.h
wic -a %devpackagefilename%.wpi -s %devpackagename%.wis 

echo Creating extra package
call substver.cmd %extrapackagename% %version%
if exist %extrapackagefilename%.wpi del %extrapackagefilename%.wpi

call copyplugins 101 pmserial
set pluginlist=map mapscene
call copyplugins 103 
call copyresources 103 mapicons\shared
call copyplugins 104 msosm
call copyplugins 105 mswiki
call copyplugins 106 msgoogle
call copyplugins 107 msyahoo
call copyplugins 108 msbing
call copyplugins 109 msovi
call copyplugins 110 msrumap
call copyplugins 111 msvitel
call copyplugins 112 msesri
call copyplugins 113 msnavteq
call copyplugins 114 msyandex
call copyplugins 115 msnavitl
call copyplugins 116 ms2gis
call copyplugins 117 mskosmos
call copyplugins 118 msmailru
call copyplugins 119 msrosrsr
call copyplugins 120 msmegafn
call copyplugins 121 msprogrd
call copyplugins 125 mpcntcts
call copyplugins 126 mapmessg
call copyplugins 127 conproxn
call copyplugins 130 mpsearch
call copyplugins 131 msposm
call copyplugins 132 mspgoogl
call copyplugins 133 msp2gis
call copyplugins 134 mspyandx
call copyplugins 135 mspnavtl
call copyresources 135 navitel\shared
call copyplugins 136 msphere
call copyplugins 137 mpmagnif
call copyplugins 138 maplcsel
call copyplugins 140 strtview
call copyplugins 141 svpgoogl
call copyplugins 145 plceview
call copyplugins 146 pvpgoogl
set resources=typepoint country\shared
call copyresources 150
call copyplugins 150 poi
call copyplugins 152 nickname
call copyplugins 153 abbrvtns
call copyresources 153 abbreviations\shared
call copyplugins 156 receipts
call copyplugins 157 attntion
call copyplugins 160 oob
call copyplugins 161 clnticns
call copyresources 161 clienticons\shared
call copyplugins 165 mood
call copyresources 165 moodicons\shared
call copyplugins 166 activity
call copyresources 166 activityicons\shared
call copyplugins 170 tune
call copyplugins 171 tlfile
call copyplugins 172 tlpm123
call copyplugins 173 tlquplay
call copyplugins 174 tlz
call copyplugins 175 tirlstfm

md 180
set resources=wizards\shared\wizard.def.xml wizards\shared\wizard.png wizards\shared\wiz-banner.png
call copyresources1 180
call copyplugins 181 waccount
set resources=wizards\shared\*.html wizards\shared\servers.xml wizards\shared\software.def.xml wizards\shared\ejabberd.png wizards\shared\jabberd.png wizards\shared\openfire.png wizards\shared\prosody.png wizards\shared\gtalk.png wizards\shared\yaonline.png wizards\shared\wiz-acc.png wizards\shared\wiz-acc2.png
call copyresources1 181
call copyplugins 182 wtrnsprt
set resources=wizards\shared\wiz-trans.png wizards\shared\wiz-trans-end.png wizards\shared\gateway.def.xml
call copyresources1 182

wic -a %extrapackagefilename%.wpi 100 extra 101 -r -c101 * 103 -r -c103 * 104 -r -c104 * 105 -r -c105 * 106 -r -c106 * 107 -r -c107 * 108 -r -c108 * 109 -r -c109 * 110 -r -c110 * 111 -r -c111 * 112 -r -c112 * 113 -r -c113 * 114 -r -c114 * 115 -r -c115 * 116 -r -c116 * 117 -r -c117 * 118 -r -c118 * 119 -r -c119 * 120 -r -c120 * 121 -r -c121 * 125 -r -c125 * 126 -r -c126 * 127 -r -c127 * 130 -r -c130 -r * 131 -c131 -r * 132 -r -c132 * 133 -r -c133 * 134 -r -c134 * 135 -r -c135 * 136 -r -c136 -r * 137 -r -c137 * 138 -r -c138 * 140 -r -c140 * 141 -r -c141 * 145 -r -c145 * 146 -r -c146 * 150 -r -c150 * 152 -r -c152 * 153 -r -c153 * 156 -r -c156 * 157 -r -c157 * 160 -r -c160 * 161 -r -c161 * 165 -r -c165 * 166 -r -c166 * 170 -r -c170 * 171 -r -c171 * 172 -r -c172 * 173 -r -c173 * 174 -r -c174 * 175 -r -c175 * 180 -r -c180 * 181 -r -c181 * 182 -r -c182 * -s %extrapackagename%.wis

exit

call substver.cmd %extradevpackagename% %version%
del %extradevpackagefilename%.wpi
wic -a %extradevpackagefilename%.wpi 199 -r -cc:\eyecu\sdk interfaces\iabbreviations.h interfaces\iactivity.h interfaces\iattention.h interfaces\iclienticons.h interfaces\igeoloc.h interfaces\imap.h interfaces\imapcontacts.h interfaces\imaplocationselector.h interfaces\imapmagnifier.h interfaces\imapmessage.h interfaces\imapscene.h interfaces\imapsearch.h interfaces\imood.h interfaces\imultiuserchat.h interfaces\inickname.h interfaces\ioob.h interfaces\ipoi.h interfaces\ipositioning.h interfaces\ireceipts.h interfaces\istreetview.h interfaces\itune.h interfaces\iwizardaccount.h interfaces\iwizardtransport.h
wic -a %extradevpackagefilename%.wpi -s %extradevpackagename%.wis 