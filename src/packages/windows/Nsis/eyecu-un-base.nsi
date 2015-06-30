/*
+Base
*/
Section "-Un.$(SecUnBase)" UnBase
SectionIn 1
	; Set output path to the installation directory.
	; Info files
	Delete "$INSTDIR\Authors"
	Delete "$INSTDIR\Readme"
	;Delete "$INSTDIR\CHANGELOG"
	Delete "$INSTDIR\Copying"
	Delete "$INSTDIR\TRANSLATORS"
;	Delete "$INSTDIR\Install"
	Delete "$INSTDIR\eyecu.ico"
	Delete "$INSTDIR\eyecu.icns"
	; Binaries
	Delete "$INSTDIR\eyecu.exe"
	Delete "$INSTDIR\eyecuutils.dll"
	; OpenSSL libraries
	Delete "$INSTDIR\libeay32.dll"
	Delete "$INSTDIR\ssleay32.dll"
	; Plugins
	Delete "$INSTDIR\plugins\accountmanager.dll"
	Delete "$INSTDIR\plugins\chatmessagehandler.dll"
	Delete "$INSTDIR\plugins\connectionmanager.dll"
	Delete "$INSTDIR\plugins\defaultconnection.dll"
	Delete "$INSTDIR\plugins\mainwindow.dll"
	Delete "$INSTDIR\plugins\messageprocessor.dll"
	Delete "$INSTDIR\plugins\messagestyles.dll"
	Delete "$INSTDIR\plugins\messagewidgets.dll"
	Delete "$INSTDIR\plugins\normalmessagehandler.dll"
	Delete "$INSTDIR\plugins\notifications.dll"
	Delete "$INSTDIR\plugins\optionsmanager.dll"
	Delete "$INSTDIR\plugins\presence.dll"
	Delete "$INSTDIR\plugins\roster.dll"
	Delete "$INSTDIR\plugins\rosterchanger.dll"
	Delete "$INSTDIR\plugins\rostersmodel.dll"
	Delete "$INSTDIR\plugins\rostersview.dll"
	Delete "$INSTDIR\plugins\saslauth.dll"
	Delete "$INSTDIR\plugins\simplemessagestyle.dll"
	Delete "$INSTDIR\plugins\stanzaprocessor.dll"
	Delete "$INSTDIR\plugins\starttls.dll"
	Delete "$INSTDIR\plugins\statuschanger.dll"
	Delete "$INSTDIR\plugins\statusicons.dll"
	Delete "$INSTDIR\plugins\traymanager.dll"
	Delete "$INSTDIR\plugins\xmppstreams.dll"
	;------------------------------------
	; Resources
	;accountmanager: account.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\account.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\account.png"
	Delete "$INSTDIR\resources\menuicons\shared\accountlist.png"
	Delete "$INSTDIR\resources\menuicons\shared\accountchange.png"
	Delete "$INSTDIR\resources\menuicons\shared\accountmove.png"
	;Delete "$INSTDIR\resources\menuicons\shared\autostatus.png"
	;chatmessagehandler: chatmessagehandler.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\chatmessagehandler.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\chatmessagehandlermessage.png"
	Delete "$INSTDIR\resources\menuicons\shared\chatmessagehandlerclearchat.png"
	;chatmessagehandler\sounds
	Delete "$INSTDIR\resources\sounds\shared\chatmessagehandler.def.xml"
	Delete "$INSTDIR\resources\sounds\shared\message.wav"
	;connectionmanager: chatmessagehandler.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\chatmessagehandler.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\chatmessagehandlermessage.png"
	Delete "$INSTDIR\resources\menuicons\shared\chatmessagehandlerclearchat.png"
	;connectionmanager: connection.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\connection.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\connectionencrypted.png"  
	;mainwindow: mainwindow.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\mainwindow.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\eyecu.svg"
	Delete "$INSTDIR\resources\menuicons\shared\mainwindowlogo16.png"
	Delete "$INSTDIR\resources\menuicons\shared\mainwindowlogo20.png"
	Delete "$INSTDIR\resources\menuicons\shared\mainwindowlogo24.png"
	Delete "$INSTDIR\resources\menuicons\shared\mainwindowlogo32.png"
	Delete "$INSTDIR\resources\menuicons\shared\mainwindowlogo40.png"
	Delete "$INSTDIR\resources\menuicons\shared\mainwindowlogo48.png"
	Delete "$INSTDIR\resources\menuicons\shared\mainwindowlogo64.png"
	Delete "$INSTDIR\resources\menuicons\shared\mainwindowlogo96.png"
	Delete "$INSTDIR\resources\menuicons\shared\mainwindowlogo128.png"
	Delete "$INSTDIR\resources\menuicons\shared\mainwindowquit.png"
	Delete "$INSTDIR\resources\menuicons\shared\mainwindowmenu.png"
	Delete "$INSTDIR\resources\menuicons\shared\mainwindowshowroster.png"
