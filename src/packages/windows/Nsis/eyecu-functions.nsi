/* ------------------------------------- */
Function .onInit
	;Language selection dialog
	!insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd 

;---------------------------------------------------------------------
; Add version checking - VERSION "${VERSION}"
!macro READ_INI_FILE SECTION_NB SECTION_NAME 
    Push $0
	Push $1
	Push $2
	ClearErrors
	ReadINIStr $0 $INSTDIR\${Name1}.ini ${SECTION_NAME} Status
	IfErrors 0 +2		; Checks and clears the error flag 
		DetailPrint "READ_INI_FILE-0/ERROR IN $1"	; MessageBox MB_OK 
	IntOp $1 $0 & ${SF_SELECTED}
	${If} $1 == ${SF_SELECTED}		# Disable
		IntOp $0 $0 & ${SF_DESELECTED}
		IntOp $0 $0 | ${SF_RO}
	${Else}							# Enable for install
		IntOp $0 $0 | ${SF_SELECTED}
	${EndIf}
	SectionSetFlags "${SECTION_NB}" $0
	Pop $2
	Pop $1
	Pop $0
!macroEnd
;---------------------------------------------------------------------
!macro WRITE_INI_FILE SECTION_NB SECTION_NAME ; PLUGIN
	Push $0
	Push $1
	Push $2
	ClearErrors
	SectionGetFlags "${SECTION_NB}" $0 
	IntOp $1 $0 & ${SF_RO}
 	${If} $1 == ${SF_RO}
		IntOp $0 $0 & ${SF_RO_INVERS}
		IntOp $0 $0 | ${SF_SELECTED}
	${EndIf} 
	IntOp $1 $0 & ${SF_SELECTED}
	IntOp $SecIsFull $SecIsFull & $1
	IntOp $DelAll $DelAll + $1	
	WriteINIStr $INSTDIR\${Name1}.ini ${SECTION_NAME} "Status" $0
	IfErrors 0 +2		; Checks and clears the error flag 
		DetailPrint "WRITE_INI_FILE-1/ERROR IN $1"
	Pop $2
	Pop $1
	Pop $0	
!macroEnd
;---------------------------------------------------------------------
!macro WRITE_INI_GROUP GROUP_NB GROUP_NAME ; PLUGIN
	Push $0
	Push $1
	Push $2
	ClearErrors
	SectionGetFlags "${GROUP_NB}" $0 
	IntOp $1 $0 & ${SF_RO}
 	${If} $1 == ${SF_RO}
		IntOp $0 $0 & ${SF_RO_INVERS}
		IntOp $0 $0 | ${SF_SELECTED}
	${EndIf} 
	${If} $SecIsFull == 1
		IntOp $0 $0 | ${SF_SELECTED}
		IntOp $0 $0 & ${SF_PSELECTED_INV}
	${EndIf}
	IntOp $0 $0 & ${SF_EXPAND_INVERS}	; Close group section
	WriteINIStr $INSTDIR\${Name1}.ini ${GROUP_NAME} "Status" $0
	IfErrors 0 +2		; Checks and clears the error flag 
		DetailPrint "WRITE_INI_FILE-1/ERROR IN $1"
	Pop $2
	Pop $1
	Pop $0	
!macroEnd
;---------------------------------------------------------------------
!macro IF_SECTION_RO SECTION_NB
 	Push $0
	Push $1
	ClearErrors
	SectionGetFlags "${SECTION_NB}" $0 
	IntOp $1 $0 & ${SF_RO}
	${If} $1 == ${SF_RO}
		IntOp $0 $0 & ${SF_DESELECTED}
		SectionSetFlags "${SECTION_NB}" $0 
	${EndIf} 
	Pop $1
	Pop $0	
!macroEnd
;---------------------------------------------------------------------

###################################
!define ReadSecIniFile 		'!insertmacro READ_INI_FILE'
!define WriteSecIniFile   	'!insertmacro WRITE_INI_FILE'
!define WriteGroupIniFile 	'!insertmacro WRITE_INI_GROUP'
!define IfSecReadOnly 		'!insertmacro IF_SECTION_RO'
###################################


Function ReadIniFile
	StrCpy $DelAll  0
	IfFileExists $INSTDIR\${Name1}.ini +3 +1
		DetailPrint "Function ReadIniFile- File:$INSTDIR\${Name1}.ini Not Found!"
	Goto Done
;--------------------------	
	ReadINIStr $DelAll $INSTDIR\${Name1}.ini "Application" "Numbers Components"
	${ReadSecIniFile} ${AbInstall} AbInstall
	${ReadSecIniFile} ${AbUnInstall} AbUnInstall
	${ReadSecIniFile} ${Base} QtBase
	${ReadSecIniFile} ${Base} Base
	${ReadSecIniFile} ${RussianBase} RussianBase
	${ReadSecIniFile} ${DeutschlandBase} DeutschlandBase
	${ReadSecIniFile} ${DeutchBase} DeutchBase
	${ReadSecIniFile} ${JapaneseBase} JapaneseBase
	${ReadSecIniFile} ${PolishBase} PolishBase
	${ReadSecIniFile} ${SpainBase} SpainBase
	${ReadSecIniFile} ${UkraineBase} UkraineBase
	;----------------------------	
	${ReadSecIniFile} ${Privatestorage} Privatestorage
	${ReadSecIniFile} ${PrivatestorageBase} PrivatestorageBase
	${ReadSecIniFile} ${Bookmarks} Bookmarks
	${ReadSecIniFile} ${Annotations} Annotations
	${ReadSecIniFile} ${Recent_contacts} Recent_contacts
	${ReadSecIniFile} ${Poi} Poi
	;----------------------------
	${ReadSecIniFile} ${File_transfer} File_transfer
	${ReadSecIniFile} ${FileTransferBase} FileTransferBase
	${ReadSecIniFile} ${Socks5} Socks5
	${ReadSecIniFile} ${In_band} In_band
	;----------------------------
	${ReadSecIniFile} ${Vcard} Vcard
	${ReadSecIniFile} ${Jabber_search} Jabber_search
	${ReadSecIniFile} ${Roster_search} Roster_search
	${ReadSecIniFile} ${MUC} MUC
	${ReadSecIniFile} ${Metacontack} Metacontack
	;----------------------------
	${ReadSecIniFile} ${Data_Form} Data_Form
	${ReadSecIniFile} ${Data_Form_Base} Data_Form_Base
	${ReadSecIniFile} ${Capcha} Capcha
	${ReadSecIniFile} ${Registration} Registration
	${ReadSecIniFile} ${Ad_hoc_commands} Ad_hoc_commands
	${ReadSecIniFile} ${Ad_hoc_commands_Base} Ad_hoc_commands_Base
	${ReadSecIniFile} ${Remote} Remote
	;----------------------------
	${ReadSecIniFile} ${Console} Console
	${ReadSecIniFile} ${Console_Base} Console_Base
	${ReadSecIniFile} ${Compress} Compress
	;----------------------------
	${ReadSecIniFile} ${Message_archive} Message_archive
	${ReadSecIniFile} ${Message_archive_Base} Message_archive_Base
	${ReadSecIniFile} ${File_archive} File_archive
	${ReadSecIniFile} ${Server_archive} Server_archive
	;----------------------------
	${ReadSecIniFile} ${Discovery} Discovery
	${ReadSecIniFile} ${Duplicate_messages} Duplicate_messages
	${ReadSecIniFile} ${Gateways} Gateways
	${ReadSecIniFile} ${Bob} Bob
	${ReadSecIniFile} ${OOB} OOB
	${ReadSecIniFile} ${Autostatus} Autostatus
	${ReadSecIniFile} ${Chat_states} Chat_states
	${ReadSecIniFile} ${Privacy_lists} Privacy_lists
	${ReadSecIniFile} ${Roster_exchange} Roster_exchange
	${ReadSecIniFile} ${Client_info} Client_info
	${ReadSecIniFile} ${Aut_via_iq} Aut_via_iq
	${ReadSecIniFile} ${Short_cut} Short_cut
	${ReadSecIniFile} ${Avatar} Avatar
	${ReadSecIniFile} ${Birthday} Birthday
	;----------------------------
	${ReadSecIniFile} ${URL} URL
	${ReadSecIniFile} ${URL_Base} URL_Base
	${ReadSecIniFile} ${Bob_URL_Handler} Bob_URL_Handler
	;----------------------------
	${ReadSecIniFile} ${Location} Location
	${ReadSecIniFile} ${Location_Base} Location_Base
	${ReadSecIniFile} ${Positioning} Positioning
	${ReadSecIniFile} ${Positioning_Base} Positioning_Base
	${ReadSecIniFile} ${Manual} Manual
	${ReadSecIniFile} ${Serialport} Serialport
	${ReadSecIniFile} ${MetodIP_Base} MetodIP_Base
	${ReadSecIniFile} ${ProviderIPGeoip} ProviderIPGeoip
	${ReadSecIniFile} ${ContactNotifies} ContactNotifies
	;----------------------------
	${ReadSecIniFile} ${Abbreviations} Abbreviations
	${ReadSecIniFile} ${NickName} NickName
	${ReadSecIniFile} ${XHTML_IM} XHTML_IM
	${ReadSecIniFile} ${XMPP_URI} XMPP_URI
	${ReadSecIniFile} ${Statistics} Statistics
	;----------------------------
	${ReadSecIniFile} ${Message_style} Message_style
	${ReadSecIniFile} ${Adiummessagestyle} Adiummessagestyle
	${ReadSecIniFile} ${Adiummessagestyle_Base} Adiummessagestyle_Base
	${ReadSecIniFile} ${Renkoo} Renkoo
	${ReadSecIniFile} ${yMouse} yMouse
	;----------------------------
	${ReadSecIniFile} ${Status_icons} Status_icons
	${ReadSecIniFile} ${Status_icons_Base} Status_icons_Base
	${ReadSecIniFile} ${Aim} Aim
	${ReadSecIniFile} ${Bot} Bot
	${ReadSecIniFile} ${Car} Car
	${ReadSecIniFile} ${Conference} Conference
	${ReadSecIniFile} ${Facebook} Facebook
	${ReadSecIniFile} ${Gadu_gadu} Gadu_gadu)
	${ReadSecIniFile} ${Google_talk} Google_talk
	${ReadSecIniFile} ${ICQ} ICQ
	${ReadSecIniFile} ${Livejornal} Livejornal
	${ReadSecIniFile} ${MailRuIm} MailRuIm
	${ReadSecIniFile} ${MSN} MSN)
	${ReadSecIniFile} ${Odnoklassniki} Odnoklassniki
	${ReadSecIniFile} ${RSS} RSS
	${ReadSecIniFile} ${Skype} Skype
	${ReadSecIniFile} ${SMS} SMS
	${ReadSecIniFile} ${SMTP} SMTP
	${ReadSecIniFile} ${Twitter} Twitter
	${ReadSecIniFile} ${vKontakte} vKontakte
	${ReadSecIniFile} ${Weather} Weather
	${ReadSecIniFile} ${YahooIm} YahooIm
	${ReadSecIniFile} ${YaOnline} YaOnline
	;----------------------------
	${ReadSecIniFile} ${Emoticons} Emoticons
	${ReadSecIniFile} ${Emoticons_Base} Emoticons_Base
	${ReadSecIniFile} ${EI_Default} EI_Default
	${ReadSecIniFile} ${EI_Blobs_purple} EI_Blobs_purple
	;----------------------------
	${ReadSecIniFile} ${Client_icone} Client_icone
	;----------------------------
	${ReadSecIniFile} ${Spellchecker} Spellchecker
	${ReadSecIniFile} ${Spellchecker_Base} Spellchecker_Base
	${ReadSecIniFile} ${Lng_Russian} Lng_Russian
	${ReadSecIniFile} ${Lng_English_US} Lng_English_US
	${ReadSecIniFile} ${Lng_English_AU} Lng_English_AU
	${ReadSecIniFile} ${Lng_English_GB} Lng_English_GB
	${ReadSecIniFile} ${Lng_English_SA} Lng_English_SA
	${ReadSecIniFile} ${Lng_Ukraine} Lng_Ukraine
	${ReadSecIniFile} ${Lng_Deutschland} Lng_Deutschland
	${ReadSecIniFile} ${Lng_Polish} Lng_Polish
	;----------------------------
	${ReadSecIniFile} ${Translations} Translations
	${ReadSecIniFile} ${Russian} Russian
	${ReadSecIniFile} ${Deutschland} Deutschland
	${ReadSecIniFile} ${Deutch} Deutch
	${ReadSecIniFile} ${Japanese} Japanese
	${ReadSecIniFile} ${Polish} Polish
	${ReadSecIniFile} ${Spain} Spain
	${ReadSecIniFile} ${Ukraine} Ukraine
	;----------------------------
	${ReadSecIniFile} ${Doc} Doc)
