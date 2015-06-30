/*
+Base
*/
Section -$(SecBase) Base
SectionIn 1 2 3 
	; Set output path to the installation directory.
	SetOutPath $INSTDIR
	; Info files
	File "${PROGRAM_BIN_FOLDER}\Authors"
	File "${PROGRAM_BIN_FOLDER}\Readme"
	;File "${PROGRAM_BIN_FOLDER}\CHANGELOG"
	File "${PROGRAM_BIN_FOLDER}\Copying"
	File "${PROGRAM_BIN_FOLDER}\TRANSLATORS"
;	File "${PROGRAM_BIN_FOLDER}\Install"
	File "${PROGRAM_BIN_FOLDER}\eyecu.ico"
	File "${PROGRAM_BIN_FOLDER}\eyecu.icns"
	; Binaries
	File "${PROGRAM_BIN_FOLDER}\eyecu.exe"
	File "${PROGRAM_BIN_FOLDER}\eyecuutils.dll"
	; OpenSSL libraries
	File "${PROGRAM_BIN_FOLDER}\libeay32.dll"
	File "${PROGRAM_BIN_FOLDER}\ssleay32.dll"
	; List of plugins that SHOULD NOT be disabled
	SetOutPath $INSTDIR\plugins
	File "${PROGRAM_BIN_FOLDER}\plugins\accountmanager.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\chatmessagehandler.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\connectionmanager.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\defaultconnection.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\mainwindow.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\messageprocessor.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\messagestyles.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\messagewidgets.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\normalmessagehandler.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\notifications.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\optionsmanager.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\presence.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\roster.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\rosterchanger.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\rostersmodel.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\rostersview.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\saslauth.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\simplemessagestyle.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\stanzaprocessor.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\starttls.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\statuschanger.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\statusicons.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\traymanager.dll"
	File "${PROGRAM_BIN_FOLDER}\plugins\xmppstreams.dll"
	;------------------------------------
	; Resources
	SetOutPath $INSTDIR\resources\menuicons\shared
	;accountmanager: account.def.xml
;	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\account.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\accountmanager.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\account.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\accountlist.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\accountchange.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\accountmove.png"
;	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\autostatus.png"
	;chatmessagehandler: chatmessagehandler.def.xml
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\chatmessagehandler.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\chatmessagehandlermessage.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\chatmessagehandlerclearchat.png"
	;chatmessagehandler\sounds
	SetOutPath $INSTDIR\resources\sounds\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\chatmessagehandler.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\message.wav"
	;connectionmanager: chatmessagehandler.def.xml
	SetOutPath $INSTDIR\resources\menuicons\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\chatmessagehandler.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\chatmessagehandlermessage.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\chatmessagehandlerclearchat.png"
	;connectionmanager: connection.def.xml
	SetOutPath $INSTDIR\resources\menuicons\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\connection.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\connectionencrypted.png"  
	;mainwindow: mainwindow.def.xml
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mainwindow.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\eyecu.svg"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mainwindowlogo16.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mainwindowlogo20.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mainwindowlogo24.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mainwindowlogo32.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mainwindowlogo40.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mainwindowlogo48.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mainwindowlogo64.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mainwindowlogo96.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mainwindowlogo128.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mainwindowquit.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mainwindowmenu.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mainwindowshowroster.png"
	;messagestyles.dll: messagestyles.def.xml
;	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\messagestyles.def.xml"
;	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\messagestyles.png"
	;messagewidgets: messagewidgets.def.xml
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\messagewidgets.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\messagewidgetssend.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\messagewidgetstabmenu.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\messagewidgetsquote.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\messagewidgetsselect.png"
	;normalmessagehandler: normalmessagehandler.def.xml
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\normalmessagehandler.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\normalmessagehandlermessage.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\normalmessagehandlersend.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\normalmessagehandlerreply.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\normalmessagehandlerforward.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\normalmessagehandlernext.png"
	;normalmessagehandler\sound: normalmessagehandler.def.xml
	SetOutPath $INSTDIR\resources\sounds\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\normalmessagehandler.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\message.wav"
;===========================================================================================	
	;notifications
	SetOutPath $INSTDIR\resources\menuicons\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\notifications.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\notifications.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\notificationsactivateall.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\notificationsremoveall.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\notificationssoundon.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\notificationssoundoff.png"