;	;messagestyles.dll: messagestyles.def.xml
;	Delete "$INSTDIR\resources\menuicons\shared\messagestyles.def.xml"
;	Delete "$INSTDIR\resources\menuicons\shared\messagestyles.png"
	;messagewidgets: messagewidgets.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\messagewidgets.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\messagewidgetssend.png"
	Delete "$INSTDIR\resources\menuicons\shared\messagewidgetstabmenu.png"
	Delete "$INSTDIR\resources\menuicons\shared\messagewidgetsquote.png"
	Delete "$INSTDIR\resources\menuicons\shared\messagewidgetsselect.png"
	;normalmessagehandler: normalmessagehandler.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\normalmessagehandler.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\normalmessagehandlermessage.png"
	Delete "$INSTDIR\resources\menuicons\shared\normalmessagehandlersend.png"
	Delete "$INSTDIR\resources\menuicons\shared\normalmessagehandlerreply.png"
	Delete "$INSTDIR\resources\menuicons\shared\normalmessagehandlerforward.png"
	Delete "$INSTDIR\resources\menuicons\shared\normalmessagehandlernext.png"
	;normalmessagehandler\sound: normalmessagehandler.def.xml
	Delete "$INSTDIR\resources\sounds\shared\normalmessagehandler.def.xml"
	Delete "$INSTDIR\resources\sounds\shared\message.wav"
;==================================================================================
	;notifications
	Delete "$INSTDIR\resources\menuicons\shared\notifications.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\notifications.png"
	Delete "$INSTDIR\resources\menuicons\shared\notificationsactivateall.png"
	Delete "$INSTDIR\resources\menuicons\shared\notificationsremoveall.png"
	Delete "$INSTDIR\resources\menuicons\shared\notificationssoundon.png"
	Delete "$INSTDIR\resources\menuicons\shared\notificationssoundoff.png"
