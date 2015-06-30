/*
+ PersonalEvent
 - + Pepmanager
 - + Activity
 - + Mood
 - + Tune
 - - + TuneInfo
 - - + TuneListeners
 - - - + WINAMP
 - - - + AIMP
 - - - + File
*/
SectionGroup $(SecPersonalEvent) PersonalEvent
Section -$(SecPepmanager) Pepmanager
SectionIn  1
	; Plugins
	SetOutPath $INSTDIR\plugins
	File "${PROGRAM_BIN_FOLDER}\plugins\pepmanager.dll"
	; Resources: pepmanager.def.xml
	SetOutPath $INSTDIR\resources\menuicons\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\pepmanager.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\pepmanager.png"
	; Resources: pep.def.xml
	SetOutPath $INSTDIR\resources\sounds\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\pep.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\plopp.wav"
SectionEnd 
Section $(SecActivity) Activity  
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\activity.dll"
  ; Resources: \resources\activityicons
  SetOutPath $INSTDIR\resources\activityicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\activityicons\shared\*.*"    
SectionEnd 

Section $(SecMood) Mood
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mood.dll"
  ; Resources:
  SetOutPath $INSTDIR\resources\moodicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\moodicons\shared\*.*"      
SectionEnd 

SectionGroup $(SecTune) Tune
Section -$(SecTuneMain) TuneMain
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\tune.dll"
  ; Resources:
SectionEnd 
Section $(SecTuneInfo) TuneInfo
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\tuneinforequesterlastfm.dll"
  ; Resources:
SectionEnd 

SectionGroup $(SecTuneListeners) TuneListeners
Section $(SecWINAMP) WINAMP
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\tunelistenerwinamp.dll"
  ; Resources:
SectionEnd 
Section $(SecAIMP) AIMP
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\tunelisteneraimp.dll"
  ; Resources:
SectionEnd 
Section $(SecFileSource) FileSource
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\tunelistenerfile.dll"
  ; Resources:
SectionEnd 
SectionGroupEnd
SectionGroupEnd
SectionGroupEnd

/*
+ Location
 - + Positioning
 - - + Manual
 - - + InterfaceLocation
 - - + Serial_port
 - - + IP
 - - - + Provider IP
 - + Contact_Notifies
*/
SectionGroup $(SecLocation) Location
Section "-geoloc" Location_Base
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\geoloc.dll"
  ; Resources:  geoloc.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\geoloc.def.xml"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\geoloc.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\no_geoloc.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\map.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\satellite.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\satelliteOFF.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\satelliteON.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\satelliteONR.png"
SectionEnd

SectionGroup $(SecPositioning) Positioning
Section "-Positioning" Positioning_Base
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\positioning.dll"
  ; Resources: 
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\positioning.def.xml"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\positioning.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\serialport.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\location.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\manual.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\geoip.gif"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\freegeoip.png"
SectionEnd

Section $(SecManual) Manual
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\positioningmethodmanual.dll"
  ; Resources:     
SectionEnd

####### TEMP #####################################################
Section  $(SecSerialport) Serialport
SectionIn  1
	; Plugins
	SetOutPath $INSTDIR\plugins
	####### TEMP #########
	IfFileExists "${PROGRAM_BIN_FOLDER}\plugins\positioningmethodserialport.dll" 0 Done
		File "${PROGRAM_BIN_FOLDER}\plugins\positioningmethodserialport.dll"
	; Resources
	Done:
SectionEnd 

####### TEMP #####################################################

SectionGroup $(SecMetodIP) MetodIP
Section "-SecMetodIP" MetodIP_Base
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\positioningmethodip.dll"
  ; Resources
SectionEnd 

Section $(SecProviderIPGeoip) ProviderIPGeoip
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\positioningmethodipproviderfreegeoip.dll"
  ; Resources
SectionEnd
SectionGroupEnd

/* ---now not used -- INTERFACE-LOCATION---
Section  -$(SecIntLocation) IntLocation
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\positioningmethodserialport.dll"
  ; Translations
  ; Resources
SectionEnd 
*/
SectionGroupEnd

Section $(SecContactNotifies) ContactNotifies
SectionIn  1
	; Plugins
	SetOutPath $INSTDIR\plugins
	File "${PROGRAM_BIN_FOLDER}\plugins\contactproximitynotification.dll"
	; Resources: proximity.def.xml
	SetOutPath $INSTDIR\resources\menuicons\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\proximity.def.xml" 
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\proximity.png" 
	; Resources/sound: proximity.def.xml
	SetOutPath $INSTDIR\resources\sounds\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\proximity.def.xml" 
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\sonar.wav"   
  
SectionEnd 
SectionGroupEnd