;===========================================================================================
	;optionsmanager: options.def.xml
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\options.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\optionsdialog.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\optionsprofile.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\optionsprofiles.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\optionseditprofiles.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\optionsappearance.png"
	;pluginmanager: pluginmanager.def.xml
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\pluginmanager.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\pluginmanagersetup.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\pluginmanagerabout.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\pluginmanageraboutqt.png"
	;rosterchanger: rchanger.def.xml
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchanger.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangeraddcontact.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangergroup.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangerrootgroup.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangerthisgroup.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangercopygroup.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangermovegroup.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangercreategroup.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangerrename.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangerremovegroup.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangerremovefromgroup.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangerremovecontact.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangerremovecontacts.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangersubscription.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangersubscribe.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangerunsubscribe.png"
	;---not this files in dir ----
	;File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangersubscriptionsend.png"
	;File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangersubscriptionrequest.png"
	;File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangersubscriptionremove.png"
	;File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rchangersubscriptionrefuse.png"

	;rosterchanger\sounds: rchanger.def.xml
	SetOutPath $INSTDIR\resources\sounds\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\rchanger.def.xml"  

	;rostersview: rosterview.def.xml
	SetOutPath $INSTDIR\resources\menuicons\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rosterview.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rosterviewcontacts.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rosterviewoptions.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rosterviewshowoffline.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rosterviewhideoffline.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rosterviewclipboard.png"
	;statuschanger.dll: schanger.def.xml
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\schanger.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\schangermodifystatus.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\schangereditstatuses.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\schangerconnecting.png"
	;statuschanger\sounds: schanger.def.xml
	SetOutPath $INSTDIR\resources\sounds\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\schanger.def.xml"
	;statusicons: statusicons.def.xml
;	SetOutPath $INSTDIR\resources\menuicons\shared
;	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\statusicons.def.xml"
;	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\statusiconsoptions.png"

	;--------Not Resource -------------------------------
	;defaultconnection.dll		; ??
	;messageprocessor.dll		;??
	;presence.dll				;??
	;roster.dll					;??
	;rostersmodel.dll			;??
	;saslauth.dll				;??
	;stanzaprocessor.dll		;??
	;starttls.dll				;??
	;traymanager.dll
	;xmppstreams.dll			;??

	;---------S O U N D ------------------------------
	; sound.def.xml
	SetOutPath $INSTDIR\resources\sounds\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\sound.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\default.wav"
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\message.wav"
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\auth.wav"
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\error.wav"
	;---------------------------------------
	;simplemessagestyle
	SetOutPath $INSTDIR\resources\simplemessagestyles
	File /r "${PROGRAM_BIN_FOLDER}\resources\simplemessagestyles\*.*"
	;statusicons
	SetOutPath $INSTDIR\resources\statusicons
	File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\*.*"
	;statusicons
	SetOutPath $INSTDIR\resources\serviceicons
	File /r "${PROGRAM_BIN_FOLDER}\resources\serviceicons\*.*"
	;==============================================================
	; Resources EYECU:
	SetOutPath $INSTDIR\resources\menuicons\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\eyecuicon.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\jingle.png" ;
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\xhtml.png" ;
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\voip.png" ;
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\voip_active.png" ;
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\receipts.png" ;
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\tune.png" ;
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\bell.gif" ;
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\tracker.png" ;
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\lastfm.png" ;
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mapstreetview.png" ;
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\streetman.png" ;
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\viewcenter.png" ;
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\key16.png" ;
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\clipboard_copy_gr.png" ;
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\yes.png" ;
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\no.png" ;
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\weather.png" ;
;	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\positioning.png"
;	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\serialport.png"
;	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\location.png"
;	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\manual.png"
;	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\receipts.png"	
	
	; Resources EYECU-SOUND: eyecu.def.xml
	SetOutPath $INSTDIR\resources\sounds\shared
;NOT	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\eyecu.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\bells.wav"
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\call.wav"
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\floop.wav"
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\plopp.wav"
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\sonar.wav"
	;------ ?????? ------------------------
	SetOutPath $INSTDIR\resources\menuicons\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\edit.def.xml"  
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\edit.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\add.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\delete.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\accept.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\decline.png"
	;====================================================================
SectionEnd

/*
+TS-Base
*/
Section "-RussianBase" RussianBase
SectionIn  1 2 3 
	SetOutPath $INSTDIR\translations\ru
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\eyecu.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\eyecuutils.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\accountmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\chatmessagehandler.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\connectionmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\defaultconnection.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\mainwindow.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\messageprocessor.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\messagestyles.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\messagewidgets.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\normalmessagehandler.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\notifications.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\optionsmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\presence.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\roster.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\rosterchanger.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\rostersmodel.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\rostersview.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\saslauth.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\simplemessagestyle.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\stanzaprocessor.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\starttls.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\statuschanger.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\statusicons.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\traymanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ru\xmppstreams.qm"	
SectionEnd