;==================================================================================
	;optionsmanager: options.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\options.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\optionsdialog.png"
	Delete "$INSTDIR\resources\menuicons\shared\optionsprofile.png"
	Delete "$INSTDIR\resources\menuicons\shared\optionsprofiles.png"
	Delete "$INSTDIR\resources\menuicons\shared\optionseditprofiles.png"
	Delete "$INSTDIR\resources\menuicons\shared\optionsappearance.png"
	;pluginmanager: pluginmanager.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\pluginmanager.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\pluginmanagersetup.png"
	Delete "$INSTDIR\resources\menuicons\shared\pluginmanagerabout.png"
	Delete "$INSTDIR\resources\menuicons\shared\pluginmanageraboutqt.png"
	;rosterchanger: rchanger.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\rchanger.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\rchangeraddcontact.png"
	Delete "$INSTDIR\resources\menuicons\shared\rchangergroup.png"
	Delete "$INSTDIR\resources\menuicons\shared\rchangerrootgroup.png"
	Delete "$INSTDIR\resources\menuicons\shared\rchangerthisgroup.png"
	Delete "$INSTDIR\resources\menuicons\shared\rchangercopygroup.png"
	Delete "$INSTDIR\resources\menuicons\shared\rchangermovegroup.png"
	Delete "$INSTDIR\resources\menuicons\shared\rchangercreategroup.png"
	Delete "$INSTDIR\resources\menuicons\shared\rchangerrename.png"
	Delete "$INSTDIR\resources\menuicons\shared\rchangerremovegroup.png"
	Delete "$INSTDIR\resources\menuicons\shared\rchangerremovefromgroup.png"
	Delete "$INSTDIR\resources\menuicons\shared\rchangerremovecontact.png"
	Delete "$INSTDIR\resources\menuicons\shared\rchangerremovecontacts.png"
	Delete "$INSTDIR\resources\menuicons\shared\rchangersubscription.png"
	Delete "$INSTDIR\resources\menuicons\shared\rchangersubscribe.png"
	Delete "$INSTDIR\resources\menuicons\shared\rchangerunsubscribe.png"
	;---not this files in dir ----
	;Delete "$INSTDIR\resources\menuicons\shared\rchangersubscriptionsend.png"
	;Delete "$INSTDIR\resources\menuicons\shared\rchangersubscriptionrequest.png"
	;Delete "$INSTDIR\resources\menuicons\shared\rchangersubscriptionremove.png"
	;Delete "$INSTDIR\resources\menuicons\shared\rchangersubscriptionrefuse.png"

	;rosterchanger\sounds: rchanger.def.xml
	Delete "$INSTDIR\resources\sounds\shared\rchanger.def.xml"  
	;rostersview: rosterview.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\rosterview.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\rosterviewcontacts.png"
	Delete "$INSTDIR\resources\menuicons\shared\rosterviewoptions.png"
	Delete "$INSTDIR\resources\menuicons\shared\rosterviewshowoffline.png"
	Delete "$INSTDIR\resources\menuicons\shared\rosterviewhideoffline.png"
	Delete "$INSTDIR\resources\menuicons\shared\rosterviewclipboard.png"
	;servicediscovery: sdiscovery.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\sdiscovery.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\arrowleft.png"
	Delete "$INSTDIR\resources\menuicons\shared\arrowright.png"
	Delete "$INSTDIR\resources\menuicons\shared\sdiscoverydiscoinfo.png"
	Delete "$INSTDIR\resources\menuicons\shared\sdiscoverydiscover.png"
	Delete "$INSTDIR\resources\menuicons\shared\sdiscoveryreload.png"
	;statuschanger.dll: schanger.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\schanger.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\schangermodifystatus.png"
	Delete "$INSTDIR\resources\menuicons\shared\schangereditstatuses.png"
	Delete "$INSTDIR\resources\menuicons\shared\schangerconnecting.png"
	;statuschanger\sounds: schanger.def.xml
	Delete "$INSTDIR\resources\sounds\shared\schanger.def.xml"
;	;statusicons: statusicons.def.xml
;	Delete "$INSTDIR\resources\menuicons\shared\statusicons.def.xml"
;	Delete "$INSTDIR\resources\menuicons\shared\statusiconsoptions.png"

	;--------Not Resource -------------------------------
	;defaultconnection.dll	; ??
	;messageprocessor.dll		;??
	;presence.dll				;??
	;roster.dll				;??
	;rostersmodel.dll			;??
	;saslauth.dll				;??
	;stanzaprocessor.dll		;??
	;starttls.dll				;??
	;traymanager.dll
	;xmppstreams.dll			;??

	;---------S O U N D ------------------------------
	; sound.def.xml
	Delete "$INSTDIR\resources\sounds\shared\sound.def.xml"
	Delete "$INSTDIR\resources\sounds\shared\default.wav"
	Delete "$INSTDIR\resources\sounds\shared\message.wav"
	Delete "$INSTDIR\resources\sounds\shared\auth.wav"
	Delete "$INSTDIR\resources\sounds\shared\error.wav"
	;---------------------------------------
	;simplemessagestyle
	RMDir /r "$INSTDIR\resources\simplemessagestyles"
	;statusicons
	RMDir /r "$INSTDIR\resources\statusicons"
	;statusicons
	RMDir /r "$INSTDIR\resources\serviceicons"
	;==============================================================
	; Resources EYECU:
	Delete "$INSTDIR\resources\menuicons\shared\eyecuicon.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\jingle.png"
	Delete "$INSTDIR\resources\menuicons\shared\xhtml.png"
	Delete "$INSTDIR\resources\menuicons\shared\voip.png"
	Delete "$INSTDIR\resources\menuicons\shared\voip_active.png"
	Delete "$INSTDIR\resources\menuicons\shared\receipts.png"
	Delete "$INSTDIR\resources\menuicons\shared\tune.png"
	Delete "$INSTDIR\resources\menuicons\shared\bell.gif"
	Delete "$INSTDIR\resources\menuicons\shared\tracker.png"
	Delete "$INSTDIR\resources\menuicons\shared\lastfm.png"
	Delete "$INSTDIR\resources\menuicons\shared\mapstreetview.png"
	Delete "$INSTDIR\resources\menuicons\shared\streetman.png"
	Delete "$INSTDIR\resources\menuicons\shared\viewcenter.png"
	Delete "$INSTDIR\resources\menuicons\shared\key16.png"
	Delete "$INSTDIR\resources\menuicons\shared\yes.png"
	Delete "$INSTDIR\resources\menuicons\shared\no.png"
	Delete "$INSTDIR\resources\menuicons\shared\weather.png"