/*
+ Map
 - + Sources
 - - + Google
 - - + DGIS
 - - + Bing
 - - + Esri
 - - + Here
 - - + Kosmosnimki
 - - + MailRu
 - - + Megafon
 - - + Navitel
 - - + Navteq
 - - + OpenStreetMap
 - - + PROGOROD
 - - + RosReestr
 - - + RuMap
 - - + Vi_Tel
 - - + Wikimapia
 - - + Yahoo
 - - + Yandex
 
 - + Magnifier
 - + Map Contacts
 - - + MapMessage
*/
SectionGroup $(SecMap) Map
Section -$(SecMapMain) MapMain
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\map.dll"
;  File "${PROGRAM_BIN_FOLDER}\plugins\mapscene.dll"
  File "${PROGRAM_BIN_FOLDER}\plugins\maplocationselector.dll"
  ; Resources: map-\mapicons
  SetOutPath $INSTDIR\resources\mapicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\mapicons\shared\*.*"
  ; Resources: 
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mapsources.def.xml"  
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\google.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\yandex.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\yahoo.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\kosmosnimki.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\bing.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\osm.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\wiki.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\navitel.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mail.ru.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\here.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\2gis.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\navteq.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\esri.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\megafon.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\vitel.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\progorod.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rosreestr.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\geocon.png" 
SectionEnd 

SectionGroup $(SecSources) Sources
Section $(SecGoogle) Google
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsourcegoogle.dll"
  ; Resources:
SectionEnd 

Section $(SecDGIS) DGIS
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsource2gis.dll"
  ; Resources:
SectionEnd 
Section $(SecBing) Bing
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsourcebing.dll"
  ; Resources:
SectionEnd 

Section $(SecEsri) Esri
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsourceesri.dll"
  ; Resources:
SectionEnd 
;================
Section $(SecHere) Here
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsourceovi.dll"
  ; Resources:
SectionEnd 
Section $(SecKosmosnimki) Kosmosnimki
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsourcekosmosnimki.dll"
  ; Resources:
SectionEnd 
Section $(SecMailRu) MailRu
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsourcemailru.dll"
  ; Resources:
SectionEnd 
Section $(SecMegafon) Megafon
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsourcemegafon.dll"
  ; Resources:
SectionEnd 
Section $(SecNavitel) Navitel
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsourcenavitel.dll"
  ; Resources:
SectionEnd 
Section $(SecNavteq) Navteq
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsourcenavteq.dll"
  ; Resources:
SectionEnd 
Section $(SecOpenStreetMap) OpenStreetMap
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsourceosm.dll"
  ; Resources:
SectionEnd 
Section $(SecPROGOROD) PROGOROD
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsourceprogorod.dll"
  ; Resources:
SectionEnd 
Section $(SecRosReestr) RosReestr
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsourcerosreestr.dll"
  ; Resources:
SectionEnd 
Section $(SecRuMap) RuMap
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsourcerumap.dll"
  ; Resources:
SectionEnd 
Section $(SecVi_Tel) Vi_Tel
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsourcevitel.dll"
  ; Resources:
SectionEnd 
Section $(SecWikimapia) Wikimapia
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsourcewiki.dll"
  ; Resources:
SectionEnd 
Section $(SecYahoo) Yahoo
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsourceyahoo.dll"
  ; Resources:
SectionEnd 
Section $(SecYandex) Yandex
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsourceyandex.dll"
  ; Resources:
SectionEnd 

SectionGroupEnd
;--------------------------------
SectionGroup $(SecMapContacts) MapContacts
Section -$(SecMapContactsMain) MapContactsMain
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapcontacts.dll"
  ; Resources:
SectionEnd 

Section $(SecMapMessage) MapMessage
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapmessage.dll"
  ; Resources: buttons.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\buttons.def.xml"  
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\close_active.png"  
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\close_inactive.png"
  ;----in [poi.def.xml] ........
  ;File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mapmessage.png"
  
SectionEnd 
SectionGroupEnd

;--------------------------------
Section $(SecMagnifier) Magnifier
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapmagnifier.dll"
  ; Resources: mapmagnifier.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mapmagnifier.def.xml"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\magnifier.png" 
SectionEnd 
SectionGroupEnd

/*
+ MapSearch
 - + SearchFromGoogle
 - + SearchOpenStreetMap
 - + SearchFromDGIS
 - + SearchFromYandex
 - + SearchFromHere
 - + SearchNavitel
*/
SectionGroup $(SecMapSearch) MapSearch
Section "-MapSearch"  MapSearchMain
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsearch.dll"
  ; Resources: mapsearch.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mapsearch.def.xml"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mapsearch.png" 
SectionEnd 

Section $(SecSearchFromGoogle) SearchFromGoogle
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsearchprovidergoogle.dll"
  ; Resources:
SectionEnd 
Section $(SecSearchOpenStreetMap) SearchOpenStreetMap
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsearchproviderosm.dll"
  ; Resources:
  