;----Step-2 -------
	${ReadSecIniFile} ${PersonalEvent} PersonalEvent
	${ReadSecIniFile} ${Pepmanager} Pepmanager 
	${ReadSecIniFile} ${Activity} Activity 
	${ReadSecIniFile} ${Mood} Mood
	${ReadSecIniFile} ${Tune} Tune
	${ReadSecIniFile} ${TuneMain} TuneMain
	${ReadSecIniFile} ${TuneInfo} TuneInfo
	${ReadSecIniFile} ${TuneListeners} TuneListeners 
	${ReadSecIniFile} ${WINAMP} WINAMP
	${ReadSecIniFile} ${AIMP} AIMP 
	${ReadSecIniFile} ${FileSource} FileSource 
;---------------------------
	${ReadSecIniFile} ${FFMpeg} FFMpeg
	${ReadSecIniFile} ${MMplayer} MMplayer
	${ReadSecIniFile} ${FFMpegDll} FFMpegDll
;---------------------------
	${ReadSecIniFile} ${Attention} Attention
	${ReadSecIniFile} ${Receipts} Receipts
;-----------------------------
	${ReadSecIniFile} ${Map} Map
	${ReadSecIniFile} ${MapMain} MapMain
	${ReadSecIniFile} ${Sources} Sources
	${ReadSecIniFile} ${Google} Google
	${ReadSecIniFile} ${DGIS} DGIS
	${ReadSecIniFile} ${Bing} Bing
	${ReadSecIniFile} ${Esri} Esri
	${ReadSecIniFile} ${Here} Here
	${ReadSecIniFile} ${Kosmosnimki} Kosmosnimki
	${ReadSecIniFile} ${MailRu} MailRu
	${ReadSecIniFile} ${Megafon} Megafon
	${ReadSecIniFile} ${Navitel} Navitel
	${ReadSecIniFile} ${Navteq} Navteq
	${ReadSecIniFile} ${OpenStreetMap} OpenStreetMap
	${ReadSecIniFile} ${PROGOROD} PROGOROD
	${ReadSecIniFile} ${RosReestr} RosReestr
	${ReadSecIniFile} ${RuMap} RuMap
	${ReadSecIniFile} ${Vi_Tel} Vi_Tel
	${ReadSecIniFile} ${Wikimapia} Wikimapia
	${ReadSecIniFile} ${Yahoo} Yahoo
	${ReadSecIniFile} ${Yandex} Yandex
	${ReadSecIniFile} ${MapContacts} MapContacts
	${ReadSecIniFile} ${MapContactsMain} MapContactsMain
	${ReadSecIniFile} ${MapMessage} MapMessage
	${ReadSecIniFile} ${Magnifier} Magnifier
;------------------------------
	${ReadSecIniFile} ${MapSearch} MapSearch
	${ReadSecIniFile} ${MapSearchMain} MapSearchMain
	${ReadSecIniFile} ${SearchFromGoogle} SearchFromGoogle
	${ReadSecIniFile} ${SearchOpenStreetMap} SearchOpenStreetMap
	${ReadSecIniFile} ${SearchFromDGIS} SearchFromDGIS
	${ReadSecIniFile} ${SearchFromYandex} SearchFromYandex
	${ReadSecIniFile} ${SearchFromHere} SearchFromHere
	${ReadSecIniFile} ${SearchNavitel} SearchNavitel
;-------------------------------
	${ReadSecIniFile} ${MapStreetView} MapStreetView
	${ReadSecIniFile} ${MapStreetViewMain} MapStreetViewMain
	${ReadSecIniFile} ${StreetProvGoogle} StreetProvGoogle
	
	${ReadSecIniFile} ${MapPlaceView} MapPlaceView
	${ReadSecIniFile} ${MapPlaceViewMain} MapPlaceViewMain
	${ReadSecIniFile} ${PlaceViewProvGoogle} PlaceViewProvGoogle
	${ReadSecIniFile} ${SDK} SDK
	${ReadSecIniFile} ${Wizards} Wizards
	${ReadSecIniFile} ${WizardsRes} WizardsRes
	${ReadSecIniFile} ${Wizardaccount} Wizardaccount
	${ReadSecIniFile} ${Wizardtransport} Wizardtransport
;-------------------------------

Done:
FunctionEnd 

;----------------------------------------------------------------------
Function ComponentsSelected
	SetOutPath $INSTDIR
	WriteINIStr $INSTDIR\${Name1}.ini "Application" "Name" ${Name1} 
	WriteINIStr $INSTDIR\${Name1}.ini "Application" "Version" "${PROGRAM_VERSION}" 
	StrCpy $DelAll  0
	WriteINIStr $INSTDIR\${Name1}.ini "Application" "Numbers Components" $DelAll 