;	Delete "$INSTDIR\resources\menuicons\shared\positioning.png"
;	Delete "$INSTDIR\resources\menuicons\shared\serialport.png"
;	Delete "$INSTDIR\resources\menuicons\shared\location.png"
;	Delete "$INSTDIR\resources\menuicons\shared\manual.png"	
	
	; Resources EYECU-SOUND: eyecu.def.xml
	Delete "$INSTDIR\resources\sounds\shared\eyecu.def.xml"
	Delete "$INSTDIR\resources\sounds\shared\bells.wav"
	Delete "$INSTDIR\resources\sounds\shared\call.wav"
	Delete "$INSTDIR\resources\sounds\shared\floop.wav"
	Delete "$INSTDIR\resources\sounds\shared\plopp.wav"
	Delete "$INSTDIR\resources\sounds\shared\sonar.wav"
	;------ ?????? ------------------------
	Delete "$INSTDIR\resources\menuicons\shared\edit.def.xml"  
	Delete "$INSTDIR\resources\menuicons\shared\edit.png"
	Delete "$INSTDIR\resources\menuicons\shared\add.png"
	Delete "$INSTDIR\resources\menuicons\shared\delete.png"
	Delete "$INSTDIR\resources\menuicons\shared\accept.png"
	Delete "$INSTDIR\resources\menuicons\shared\decline.png"
	;====================================================================
SectionEnd

Section "-Un.RussianBase" UnRussianBase
SectionIn  1
	Delete "$INSTDIR\translations\ru\eyecu.qm"
	Delete "$INSTDIR\translations\ru\eyecuutils.qm"
	Delete "$INSTDIR\translations\ru\accountmanager.qm"
	Delete "$INSTDIR\translations\ru\chatmessagehandler.qm"
	Delete "$INSTDIR\translations\ru\connectionmanager.qm"
	Delete "$INSTDIR\translations\ru\defaultconnection.qm"
	Delete "$INSTDIR\translations\ru\mainwindow.qm"
	Delete "$INSTDIR\translations\ru\messageprocessor.qm"
	Delete "$INSTDIR\translations\ru\messagestyles.qm"
	Delete "$INSTDIR\translations\ru\messagewidgets.qm"
	Delete "$INSTDIR\translations\ru\normalmessagehandler.qm"
	Delete "$INSTDIR\translations\ru\notifications.qm"
	Delete "$INSTDIR\translations\ru\optionsmanager.qm"
	Delete "$INSTDIR\translations\ru\presence.qm"
	Delete "$INSTDIR\translations\ru\roster.qm"
	Delete "$INSTDIR\translations\ru\rosterchanger.qm"
	Delete "$INSTDIR\translations\ru\rostersmodel.qm"
	Delete "$INSTDIR\translations\ru\rostersview.qm"
	Delete "$INSTDIR\translations\ru\saslauth.qm"
;	
	Delete "$INSTDIR\translations\ru\simplemessagestyle.qm"
	Delete "$INSTDIR\translations\ru\stanzaprocessor.qm"
	Delete "$INSTDIR\translations\ru\starttls.qm"
	Delete "$INSTDIR\translations\ru\statuschanger.qm"
	Delete "$INSTDIR\translations\ru\statusicons.qm"
	Delete "$INSTDIR\translations\ru\traymanager.qm"
	Delete "$INSTDIR\translations\ru\xmppstreams.qm"	