SectionEnd 
Section $(SecSearchFromDGIS) SearchFromDGIS
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsearchprovider2gis.dll"
  ; Resources:
  
SectionEnd 
Section $(SecSearchFromYandex) SearchFromYandex
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsearchprovideryandex.dll"
  ; Resources:
  
SectionEnd 
Section $(SecSearchNavitel) SearchNavitel
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsearchprovidernavitel.dll"
  ; Resources: navitel.def.xml
  SetOutPath $INSTDIR\resources\navitel\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\navitel\shared\navitel.def.xml"  
SectionEnd 

Section $(SecSearchFromHere) SearchFromHere
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mapsearchproviderhere.dll"
  ; Resources:
  
SectionEnd 
SectionGroupEnd

/*
+ MapStreetView
 - + StreetProvGoogle
*/
SectionGroup $(SecMapStreetView) MapStreetView
Section "-MapStreetView"  MapStreetViewMain
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\streetview.dll"
  ; Resources
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\streetman.png"  
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mapstreetview.png" 
SectionEnd 

Section $(SecStreetProvGoogle) StreetProvGoogle
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\streetviewprovidergoogle.dll"
  ; Resources: ......already have...............................
  SetOutPath $INSTDIR\resources\menuicons\shared
  ;File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\google.png"   
SectionEnd 
SectionGroupEnd

/*
+ MapPlaceView
 - + PlaceProvGoogle
*/
SectionGroup $(SecMapPlaceView) MapPlaceView
Section -$(SecMapPlaceViewMain)  MapPlaceViewMain
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\placeview.dll"
  ; Resources
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\viewcenter.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\key16.png"
SectionEnd 

Section $(SecPlaceViewProvGoogle) PlaceViewProvGoogle
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\placeviewprovidergoogle.dll"
  ; Resources: ......already have...............................
  SetOutPath $INSTDIR\resources\menuicons\shared
  ;File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\google.png"   
SectionEnd 
SectionGroupEnd


/*
weather.png
SectionGroup $(SecWeather) Weather
Section "-Weather"  
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\weather.dll"
  ; Translations
  ; Resources:
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\viewcenter.png"
  ; this is alse in placeview ...
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\key16.png"
SectionEnd 

Section $(SecWeatherProv1) WeatherProv1
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  ; Translations
  ; Resources:  
SectionEnd 
SectionGroupEnd

LangString SecWeather ${LANG_ENGLISH} "Weather"
LangString SecWeather ${LANG_RUSSIAN} "Погода"
LangString DESC_Weather ${LANG_ENGLISH} "Weather"
LangString DESC_Weather ${LANG_RUSSIAN} "Погода"

LangString SecWeatherProv1 ${LANG_ENGLISH} "Provider 1"
LangString SecWeatherProv1 ${LANG_RUSSIAN} "Провайдер Погоды 1"
LangString DESC_WeatherProv1 ${LANG_ENGLISH} "Provider 1"
LangString DESC_WeatherProv1 ${LANG_RUSSIAN} "Провайдер Погоды 1"

*/

/*
+ MMplayer
 -  (ffmpeg)
+ Receipts
*/
SectionGroup $(SecFFMpeg) FFMpeg
Section $(SecFFMpegDll) FFMpegDll
SectionIn  1
  ; Plugins
	SetOutPath "$INSTDIR\"
	File "${PROGRAM_BIN_FOLDER}\Qt${QTFF}FFMpeg.dll"
;	File "${PROGRAM_BIN_FOLDER}\QtFFMpeg.dll"
	; Resources: 
	; FFMpeg libraries
	SetOutPath $INSTDIR
	File "${PROGRAM_BIN_FOLDER}\avcodec-56.dll"
	File "${PROGRAM_BIN_FOLDER}\avdevice-56.dll"
	File "${PROGRAM_BIN_FOLDER}\avfilter-5.dll"
	File "${PROGRAM_BIN_FOLDER}\avformat-56.dll"
	File "${PROGRAM_BIN_FOLDER}\avutil-54.dll"
	File "${PROGRAM_BIN_FOLDER}\postproc-53.dll"
	File "${PROGRAM_BIN_FOLDER}\swresample-1.dll"
	File "${PROGRAM_BIN_FOLDER}\swscale-3.dll"

SectionEnd 

Section $(SecMMplayer) MMplayer
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\mmplayer.dll"
  ; Resources: mmplayer.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mmplayer.def.xml"  
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mmplayer.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\eject.png"  
SectionEnd 
SectionGroupEnd