Section "-DeutschlandBase" DeutschlandBase
SectionIn  1 2 3 
	SetOutPath $INSTDIR\translations\de
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\eyecu.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\eyecuutils.qm"	
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\accountmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\chatmessagehandler.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\connectionmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\defaultconnection.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\mainwindow.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\messageprocessor.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\messagestyles.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\messagewidgets.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\normalmessagehandler.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\notifications.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\optionsmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\pepmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\presence.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\roster.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\rosterchanger.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\rostersmodel.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\rostersview.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\saslauth.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\simplemessagestyle.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\stanzaprocessor.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\starttls.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\statuschanger.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\statusicons.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\traymanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\de\xmppstreams.qm"
SectionEnd

Section "-DeutchBase" DeutchBase
SectionIn  1 2 3 
	SetOutPath $INSTDIR\translations\nl
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\eyecu.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\eyecuutils.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\accountmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\chatmessagehandler.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\connectionmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\defaultconnection.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\mainwindow.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\messageprocessor.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\messagestyles.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\messagewidgets.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\normalmessagehandler.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\notifications.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\optionsmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\presence.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\roster.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\rosterchanger.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\rostersmodel.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\rostersview.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\saslauth.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\simplemessagestyle.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\stanzaprocessor.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\starttls.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\statuschanger.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\statusicons.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\traymanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\nl\xmppstreams.qm"
SectionEnd

Section "-JapaneseBase" JapaneseBase
SectionIn  1 2 3 
	SetOutPath $INSTDIR\translations\ja
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\eyecu.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\eyecuutils.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\accountmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\chatmessagehandler.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\connectionmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\defaultconnection.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\mainwindow.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\messageprocessor.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\messagestyles.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\messagewidgets.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\normalmessagehandler.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\notifications.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\optionsmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\presence.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\roster.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\rosterchanger.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\rostersmodel.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\rostersview.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\saslauth.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\simplemessagestyle.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\stanzaprocessor.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\starttls.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\statuschanger.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\statusicons.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\traymanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\ja\xmppstreams.qm"
SectionEnd

Section "-PolishBase" PolishBase
SectionIn  1 2 3
	SetOutPath $INSTDIR\translations\pl
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\eyecu.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\eyecuutils.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\accountmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\chatmessagehandler.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\connectionmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\defaultconnection.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\mainwindow.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\messageprocessor.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\messagestyles.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\messagewidgets.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\normalmessagehandler.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\notifications.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\optionsmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\presence.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\roster.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\rosterchanger.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\rostersmodel.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\rostersview.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\saslauth.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\simplemessagestyle.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\stanzaprocessor.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\starttls.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\statuschanger.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\statusicons.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\traymanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\pl\xmppstreams.qm"
SectionEnd

Section "-SpainBase" SpainBase
SectionIn  1 2 3 
	SetOutPath $INSTDIR\translations\es
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\eyecu.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\eyecuutils.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\accountmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\chatmessagehandler.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\connectionmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\defaultconnection.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\mainwindow.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\messageprocessor.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\messagestyles.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\messagewidgets.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\normalmessagehandler.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\notifications.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\optionsmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\presence.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\roster.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\rosterchanger.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\rostersmodel.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\rostersview.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\saslauth.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\simplemessagestyle.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\stanzaprocessor.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\starttls.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\statuschanger.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\statusicons.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\traymanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\es\xmppstreams.qm"
SectionEnd

Section "-UkraineBase" UkraineBase
SectionIn  1 2 3 
	SetOutPath $INSTDIR\translations\uk
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\eyecu.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\eyecuutils.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\accountmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\chatmessagehandler.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\connectionmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\defaultconnection.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\mainwindow.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\messageprocessor.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\messagestyles.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\messagewidgets.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\normalmessagehandler.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\notifications.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\optionsmanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\presence.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\roster.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\rosterchanger.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\rostersmodel.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\rostersview.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\saslauth.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\simplemessagestyle.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\stanzaprocessor.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\starttls.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\statuschanger.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\statusicons.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\traymanager.qm"
	File /r "${PROGRAM_BIN_FOLDER}\translations\uk\xmppstreams.qm"
SectionEnd