SectionEnd

Section "-Un.DeutschlandBase" UnDeutschlandBase
SectionIn  1
	Delete "$INSTDIR\translations\de\eyecu.qm"
	Delete "$INSTDIR\translations\de\eyecuutils.qm"
	Delete "$INSTDIR\translations\de\accountmanager.qm"
	Delete "$INSTDIR\translations\de\chatmessagehandler.qm"
	Delete "$INSTDIR\translations\de\connectionmanager.qm"
	Delete "$INSTDIR\translations\de\defaultconnection.qm"
	Delete "$INSTDIR\translations\de\mainwindow.qm"
	Delete "$INSTDIR\translations\de\messageprocessor.qm"
	Delete "$INSTDIR\translations\de\messagestyles.qm"
	Delete "$INSTDIR\translations\de\messagewidgets.qm"
	Delete "$INSTDIR\translations\de\normalmessagehandler.qm"
	Delete "$INSTDIR\translations\de\notifications.qm"
	Delete "$INSTDIR\translations\de\optionsmanager.qm"
	Delete "$INSTDIR\translations\de\presence.qm"
	Delete "$INSTDIR\translations\de\roster.qm"
	Delete "$INSTDIR\translations\de\rosterchanger.qm"
	Delete "$INSTDIR\translations\de\rostersmodel.qm"
	Delete "$INSTDIR\translations\de\rostersview.qm"
	Delete "$INSTDIR\translations\de\saslauth.qm"
;	Delete "$INSTDIR\translations\de\servicediscovery.qm"
	Delete "$INSTDIR\translations\de\simplemessagestyle.qm"
	Delete "$INSTDIR\translations\de\stanzaprocessor.qm"
	Delete "$INSTDIR\translations\de\starttls.qm"
	Delete "$INSTDIR\translations\de\statuschanger.qm"
	Delete "$INSTDIR\translations\de\statusicons.qm"
	Delete "$INSTDIR\translations\de\traymanager.qm"
	Delete "$INSTDIR\translations\de\xmppstreams.qm"
SectionEnd

Section "-Un.DeutchBase" UnDeutchBase
SectionIn  1
	Delete "$INSTDIR\translations\nl\eyecu.qm"
	Delete "$INSTDIR\translations\nl\eyecuutils.qm"
	Delete "$INSTDIR\translations\nl\accountmanager.qm"
	Delete "$INSTDIR\translations\nl\chatmessagehandler.qm"
	Delete "$INSTDIR\translations\nl\connectionmanager.qm"
	Delete "$INSTDIR\translations\nl\defaultconnection.qm"
	Delete "$INSTDIR\translations\nl\mainwindow.qm"
	Delete "$INSTDIR\translations\nl\messageprocessor.qm"
	Delete "$INSTDIR\translations\nl\messagestyles.qm"
	Delete "$INSTDIR\translations\nl\messagewidgets.qm"
	Delete "$INSTDIR\translations\nl\normalmessagehandler.qm"
	Delete "$INSTDIR\translations\nl\notifications.qm"
	Delete "$INSTDIR\translations\nl\optionsmanager.qm"
	Delete "$INSTDIR\translations\nl\presence.qm"
	Delete "$INSTDIR\translations\nl\roster.qm"
	Delete "$INSTDIR\translations\nl\rosterchanger.qm"
	Delete "$INSTDIR\translations\nl\rostersmodel.qm"
	Delete "$INSTDIR\translations\nl\rostersview.qm"
	Delete "$INSTDIR\translations\nl\saslauth.qm"
;	Delete "$INSTDIR\translations\nl\servicediscovery.qm"
	Delete "$INSTDIR\translations\nl\simplemessagestyle.qm"
	Delete "$INSTDIR\translations\nl\stanzaprocessor.qm"
	Delete "$INSTDIR\translations\nl\starttls.qm"
	Delete "$INSTDIR\translations\nl\statuschanger.qm"
	Delete "$INSTDIR\translations\nl\statusicons.qm"
	Delete "$INSTDIR\translations\nl\traymanager.qm"
	Delete "$INSTDIR\translations\nl\xmppstreams.qm"