/*
+Attention
*/
Section $(SecAttention) Attention 
SectionIn  1
	;Plugins
	SetOutPath $INSTDIR\plugins
	File "${PROGRAM_BIN_FOLDER}\plugins\attention.dll"
	; Resources: resources\menuicons\shared\notifications.def.xml
	SetOutPath $INSTDIR\resources\menuicons\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\notifications.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\notifications.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\notificationsactivateall.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\notificationsremoveall.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\notificationssoundon.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\notificationssoundoff.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\notificationssoundplay.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\notificationspopupwindow.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\notificationsshowminimized.png"
	; Resources: resources\sounds\shared\attention.def.xml
	SetOutPath $INSTDIR\resources\sounds\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\attention.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\bells.wav"
SectionEnd 

/*
+ Receipts
*/
Section $(SecReceipts) Receipts
SectionIn  1
	; Plugins
	SetOutPath $INSTDIR\plugins
	File "${PROGRAM_BIN_FOLDER}\plugins\receipts.dll"
	; Resources: in eyecuicon.def.xml
	SetOutPath $INSTDIR\resources\menuicons\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\receipts.png"
	; Resources/sounds: in receipts.def.xml
	SetOutPath $INSTDIR\resources\sounds\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\receipts.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\floop.wav"
	
SectionEnd 

;=========================================================================================

/*
+Private Storage
  -Bookmarks
  -Annotations
  -Recent contacts
  -Poi
*/
SectionGroup $(SecPrivatestorage) Privatestorage
Section "-SecPrivatestorageBase"  PrivatestorageBase
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\privatestorage.dll"
  ; Resources
SectionEnd 

Section $(SecBookmarks) Bookmarks 
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\bookmarks.dll"
  ; Resources: bookmarks.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\bookmarks.def.xml"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\bookmarks.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\bookmarksadd.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\bookmarksremove.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\bookmarksroom.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\bookmarksurl.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\bookmarksedit.png"
SectionEnd 

Section $(SecAnnotations) Annotations 
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\annotations.dll"
  ; Resources: annotations.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\annotations.def.xml"  
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\annotations.png"
SectionEnd 

Section $(SecRecent_contacts) Recent_contacts 
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\recentcontacts.dll"
  ; Resources - recentcontacts.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\recentcontacts.def.xml"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\recentcontacts.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\recentcontactsfavorite.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\recentcontactsinsertfavorite.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\recentcontactsremovefavorite.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\recentcontactsremoverecent.png" 
SectionEnd 

Section $(SecPoi) Poi 
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\poi.dll"
  ; Resources: resources\country\shared, resources\menuicons\shared, resources\typepoint\shared
  SetOutPath $INSTDIR\resources\country\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\country\shared\*.*"
  SetOutPath $INSTDIR\resources\typepoint\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\typepoint\shared\*.*"
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\poi.def.xml"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\poi.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\poiadd.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\poinone.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\poitoolbar.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\poiview.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\globus.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\flag24.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\tracker.png"
;  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mapmessage.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\description.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\connect.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\connectend.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\connectlong.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\folder.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\folderopen.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\bgrmap.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\bgrsat.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\poinotype.png"
SectionEnd 
SectionGroupEnd

/*
+File transfer
  -SOcks5
  -In band
*/
SectionGroup $(SecFile_transfer) File_transfer
Section "-File transfer" FileTransferBase
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\filetransfer.dll"
  File "${PROGRAM_BIN_FOLDER}\plugins\datastreamsmanager.dll"
  File "${PROGRAM_BIN_FOLDER}\plugins\filestreamsmanager.dll"
  ; Resources: filetransfer.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\filetransfer.def.xml"  
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\filetransfersend.png" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\filetransferreceive.png"
  ; Resources: filestreamsmanager.def.xml
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\filestreamsmanager.def.xml" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\filestreamsmanager.png"
  ; Resources: datastreamsmanager.def.xml
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\datastreamsmanager.def.xml" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\datastreamsmanager.png"
  ; Resources: sound\shared\filetransfer.def.xml
  SetOutPath $INSTDIR\resources\sounds\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\filetransfer.def.xml" 
SectionEnd 

Section $(SecSOcks5) Socks5 
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\socksstreams.dll"
  ; Resources
SectionEnd 
Section $(SecIn_band) In_band 
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\inbandstreams.dll"
  ; Resources
SectionEnd 
SectionGroupEnd
/*
+Vcard
+Jabber search
+Roster search
+MUC
+Metacontack
*/
Section $(SecVcard) Vcard 
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\vcard.dll"
  ; Resources: vcard.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\vcard.def.xml"    
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\vcard.png"
SectionEnd

Section $(SecJabber_search) Jabber_search  
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\jabbersearch.dll"
  ; Resources: jsearch.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\jsearch.def.xml"      
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\jsearch.png"
SectionEnd 

Section $(SecRoster_search) Roster_search  
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\rostersearch.dll"
  ; Resources: rostersearch.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rostersearch.def.xml"      
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rostersearch.png"
SectionEnd 