;----------------------------
	${WriteSecIniFile} ${AbInstall} AbInstall
	${WriteSecIniFile} ${AbUnInstall} AbUnInstall
	${WriteSecIniFile} ${Base} QtBase
	${WriteSecIniFile} ${Base} Base
	${WriteSecIniFile} ${RussianBase} RussianBase
	${WriteSecIniFile} ${DeutschlandBase} DeutschlandBase
	${WriteSecIniFile} ${DeutchBase} DeutchBase
	${WriteSecIniFile} ${JapaneseBase} JapaneseBase
	${WriteSecIniFile} ${PolishBase} PolishBase
	${WriteSecIniFile} ${SpainBase} SpainBase
	${WriteSecIniFile} ${UkraineBase} UkraineBase	
	;----------------------------	
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${Bookmarks} Bookmarks
	${WriteSecIniFile} ${Annotations} Annotations
	${WriteSecIniFile} ${Recent_contacts} Recent_contacts
	${WriteSecIniFile} ${Poi} Poi
	${WriteSecIniFile} ${PrivatestorageBase} PrivatestorageBase
	${WriteGroupIniFile} ${Privatestorage} Privatestorage
	;----------------------------
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${Socks5} Socks5
	${WriteSecIniFile} ${In_band} In_band
	${WriteSecIniFile} ${FileTransferBase} FileTransferBase
	${WriteGroupIniFile} ${File_transfer} File_transfer
	;----------------------------
	${WriteSecIniFile} ${Vcard} Vcard
	${WriteSecIniFile} ${Jabber_search} Jabber_search
	${WriteSecIniFile} ${Roster_search} Roster_search
	${WriteSecIniFile} ${MUC} MUC
	${WriteSecIniFile} ${Metacontack} Metacontack
	;----------------------------
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${Remote} Remote
	${WriteSecIniFile} ${Ad_hoc_commands_Base} Ad_hoc_commands_Base
	${WriteGroupIniFile} ${Ad_hoc_commands} Ad_hoc_commands
	${WriteSecIniFile} ${Capcha} Capcha
	${WriteSecIniFile} ${Registration} Registration
	${WriteSecIniFile} ${Data_Form_Base} Data_Form_Base
	${WriteGroupIniFile} ${Data_Form} Data_Form
	;----------------------------
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${Compress} Compress
	${WriteSecIniFile} ${Console_Base} Console_Base
	${WriteGroupIniFile} ${Console} Console
	;----------------------------
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${File_archive} File_archive
	${WriteSecIniFile} ${Server_archive} Server_archive
	${WriteSecIniFile} ${Message_archive_Base} Message_archive_Base
	${WriteGroupIniFile} ${Message_archive} Message_archive
	;----------------------------
	${WriteSecIniFile} ${Discovery} Discovery
	${WriteSecIniFile} ${Duplicate_messages} Duplicate_messages
	${WriteSecIniFile} ${Gateways} Gateways
	${WriteSecIniFile} ${Bob} Bob
	${WriteSecIniFile} ${OOB} OOB
	${WriteSecIniFile} ${Autostatus} Autostatus
	${WriteSecIniFile} ${Chat_states} Chat_states
	${WriteSecIniFile} ${Privacy_lists} Privacy_lists
	${WriteSecIniFile} ${Roster_exchange} Roster_exchange
	${WriteSecIniFile} ${Client_info} Client_info
	${WriteSecIniFile} ${Aut_via_iq} Aut_via_iq
	${WriteSecIniFile} ${Short_cut} Short_cut
	${WriteSecIniFile} ${Avatar} Avatar
	${WriteSecIniFile} ${Birthday} Birthday
	;----------------------------
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${Bob_URL_Handler} Bob_URL_Handler
	${WriteSecIniFile} ${URL_Base} URL_Base
	${WriteGroupIniFile} ${URL} URL
	;----------------------------
	StrCpy $SecIsFull 1	
	${WriteSecIniFile} ${ContactNotifies} ContactNotifies
	${WriteSecIniFile} ${MetodIP_Base} MetodIP_Base
	${WriteSecIniFile} ${ProviderIPGeoip} ProviderIPGeoip
	${WriteGroupIniFile} ${MetodIP} MetodIP
	${WriteSecIniFile} ${Serialport} Serialport
	${WriteSecIniFile} ${Manual} Manual
	${WriteSecIniFile} ${Positioning_Base} Positioning_Base
	${WriteGroupIniFile} ${Positioning} Positioning
	${WriteSecIniFile} ${Location_Base} Location_Base
	${WriteGroupIniFile} ${Location} Location
	;----------------------------
	${WriteSecIniFile} ${Abbreviations} Abbreviations
	${WriteSecIniFile} ${NickName} NickName
	${WriteSecIniFile} ${XHTML_IM} XHTML_IM
	${WriteSecIniFile} ${XMPP_URI} XMPP_URI
	${WriteSecIniFile} ${Statistics} Statistics
	;----------------------------
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${Renkoo} Renkoo
	${WriteSecIniFile} ${yMouse} yMouse
	${WriteSecIniFile} ${Adiummessagestyle_Base} Adiummessagestyle_Base
	${WriteGroupIniFile} ${Adiummessagestyle} Adiummessagestyle
	${WriteGroupIniFile} ${Message_style} Message_style
	;----------------------------
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${Aim} Aim
	${WriteSecIniFile} ${Bot} Bot
	${WriteSecIniFile} ${Car} Car
	${WriteSecIniFile} ${Conference} Conference
	${WriteSecIniFile} ${Facebook} Facebook
	${WriteSecIniFile} ${Gadu_gadu} Gadu_gadu)
	${WriteSecIniFile} ${Google_talk} Google_talk
	${WriteSecIniFile} ${ICQ} ICQ
	${WriteSecIniFile} ${Livejornal} Livejornal
	${WriteSecIniFile} ${MailRuIm} MailRuIm
	${WriteSecIniFile} ${MSN} MSN)
	${WriteSecIniFile} ${Odnoklassniki} Odnoklassniki
	${WriteSecIniFile} ${RSS} RSS
	${WriteSecIniFile} ${Skype} Skype
	${WriteSecIniFile} ${SMS} SMS
	${WriteSecIniFile} ${SMTP} SMTP
	${WriteSecIniFile} ${Twitter} Twitter
	${WriteSecIniFile} ${vKontakte} vKontakte
	${WriteSecIniFile} ${Weather} Weather
	${WriteSecIniFile} ${YahooIm} YahooIm
	${WriteSecIniFile} ${YaOnline} YaOnline
	${WriteSecIniFile} ${Status_icons_Base} Status_icons_Base
	${WriteGroupIniFile} ${Status_icons} Status_icons
	;----------------------------
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${EI_Default} EI_Default
	${WriteSecIniFile} ${EI_Blobs_purple} EI_Blobs_purple
	${WriteSecIniFile} ${Emoticons_Base} Emoticons_Base
	${WriteGroupIniFile} ${Emoticons} Emoticons
	;----------------------------
	${WriteSecIniFile} ${Client_icone} Client_icone
	;----------------------------
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${Lng_Russian} Lng_Russian
	${WriteSecIniFile} ${Lng_English_US} Lng_English_US
	${WriteSecIniFile} ${Lng_English_AU} Lng_English_AU
	${WriteSecIniFile} ${Lng_English_GB} Lng_English_GB
	${WriteSecIniFile} ${Lng_English_SA} Lng_English_SA
	${WriteSecIniFile} ${Lng_Ukraine} Lng_Ukraine
	${WriteSecIniFile} ${Lng_Deutschland} Lng_Deutschland
	${WriteSecIniFile} ${Lng_Polish} Lng_Polish
	${WriteSecIniFile} ${Spellchecker_Base} Spellchecker_Base
	${WriteGroupIniFile} ${Spellchecker} Spellchecker
	;----------------------------
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${Russian} Russian
	${WriteSecIniFile} ${Deutschland} Deutschland
	${WriteSecIniFile} ${Deutch} Deutch
	${WriteSecIniFile} ${Japanese} Japanese
	${WriteSecIniFile} ${Polish} Polish
	${WriteSecIniFile} ${Spain} Spain
	${WriteSecIniFile} ${Ukraine} Ukraine
	${WriteGroupIniFile} ${Translations} Translations
	;----------------------------
	${WriteSecIniFile} ${Doc} Doc)
	;----------------------------
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${WINAMP} WINAMP
	${WriteSecIniFile} ${AIMP} AIMP
	${WriteSecIniFile} ${FileSource} FileSource
	${WriteGroupIniFile} ${TuneListeners} TuneListeners
	${WriteSecIniFile} ${TuneInfo} TuneInfo
	${WriteSecIniFile} ${TuneMain} TuneMain
	${WriteGroupIniFile} ${Tune} Tune
	${WriteSecIniFile} ${Activity} Activity
	${WriteSecIniFile} ${Mood} Mood
	${WriteSecIniFile} ${Pepmanager} Pepmanager 
	${WriteGroupIniFile} ${PersonalEvent} PersonalEvent
;----------------------------
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${FFMpegDll} FFMpegDll
	${WriteSecIniFile} ${MMplayer} MMplayer
	${WriteGroupIniFile} ${FFMpeg} FFMpeg	
;----------------------------
	${WriteSecIniFile} ${Attention} Attention
	${WriteSecIniFile} ${Receipts} Receipts
;----------------------------
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${Google} Google
	${WriteSecIniFile} ${DGIS} DGIS
	${WriteSecIniFile} ${Bing} Bing
	${WriteSecIniFile} ${Esri} Esri
	${WriteSecIniFile} ${Here} Here
	${WriteSecIniFile} ${Kosmosnimki} Kosmosnimki
	${WriteSecIniFile} ${MailRu} MailRu
	${WriteSecIniFile} ${Megafon} Megafon
	${WriteSecIniFile} ${Navitel} Navitel
	${WriteSecIniFile} ${Navteq} Navteq
	${WriteSecIniFile} ${OpenStreetMap} OpenStreetMap
	${WriteSecIniFile} ${PROGOROD} PROGOROD
	${WriteSecIniFile} ${RosReestr} RosReestr
	${WriteSecIniFile} ${RuMap} RuMap
	${WriteSecIniFile} ${Vi_Tel} Vi_Tel
	${WriteSecIniFile} ${Wikimapia} Wikimapia
	${WriteSecIniFile} ${Yahoo} Yahoo
	${WriteSecIniFile} ${Yandex} Yandex
	${WriteGroupIniFile} ${Sources} Sources
	${WriteSecIniFile} ${MapMessage} MapMessage
	${WriteSecIniFile} ${MapContactsMain} MapContactsMain
	${WriteGroupIniFile} ${MapContacts} MapContacts
	${WriteSecIniFile} ${Magnifier} Magnifier
	${WriteSecIniFile} ${MapMain} MapMain
	${WriteGroupIniFile} ${Map} Map
;----------------------------
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${SearchFromGoogle} SearchFromGoogle
	${WriteSecIniFile} ${SearchOpenStreetMap} SearchOpenStreetMap
	${WriteSecIniFile} ${SearchFromDGIS} SearchFromDGIS
	${WriteSecIniFile} ${SearchFromYandex} SearchFromYandex
	${WriteSecIniFile} ${SearchFromHere} SearchFromHere
	${WriteSecIniFile} ${SearchNavitel} SearchNavitel
	${WriteSecIniFile} ${MapSearchMain} MapSearchMain
	${WriteGroupIniFile} ${MapSearch} MapSearch
;----------------------------
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${StreetProvGoogle} StreetProvGoogle
	${WriteSecIniFile} ${MapStreetViewMain} MapStreetViewMain
	${WriteGroupIniFile} ${MapStreetView} MapStreetView
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${PlaceViewProvGoogle} PlaceViewProvGoogle
	${WriteSecIniFile} ${MapPlaceViewMain} MapPlaceViewMain
	${WriteGroupIniFile} ${MapPlaceView} MapPlaceView
	StrCpy $SecIsFull 1
	${WriteSecIniFile} ${Wizardaccount} Wizardaccount
	${WriteSecIniFile} ${Wizardtransport} Wizardtransport
	${WriteSecIniFile} ${WizardsRes} WizardsRes
	${WriteGroupIniFile} ${Wizards} Wizards
	${WriteSecIniFile} ${SDK} SDK
;---------------------------
	;---Save change to disk ------
	WriteINIStr $INSTDIR\${Name1}.ini "Application" "Numbers Components" $DelAll
	FlushINI $INSTDIR\${Name1}.ini
	Delete $TEMP\${Name1}.ini
FunctionEnd