SectionEnd

Section "-Un.JapaneseBase" UnJapaneseBase
SectionIn  1
	Delete "$INSTDIR\translations\ja\eyecu.qm"
	Delete "$INSTDIR\translations\ja\eyecuutils.qm"
	Delete "$INSTDIR\translations\ja\accountmanager.qm"
	Delete "$INSTDIR\translations\ja\chatmessagehandler.qm"
	Delete "$INSTDIR\translations\ja\connectionmanager.qm"
	Delete "$INSTDIR\translations\ja\defaultconnection.qm"
	Delete "$INSTDIR\translations\ja\mainwindow.qm"
	Delete "$INSTDIR\translations\ja\messageprocessor.qm"
	Delete "$INSTDIR\translations\ja\messagestyles.qm"
	Delete "$INSTDIR\translations\ja\messagewidgets.qm"
	Delete "$INSTDIR\translations\ja\normalmessagehandler.qm"
	Delete "$INSTDIR\translations\ja\notifications.qm"
	Delete "$INSTDIR\translations\ja\optionsmanager.qm"
	Delete "$INSTDIR\translations\ja\presence.qm"
	Delete "$INSTDIR\translations\ja\roster.qm"
	Delete "$INSTDIR\translations\ja\rosterchanger.qm"
	Delete "$INSTDIR\translations\ja\rostersmodel.qm"
	Delete "$INSTDIR\translations\ja\rostersview.qm"
	Delete "$INSTDIR\translations\ja\saslauth.qm"
;	Delete "$INSTDIR\translations\ja\servicediscovery.qm"
	Delete "$INSTDIR\translations\ja\simplemessagestyle.qm"
	Delete "$INSTDIR\translations\ja\stanzaprocessor.qm"
	Delete "$INSTDIR\translations\ja\starttls.qm"
	Delete "$INSTDIR\translations\ja\statuschanger.qm"
	Delete "$INSTDIR\translations\ja\statusicons.qm"
	Delete "$INSTDIR\translations\ja\traymanager.qm"
	Delete "$INSTDIR\translations\ja\xmppstreams.qm"
SectionEnd

Section "-Un.PolishBase" UnPolishBase
SectionIn  1
	Delete "$INSTDIR\translations\pl\eyecu.qm"
	Delete "$INSTDIR\translations\pl\eyecuutils.qm"
	Delete "$INSTDIR\translations\pl\accountmanager.qm"
	Delete "$INSTDIR\translations\pl\chatmessagehandler.qm"
	Delete "$INSTDIR\translations\pl\connectionmanager.qm"
	Delete "$INSTDIR\translations\pl\defaultconnection.qm"
	Delete "$INSTDIR\translations\pl\mainwindow.qm"
	Delete "$INSTDIR\translations\pl\messageprocessor.qm"
	Delete "$INSTDIR\translations\pl\messagestyles.qm"
	Delete "$INSTDIR\translations\pl\messagewidgets.qm"
	Delete "$INSTDIR\translations\pl\normalmessagehandler.qm"
	Delete "$INSTDIR\translations\pl\notifications.qm"
	Delete "$INSTDIR\translations\pl\optionsmanager.qm"
	Delete "$INSTDIR\translations\pl\presence.qm"
	Delete "$INSTDIR\translations\pl\roster.qm"
	Delete "$INSTDIR\translations\pl\rosterchanger.qm"
	Delete "$INSTDIR\translations\pl\rostersmodel.qm"
	Delete "$INSTDIR\translations\pl\rostersview.qm"
	Delete "$INSTDIR\translations\pl\saslauth.qm"
;	Delete "$INSTDIR\translations\pl\servicediscovery.qm"
	Delete "$INSTDIR\translations\pl\simplemessagestyle.qm"
	Delete "$INSTDIR\translations\pl\stanzaprocessor.qm"
	Delete "$INSTDIR\translations\pl\starttls.qm"
	Delete "$INSTDIR\translations\pl\statuschanger.qm"
	Delete "$INSTDIR\translations\pl\statusicons.qm"
	Delete "$INSTDIR\translations\pl\traymanager.qm"
	Delete "$INSTDIR\translations\pl\xmppstreams.qm"