Section $(SecMUC) MUC  
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\multiuserchat.dll"
  ; Resources: muc.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\muc.def.xml"      
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mucconference.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mucexitroom.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mucjoin.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mucinvite.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mucchangenick.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mucchangetopic.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mucclearchat.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mucenterroom.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mucexitroom.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mucrequestvoice.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\muceditbanList.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\muceditmemberslist.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\muceditadminslist.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\muceditownerslist.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mucconfigureroom.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mucdestroyroom.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mucmessage.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mucprivatemessage.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mucdatamessage.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\mucusermenu.png"
  ; sound - captchaforms.def.xml
  SetOutPath $INSTDIR\resources\sounds\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\muc.def.xml"   
SectionEnd 

Section $(SecMetacontack) Metacontack  
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\metacontacts.dll"
  ; Resources: metacontacts.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\metacontacts.def.xml"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\metacontactscombine.png"  
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\metacontactsdetach.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\metacontactsdestroy.png"
SectionEnd

/*
+Data Form
  -Capcha
  -Registration
  -Ad hoc commands
    -Remote
*/
SectionGroup $(SecData_Form) Data_Form
Section "-Data Form" Data_Form_Base
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\dataforms.dll"
  File "${PROGRAM_BIN_FOLDER}\plugins\sessionnegotiation.dll"
  ; Resources: snegotiation.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\snegotiation.def.xml"  
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\snegotiation.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\snegotiationinit.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\snegotiationterminate.png"
  ; Resources\sound: snegotiation.def.xml
  SetOutPath $INSTDIR\resources\sounds\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\snegotiation.def.xml" 
SectionEnd

Section $(SecCapcha) Capcha
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\captchaforms.dll"
  ; Resources: captchaforms.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\captchaforms.def.xml"    
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\captchaforms.png"
  ; sound - captchaforms.def.xml
  SetOutPath $INSTDIR\resources\sounds\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\captchaforms.def.xml"
SectionEnd

Section $(SecRegistration) Registration
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\registration.dll"
  ; Resources: register.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\register.def.xml"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\register.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\registerremove.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\registerchange.png" 
SectionEnd

SectionGroup $(SecAd_hoc_commands) Ad_hoc_commands
Section "-Ad hoc commands" Ad_hoc_commands_Base
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\commands.dll"
  ; Resources: commands.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\commands.def.xml"  
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\commands.png"
SectionEnd
Section $(SecRemote) Remote
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\remotecontrol.dll"
  ; Resources 
SectionEnd
SectionGroupEnd
SectionGroupEnd

/*
+Console
  -Compress
*/
SectionGroup $(SecConsole) Console
Section "-Console" Console_Base
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\console.dll"
  ; Resources: console.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\console.def.xml"    
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\console.png"
SectionEnd
Section $(SecCompress) Compress
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\compress.dll"
  ; Resources     
SectionEnd

SectionGroupEnd
/*
+Message archive
  -File archive
  -Server archive
*/
SectionGroup $(SecMessage_archive) Message_archive
Section "-Message archive" Message_archive_Base
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\messagearchiver.dll"
  ; Resources: history.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\history.def.xml"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\history.png"  
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\historydate.png"
SectionEnd

Section $(SecFile_archive) File_archive
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\filemessagearchive.dll"
  ; Resources      
SectionEnd
Section $(SecServer_archive) Server_archive
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\servermessagearchive.dll"
  ; Resources    
SectionEnd
SectionGroupEnd

/*
+ Servicediscovery
+ Duplicate_messages
+ Gateways
+ Bob
+ OOB
+ Autostatus
+ Chat_states
+ Privacy_lists
+ Roster_exchange
+ Client_info
+ Aut_via_iq
+ Short_cut
+ Avatar
+ Birthday
+ URL
+ Bob_URL_Handler
*/

Section $(SecDiscovery) Discovery
SectionIn  1
	;Plugins
	SetOutPath $INSTDIR\plugins
	File "${PROGRAM_BIN_FOLDER}\plugins\servicediscovery.dll"
	; Resources: sdiscovery.def.xml
	SetOutPath $INSTDIR\resources\menuicons\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\sdiscovery.def.xml"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\arrowleft.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\arrowright.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\sdiscoverydiscoinfo.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\sdiscoverydiscover.png"
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\sdiscoveryreload.png"	
	
SectionEnd

Section $(SecDuplicate_messages) Duplicate_messages
SectionIn  1
	;Plugins
	SetOutPath $INSTDIR\plugins
	File "${PROGRAM_BIN_FOLDER}\plugins\messagecarbons.dll"
	; Resources
	SetOutPath $INSTDIR\resources\menuicons\shared
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\messagecarbons.def.xml" 
	File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\messagecarbons.png"
SectionEnd