;----------------------------------------------------------------------
Function CheckROSection
	${IfSecReadOnly} ${AbInstall}
	${IfSecReadOnly} ${AbUnInstall}
	${IfSecReadOnly} ${QtBase}
	${IfSecReadOnly} ${Base}
	${IfSecReadOnly} ${RussianBase}
	${IfSecReadOnly} ${DeutschlandBase}
	${IfSecReadOnly} ${DeutchBase}
	${IfSecReadOnly} ${JapaneseBase}
	${IfSecReadOnly} ${PolishBase}
	${IfSecReadOnly} ${SpainBase}
	${IfSecReadOnly} ${UkraineBase}
	;----------------------------	
	;${IfSecReadOnly} ${Privatestorage}
	${IfSecReadOnly} ${PrivatestorageBase}
	${IfSecReadOnly} ${Bookmarks}
	${IfSecReadOnly} ${Annotations}
	${IfSecReadOnly} ${Recent_contacts}
	${IfSecReadOnly} ${Poi}
	;${IfSecReadOnly} ${File_transfer}
	${IfSecReadOnly} ${FileTransferBase}
	${IfSecReadOnly} ${Socks5}
	${IfSecReadOnly} ${In_band}
	${IfSecReadOnly} ${Vcard}
	${IfSecReadOnly} ${Jabber_search}
	${IfSecReadOnly} ${Roster_search}
	${IfSecReadOnly} ${MUC}
	${IfSecReadOnly} ${Metacontack}
	;${IfSecReadOnly} ${Data_Form}
	${IfSecReadOnly} ${Data_Form_Base}
	${IfSecReadOnly} ${Capcha}
	${IfSecReadOnly} ${Registration}
	;${IfSecReadOnly} ${Ad_hoc_commands}
	${IfSecReadOnly} ${Ad_hoc_commands_Base}
	${IfSecReadOnly} ${Remote}
	;${IfSecReadOnly} ${Console}
	${IfSecReadOnly} ${Console_Base}
	${IfSecReadOnly} ${Compress}
	;${IfSecReadOnly} ${Message_archive}
	${IfSecReadOnly} ${Message_archive_Base}
	${IfSecReadOnly} ${File_archive}
	${IfSecReadOnly} ${Server_archive}
	${IfSecReadOnly} ${Discovery}
	${IfSecReadOnly} ${Duplicate_messages}
	${IfSecReadOnly} ${Gateways}
	${IfSecReadOnly} ${Bob}
	${IfSecReadOnly} ${OOB}
	${IfSecReadOnly} ${Autostatus}
	${IfSecReadOnly} ${Chat_states}
	${IfSecReadOnly} ${Privacy_lists}
	${IfSecReadOnly} ${Roster_exchange}
	${IfSecReadOnly} ${Client_info}
	${IfSecReadOnly} ${Aut_via_iq}
	${IfSecReadOnly} ${Short_cut}
	${IfSecReadOnly} ${Avatar}
	${IfSecReadOnly} ${Birthday}
	;${IfSecReadOnly} ${URL}
	${IfSecReadOnly} ${URL_Base}
	${IfSecReadOnly} ${Bob_URL_Handler}
	;${IfSecReadOnly} ${Location}
	${IfSecReadOnly} ${Location_Base}
	;${IfSecReadOnly} ${Positioning}
	${IfSecReadOnly} ${Positioning_Base}
	${IfSecReadOnly} ${Manual}
	${IfSecReadOnly} ${Serialport}
	${IfSecReadOnly} ${MetodIP_Base}
	${IfSecReadOnly} ${ProviderIPGeoip}
	${IfSecReadOnly} ${ContactNotifies}
	
	${IfSecReadOnly} ${Abbreviations}
	${IfSecReadOnly} ${NickName}
	${IfSecReadOnly} ${XHTML_IM}
	${IfSecReadOnly} ${XMPP_URI}
	${IfSecReadOnly} ${Statistics}
	;${IfSecReadOnly} ${Message_style}
	;${IfSecReadOnly} ${Adiummessagestyle}
	${IfSecReadOnly} ${Adiummessagestyle_Base}
	${IfSecReadOnly} ${Renkoo}
	${IfSecReadOnly} ${yMouse}
	;${IfSecReadOnly} ${Status_icons}
	${IfSecReadOnly} ${Status_icons_Base}
	${IfSecReadOnly} ${Aim}
	${IfSecReadOnly} ${Bot}
	${IfSecReadOnly} ${Car}
	${IfSecReadOnly} ${Conference}
	${IfSecReadOnly} ${Facebook}
	${IfSecReadOnly} ${Gadu_gadu}
	${IfSecReadOnly} ${Google_talk}
	${IfSecReadOnly} ${ICQ}
	${IfSecReadOnly} ${Livejornal}
	${IfSecReadOnly} ${MailRuIm}
	${IfSecReadOnly} ${MSN}
	${IfSecReadOnly} ${Odnoklassniki}
	${IfSecReadOnly} ${RSS}
	${IfSecReadOnly} ${Skype}
	${IfSecReadOnly} ${SMS}
	${IfSecReadOnly} ${SMTP}
	${IfSecReadOnly} ${Twitter}
	${IfSecReadOnly} ${vKontakte}
	${IfSecReadOnly} ${Weather}
	${IfSecReadOnly} ${YahooIm}
	${IfSecReadOnly} ${YaOnline}
	;${IfSecReadOnly} ${Emoticons}
	${IfSecReadOnly} ${Emoticons_Base}
	${IfSecReadOnly} ${EI_Default}
	${IfSecReadOnly} ${EI_Blobs_purple}
	${IfSecReadOnly} ${Client_icone}
	;${IfSecReadOnly} ${Spellchecker}
	${IfSecReadOnly} ${Spellchecker_Base}
	${IfSecReadOnly} ${Lng_Russian}
	${IfSecReadOnly} ${Lng_English_US}
	${IfSecReadOnly} ${Lng_English_AU}
	${IfSecReadOnly} ${Lng_English_GB}
	${IfSecReadOnly} ${Lng_English_SA}
	${IfSecReadOnly} ${Lng_Ukraine}
	${IfSecReadOnly} ${Lng_Deutschland}
	${IfSecReadOnly} ${Lng_Polish}
	${IfSecReadOnly} ${Russian}
	${IfSecReadOnly} ${Deutschland}
	${IfSecReadOnly} ${Deutch}
	${IfSecReadOnly} ${Japanese}
	${IfSecReadOnly} ${Polish}
	${IfSecReadOnly} ${Spain}
	${IfSecReadOnly} ${Ukraine}
	;----------------------------
	${IfSecReadOnly} ${Doc}
	;---------------	
	${IfSecReadOnly} ${WINAMP}
	${IfSecReadOnly} ${AIMP}
	${IfSecReadOnly} ${FileSource}
	;${IfSecReadOnly} ${TuneListeners}
	${IfSecReadOnly} ${TuneInfo}
	${IfSecReadOnly} ${TuneMain}
	;${IfSecReadOnly} ${Tune}
	${IfSecReadOnly} ${Activity}
	${IfSecReadOnly} ${Mood}
	${IfSecReadOnly} ${Pepmanager}
	;${IfSecReadOnly} ${PersonalEvent}
	${IfSecReadOnly} ${FFMpegDll}
	${IfSecReadOnly} ${MMplayer}
	;${IfSecReadOnly} ${FFMpeg}
	${IfSecReadOnly} ${Attention}
	${IfSecReadOnly} ${Receipts}
	${IfSecReadOnly} ${Google}
	${IfSecReadOnly} ${DGIS}
	${IfSecReadOnly} ${Bing}
	${IfSecReadOnly} ${Esri}
	${IfSecReadOnly} ${Here}
	${IfSecReadOnly} ${Kosmosnimki}
	${IfSecReadOnly} ${MailRu}
	${IfSecReadOnly} ${Megafon}
	${IfSecReadOnly} ${Navitel}
	${IfSecReadOnly} ${Navteq}
	${IfSecReadOnly} ${OpenStreetMap}
	${IfSecReadOnly} ${PROGOROD}
	${IfSecReadOnly} ${RosReestr}
	${IfSecReadOnly} ${RuMap}
	${IfSecReadOnly} ${Vi_Tel}
	${IfSecReadOnly} ${Wikimapia}
	${IfSecReadOnly} ${Yahoo}
	${IfSecReadOnly} ${Yandex}
	;${IfSecReadOnly} ${Sources}
	${IfSecReadOnly} ${MapMessage}
	${IfSecReadOnly} ${MapContactsMain}
	;${IfSecReadOnly} ${MapContacts}
	${IfSecReadOnly} ${Magnifier}
	${IfSecReadOnly} ${MapMain}
	;${IfSecReadOnly} ${Map}
	${IfSecReadOnly} ${SearchFromGoogle}
	${IfSecReadOnly} ${SearchOpenStreetMap}
	${IfSecReadOnly} ${SearchFromDGIS}
	${IfSecReadOnly} ${SearchFromYandex}
	${IfSecReadOnly} ${SearchFromHere}
	${IfSecReadOnly} ${SearchNavitel}
	${IfSecReadOnly} ${MapSearchMain}
	;${IfSecReadOnly} ${MapSearch}

	${IfSecReadOnly} ${StreetProvGoogle}
	${IfSecReadOnly} ${MapStreetViewMain}
	;${IfSecReadOnly} ${MapStreetView}
	${IfSecReadOnly} ${PlaceViewProvGoogle}
	${IfSecReadOnly} ${MapPlaceViewMain}
	;${IfSecReadOnly} ${MapPlaceView}
	${IfSecReadOnly} ${Wizardaccount}
	${IfSecReadOnly} ${Wizardtransport}
	${IfSecReadOnly} ${WizardsRes}
	;${IfSecReadOnly} ${Wizards}
	${IfSecReadOnly} ${SDK}
;---------------------------	

FunctionEnd

;---------------------------

Function .onSelChange
;Status in variable $0"
	${If} $0 == -1
		;GetCurInstType $1
		;MessageBox MB_OK ".onSelChange/st = $0  CurInstType= $1 "
		Call CheckROSection
	${EndIf}
FunctionEnd


######################################################################
;----Uninstaller----------------------
######################################################################
Function un.onInit
	SetShellVarContext current  ; param all not used!
	!insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd  

;---------------------------------------------------------------------
!macro READ_UNINI_FILE SECTION_NB SECTION_NAME
	Push $0
	Push $1
	Push $2
	ClearErrors
	ReadINIStr $0 $INSTDIR\${Name1}.ini ${SECTION_NAME} Status
	IfErrors 0 +2		; Checks and clears the error flag 
		DetailPrint "READ_UNINI_FILE-3/ERROR IN $1"
	IntOp $1 $0 & ${SF_SELECTED}
	${If} $1 != ${SF_SELECTED}
		IntOp $0 $0 | ${SF_RO}
	${EndIf}
	SectionSetFlags "${SECTION_NB}" $0
	Pop $2
	Pop $1
	Pop $0	
!macroEnd
;---------------------------------------------------------------------
!macro WRITE_UNINI_FILE SECTION_NB SECTION_NAME
	Push $0
	Push $1
	Push $2
	ClearErrors
	SectionGetFlags "${SECTION_NB}" $0 
	IntOp $1 $0 & ${SF_RO}
	${If} $1 == ${SF_RO}
		IntOp $0 $0 & ${SF_RO_INVERS}
	${Else}
		IntOp $2 $0 & ${SF_SELECTED}
		${If} $2 == ${SF_SELECTED}
			IntOp $0 $0 & ${SF_DESELECTED}
		${Else}
			IntOp $0 $0 | ${SF_SELECTED}
		${EndIf}
		IntOp $UnDelAll $UnDelAll + $2
	${EndIf}
	IntOp $1 $0 & ${SF_SELECTED}
	${If} $1 == ${SF_SELECTED}
		IntOp $SecIsEmpty $SecIsEmpty + 1 ; | $1
	${EndIf}
	WriteINIStr $INSTDIR\${Name1}.ini ${SECTION_NAME} "Status" $0
	IfErrors 0 +2		; Checks and clears the error flag 
		DetailPrint "WRITE_UNINI_FILE-4/ERROR IN $1"
	Pop $2
	Pop $1
	Pop $0