SectionEnd

Section "-Un.SpainBase" UnSpainBase
SectionIn  1
	Delete "$INSTDIR\translations\es\eyecu.qm"
	Delete "$INSTDIR\translations\es\eyecuutils.qm"
	Delete "$INSTDIR\translations\es\accountmanager.qm"
	Delete "$INSTDIR\translations\es\chatmessagehandler.qm"
	Delete "$INSTDIR\translations\es\connectionmanager.qm"
	Delete "$INSTDIR\translations\es\defaultconnection.qm"
	Delete "$INSTDIR\translations\es\mainwindow.qm"
	Delete "$INSTDIR\translations\es\messageprocessor.qm"
	Delete "$INSTDIR\translations\es\messagestyles.qm"
	Delete "$INSTDIR\translations\es\messagewidgets.qm"
	Delete "$INSTDIR\translations\es\normalmessagehandler.qm"
	Delete "$INSTDIR\translations\es\notifications.qm"
	Delete "$INSTDIR\translations\es\optionsmanager.qm"
	Delete "$INSTDIR\translations\es\presence.qm"
	Delete "$INSTDIR\translations\es\roster.qm"
	Delete "$INSTDIR\translations\es\rosterchanger.qm"
	Delete "$INSTDIR\translations\es\rostersmodel.qm"
	Delete "$INSTDIR\translations\es\rostersview.qm"
	Delete "$INSTDIR\translations\es\saslauth.qm"
;	Delete "$INSTDIR\translations\es\servicediscovery.qm"
	Delete "$INSTDIR\translations\es\simplemessagestyle.qm"
	Delete "$INSTDIR\translations\es\stanzaprocessor.qm"
	Delete "$INSTDIR\translations\es\starttls.qm"
	Delete "$INSTDIR\translations\es\statuschanger.qm"
	Delete "$INSTDIR\translations\es\statusicons.qm"
	Delete "$INSTDIR\translations\es\traymanager.qm"
	Delete "$INSTDIR\translations\es\xmppstreams.qm"
SectionEnd

Section "-Un.UkraineBase" UnUkraineBase
SectionIn  1
	Delete "$INSTDIR\translations\uk\eyecu.qm"
	Delete "$INSTDIR\translations\uk\eyecuutils.qm"
 	Delete "$INSTDIR\translations\uk\accountmanager.qm"
	Delete "$INSTDIR\translations\uk\chatmessagehandler.qm"
	Delete "$INSTDIR\translations\uk\connectionmanager.qm"
	Delete "$INSTDIR\translations\uk\defaultconnection.qm"
	Delete "$INSTDIR\translations\uk\mainwindow.qm"
	Delete "$INSTDIR\translations\uk\messageprocessor.qm"
	Delete "$INSTDIR\translations\uk\messagestyles.qm"
	Delete "$INSTDIR\translations\uk\messagewidgets.qm"
	Delete "$INSTDIR\translations\uk\normalmessagehandler.qm"
	Delete "$INSTDIR\translations\uk\notifications.qm"
	Delete "$INSTDIR\translations\uk\optionsmanager.qm"
	Delete "$INSTDIR\translations\uk\presence.qm"
	Delete "$INSTDIR\translations\uk\roster.qm"
	Delete "$INSTDIR\translations\uk\rosterchanger.qm"
	Delete "$INSTDIR\translations\uk\rostersmodel.qm"
	Delete "$INSTDIR\translations\uk\rostersview.qm"
	Delete "$INSTDIR\translations\uk\saslauth.qm"
;	Delete "$INSTDIR\translations\uk\servicediscovery.qm"
	Delete "$INSTDIR\translations\uk\simplemessagestyle.qm"
	Delete "$INSTDIR\translations\uk\stanzaprocessor.qm"
	Delete "$INSTDIR\translations\uk\starttls.qm"
	Delete "$INSTDIR\translations\uk\statuschanger.qm"
	Delete "$INSTDIR\translations\uk\statusicons.qm"
	Delete "$INSTDIR\translations\uk\traymanager.qm"
	Delete "$INSTDIR\translations\uk\xmppstreams.qm"
SectionEnd