Section $(SecGateways) Gateways
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\gateways.dll"
  ; Resources: gateways.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\gateways.def.xml"    
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\gateways.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\gatewaysaddcontact.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\gatewayslogin.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\gatewayslogout.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\gatewayskeepconnection.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\gatewayschange.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\gatewaysresolve.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\gatewaysremove.png"
SectionEnd

Section $(SecBob) Bob
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\bitsofbinary.dll"
  ; Resources:  
SectionEnd

Section $(SecOOB) OOB
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\oob.dll"
  ; Resources: oob.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\oob.def.xml"    
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\link.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\linkadd.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\linkready.png"
SectionEnd

Section $(SecAutostatus) Autostatus
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\autostatus.dll"
  ; Resources:  autostatus.def.xml
;  SetOutPath $INSTDIR\resources\menuicons\shared
;  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\autostatus.def.xml"   
;  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\autostatus.png"
SectionEnd

Section $(SecChat_states) Chat_states
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\chatstates.dll"
  ; Resources:  chatstates.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\chatstates.def.xml"     
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\chatstatesunknown.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\chatstatesactive.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\chatstatescomposing.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\chatstatespaused.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\chatstatesinactive.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\chatstatesgone.png"
SectionEnd

Section $(SecPrivacy_lists) Privacy_lists
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\privacylists.dll"
  ; Resources: privacylists.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\privacylists.def.xml" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\privacylists.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\privacylistslist.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\privacylistsvisible.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\privacylistsinvisible.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\privacylistsignore.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\privacylistsenable.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\privacylistsdisable.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\privacylistsblock.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\privacylistsadvanced.png"
SectionEnd

Section $(SecRoster_exchange) Roster_exchange
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\rosteritemexchange.dll"
  ; Resources:  rosterexchange.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rosterexchange.def.xml"     
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\rosterexchangerequest.png"
  ; Resources\sounds:  rosterexchange.def.xml
  SetOutPath $INSTDIR\resources\sounds\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\rosterexchange.def.xml"       
SectionEnd

Section $(SecClient_info) Client_info
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\clientinfo.dll"
  ; Resources:  clientinfo.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\clientinfo.def.xml"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\clientinfo.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\clientinfoactivity.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\clientinfotime.png"
SectionEnd

Section $(SecAut_via_iq) Aut_via_iq
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\iqauth.dll"
  ; Resources:   
SectionEnd

Section $(SecShort_cut) Short_cut
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\shortcutmanager.dll"
  ; Resources:   shortcuts.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\shortcuts.def.xml"  
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\shortcuts.png"
SectionEnd

Section $(SecAvatar) Avatar
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\avatars.dll"
  ; Resources: avatar.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\avatar.def.xml"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\avatarset.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\avatarremove.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\avatarchange.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\avatarcustom.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\avatarempty.png"
SectionEnd

Section $(SecBirthday) Birthday
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\birthdayreminder.dll"
  ; Resources:   birthdayreminder.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\birthdayreminder.def.xml" 
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\birthdayremindernotify.png"
  ; Resources\sounds:   birthdayreminder.def.xml
  SetOutPath $INSTDIR\resources\sounds\shared  
  File /r "${PROGRAM_BIN_FOLDER}\resources\sounds\shared\birthdayreminder.def.xml" 
SectionEnd

SectionGroup $(SecURL) URL
Section "-URL" URL_Base
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\urlprocessor.dll"
  ; Resources:   
SectionEnd

Section $(SecBob_URL_Handler) Bob_URL_Handler
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\boburlhandler.dll"
  ; Resources:   
SectionEnd
SectionGroupEnd


;+Abbreviations
/*-----------------------------------------------*/
Section $(SecAbbreviations) Abbreviations
SectionIn  1 
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\abbreviations.dll"
  ; Resources
  SetOutPath $INSTDIR\resources\abbreviations\shared
  File /r /x .svn "${PROGRAM_BIN_FOLDER}\resources\abbreviations\shared\*.*"
SectionEnd 

Section $(SecNickName) NickName
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\nickname.dll"
  ; Resources: nickname.def.xml
  SetOutPath $INSTDIR\resources\menuicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\nickname.def.xml"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\nickname.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\nicknameresolve.png"
  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\nicknamereset.png"
  
SectionEnd

 Section $(SecXHTML_IM) XHTML_IM
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\xhtmlim.dll"
  ; Resources - resources\xhtml
  SetOutPath $INSTDIR\resources\xhtml
  File /r "${PROGRAM_BIN_FOLDER}\resources\xhtml\*.*"
SectionEnd

Section $(SecXMPP_URI) XMPP_URI
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\xmppuriqueries.dll"
  ; Resources  
SectionEnd

Section $(SecStatistics) Statistics
SectionIn  1
  ;Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\statistics.dll"
  ; Resources: 
SectionEnd