!macroEnd
;---------------------------------------------------------------------
!macro WRITE_UNINI_GROUP GROUP_NB SECTION_NAME
	Push $0
	Push $1
	Push $2
	ClearErrors
	SectionGetFlags "${GROUP_NB}" $0 
	IntOp $1 $0 & ${SF_RO}
	${If} $1 == ${SF_RO}
		IntOp $0 $0 & ${SF_RO_INVERS}
	${ElseIf} $SecIsEmpty == 0
		IntOp $0 $0 & ${SF_DESELECTED}
		IntOp $0 $0 & ${SF_PSELECTED_INV}
	${Else}
		IntOp $0 $0 | ${SF_SELECTED}	
	${EndIf}
	
	IntOp $0 $0 & ${SF_EXPAND_INVERS}	; Close expanding section
	WriteINIStr $INSTDIR\${Name1}.ini ${SECTION_NAME} "Status" $0
	IfErrors 0 +2		; Checks and clears the error flag 
		DetailPrint "WRITE_UNINI_FILE-4/ERROR IN $1"
	Pop $2
	Pop $1
	Pop $0
!macroEnd
;---------------------------------------------------------------------

!macro CHECK_UN_SECTION_RO SECTION_NB
 	Push $0
	Push $1
	Push $2
	ClearErrors
	SectionGetFlags "${SECTION_NB}" $0 
	IntOp $1 $0 & ${SF_RO}
 	${If} $1 == ${SF_RO}
		IntOp $0 $0 & ${SF_DESELECTED}
		SectionSetFlags "${SECTION_NB}" $0 
	${EndIf} 
	Pop $2
	Pop $1
	Pop $0	
!macroEnd

###################################
!define ReadSecUnIniFile 	'!insertmacro READ_UNINI_FILE'
!define WriteSecUnIniFile   '!insertmacro WRITE_UNINI_FILE'
!define WriteGroupUnIniFile '!insertmacro WRITE_UNINI_GROUP'
!define IfUnSecReadOnly 	'!insertmacro CHECK_UN_SECTION_RO'
###################################


Function un.ReadIniFile
	IfFileExists $INSTDIR\${Name1}.ini 0 Done
;-----------------------------	 
	ReadINIStr $DelAll $INSTDIR\${Name1}.ini "Application" "Numbers Components"
	;----------------------------
	${ReadSecUnIniFile} ${AbInstall} AbInstall
	${ReadSecUnIniFile} ${AbUnInstall} AbUnInstall
	${ReadSecUnIniFile} ${QtBase} QtBase
	${ReadSecUnIniFile} ${Base} Base
	${ReadSecUnIniFile} ${RussianBase} RussianBase
	${ReadSecUnIniFile} ${DeutschlandBase} DeutschlandBase
	${ReadSecUnIniFile} ${DeutchBase} DeutchBase
	${ReadSecUnIniFile} ${JapaneseBase} JapaneseBase
	${ReadSecUnIniFile} ${PolishBase} PolishBase
	${ReadSecUnIniFile} ${SpainBase} SpainBase
	${ReadSecUnIniFile} ${UkraineBase} UkraineBase	
	;----------------------------
	${ReadSecUnIniFile} ${Privatestorage} Privatestorage
	${ReadSecUnIniFile} ${PrivatestorageBase} PrivatestorageBase
	${ReadSecUnIniFile} ${Bookmarks} Bookmarks
	${ReadSecUnIniFile} ${Annotations} Annotations
	${ReadSecUnIniFile} ${Recent_contacts} Recent_contacts
	${ReadSecUnIniFile} ${Poi} Poi
	;----------------------------
	${ReadSecUnIniFile} ${File_transfer} File_transfer
	${ReadSecUnIniFile} ${FileTransferBase} FileTransferBase
	${ReadSecUnIniFile} ${Socks5} Socks5
	${ReadSecUnIniFile} ${In_band} In_band
	;----------------------------
	${ReadSecUnIniFile} ${Vcard} Vcard
	${ReadSecUnIniFile} ${Jabber_search} Jabber_search
	${ReadSecUnIniFile} ${Roster_search} Roster_search
	${ReadSecUnIniFile} ${MUC} MUC
	${ReadSecUnIniFile} ${Metacontack} Metacontack
	;----------------------------
	${ReadSecUnIniFile} ${Data_Form} Data_Form
	${ReadSecUnIniFile} ${Data_Form_Base} Data_Form_Base
	${ReadSecUnIniFile} ${Capcha} Capcha
	${ReadSecUnIniFile} ${Registration} Registration
	${ReadSecUnIniFile} ${Ad_hoc_commands} Ad_hoc_commands
	${ReadSecUnIniFile} ${Ad_hoc_commands_Base} Ad_hoc_commands_Base
	${ReadSecUnIniFile} ${Remote} Remote
	;----------------------------
	${ReadSecUnIniFile} ${Console} Console
	${ReadSecUnIniFile} ${Console_Base} Console_Base
	${ReadSecUnIniFile} ${Compress} Compress
	;----------------------------
	${ReadSecUnIniFile} ${Message_archive} Message_archive
	${ReadSecUnIniFile} ${Message_archive_Base} Message_archive_Base
	${ReadSecUnIniFile} ${File_archive} File_archive
	${ReadSecUnIniFile} ${Server_archive} Server_archive
	;----------------------------
	${ReadSecUnIniFile} ${Discovery} Discovery
	${ReadSecUnIniFile} ${Duplicate_messages} Duplicate_messages
	${ReadSecUnIniFile} ${Gateways} Gateways
	${ReadSecUnIniFile} ${Bob} Bob
	${ReadSecUnIniFile} ${OOB} OOB
	${ReadSecUnIniFile} ${Autostatus} Autostatus
	${ReadSecUnIniFile} ${Chat_states} Chat_states
	${ReadSecUnIniFile} ${Privacy_lists} Privacy_lists
	${ReadSecUnIniFile} ${Roster_exchange} Roster_exchange
	${ReadSecUnIniFile} ${Client_info} Client_info
	${ReadSecUnIniFile} ${Aut_via_iq} Aut_via_iq
	${ReadSecUnIniFile} ${Short_cut} Short_cut
	${ReadSecUnIniFile} ${Avatar} Avatar
	${ReadSecUnIniFile} ${Birthday} Birthday
	;----------------------------
	${ReadSecUnIniFile} ${URL} URL
	${ReadSecUnIniFile} ${URL_Base} URL_Base
	${ReadSecUnIniFile} ${Bob_URL_Handler} Bob_URL_Handler
	;----------------------------
	${ReadSecUnIniFile} ${Location} Location
	${ReadSecUnIniFile} ${Location_Base} Location_Base
	${ReadSecUnIniFile} ${Positioning} Positioning
	${ReadSecUnIniFile} ${Positioning_Base} Positioning_Base
	${ReadSecUnIniFile} ${Manual} Manual
	${ReadSecUnIniFile} ${Serialport} Serialport
	${ReadSecUnIniFile} ${MetodIP_Base} MetodIP_Base
	${ReadSecUnIniFile} ${ProviderIPGeoip} ProviderIPGeoip
	${ReadSecUnIniFile} ${ContactNotifies} ContactNotifies	
	;----------------------------
	${ReadSecUnIniFile} ${Abbreviations} Abbreviations
	${ReadSecUnIniFile} ${NickName} NickName
	${ReadSecUnIniFile} ${XHTML_IM} XHTML_IM
	${ReadSecUnIniFile} ${XMPP_URI} XMPP_URI
	${ReadSecUnIniFile} ${Statistics} Statistics
	;----------------------------
	${ReadSecUnIniFile} ${Message_style} Message_style
	${ReadSecUnIniFile} ${Adiummessagestyle} Adiummessagestyle
	${ReadSecUnIniFile} ${Adiummessagestyle_Base} Adiummessagestyle_Base
	${ReadSecUnIniFile} ${Renkoo} Renkoo
	${ReadSecUnIniFile} ${yMouse} yMouse
	;----------------------------
	${ReadSecUnIniFile} ${Status_icons} Status_icons
	${ReadSecUnIniFile} ${Status_icons_Base} Status_icons_Base
	${ReadSecUnIniFile} ${Aim} Aim
	${ReadSecUnIniFile} ${Bot} Bot
	${ReadSecUnIniFile} ${Car} Car
	${ReadSecUnIniFile} ${Conference} Conference
	${ReadSecUnIniFile} ${Facebook} Facebook
	${ReadSecUnIniFile} ${Gadu_gadu} Gadu_gadu)
	${ReadSecUnIniFile} ${Google_talk} Google_talk
	${ReadSecUnIniFile} ${ICQ} ICQ
	${ReadSecUnIniFile} ${Livejornal} Livejornal
	${ReadSecUnIniFile} ${MailRuIm} MailRuIm
	${ReadSecUnIniFile} ${MSN} MSN)
	${ReadSecUnIniFile} ${Odnoklassniki} Odnoklassniki
	${ReadSecUnIniFile} ${RSS} RSS
	${ReadSecUnIniFile} ${Skype} Skype
	${ReadSecUnIniFile} ${SMS} SMS
	${ReadSecUnIniFile} ${SMTP} SMTP
	${ReadSecUnIniFile} ${Twitter} Twitter
	${ReadSecUnIniFile} ${vKontakte} vKontakte
	${ReadSecUnIniFile} ${Weather} Weather
	${ReadSecUnIniFile} ${YahooIm} YahooIm
	${ReadSecUnIniFile} ${YaOnline} YaOnline
	;----------------------------
	${ReadSecUnIniFile} ${Emoticons} Emoticons
	${ReadSecUnIniFile} ${Emoticons_Base} Emoticons_Base
	${ReadSecUnIniFile} ${EI_Default} EI_Default
	${ReadSecUnIniFile} ${EI_Blobs_purple} EI_Blobs_purple
	${ReadSecUnIniFile} ${Client_icone} Client_icone
	;----------------------------
	${ReadSecUnIniFile} ${Spellchecker} Spellchecker
	${ReadSecUnIniFile} ${Spellchecker_Base} Spellchecker_Base
	${ReadSecUnIniFile} ${Lng_Russian} Lng_Russian
	${ReadSecUnIniFile} ${Lng_English_US} Lng_English_US
	${ReadSecUnIniFile} ${Lng_English_AU} Lng_English_AU
	${ReadSecUnIniFile} ${Lng_English_GB} Lng_English_GB
	${ReadSecUnIniFile} ${Lng_English_SA} Lng_English_SA
	${ReadSecUnIniFile} ${Lng_Ukraine} Lng_Ukraine
	${ReadSecUnIniFile} ${Lng_Deutschland} Lng_Deutschland
	${ReadSecUnIniFile} ${Lng_Polish} Lng_Polish
	;----------------------------
	${ReadSecUnIniFile} ${Translations} Translations
	${ReadSecUnIniFile} ${Russian} Russian
	${ReadSecUnIniFile} ${Deutschland} Deutschland
	${ReadSecUnIniFile} ${Deutch} Deutch
	${ReadSecUnIniFile} ${Japanese} Japanese
	${ReadSecUnIniFile} ${Polish} Polish
	${ReadSecUnIniFile} ${Spain} Spain
	${ReadSecUnIniFile} ${Ukraine} Ukraine
	;----------------------------
	${ReadSecUnIniFile} ${Doc} Doc)