/*
+Message style
 --Adium
   --Renkoo
   --yMouse
 */
SectionGroup $(SecMessage_style) Message_style
SectionGroup $(SecAdiummessagestyle) Adiummessagestyle
Section "-Adiummessagestyle" Adiummessagestyle_Base
SectionIn  1 
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\adiummessagestyle.dll"
  ; Resources
  SetOutPath $INSTDIR\resources\adiummessagestyles\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\adiummessagestyles\shared\Template.html"
SectionEnd

Section $(SecRenkoo) Renkoo
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\adiummessagestyles\renkoo
  File /r "${PROGRAM_BIN_FOLDER}\resources\adiummessagestyles\renkoo\*.*"
SectionEnd

Section $(SecyMouse) yMouse
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\adiummessagestyles\yMous
  File /r "${PROGRAM_BIN_FOLDER}\resources\adiummessagestyles\yMous\*.*"
SectionEnd
SectionGroupEnd
SectionGroupEnd

/*
;Status icone
*/
SectionGroup $(SecStatus_icons) Status_icons
Section "-statusicons" Status_icons_Base
SectionIn  1
  ; Plugins 
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\statusicons.dll"
  ;Resources: statusicons.def.xml
;  SetOutPath $INSTDIR\resources\menuicons\shared
;  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\statusicons.def.xml"
;  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\statusiconsoptions.png"
  SetOutPath $INSTDIR\resources\statusicons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\shared\*.*"  
SectionEnd

Section $(SecAim) Aim
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\aim
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\aim\*.*"
SectionEnd
Section $(SecBot) Bot
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\bot
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\bot\*.*"
SectionEnd
Section $(SecCar) Car
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\car
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\car\*.*"
SectionEnd
Section $(SecConference) Conference
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\conference
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\conference\*.*"
SectionEnd
Section $(SecFacebook) Facebook
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\facebook
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\facebook\*.*"
SectionEnd
Section $(SecGadu_gadu) Gadu_gadu
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\gadu
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\gadu\*.*"
SectionEnd
Section $(SecGoogle_talk) Google_talk
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\gtalk
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\gtalk\*.*"  
SectionEnd
Section $(SecICQ) ICQ
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\icq
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\icq\*.*"  
SectionEnd
Section $(SecLivejornal) Livejornal
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\livejournal
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\livejournal\*.*"  
SectionEnd
Section $(SecMailRuIm) MailRuIm
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\mrim
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\mrim\*.*"  
SectionEnd
Section $(SecMSN) MSN
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\msn
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\msn\*.*"  
SectionEnd
Section $(SecOdnoklassniki) Odnoklassniki
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\odnoklassniki
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\odnoklassniki\*.*"  
SectionEnd
Section $(SecRSS) RSS
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\rss
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\rss\*.*"  
SectionEnd
Section $(SecSkype) Skype
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\skype
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\skype\*.*"  
SectionEnd
Section $(SecSMS) SMS
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\sms
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\sms\*.*"  
SectionEnd
Section $(SecSMTP) SMTP
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\smtp
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\smtp\*.*"  
SectionEnd
Section $(SecTwitter) Twitter
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\twitter
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\twitter\*.*"  
SectionEnd
Section $(SecvKontakte) vKontakte
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\vkontakte
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\vkontakte\*.*"  
SectionEnd
Section $(SecWeather) Weather
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\weather
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\weather\*.*"  
SectionEnd
Section $(SecYahooIm) YahooIm
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\yahoo
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\yahoo\*.*"  
SectionEnd
Section $(SecYaOnline) YaOnline
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\resources\statusicons\yaonline
  File /r "${PROGRAM_BIN_FOLDER}\resources\statusicons\yaonline\*.*"  
SectionEnd
SectionGroupEnd
/*
+Emoticons
  --Default
  --Blobs_purple
  SetOutPath $INSTDIR\plugins
*/
SectionGroup $(SecEmoticons) Emoticons
Section "-Emoticons" Emoticons_Base
SectionIn  1
  ; Plugins 
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\emoticons.dll"
  ; Resources : emoticons.def.xml
;  SetOutPath $INSTDIR\resources\menuicons\shared
;  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\emoticons.def.xml"
;  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\emoticons.png"
;  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\arrowup.png"
;  File /r "${PROGRAM_BIN_FOLDER}\resources\menuicons\shared\arrowdown.png"
SectionEnd

Section $(SecEI_Default) EI_Default
SectionIn  1
  ; Resources - resources\emoticons\default
  SetOutPath $INSTDIR\resources\emoticons\default
  File /r "${PROGRAM_BIN_FOLDER}\resources\emoticons\default\*.*"  
SectionEnd