;------------------------------ 	
	${ReadSecUnIniFile} ${PersonalEvent} PersonalEvent
	${ReadSecUnIniFile} ${Pepmanager} Pepmanager
	${ReadSecUnIniFile} ${Activity} Activity
	${ReadSecUnIniFile} ${Mood} Mood
	${ReadSecUnIniFile} ${Tune} Tune
	${ReadSecUnIniFile} ${TuneMain} TuneMain
	${ReadSecUnIniFile} ${TuneInfo} TuneInfo 
	${ReadSecUnIniFile} ${TuneListeners} TuneListeners 
	${ReadSecUnIniFile} ${WINAMP} WINAMP
	${ReadSecUnIniFile} ${AIMP} AIMP 
	${ReadSecUnIniFile} ${FileSource} FileSource 
;-----------------------------
	${ReadSecUnIniFile} ${FFMpeg} FFMpeg 
	${ReadSecUnIniFile} ${FFMpegDll} FFMpegDll
	${ReadSecUnIniFile} ${MMplayer} MMplayer
;-----------------------------	
	${ReadSecUnIniFile} ${Attention} Attention	
	${ReadSecUnIniFile} ${Receipts} Receipts
;-----------------------------
	${ReadSecUnIniFile} ${Map} Map
	${ReadSecUnIniFile} ${MapMain} MapMain
	${ReadSecUnIniFile} ${Sources} Sources
	${ReadSecUnIniFile} ${Google} Google
	${ReadSecUnIniFile} ${DGIS} DGIS
	${ReadSecUnIniFile} ${Bing} Bing
	${ReadSecUnIniFile} ${Esri} Esri
	${ReadSecUnIniFile} ${Here} Here
	${ReadSecUnIniFile} ${Kosmosnimki} Kosmosnimki
	${ReadSecUnIniFile} ${MailRu} MailRu
	${ReadSecUnIniFile} ${Megafon} Megafon
	${ReadSecUnIniFile} ${Navitel} Navitel
	${ReadSecUnIniFile} ${Navteq} Navteq
	${ReadSecUnIniFile} ${OpenStreetMap} OpenStreetMap
	${ReadSecUnIniFile} ${PROGOROD} PROGOROD
	${ReadSecUnIniFile} ${RosReestr} RosReestr
	${ReadSecUnIniFile} ${RuMap} RuMap
	${ReadSecUnIniFile} ${Vi_Tel} Vi_Tel
	${ReadSecUnIniFile} ${Wikimapia} Wikimapia
	${ReadSecUnIniFile} ${Yahoo} Yahoo
	${ReadSecUnIniFile} ${Yandex} Yandex
	${ReadSecUnIniFile} ${MapContacts} MapContacts
	${ReadSecUnIniFile} ${MapContactsMain} MapContactsMain
	${ReadSecUnIniFile} ${MapMessage} MapMessage
	${ReadSecUnIniFile} ${Magnifier} Magnifier
;------------------------------
	${ReadSecUnIniFile} ${MapSearch} MapSearch
	${ReadSecUnIniFile} ${MapSearchMain} MapSearchMain
	${ReadSecUnIniFile} ${SearchFromGoogle} SearchFromGoogle
	${ReadSecUnIniFile} ${SearchOpenStreetMap} SearchOpenStreetMap
	${ReadSecUnIniFile} ${SearchFromDGIS} SearchFromDGIS
	${ReadSecUnIniFile} ${SearchFromYandex} SearchFromYandex
	${ReadSecUnIniFile} ${SearchFromHere} SearchFromHere
	${ReadSecUnIniFile} ${SearchNavitel} SearchNavitel

;------------------------------
	${ReadSecUnIniFile} ${MapStreetView} MapStreetView
	${ReadSecUnIniFile} ${MapStreetViewMain} MapStreetViewMain
	${ReadSecUnIniFile} ${StreetProvGoogle} StreetProvGoogle
	
	${ReadSecUnIniFile} ${MapPlaceView} MapPlaceView
	${ReadSecUnIniFile} ${MapPlaceViewMain} MapPlaceViewMain
	${ReadSecUnIniFile} ${PlaceViewProvGoogle} PlaceViewProvGoogle
	${ReadSecUnIniFile} ${SDK} SDK
	${ReadSecUnIniFile} ${Wizards} Wizards
	${ReadSecUnIniFile} ${WizardsRes} WizardsRes
	${ReadSecUnIniFile} ${Wizardaccount} Wizardaccount
	${ReadSecUnIniFile} ${Wizardtransport} Wizardtransport
;------------------------------ 
Done:

FunctionEnd