Section $(SecEI_Blobs_purple) EI_Blobs_purple
SectionIn  1
  ; Resources - resources\emoticons\blobs_purple
  SetOutPath $INSTDIR\resources\emoticons\blobs_purple
  File /r "${PROGRAM_BIN_FOLDER}\resources\emoticons\blobs_purple\*.*"  
SectionEnd
SectionGroupEnd
  
;Client icone
Section $(SecClient_icone) Client_icone
SectionIn  1
  ; Plugins 
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\clienticons.dll"
  ; Resources
  SetOutPath $INSTDIR\resources\clienticons\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\clienticons\shared\*.*"    
SectionEnd

/*
+ Wizards
 - + Wizardaccount
 - + Wizardtransport
*/
SectionGroup $(SecWizards) Wizards
Section "-SecWizardsRes" WizardsRes
SectionIn  1
  ; Resources: \resources\wizards
  SetOutPath $INSTDIR\resources\wizards\shared
  File /r "${PROGRAM_BIN_FOLDER}\resources\wizards\shared\*.*"
SectionEnd 
Section $(SecWizardaccount) Wizardaccount 
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\wizardaccount.dll"
  ; Resources
SectionEnd 

Section $(SecWizardtransport) Wizardtransport
SectionIn  1
  ; Plugins
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\wizardtransport.dll"
  ; Translations
  ; Resources:
  
SectionEnd 
SectionGroupEnd

/*
+ SDK
*/
Section $(SecSDK) SDK
SectionIn  1 
  ; Plugins
  ; Resources
  SetOutPath $INSTDIR\sdk
  File /r "${PROGRAM_BIN_FOLDER}\sdk\*.*"
  SetOutPath $INSTDIR
  File "${PROGRAM_BIN_FOLDER}\libeyecuutils.a"
SectionEnd

/*
+ Spellchecker
*/
SectionGroup $(SecSpellchecker) Spellchecker
Section "-Spellchecker" Spellchecker_Base
SectionIn  1 
  ; Plugins 
  SetOutPath $INSTDIR\plugins
  File "${PROGRAM_BIN_FOLDER}\plugins\spellchecker.dll"
  ; Resources
SectionEnd

Section $(SecLng_Russian) Lng_Russian
SectionIn  1 
  ; Resources
  SetOutPath $INSTDIR\hunspell
  File /r "${PROGRAM_BIN_FOLDER}\hunspell\ru_RU.dic"
  File /r "${PROGRAM_BIN_FOLDER}\hunspell\ru_RU.aff"
SectionEnd
Section $(SecLng_English_US) Lng_English_US
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\hunspell
  File /r "${PROGRAM_BIN_FOLDER}\hunspell\en_US.dic"
  File /r "${PROGRAM_BIN_FOLDER}\hunspell\en_US.aff"
SectionEnd
Section $(SecLng_English_AU) Lng_English_AU
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\hunspell
  File /r "${PROGRAM_BIN_FOLDER}\hunspell\en_AU.dic"
  File /r "${PROGRAM_BIN_FOLDER}\hunspell\en_AU.aff"
SectionEnd
Section $(SecLng_English_GB) Lng_English_GB
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\hunspell
  File /r "${PROGRAM_BIN_FOLDER}\hunspell\en_GB.dic"
  File /r "${PROGRAM_BIN_FOLDER}\hunspell\en_GB.aff"
SectionEnd
Section $(SecLng_English_SA) Lng_English_SA
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\hunspell
  File /r "${PROGRAM_BIN_FOLDER}\hunspell\en_ZA.dic"
  File /r "${PROGRAM_BIN_FOLDER}\hunspell\en_ZA.aff"
SectionEnd
Section $(SecLng_Deutschland) Lng_Deutschland
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\hunspell
  File /r "${PROGRAM_BIN_FOLDER}\hunspell\de_DE.dic"
  File /r "${PROGRAM_BIN_FOLDER}\hunspell\de_DE.aff"
SectionEnd
Section $(SecLng_Polish) Lng_Polish
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\hunspell
  File /r "${PROGRAM_BIN_FOLDER}\hunspell\pl_PL.dic"
  File /r "${PROGRAM_BIN_FOLDER}\hunspell\pl_PL.aff"
SectionEnd
Section $(SecLng_Ukraine) Lng_Ukraine
SectionIn  1
  ; Resources
  SetOutPath $INSTDIR\hunspell
  File /r "${PROGRAM_BIN_FOLDER}\hunspell\uk_UA.dic"
  File /r "${PROGRAM_BIN_FOLDER}\hunspell\uk_UA.aff"
SectionEnd
SectionGroupEnd

/*
+TS
*/
;-------------------------------------
	!include eyecu-langv.nsi
;-------------------------------------

Section $(SecDoc) Doc
SectionIn  1 
; Resources
  SetOutPath $INSTDIR\Docs
  File /r "${PROGRAM_BIN_FOLDER}\Docs\*.*"  
SectionEnd
  