/* Save Current Status All Sections after install selected in 'file.ini' */
Function un.ComponentsSelected
	SetOutPath $INSTDIR
	StrCpy $SecIsEmpty 0
	;----------------------------
	${WriteSecUnIniFile} ${AbInstall} AbInstall
	${WriteSecUnIniFile} ${AbUnInstall} AbUnInstall
	${WriteSecUnIniFile} ${QtBase} QtBase
	${WriteSecUnIniFile} ${Base} Base
	${WriteSecUnIniFile} ${RussianBase} RussianBase
	${WriteSecUnIniFile} ${DeutschlandBase} DeutschlandBase
	${WriteSecUnIniFile} ${DeutchBase} DeutchBase
	${WriteSecUnIniFile} ${JapaneseBase} JapaneseBase
	${WriteSecUnIniFile} ${PolishBase} PolishBase
	${WriteSecUnIniFile} ${SpainBase} SpainBase
	${WriteSecUnIniFile} ${UkraineBase} UkraineBase	
	;----------------------------	
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${Bookmarks} Bookmarks
	${WriteSecUnIniFile} ${Annotations} Annotations
	${WriteSecUnIniFile} ${Recent_contacts} Recent_contacts
	${WriteSecUnIniFile} ${Poi} Poi
	${WriteSecUnIniFile} ${PrivatestorageBase} PrivatestorageBase
	${WriteGroupUnIniFile} ${Privatestorage} Privatestorage
	;----------------------------
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${Socks5} Socks5
	${WriteSecUnIniFile} ${In_band} In_band
	${WriteSecUnIniFile} ${FileTransferBase} FileTransferBase
	${WriteGroupUnIniFile} ${File_transfer} File_transfer
	;----------------------------
	${WriteSecUnIniFile} ${Vcard} Vcard
	${WriteSecUnIniFile} ${Jabber_search} Jabber_search
	${WriteSecUnIniFile} ${Roster_search} Roster_search
	${WriteSecUnIniFile} ${MUC} MUC
	${WriteSecUnIniFile} ${Metacontack} Metacontack
	;----------------------------
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${Remote} Remote
	${WriteSecUnIniFile} ${Ad_hoc_commands_Base} Ad_hoc_commands_Base
	${WriteGroupUnIniFile} ${Ad_hoc_commands} Ad_hoc_commands
	${WriteSecUnIniFile} ${Capcha} Capcha
	${WriteSecUnIniFile} ${Registration} Registration
	${WriteSecUnIniFile} ${Data_Form_Base} Data_Form_Base
	${WriteGroupUnIniFile} ${Data_Form} Data_Form
	;----------------------------
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${Compress} Compress
	${WriteSecUnIniFile} ${Console_Base} Console_Base
	${WriteGroupUnIniFile} ${Console} Console
	;----------------------------
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${File_archive} File_archive
	${WriteSecUnIniFile} ${Server_archive} Server_archive
	${WriteSecUnIniFile} ${Message_archive_Base} Message_archive_Base
	${WriteGroupUnIniFile} ${Message_archive} Message_archive
	;----------------------------
	${WriteSecUnIniFile} ${Discovery} Discovery
	${WriteSecUnIniFile} ${Duplicate_messages} Duplicate_messages
	${WriteSecUnIniFile} ${Gateways} Gateways
	${WriteSecUnIniFile} ${Bob} Bob
	${WriteSecUnIniFile} ${OOB} OOB
	${WriteSecUnIniFile} ${Autostatus} Autostatus
	${WriteSecUnIniFile} ${Chat_states} Chat_states
	${WriteSecUnIniFile} ${Privacy_lists} Privacy_lists
	${WriteSecUnIniFile} ${Roster_exchange} Roster_exchange
	${WriteSecUnIniFile} ${Client_info} Client_info
	${WriteSecUnIniFile} ${Aut_via_iq} Aut_via_iq
	${WriteSecUnIniFile} ${Short_cut} Short_cut
	${WriteSecUnIniFile} ${Avatar} Avatar
	${WriteSecUnIniFile} ${Birthday} Birthday
	;----------------------------
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${Bob_URL_Handler} Bob_URL_Handler
	${WriteSecUnIniFile} ${URL_Base} URL_Base
	${WriteGroupUnIniFile} ${URL} URL
	;----------------------------
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${ContactNotifies} ContactNotifies
	${WriteSecUnIniFile} ${MetodIP_Base} MetodIP_Base
	${WriteSecUnIniFile} ${ProviderIPGeoip} ProviderIPGeoip
	${WriteGroupUnIniFile} ${MetodIP} MetodIP
	${WriteSecUnIniFile} ${Serialport} Serialport
	${WriteSecUnIniFile} ${Manual} Manual
	${WriteSecUnIniFile} ${Positioning_Base} Positioning_Base
	${WriteGroupUnIniFile} ${Positioning} Positioning
	${WriteSecUnIniFile} ${Location_Base} Location_Base
	${WriteGroupUnIniFile} ${Location} Location
	;----------------------------
	${WriteSecUnIniFile} ${Abbreviations} Abbreviations
	${WriteSecUnIniFile} ${NickName} NickName
	${WriteSecUnIniFile} ${XHTML_IM} XHTML_IM
	${WriteSecUnIniFile} ${XMPP_URI} XMPP_URI
	${WriteSecUnIniFile} ${Statistics} Statistics
	;----------------------------
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${Renkoo} Renkoo
	${WriteSecUnIniFile} ${yMouse} yMouse
	${WriteSecUnIniFile} ${Adiummessagestyle_Base} Adiummessagestyle_Base
	${WriteGroupUnIniFile} ${Adiummessagestyle} Adiummessagestyle
	${WriteGroupUnIniFile} ${Message_style} Message_style
	;----------------------------
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${Aim} Aim
	${WriteSecUnIniFile} ${Bot} Bot
	${WriteSecUnIniFile} ${Car} Car
	${WriteSecUnIniFile} ${Conference} Conference
	${WriteSecUnIniFile} ${Facebook} Facebook
	${WriteSecUnIniFile} ${Gadu_gadu} Gadu_gadu)
	${WriteSecUnIniFile} ${Google_talk} Google_talk
	${WriteSecUnIniFile} ${ICQ} ICQ
	${WriteSecUnIniFile} ${Livejornal} Livejornal
	${WriteSecUnIniFile} ${MailRuIm} MailRuIm
	${WriteSecUnIniFile} ${MSN} MSN)
	${WriteSecUnIniFile} ${Odnoklassniki} Odnoklassniki
	${WriteSecUnIniFile} ${RSS} RSS
	${WriteSecUnIniFile} ${Skype} Skype
	${WriteSecUnIniFile} ${SMS} SMS
	${WriteSecUnIniFile} ${SMTP} SMTP
	${WriteSecUnIniFile} ${Twitter} Twitter
	${WriteSecUnIniFile} ${vKontakte} vKontakte
	${WriteSecUnIniFile} ${Weather} Weather
	${WriteSecUnIniFile} ${YahooIm} YahooIm
	${WriteSecUnIniFile} ${YaOnline} YaOnline
	${WriteSecUnIniFile} ${Status_icons_Base} Status_icons_Base
	${WriteGroupUnIniFile} ${Status_icons} Status_icons
	;----------------------------
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${EI_Default} EI_Default
	${WriteSecUnIniFile} ${EI_Blobs_purple} EI_Blobs_purple
	${WriteSecUnIniFile} ${Emoticons_Base} Emoticons_Base
	${WriteGroupUnIniFile} ${Emoticons} Emoticons
	;----------------------------
	${WriteSecUnIniFile} ${Client_icone} Client_icone
	;----------------------------
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${Lng_Russian} Lng_Russian
	${WriteSecUnIniFile} ${Lng_English_US} Lng_English_US
	${WriteSecUnIniFile} ${Lng_English_AU} Lng_English_AU
	${WriteSecUnIniFile} ${Lng_English_GB} Lng_English_GB
	${WriteSecUnIniFile} ${Lng_English_SA} Lng_English_SA
	${WriteSecUnIniFile} ${Lng_Ukraine} Lng_Ukraine
	${WriteSecUnIniFile} ${Lng_Deutschland} Lng_Deutschland
	${WriteSecUnIniFile} ${Lng_Polish} Lng_Polish
	${WriteSecUnIniFile} ${Spellchecker_Base} Spellchecker_Base
	${WriteGroupUnIniFile} ${Spellchecker} Spellchecker
	;----------------------------
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${Russian} Russian
	${WriteSecUnIniFile} ${Deutschland} Deutschland
	${WriteSecUnIniFile} ${Deutch} Deutch
	${WriteSecUnIniFile} ${Japanese} Japanese
	${WriteSecUnIniFile} ${Polish} Polish
	${WriteSecUnIniFile} ${Spain} Spain
	${WriteSecUnIniFile} ${Ukraine} Ukraine
	${WriteGroupUnIniFile} ${Translations} Translations
	;----------------------------
	${WriteSecUnIniFile} ${Doc} Doc)
	;----------------------------
  
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${WINAMP} WINAMP
	${WriteSecUnIniFile} ${AIMP} AIMP 
	${WriteSecUnIniFile} ${FileSource} FileSource 
	${WriteGroupUnIniFile} ${TuneListeners} TuneListeners 
	${WriteSecUnIniFile} ${TuneInfo} TuneInfo 
	${WriteSecUnIniFile} ${TuneMain} TuneMain
	${WriteGroupUnIniFile} ${Tune} Tune	
	${WriteSecUnIniFile} ${Activity} Activity
	${WriteSecUnIniFile} ${Mood} Mood	
	${WriteSecUnIniFile} ${Pepmanager} Pepmanager
	${WriteGroupUnIniFile} ${PersonalEvent} PersonalEvent	  
;------------------------------
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${FFMpegDll} FFMpegDll
	${WriteSecUnIniFile} ${MMplayer} MMplayer
	${WriteGroupUnIniFile} ${FFMpeg} FFMpeg
;------------------------------	
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${Attention} Attention
	${WriteSecUnIniFile} ${Receipts} Receipts
;------------------------------
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${Google} Google
	${WriteSecUnIniFile} ${DGIS} DGIS
	${WriteSecUnIniFile} ${Bing} Bing
	${WriteSecUnIniFile} ${Esri} Esri
	${WriteSecUnIniFile} ${Here} Here
	${WriteSecUnIniFile} ${Kosmosnimki} Kosmosnimki
	${WriteSecUnIniFile} ${MailRu} MailRu
	${WriteSecUnIniFile} ${Megafon} Megafon
	${WriteSecUnIniFile} ${Navitel} Navitel
	${WriteSecUnIniFile} ${Navteq} Navteq
	${WriteSecUnIniFile} ${OpenStreetMap} OpenStreetMap
	${WriteSecUnIniFile} ${PROGOROD} PROGOROD
	${WriteSecUnIniFile} ${RosReestr} RosReestr
	${WriteSecUnIniFile} ${RuMap} RuMap
	${WriteSecUnIniFile} ${Vi_Tel} Vi_Tel
	${WriteSecUnIniFile} ${Wikimapia} Wikimapia
	${WriteSecUnIniFile} ${Yahoo} Yahoo
	${WriteSecUnIniFile} ${Yandex} Yandex
	${WriteGroupUnIniFile} ${Sources} Sources
	${WriteSecUnIniFile} ${MapMessage} MapMessage
	${WriteSecUnIniFile} ${MapContactsMain} MapContactsMain
	${WriteGroupUnIniFile} ${MapContacts} MapContacts
	${WriteSecUnIniFile} ${Magnifier} Magnifier
	${WriteSecUnIniFile} ${MapMain} MapMain
	${WriteGroupUnIniFile} ${Map} Map
;------------------------------
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${SearchFromGoogle} SearchFromGoogle
	${WriteSecUnIniFile} ${SearchOpenStreetMap} SearchOpenStreetMap
	${WriteSecUnIniFile} ${SearchFromDGIS} SearchFromDGIS
	${WriteSecUnIniFile} ${SearchFromYandex} SearchFromYandex
	${WriteSecUnIniFile} ${SearchFromHere} SearchFromHere
	${WriteSecUnIniFile} ${SearchNavitel} SearchNavitel
	${WriteSecUnIniFile} ${MapSearchMain} MapSearchMain
	${WriteGroupUnIniFile} ${MapSearch} MapSearch
;------------------------------
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${StreetProvGoogle} StreetProvGoogle
	${WriteSecUnIniFile} ${MapStreetViewMain} MapStreetViewMain
	${WriteGroupUnIniFile} ${MapStreetView} MapStreetView	
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${PlaceViewProvGoogle} PlaceViewProvGoogle
	${WriteSecUnIniFile} ${MapPlaceViewMain} MapPlaceViewMain
	${WriteGroupUnIniFile} ${MapPlaceView} MapPlaceView
	StrCpy $SecIsEmpty 0
	${WriteSecUnIniFile} ${Wizardaccount} Wizardaccount
	${WriteSecUnIniFile} ${Wizardtransport} Wizardtransport
	${WriteSecUnIniFile} ${WizardsRes} WizardsRes
	${WriteGroupUnIniFile} ${Wizards} Wizards
	
	${WriteSecUnIniFile} ${SDK} SDK
  ;---Save change to disk ------
  IntOp $DelAll $DelAll - $UnDelAll
  WriteINIStr $INSTDIR\${Name1}.ini "Application" "Numbers Components" $DelAll
  FlushINI $INSTDIR\${Name1}.ini
  Delete $TEMP\${Name1}.ini
  ;--------
  Call Un.CheckDelAll
  
FunctionEnd  
;----------------------------------------------------------------------
Function Un.CheckROSection
; No Directory
	${IfUnSecReadOnly} ${AbInstall}
	${IfUnSecReadOnly} ${AbUnInstall}
	${IfUnSecReadOnly} ${QtBase}
	${IfUnSecReadOnly} ${Base}
	${IfUnSecReadOnly} ${RussianBase}
	${IfUnSecReadOnly} ${DeutschlandBase}
	${IfUnSecReadOnly} ${DeutchBase}
	${IfUnSecReadOnly} ${JapaneseBase}
	${IfUnSecReadOnly} ${PolishBase}
	${IfUnSecReadOnly} ${SpainBase}
	${IfUnSecReadOnly} ${UkraineBase}
	;----------------------------	
	;${IfUnSecReadOnly} ${Privatestorage}
	${IfUnSecReadOnly} ${PrivatestorageBase}
	${IfUnSecReadOnly} ${Bookmarks}
	${IfUnSecReadOnly} ${Annotations}
	${IfUnSecReadOnly} ${Recent_contacts}
	${IfUnSecReadOnly} ${Poi}
	;${IfUnSecReadOnly} ${File_transfer}
	${IfUnSecReadOnly} ${FileTransferBase}
	${IfUnSecReadOnly} ${Socks5}
	${IfUnSecReadOnly} ${In_band}
	${IfUnSecReadOnly} ${Vcard}
	${IfUnSecReadOnly} ${Jabber_search}
	${IfUnSecReadOnly} ${Roster_search}
	${IfUnSecReadOnly} ${MUC}
	${IfUnSecReadOnly} ${Metacontack}
	;${IfUnSecReadOnly} ${Data_Form}
	${IfUnSecReadOnly} ${Data_Form_Base}
	${IfUnSecReadOnly} ${Capcha}
	${IfUnSecReadOnly} ${Registration}
	;${IfUnSecReadOnly} ${Ad_hoc_commands}
	${IfUnSecReadOnly} ${Ad_hoc_commands_Base}
	${IfUnSecReadOnly} ${Remote}
	;${IfUnSecReadOnly} ${Console}
	${IfUnSecReadOnly} ${Console_Base}
	${IfUnSecReadOnly} ${Compress}
	;${IfUnSecReadOnly} ${Message_archive}
	${IfUnSecReadOnly} ${Message_archive_Base}
	${IfUnSecReadOnly} ${File_archive}
	${IfUnSecReadOnly} ${Server_archive}
	${IfUnSecReadOnly} ${Discovery}
	${IfUnSecReadOnly} ${Duplicate_messages}
	${IfUnSecReadOnly} ${Gateways}
	${IfUnSecReadOnly} ${Bob}
	${IfUnSecReadOnly} ${OOB}
	${IfUnSecReadOnly} ${Autostatus}
	${IfUnSecReadOnly} ${Chat_states}
	${IfUnSecReadOnly} ${Privacy_lists}
	${IfUnSecReadOnly} ${Roster_exchange}
	${IfUnSecReadOnly} ${Client_info}
	${IfUnSecReadOnly} ${Aut_via_iq}
	${IfUnSecReadOnly} ${Short_cut}
	${IfUnSecReadOnly} ${Avatar}
	${IfUnSecReadOnly} ${Birthday}
	;${IfUnSecReadOnly} ${URL}
	${IfUnSecReadOnly} ${URL_Base}
	${IfUnSecReadOnly} ${Bob_URL_Handler}
	;${IfUnSecReadOnly} ${Location}
	${IfUnSecReadOnly} ${Location_Base}
	;${IfUnSecReadOnly} ${Positioning}
	${IfUnSecReadOnly} ${Positioning_Base}
	${IfUnSecReadOnly} ${Manual}
	${IfUnSecReadOnly} ${Serialport}
	${IfUnSecReadOnly} ${MetodIP_Base}
	${IfUnSecReadOnly} ${ProviderIPGeoip}
	${IfUnSecReadOnly} ${ContactNotifies}

	${IfUnSecReadOnly} ${Abbreviations}
	${IfUnSecReadOnly} ${NickName}
	${IfUnSecReadOnly} ${XHTML_IM}
	${IfUnSecReadOnly} ${XMPP_URI}
	${IfUnSecReadOnly} ${Statistics}
	;${IfUnSecReadOnly} ${Message_style}
	;${IfUnSecReadOnly} ${Adiummessagestyle}
	${IfUnSecReadOnly} ${Adiummessagestyle_Base}
	${IfUnSecReadOnly} ${Renkoo}
	${IfUnSecReadOnly} ${yMouse}
	;${IfUnSecReadOnly} ${Status_icons}
	${IfUnSecReadOnly} ${Status_icons_Base}
	${IfUnSecReadOnly} ${Aim}
	${IfUnSecReadOnly} ${Bot}
	${IfUnSecReadOnly} ${Car}
	${IfUnSecReadOnly} ${Conference}
	${IfUnSecReadOnly} ${Facebook}
	${IfUnSecReadOnly} ${Gadu_gadu}
	${IfUnSecReadOnly} ${Google_talk}
	${IfUnSecReadOnly} ${ICQ}
	${IfUnSecReadOnly} ${Livejornal}
	${IfUnSecReadOnly} ${MailRuIm}
	${IfUnSecReadOnly} ${MSN}
	${IfUnSecReadOnly} ${Odnoklassniki}
	${IfUnSecReadOnly} ${RSS}
	${IfUnSecReadOnly} ${Skype}
	${IfUnSecReadOnly} ${SMS}
	${IfUnSecReadOnly} ${SMTP}
	${IfUnSecReadOnly} ${Twitter}
	${IfUnSecReadOnly} ${vKontakte}
	${IfUnSecReadOnly} ${Weather}
	${IfUnSecReadOnly} ${YahooIm}
	${IfUnSecReadOnly} ${YaOnline}
	;${IfUnSecReadOnly} ${Emoticons}
	${IfUnSecReadOnly} ${Emoticons_Base}
	${IfUnSecReadOnly} ${EI_Default}
	${IfUnSecReadOnly} ${EI_Blobs_purple}
	${IfUnSecReadOnly} ${Client_icone}
	;----------------------------
	;${IfUnSecReadOnly} ${Spellchecker}
	${IfUnSecReadOnly} ${Spellchecker_Base}
	${IfUnSecReadOnly} ${Lng_Russian}
	${IfUnSecReadOnly} ${Lng_English_US}
	${IfUnSecReadOnly} ${Lng_English_AU}
	${IfUnSecReadOnly} ${Lng_English_GB}
	${IfUnSecReadOnly} ${Lng_English_SA}
	${IfUnSecReadOnly} ${Lng_Ukraine}
	${IfUnSecReadOnly} ${Lng_Deutschland}
	${IfUnSecReadOnly} ${Lng_Polish}
	;----------------------------
	;${IfUnSecReadOnly} ${Translations}
	${IfUnSecReadOnly} ${Russian}
	${IfUnSecReadOnly} ${Deutschland}
	${IfUnSecReadOnly} ${Deutch}
	${IfUnSecReadOnly} ${Japanese}
	${IfUnSecReadOnly} ${Polish}
	${IfUnSecReadOnly} ${Spain}
	${IfUnSecReadOnly} ${Ukraine}
	;----------------------------
	${IfUnSecReadOnly} ${Doc}
;----------------------------
	${IfUnSecReadOnly} ${WINAMP}
	${IfUnSecReadOnly} ${AIMP}
	${IfUnSecReadOnly} ${FileSource}
	;${IfUnSecReadOnly} ${TuneListeners}
	${IfUnSecReadOnly} ${TuneInfo}
	${IfUnSecReadOnly} ${TuneMain}
	;${IfUnSecReadOnly} ${Tune}
	${IfUnSecReadOnly} ${Pepmanager}
	${IfUnSecReadOnly} ${Activity}
	${IfUnSecReadOnly} ${Mood}
	;${IfUnSecReadOnly} ${PersonalEvent}
	${IfUnSecReadOnly} ${FFMpegDll}
	${IfUnSecReadOnly} ${MMplayer}
	;${IfUnSecReadOnly} ${FFMpeg}
	${IfUnSecReadOnly} ${Attention}
	${IfUnSecReadOnly} ${Receipts}
	${IfUnSecReadOnly} ${Google}
	${IfUnSecReadOnly} ${DGIS}
	${IfUnSecReadOnly} ${Bing}
	${IfUnSecReadOnly} ${Esri}
	${IfUnSecReadOnly} ${Here}
	${IfUnSecReadOnly} ${Kosmosnimki}
	${IfUnSecReadOnly} ${MailRu}
	${IfUnSecReadOnly} ${Megafon}
	${IfUnSecReadOnly} ${Navitel}
	${IfUnSecReadOnly} ${Navteq}
	${IfUnSecReadOnly} ${OpenStreetMap}
	${IfUnSecReadOnly} ${PROGOROD}
	${IfUnSecReadOnly} ${RosReestr}
	${IfUnSecReadOnly} ${RuMap}
	${IfUnSecReadOnly} ${Vi_Tel}
	${IfUnSecReadOnly} ${Wikimapia}
	${IfUnSecReadOnly} ${Yahoo}
	${IfUnSecReadOnly} ${Yandex}
	;${IfUnSecReadOnly} ${Sources}
	${IfUnSecReadOnly} ${MapMessage}
	${IfUnSecReadOnly} ${MapContactsMain}
	;${IfUnSecReadOnly} ${MapContacts}
	${IfUnSecReadOnly} ${Magnifier}
	${IfUnSecReadOnly} ${MapMain}
	;${IfUnSecReadOnly} ${Map}
	${IfUnSecReadOnly} ${SearchFromGoogle}
	${IfUnSecReadOnly} ${SearchOpenStreetMap}
	${IfUnSecReadOnly} ${SearchFromDGIS}
	${IfUnSecReadOnly} ${SearchFromYandex}
	${IfUnSecReadOnly} ${SearchFromHere}
	${IfUnSecReadOnly} ${SearchNavitel}
	${IfUnSecReadOnly} ${MapSearchMain}
	;${IfUnSecReadOnly} ${MapSearch}

	${IfUnSecReadOnly} ${StreetProvGoogle}
	${IfUnSecReadOnly} ${MapStreetViewMain}
	;${IfUnSecReadOnly} ${MapStreetView}
	${IfUnSecReadOnly} ${PlaceViewProvGoogle}
	${IfUnSecReadOnly} ${MapPlaceViewMain}
	;${IfUnSecReadOnly} ${MapPlaceView}
	;${IfUnSecReadOnly} ${Wizards}
	${IfUnSecReadOnly} ${WizardsRes}
	${IfUnSecReadOnly} ${Wizardaccount}
	${IfUnSecReadOnly} ${Wizardtransport}
	${IfUnSecReadOnly} ${SDK}

FunctionEnd
;----------------------------------------------------------------------

Function Un.onSelChange
;Index in $0 
	${If} $0 == -1
		Call Un.CheckROSection
	${EndIf}
FunctionEnd  

Function Un.CheckDelAll
	${If} $DelAll == 0
		; Remove registry keys
		DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_REG_KEY}"
		DeleteRegKey HKLM "SOFTWARE\${PROGRAM_REG_KEY}"
		; Remove ShortCut Menu icon
		Delete "$SMPROGRAMS\${PROGRAM_SM_FOLDER}\${UnIstName}.lnk"
		Delete "$SMPROGRAMS\${PROGRAM_SM_FOLDER}\${Name1}.lnk"
		; Remove desktop icon
		Delete "$DESKTOP\${Name1}.lnk"
		;-------------------
		Delete "$INSTDIR\${UnIstName}.exe"
		Delete "$INSTDIR\${Name1}.ini"
		;Delete "$TEMP\${Name1}.ini"
		; Remove directories used
		RMDir /r "$SMPROGRAMS\${PROGRAM_SM_FOLDER}"
		RMDir /r "$INSTDIR"		
	
	${EndIf}
FunctionEnd 
;MessageBox MB_OK ".onSelChange/st = $0  CurInstType= $1 "
