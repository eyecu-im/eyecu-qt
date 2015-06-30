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
SectionGroup "Un.$(SecPersonalEvent)" UnPersonalEvent
Section "-Un.$(SecPepmanager)" UnPepmanager  
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\pepmanager.dll"
	; Resources:
	Delete "$INSTDIR\resources\menuicons\shared\pepmanager.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\pepmanager.png"	
	; Resources: pep.def.xml
	Delete "$INSTDIR\resources\sounds\shared\pep.def.xml"
	Delete "$INSTDIR\resources\sounds\shared\plopp.wav"
SectionEnd
Section "Un.$(SecActivity)" UnActivity  
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\activity.dll"
	RMDir /r "$INSTDIR\resources\activityicons"
SectionEnd 
Section "Un.$(SecMood)" UnMood
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mood.dll"
	; Resources:
	RMDir /r "$INSTDIR\resources\moodicons"
SectionEnd 
SectionGroup "Un.$(SecTune)" UnTune
Section "-Un.$(SecTuneMain)" UnTuneMain
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\tune.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecTuneInfo)" UnTuneInfo
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\tuneinforequesterlastfm.dll"
   ; Resources:
SectionEnd 
SectionGroup "Un.$(SecTuneListeners)" UnTuneListeners
Section "Un.$(SecWINAMP)" UnWINAMP
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\tunelistenerwinamp.dll"
  ; Resources:
SectionEnd 
Section "Un.$(SecAIMP)" UnAIMP
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\tunelisteneraimp.dll"
   ; Resources:
SectionEnd 
Section "Un.$(SecFileSource)" UnFileSource
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\tunelistenerfile.dll"
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
SectionGroup "Un.$(SecLocation)" UnLocation
Section "-Un.Geoloc"
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\geoloc.dll"
  ; Resources:  geoloc.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\geoloc.def.xml"
  Delete "$INSTDIR\resources\menuicons\shared\geoloc.png"
  Delete "$INSTDIR\resources\menuicons\shared\no_geoloc.png"
  Delete "$INSTDIR\resources\menuicons\shared\map.png"
  Delete "$INSTDIR\resources\menuicons\shared\satellite.png"
  Delete "$INSTDIR\resources\menuicons\shared\satelliteOFF.png"
  Delete "$INSTDIR\resources\menuicons\shared\satelliteON.png"
  Delete "$INSTDIR\resources\menuicons\shared\satelliteONR.png"
SectionEnd

SectionGroup "Un.$(SecPositioning)" UnPositioning
Section "-Un.Positioning"
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\positioning.dll"
  ; Resources: 
  Delete "$INSTDIR\resources\menuicons\shared\positioning.def.xml"
  Delete "$INSTDIR\resources\menuicons\shared\positioning.png"
  Delete "$INSTDIR\resources\menuicons\shared\serialport.png"
  Delete "$INSTDIR\resources\menuicons\shared\location.png"
  Delete "$INSTDIR\resources\menuicons\shared\manual.png"
  Delete "$INSTDIR\resources\menuicons\shared\geoip.gif"
  Delete "$INSTDIR\resources\menuicons\shared\freegeoip.png"  
SectionEnd

Section "Un.$(SecManual)" UnManual
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\positioningmethodmanual.dll"
  ; Resources:     
SectionEnd
Section  "Un.$(SecSerialport)" UnSerialport
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\positioningmethodserialport.dll"
	; Resources
SectionEnd 

SectionGroup "Un.$(SecMetodIP)" UnMetodIP
Section  "Un.$(SecMetodIP)" UnMetodIP_Base
SectionIn  1
  ; Plugins
  Delete "$INSTDIR\plugins\positioningmethodip.dll"
  ; Resources
SectionEnd 

Section "Un.$(SecProviderIPGeoip)" UnProviderIPGeoip
SectionIn  1
  ; Plugins
  Delete "$INSTDIR\plugins\positioningmethodipproviderfreegeoip.dll"
  ; Resources
SectionEnd
SectionGroupEnd

/* ---now not used -- INTERFACE-LOCATION---
Section  -"Un.$(SecIntLocation)" UnIntLocation
SectionIn  1
  ; Plugins

  Delete "$INSTDIR\plugins\positioningmethodserialport.dll"
  ; Translations
  ; Resources
SectionEnd 
*/

SectionGroupEnd

Section "Un.$(SecContactNotifies)" UnContactNotifies
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\contactproximitynotification.dll"
	; Resources: proximity.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\proximity.def.xml" 
	Delete "$INSTDIR\resources\menuicons\shared\proximity.png" 
	; Resources/sound: proximity.def.xml
	Delete "$INSTDIR\resources\sounds\shared\proximity.def.xml" 
	Delete "$INSTDIR\resources\sounds\shared\sonar.wav"   	
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
SectionGroup "Un.$(SecMap)" UnMap
Section "-Un.$(SecMapMain)"  UnMapMain
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\map.dll"
;	Delete "$INSTDIR\plugins\mapscene.dll"
	Delete "$INSTDIR\plugins\maplocationselector.dll"
	; Resources: map-\mapicons
	RMDir /r "$INSTDIR\resources\mapicons"
	; Resources: 
	Delete "$INSTDIR\resources\menuicons\shared\mapsources.def.xml"  
	Delete "$INSTDIR\resources\menuicons\shared\google.png" 
	Delete "$INSTDIR\resources\menuicons\shared\yandex.png" 
	Delete "$INSTDIR\resources\menuicons\shared\yahoo.png" 
	Delete "$INSTDIR\resources\menuicons\shared\kosmosnimki.png" 
	Delete "$INSTDIR\resources\menuicons\shared\bing.png" 
	Delete "$INSTDIR\resources\menuicons\shared\osm.png" 
	Delete "$INSTDIR\resources\menuicons\shared\wiki.png" 
	Delete "$INSTDIR\resources\menuicons\shared\navitel.png" 
	Delete "$INSTDIR\resources\menuicons\shared\mail.ru.png" 
	Delete "$INSTDIR\resources\menuicons\shared\here.png" 
	Delete "$INSTDIR\resources\menuicons\shared\2gis.png" 
	Delete "$INSTDIR\resources\menuicons\shared\navteq.png" 
	Delete "$INSTDIR\resources\menuicons\shared\esri.png" 
	Delete "$INSTDIR\resources\menuicons\shared\megafon.png" 
	Delete "$INSTDIR\resources\menuicons\shared\vitel.png" 
	Delete "$INSTDIR\resources\menuicons\shared\progorod.png" 
	Delete "$INSTDIR\resources\menuicons\shared\rosreestr.png" 
	Delete "$INSTDIR\resources\menuicons\shared\geocon.png" 
SectionEnd 

SectionGroup "Un.$(SecSources)" UnSources
Section "Un.$(SecGoogle)" UnGoogle
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsourcegoogle.dll"
   ; Resources:
SectionEnd 
Section "Un.$(SecDGIS)" UnDGIS
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsource2gis.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecBing)" UnBing
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsourcebing.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecEsri)" UnEsri
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsourceesri.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecHere)" UnHere
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsourceovi.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecKosmosnimki)" UnKosmosnimki
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsourcekosmosnimki.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecMailRu)" UnMailRu
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsourcemailru.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecMegafon)" UnMegafon
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsourcemegafon.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecNavitel)" UnNavitel
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsourcenavitel.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecNavteq)" UnNavteq
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsourcenavteq.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecOpenStreetMap)" UnOpenStreetMap
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsourceosm.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecPROGOROD)" UnPROGOROD
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsourceprogorod.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecRosReestr)" UnRosReestr
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsourcerosreestr.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecRuMap)" UnRuMap
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsourcerumap.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecVi_Tel)" UnVi_Tel
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsourcevitel.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecWikimapia)" UnWikimapia
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsourcewiki.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecYahoo)" UnYahoo
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsourceyahoo.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecYandex)" UnYandex
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsourceyandex.dll"
	; Resources:
SectionEnd 

SectionGroupEnd
;--------------------------------
SectionGroup "Un.$(SecMapContacts)" UnMapContacts
;Section "-Un.MapContacts"
Section "-Un.$(SecMapContactsMain)" UnMapContactsMain
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapcontacts.dll"
	; Resources:
SectionEnd 

Section "Un.$(SecMapMessage)" UnMapMessage
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapmessage.dll"
	; Resources: buttons.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\buttons.def.xml"  
	Delete "$INSTDIR\resources\menuicons\shared\close_active.png"  
	Delete "$INSTDIR\resources\menuicons\shared\close_inactive.png"
	;----in [poi.def.xml] ........
	;Delete "$INSTDIR\resources\menuicons\shared\mapmessage.png"
SectionEnd 
SectionGroupEnd

;--------------------------------
Section "Un.$(SecMagnifier)" UnMagnifier
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapmagnifier.dll"
	; Resources: mapmagnifier.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\mapmagnifier.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\magnifier.png" 
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
SectionGroup "Un.$(SecMapSearch)" UnMapSearch
Section "-Un.MapSearch"  UnMapSearchMain
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsearch.dll"
	; Resources: mapsearch.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\mapsearch.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\mapsearch.png" 
SectionEnd 
Section "Un.$(SecSearchFromGoogle)" UnSearchFromGoogle
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsearchprovidergoogle.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecSearchOpenStreetMap)" UnSearchOpenStreetMap
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsearchproviderosm.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecSearchFromDGIS)" UnSearchFromDGIS
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsearchprovider2gis.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecSearchFromYandex)" UnSearchFromYandex
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsearchprovideryandex.dll"
	; Resources:
SectionEnd 
Section "Un.$(SecSearchNavitel)" UnSearchNavitel
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsearchprovidernavitel.dll"
	; Resources: navitel.def.xml
	RMDir /r "$INSTDIR\resources\navitel"  
SectionEnd 
Section "Un.$(SecSearchFromHere)" UnSearchFromHere
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mapsearchproviderhere.dll"
	; Resources:
SectionEnd 
SectionGroupEnd


/*
+ MapStreetView
 - + StreetProvGoogle
*/
SectionGroup "Un.$(SecMapStreetView)" UnMapStreetView
Section "-Un.MapStreetView"  UnMapStreetViewMain
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\streetview.dll"
	; Resources
	Delete "$INSTDIR\resources\menuicons\shared\streetman.png"  
	Delete "$INSTDIR\resources\menuicons\shared\mapstreetview.png" 
SectionEnd 

Section "Un.$(SecStreetProvGoogle)" UnStreetProvGoogle
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\streetviewprovidergoogle.dll"
	; Resources: ......already have...............................
	;Delete "$INSTDIR\resources\menuicons\shared\google.png"   
SectionEnd 
SectionGroupEnd

/*
+ MapPlaceView
 - + PlaceProvGoogle
*/
SectionGroup "Un.$(SecMapPlaceView)" UnMapPlaceView
Section "-Un.$(SecMapPlaceViewMain)" UnMapPlaceViewMain
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\placeview.dll"
	; Resources
	Delete "$INSTDIR\resources\menuicons\shared\viewcenter.png"
	Delete "$INSTDIR\resources\menuicons\shared\key16.png"
SectionEnd 

Section "Un.$(SecPlaceViewProvGoogle)" UnPlaceViewProvGoogle
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\placeviewprovidergoogle.dll"
	; Resources: ......already have...............................
	;Delete "$INSTDIR\resources\menuicons\shared\google.png"   
SectionEnd 
SectionGroupEnd

/* +++++++++++++++++++++++++++++++++++++++++++++++++++
Section $(SecWeatherProv1) WeatherProv1
*/

/*
+ MMplayer
 -  (ffmpeg)
+ Receipts
*/
SectionGroup "Un.$(SecFFMpeg)" UnFFMpeg
Section "Un.$(SecFFMpegDll)"  UnFFMpegDll
SectionIn  1
	; Plugins
	Delete "$INSTDIR\Qt${QTFF}FFMpeg.dll"
	; Resources: FFMpeg libraries
	Delete "$INSTDIR\avcodec-56.dll"
	Delete "$INSTDIR\avdevice-56.dll"
	Delete "$INSTDIR\avfilter-5.dll"
	Delete "$INSTDIR\avformat-56.dll"
	Delete "$INSTDIR\avutil-54.dll"
	Delete "$INSTDIR\postproc-53.dll"
	Delete "$INSTDIR\swresample-1.dll"
	Delete "$INSTDIR\swscale-3.dll"
SectionEnd 

Section "Un.$(SecMMplayer)" UnMMplayer
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\mmplayer.dll"
	; Resources: mmplayer.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\mmplayer.def.xml"  
	Delete "$INSTDIR\resources\menuicons\shared\mmplayer.png" 
	Delete "$INSTDIR\resources\menuicons\shared\eject.png"  
SectionEnd 
SectionGroupEnd

/*
+Attention
*/
Section "Un.$(SecAttention)" UnAttention 
SectionIn  1 
	;Plugins
	Delete "$INSTDIR\plugins\attention.dll"
	; Resources: resources\menuicons\shared\notifications.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\notifications.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\notifications.png"
	Delete "$INSTDIR\resources\menuicons\shared\notificationsactivateall.png"
	Delete "$INSTDIR\resources\menuicons\shared\notificationsremoveall.png"
	Delete "$INSTDIR\resources\menuicons\shared\notificationssoundon.png"
	Delete "$INSTDIR\resources\menuicons\shared\notificationssoundoff.png"
	Delete "$INSTDIR\resources\menuicons\shared\notificationssoundplay.png"
	Delete "$INSTDIR\resources\menuicons\shared\notificationspopupwindow.png"
	Delete "$INSTDIR\resources\menuicons\shared\notificationsshowminimized.png"
  ; Resources: resources\sounds\shared\attention.def.xml
	Delete "$INSTDIR\resources\sounds\shared\attention.def.xml"	
	Delete "$INSTDIR\resources\sounds\shared\bells.wav"
SectionEnd 

/*
+ Receipts
*/
Section "Un.$(SecReceipts)" UnReceipts
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\receipts.dll"
	; Resources: in eyecuicon.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\receipts.png" 
	; Resources: in receipts.def.xml
	Delete "$INSTDIR\resources\sounds\shared\receipts.def.xml"
	Delete "$INSTDIR\resources\sounds\shared\floop.wav"
SectionEnd 

/*
+Private Storage
  -Bookmarks
  -Annotations
  -Recent contacts
  -Poi
*/
SectionGroup "Un.$(SecPrivatestorage)" UnPrivatestorage
Section "-Un.$(SecPrivatestorageBase)" UnPrivatestorageBase
SectionIn  1 2 3
  ; Plugins
  Delete "$INSTDIR\plugins\privatestorage.dll"
  ; Resources
SectionEnd 

Section "Un.$(SecBookmarks)" UnBookmarks 
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\bookmarks.dll"
  ; Resources: bookmarks.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\bookmarks.def.xml"
  Delete "$INSTDIR\resources\menuicons\shared\bookmarks.png"
  Delete "$INSTDIR\resources\menuicons\shared\bookmarksadd.png"
  Delete "$INSTDIR\resources\menuicons\shared\bookmarksremove.png"
  Delete "$INSTDIR\resources\menuicons\shared\bookmarksroom.png"
  Delete "$INSTDIR\resources\menuicons\shared\bookmarksurl.png"
  Delete "$INSTDIR\resources\menuicons\shared\bookmarksedit.png"
SectionEnd 

Section "Un.$(SecAnnotations)" UnAnnotations 
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\annotations.dll"
  ; Resources: annotations.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\annotations.def.xml"  
  Delete "$INSTDIR\resources\menuicons\shared\annotations.png"
SectionEnd 

Section "Un.$(SecRecent_contacts)" UnRecent_contacts 
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\recentcontacts.dll"
  ; Resources - recentcontacts.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\recentcontacts.def.xml"
  Delete "$INSTDIR\resources\menuicons\shared\recentcontacts.png" 
  Delete "$INSTDIR\resources\menuicons\shared\recentcontactsfavorite.png" 
  Delete "$INSTDIR\resources\menuicons\shared\recentcontactsinsertfavorite.png" 
  Delete "$INSTDIR\resources\menuicons\shared\recentcontactsremovefavorite.png" 
  Delete "$INSTDIR\resources\menuicons\shared\recentcontactsremoverecent.png" 
SectionEnd 

Section "Un.$(SecPoi)" UnPoi 
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\poi.dll"
  ; Resources: resources\country\shared, resources\menuicons\shared, resources\typepoint\shared
  RMDir /r "$INSTDIR\resources\country"
  RMDir /r "$INSTDIR\resources\typepoint"
  Delete "$INSTDIR\resources\menuicons\shared\poi.def.xml"
  Delete "$INSTDIR\resources\menuicons\shared\poi.png"
  Delete "$INSTDIR\resources\menuicons\shared\poiadd.png"
  Delete "$INSTDIR\resources\menuicons\shared\poinone.png"
  Delete "$INSTDIR\resources\menuicons\shared\poitoolbar.png"
  Delete "$INSTDIR\resources\menuicons\shared\poiview.png"
  Delete "$INSTDIR\resources\menuicons\shared\globus.png"
  Delete "$INSTDIR\resources\menuicons\shared\flag24.png"
  Delete "$INSTDIR\resources\menuicons\shared\tracker.png"
;  Delete "$INSTDIR\resources\menuicons\shared\mapmessage.png"
  Delete "$INSTDIR\resources\menuicons\shared\description.png"
  Delete "$INSTDIR\resources\menuicons\shared\connect.png"
  Delete "$INSTDIR\resources\menuicons\shared\connectend.png"
  Delete "$INSTDIR\resources\menuicons\shared\connectlong.png"
  Delete "$INSTDIR\resources\menuicons\shared\folder.png"
  Delete "$INSTDIR\resources\menuicons\shared\folderopen.png"
  Delete "$INSTDIR\resources\menuicons\shared\bgrmap.png"
  Delete "$INSTDIR\resources\menuicons\shared\bgrsat.png"
  Delete "$INSTDIR\resources\menuicons\shared\poinotype.png"
SectionEnd 
SectionGroupEnd

/*
+File transfer
  -SOcks5
  -In band
*/
SectionGroup "Un.$(SecFile_transfer)" UnFile_transfer
Section "-Un.File transfer"
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\filetransfer.dll"
  Delete "$INSTDIR\plugins\datastreamsmanager.dll"
  Delete "$INSTDIR\plugins\filestreamsmanager.dll"
  ; Resources: filetransfer.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\filetransfer.def.xml"  
  Delete "$INSTDIR\resources\menuicons\shared\filetransfersend.png" 
  Delete "$INSTDIR\resources\menuicons\shared\filetransferreceive.png"
  ; Resources: filestreamsmanager.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\filestreamsmanager.def.xml" 
  Delete "$INSTDIR\resources\menuicons\shared\filestreamsmanager.png"
  ; Resources: datastreamsmanager.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\datastreamsmanager.def.xml" 
  Delete "$INSTDIR\resources\menuicons\shared\datastreamsmanager.png"
  ; Resources: sound\shared\filetransfer.def.xml
  Delete "$INSTDIR\resources\sounds\shared\filetransfer.def.xml" 
SectionEnd 

Section "Un.$(SecSOcks5)" UnSocks5 
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\socksstreams.dll"
  ; Translations
  ; Resources
SectionEnd 
Section "Un.$(SecIn_band)" UnIn_band 
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\inbandstreams.dll"
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
Section "Un.$(SecVcard)" UnVcard 
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\vcard.dll"
  ; Resources: vcard.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\vcard.def.xml"    
  Delete "$INSTDIR\resources\menuicons\shared\vcard.png"
SectionEnd

Section "Un.$(SecJabber_search)" UnJabber_search  
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\jabbersearch.dll"
  ; Resources: jsearch.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\jsearch.def.xml"      
  Delete "$INSTDIR\resources\menuicons\shared\jsearch.png"
SectionEnd 

Section "Un.$(SecRoster_search)" UnRoster_search  
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\rostersearch.dll"
  ; Resources: rostersearch.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\rostersearch.def.xml"      
  Delete "$INSTDIR\resources\menuicons\shared\rostersearch.png"
SectionEnd 

Section "Un.$(SecMUC)" UnMUC  
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\multiuserchat.dll"
  ; Resources: muc.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\muc.def.xml"      
  Delete "$INSTDIR\resources\menuicons\shared\mucconference.png"
  Delete "$INSTDIR\resources\menuicons\shared\mucexitroom.png"
  Delete "$INSTDIR\resources\menuicons\shared\mucjoin.png"
  Delete "$INSTDIR\resources\menuicons\shared\mucinvite.png"
  Delete "$INSTDIR\resources\menuicons\shared\mucchangenick.png"
  Delete "$INSTDIR\resources\menuicons\shared\mucchangetopic.png"
  Delete "$INSTDIR\resources\menuicons\shared\mucclearchat.png"
  Delete "$INSTDIR\resources\menuicons\shared\mucenterroom.png"
  Delete "$INSTDIR\resources\menuicons\shared\mucexitroom.png"
  Delete "$INSTDIR\resources\menuicons\shared\mucrequestvoice.png"
  Delete "$INSTDIR\resources\menuicons\shared\muceditbanList.png"
  Delete "$INSTDIR\resources\menuicons\shared\muceditmemberslist.png"
  Delete "$INSTDIR\resources\menuicons\shared\muceditadminslist.png"
  Delete "$INSTDIR\resources\menuicons\shared\muceditownerslist.png"
  Delete "$INSTDIR\resources\menuicons\shared\mucconfigureroom.png"
  Delete "$INSTDIR\resources\menuicons\shared\mucdestroyroom.png"
  Delete "$INSTDIR\resources\menuicons\shared\mucmessage.png"
  Delete "$INSTDIR\resources\menuicons\shared\mucprivatemessage.png"
  Delete "$INSTDIR\resources\menuicons\shared\mucdatamessage.png"
  Delete "$INSTDIR\resources\menuicons\shared\mucusermenu.png"
  ; sound - captchaforms.def.xml
  Delete "$INSTDIR\resources\sounds\shared\muc.def.xml"   
SectionEnd 

Section "Un.$(SecMetacontack)" UnMetacontack  
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\metacontacts.dll"
  ; Resources: metacontacts.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\metacontacts.def.xml"
  Delete "$INSTDIR\resources\menuicons\shared\metacontactscombine.png"  
  Delete "$INSTDIR\resources\menuicons\shared\metacontactsdetach.png"
  Delete "$INSTDIR\resources\menuicons\shared\metacontactsdestroy.png"
SectionEnd

/*
+Data Form
  -Capcha
  -Registration
  -Ad hoc commands
    -Remote
*/
SectionGroup "Un.$(SecData_Form)" UnData_Form
Section "-Un.Data Form"
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\dataforms.dll"
  Delete "$INSTDIR\plugins\sessionnegotiation.dll"
  ; Resources: snegotiation.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\snegotiation.def.xml"  
  Delete "$INSTDIR\resources\menuicons\shared\snegotiation.png"
  Delete "$INSTDIR\resources\menuicons\shared\snegotiationinit.png"
  Delete "$INSTDIR\resources\menuicons\shared\snegotiationterminate.png"
  ; Resources\sound: snegotiation.def.xml
  Delete "$INSTDIR\resources\sounds\shared\snegotiation.def.xml" 
SectionEnd

Section "Un.$(SecCapcha)" UnCapcha
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\captchaforms.dll"
  ; Resources: captchaforms.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\captchaforms.def.xml"    
  Delete "$INSTDIR\resources\menuicons\shared\captchaforms.png"
  ; sound - captchaforms.def.xml
  Delete "$INSTDIR\resources\sounds\shared\captchaforms.def.xml"
SectionEnd

Section "Un.$(SecRegistration)" UnRegistration
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\registration.dll"
  ; Resources: register.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\register.def.xml"
  Delete "$INSTDIR\resources\menuicons\shared\register.png"
  Delete "$INSTDIR\resources\menuicons\shared\registerremove.png"
  Delete "$INSTDIR\resources\menuicons\shared\registerchange.png" 
SectionEnd

SectionGroup "Un.$(SecAd_hoc_commands)" UnAd_hoc_commands
Section "-Un.Ad hoc commands"
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\commands.dll"
  ; Resources: commands.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\commands.def.xml"  
  Delete "$INSTDIR\resources\menuicons\shared\commands.png"
SectionEnd
Section "Un.$(SecRemote)" UnRemote
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\remotecontrol.dll"
  ; Resources 
SectionEnd
SectionGroupEnd
SectionGroupEnd

/*
+Console
  -Compress
*/
SectionGroup "Un.$(SecConsole)" UnConsole
Section "-Un.Console"
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\console.dll"
  ; Resources: console.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\console.def.xml"    
  Delete "$INSTDIR\resources\menuicons\shared\console.png"
SectionEnd
Section "Un.$(SecCompress)" UnCompress
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\compress.dll"
  ; Resources     
SectionEnd

SectionGroupEnd
/*
+Message archive
  -File archive
  -Server archive
*/
SectionGroup "Un.$(SecMessage_archive)" UnMessage_archive
Section "-Un.Message archive"
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\messagearchiver.dll"
  ; Resources: history.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\history.def.xml"
  Delete "$INSTDIR\resources\menuicons\shared\history.png"  
  Delete "$INSTDIR\resources\menuicons\shared\historydate.png"
SectionEnd

Section "Un.$(SecFile_archive)" UnFile_archive
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\filemessagearchive.dll"
  ; Resources      
SectionEnd
Section "Un.$(SecServer_archive)" UnServer_archive
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\servermessagearchive.dll"
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

Section "Un.$(SecDiscovery)" UnDiscovery
SectionIn  1
	;Plugins
	Delete "$INSTDIR\plugins\servicediscovery.dll"
	; Resources: sdiscovery.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\sdiscovery.def.xml"
	Delete "$INSTDIR\resources\menuicons\shared\arrowleft.png"
	Delete "$INSTDIR\resources\menuicons\shared\arrowright.png"
	Delete "$INSTDIR\resources\menuicons\shared\sdiscoverydiscoinfo.png"
	Delete "$INSTDIR\resources\menuicons\shared\sdiscoverydiscover.png"
	Delete "$INSTDIR\resources\menuicons\shared\sdiscoveryreload.png"	
	
SectionEnd

Section "Un.$(SecDuplicate_messages)" UnDuplicate_messages
SectionIn  1 2 3
	;Plugins
	Delete "$INSTDIR\plugins\messagecarbons.dll"
	; Resources: messagecarbons.def.xml
	Delete "$INSTDIR\resources\menuicons\shared\messagecarbons.def.xml" 
	Delete "$INSTDIR\resources\menuicons\shared\messagecarbons.png"	
SectionEnd

Section "Un.$(SecGateways)" UnGateways
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\gateways.dll"
  ; Resources: gateways.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\gateways.def.xml"    
  Delete "$INSTDIR\resources\menuicons\shared\gateways.png"
  Delete "$INSTDIR\resources\menuicons\shared\gatewaysaddcontact.png"
  Delete "$INSTDIR\resources\menuicons\shared\gatewayslogin.png"
  Delete "$INSTDIR\resources\menuicons\shared\gatewayslogout.png"
  Delete "$INSTDIR\resources\menuicons\shared\gatewayskeepconnection.png"
  Delete "$INSTDIR\resources\menuicons\shared\gatewayschange.png"
  Delete "$INSTDIR\resources\menuicons\shared\gatewaysresolve.png"
  Delete "$INSTDIR\resources\menuicons\shared\gatewaysremove.png"
SectionEnd

Section "Un.$(SecBob)" UnBob
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\bitsofbinary.dll"
  ; Resources:  
SectionEnd

Section "Un.$(SecOOB)" UnOOB
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\oob.dll"
  ; Resources: oob.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\oob.def.xml"    
  Delete "$INSTDIR\resources\menuicons\shared\link.png"
  Delete "$INSTDIR\resources\menuicons\shared\linkadd.png"
  Delete "$INSTDIR\resources\menuicons\shared\linkready.png"
SectionEnd

Section "Un.$(SecAutostatus)" UnAutostatus
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\autostatus.dll"
  ; Resources:  autostatus.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\autostatus.def.xml"   
  Delete "$INSTDIR\resources\menuicons\shared\autostatus.png"
SectionEnd

Section "Un.$(SecChat_states)" UnChat_states
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\chatstates.dll"
  ; Resources:  chatstates.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\chatstates.def.xml"     
  Delete "$INSTDIR\resources\menuicons\shared\chatstatesunknown.png"
  Delete "$INSTDIR\resources\menuicons\shared\chatstatesactive.png"
  Delete "$INSTDIR\resources\menuicons\shared\chatstatescomposing.png"
  Delete "$INSTDIR\resources\menuicons\shared\chatstatespaused.png"
  Delete "$INSTDIR\resources\menuicons\shared\chatstatesinactive.png"
  Delete "$INSTDIR\resources\menuicons\shared\chatstatesgone.png"
SectionEnd

Section "Un.$(SecPrivacy_lists)" UnPrivacy_lists
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\privacylists.dll"
  ; Resources: privacylists.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\privacylists.def.xml" 
  Delete "$INSTDIR\resources\menuicons\shared\privacylists.png"
  Delete "$INSTDIR\resources\menuicons\shared\privacylistslist.png"
  Delete "$INSTDIR\resources\menuicons\shared\privacylistsvisible.png"
  Delete "$INSTDIR\resources\menuicons\shared\privacylistsinvisible.png"
  Delete "$INSTDIR\resources\menuicons\shared\privacylistsignore.png"
  Delete "$INSTDIR\resources\menuicons\shared\privacylistsenable.png"
  Delete "$INSTDIR\resources\menuicons\shared\privacylistsdisable.png"
  Delete "$INSTDIR\resources\menuicons\shared\privacylistsblock.png"
  Delete "$INSTDIR\resources\menuicons\shared\privacylistsadvanced.png"
SectionEnd

Section "Un.$(SecRoster_exchange)" UnRoster_exchange
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\rosteritemexchange.dll"
  ; Resources:  rosterexchange.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\rosterexchange.def.xml"     
  Delete "$INSTDIR\resources\menuicons\shared\rosterexchangerequest.png"
  ; Resources\sounds:  rosterexchange.def.xml
  Delete "$INSTDIR\resources\sounds\shared\rosterexchange.def.xml"       
SectionEnd

Section "Un.$(SecClient_info)" UnClient_info
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\clientinfo.dll"
  ; Resources:  clientinfo.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\clientinfo.def.xml"
  Delete "$INSTDIR\resources\menuicons\shared\clientinfo.png"
  Delete "$INSTDIR\resources\menuicons\shared\clientinfoactivity.png"
  Delete "$INSTDIR\resources\menuicons\shared\clientinfotime.png"
SectionEnd

Section "Un.$(SecAut_via_iq)" UnAut_via_iq
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\iqauth.dll"
  ; Resources:   
SectionEnd

Section "Un.$(SecShort_cut)" UnShort_cut
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\shortcutmanager.dll"
  ; Resources:   shortcuts.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\shortcuts.def.xml"  
  Delete "$INSTDIR\resources\menuicons\shared\shortcuts.png"
SectionEnd

Section "Un.$(SecAvatar)" UnAvatar
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\avatars.dll"
  ; Resources: avatar.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\avatar.def.xml"
  Delete "$INSTDIR\resources\menuicons\shared\avatarset.png"
  Delete "$INSTDIR\resources\menuicons\shared\avatarremove.png"
  Delete "$INSTDIR\resources\menuicons\shared\avatarchange.png"
  Delete "$INSTDIR\resources\menuicons\shared\avatarcustom.png"
  Delete "$INSTDIR\resources\menuicons\shared\avatarempty.png"
SectionEnd

Section "Un.$(SecBirthday)" UnBirthday
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\birthdayreminder.dll"
  ; Resources:   birthdayreminder.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\birthdayreminder.def.xml" 
  Delete "$INSTDIR\resources\menuicons\shared\birthdayremindernotify.png"
  ; Resources\sounds:   birthdayreminder.def.xml
  Delete "$INSTDIR\resources\sounds\shared\birthdayreminder.def.xml" 
SectionEnd

SectionGroup "Un.$(SecURL)" UnURL
Section "-Un.URL"
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\urlprocessor.dll"
  ; Resources:   
SectionEnd

Section "Un.$(SecBob_URL_Handler)" UnBob_URL_Handler
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\boburlhandler.dll"
  ; Resources:   
SectionEnd
SectionGroupEnd


;+Abbreviations
/*-----------------------------------------------*/
Section "Un.$(SecAbbreviations)" UnAbbreviations
SectionIn  1  2 3
  ; Plugins
  Delete "$INSTDIR\plugins\abbreviations.dll"
  ; Resources
  RMDir /r "$INSTDIR\resources\abbreviations"
SectionEnd 

Section "Un.$(SecNickName)" UnNickName
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\nickname.dll"
  ; Resources: nickname.def.xml
  Delete "$INSTDIR\resources\menuicons\shared\nickname.def.xml"
  Delete "$INSTDIR\resources\menuicons\shared\nickname.png"
  Delete "$INSTDIR\resources\menuicons\shared\nicknameresolve.png"
  Delete "$INSTDIR\resources\menuicons\shared\nicknamereset.png"
  
SectionEnd

 Section "Un.$(SecXHTML_IM)" UnXHTML_IM
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\xhtmlim.dll"
  ; Resources - resources\xhtml
  RMDir /r "$INSTDIR\resources\xhtml"
SectionEnd

Section "Un.$(SecXMPP_URI)" UnXMPP_URI
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\xmppuriqueries.dll"
  ; Resources  
SectionEnd

Section "Un.$(SecStatistics)" UnStatistics
SectionIn  1 2 3
  ;Plugins
  Delete "$INSTDIR\plugins\statistics.dll"
  ; Resources: 
SectionEnd

/*
+Message style
 --Adium
   --Renkoo
   --yMouse
 */
SectionGroup "Un.$(SecMessage_style)" UnMessage_style
SectionGroup "Un.$(SecAdiummessagestyle)" UnAdiummessagestyle
Section "-Un.Adiummessagestyle"
SectionIn  1 2 3
  ; Plugins
  Delete "$INSTDIR\plugins\adiummessagestyle.dll"
  ; Resources
  Delete "$INSTDIR\resources\adiummessagestyles\shared\Template.html"
SectionEnd

Section "Un.$(SecRenkoo)" UnRenkoo
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\adiummessagestyles\renkoo"
SectionEnd

Section "Un.$(SecyMouse)" UnyMouse
SectionIn  1 2 3
  ; Resources
  RMDir /r  "$INSTDIR\resources\adiummessagestyles\yMous"
SectionEnd
SectionGroupEnd
SectionGroupEnd

/*
;Status icone
*/
SectionGroup "Un.$(SecStatus_icons)" UnStatus_icons
Section "-Un.Statusicons"
SectionIn  1 2 3
  ; Plugins 
  Delete "$INSTDIR\plugins\statusicons.dll"
  ;Resources: statusicons.def.xml
;  Delete "$INSTDIR\resources\menuicons\shared\statusicons.def.xml"
;  Delete "$INSTDIR\resources\menuicons\shared\statusiconsoptions.png"
  RMDir /r "$INSTDIR\resources\statusicons"  
SectionEnd

Section "Un.$(SecAim)" UnAim
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\aim"
SectionEnd
Section "Un.$(SecBot)" UnBot
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\bot\"
SectionEnd
Section "Un.$(SecCar)" UnCar
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\car"
SectionEnd
Section "Un.$(SecConference)" UnConference
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\conference"
SectionEnd
Section "Un.$(SecFacebook)" UnFacebook
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\facebook"
SectionEnd
Section "Un.$(SecGadu_gadu)" UnGadu_gadu
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\gadu"
SectionEnd
Section "Un.$(SecGoogle_talk)" UnGoogle_talk
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\gtalk"  
SectionEnd
Section "Un.$(SecICQ)" UnICQ
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\icq"  
SectionEnd
Section "Un.$(SecLivejornal)" UnLivejornal
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\livejournal"  
SectionEnd
Section "Un.$(SecMailRuIm)" UnMailRuIm
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\mrim"  
SectionEnd
Section "Un.$(SecMSN)" UnMSN
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\msn"  
SectionEnd
Section "Un.$(SecOdnoklassniki)" UnOdnoklassniki
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\odnoklassniki"  
SectionEnd
Section "Un.$(SecRSS)" UnRSS
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\rss"  
SectionEnd
Section "Un.$(SecSkype)" UnSkype
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\skype"  
SectionEnd
Section "Un.$(SecSMS)" UnSMS
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\sms"  
SectionEnd
Section "Un.$(SecSMTP)" UnSMTP
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\smtp"  
SectionEnd
Section "Un.$(SecTwitter)" UnTwitter
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\twitter"  
SectionEnd
Section "Un.$(SecvKontakte)" UnvKontakte
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\vkontakte"  
SectionEnd
Section "Un.$(SecWeather)" UnWeather
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\weather"  
SectionEnd
Section "Un.$(SecYahooIm)" UnYahooIm
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\yahoo"  
SectionEnd
Section "Un.$(SecYaOnline)" UnYaOnline
SectionIn  1 2 3
  ; Resources
  RMDir /r "$INSTDIR\resources\statusicons\yaonline"  
SectionEnd
SectionGroupEnd
/*
+Emoticons
  --Default
  --Blobs_purple
*/
SectionGroup "Un.$(SecEmoticons)" UnEmoticons
Section "-Un.Emoticons"
SectionIn  1 2 3
  ; Plugins 
  Delete "$INSTDIR\plugins\emoticons.dll"
  ; Resources : emoticons.def.xml
;  Delete "$INSTDIR\resources\menuicons\shared\emoticons.def.xml"
;  Delete "$INSTDIR\resources\menuicons\shared\emoticons.png"
;  Delete "$INSTDIR\resources\menuicons\shared\arrowup.png"
;  Delete "$INSTDIR\resources\menuicons\shared\arrowdown.png"
SectionEnd

Section "Un.$(SecEI_Default)" UnEI_Default
SectionIn  1 2 3
  ; Resources - resources\emoticons\default
  RMDir /r "$INSTDIR\resources\emoticons\default"  
SectionEnd

Section "Un.$(SecEI_Blobs_purple)" UnEI_Blobs_purple
SectionIn  1 2 3
  ; Resources - resources\emoticons\blobs_purple
  RMDir /r "$INSTDIR\resources\emoticons\blobs_purple"  
SectionEnd
SectionGroupEnd
  
;Client icone
Section "Un.$(SecClient_icone)" UnClient_icone
SectionIn  1 2 3
  ; Plugins 
  Delete "$INSTDIR\plugins\clienticons.dll"
  ; Resources
  RMDir /r "$INSTDIR\resources\clienticons"    
SectionEnd

/*
+ Wizards
 - + Wizardaccount
 - + Wizardtransport
*/
SectionGroup "Un.$(SecWizards)" UnWizards
Section "-Un.SecWizardsRes" UnWizardsRes
SectionIn  1
  ; Resources: \resources\wizards
  RMDir /r "$INSTDIR\resources\wizards"
SectionEnd 
Section "Un.$(SecWizardaccount)" UnWizardaccount 
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\wizardaccount.dll"
  ; Resources
SectionEnd 

Section "Un.$(SecWizardtransport)" UnWizardtransport
SectionIn  1
	; Plugins
	Delete "$INSTDIR\plugins\wizardtransport.dll"
   ; Resources:
SectionEnd 
SectionGroupEnd

/*
+ SDK
*/
Section "Un.$(SecSDK)" UnSDK
SectionIn  1 2 3
  ; Plugins 
  ; Resources
  RMDir /r "$INSTDIR\Sdk"
  Delete "$INSTDIR\libeyecuutils.a"
SectionEnd

/*
+ Spellchecker
*/
SectionGroup "Un.$(SecSpellchecker)" UnSpellchecker
Section "-Un.Spellchecker"
SectionIn  1 2 3
  ; Plugins 
  Delete "$INSTDIR\plugins\spellchecker.dll"
  ; Resources
SectionEnd

Section "Un.$(SecLng_Russian)" UnLng_Russian
SectionIn  1 2 3
  ; Resources
  Delete "$INSTDIR\hunspell\ru_RU.dic"
  Delete "$INSTDIR\hunspell\ru_RU.aff"
SectionEnd
Section "Un.$(SecLng_English_US)" UnLng_English_US
SectionIn  1 2 3
  ; Resources
  Delete "$INSTDIR\hunspell\en_US.dic"
  Delete "$INSTDIR\hunspell\en_US.aff"
SectionEnd
Section "Un.$(SecLng_English_AU)" UnLng_English_AU
SectionIn  1 2 3
  ; Resources
  Delete "$INSTDIR\hunspell\en_AU.dic"
  Delete "$INSTDIR\hunspell\en_AU.aff"
SectionEnd
Section "Un.$(SecLng_English_GB)" UnLng_English_GB
SectionIn  1 2 3
  ; Resources
  Delete "$INSTDIR\hunspell\en_GB.dic"
  Delete "$INSTDIR\hunspell\en_GB.aff"
SectionEnd
Section "Un.$(SecLng_English_SA)" UnLng_English_SA
SectionIn  1 2 3
  ; Resources
  Delete "$INSTDIR\hunspell\en_ZA.dic"
  Delete "$INSTDIR\hunspell\en_ZA.aff"
SectionEnd
Section "Un.$(SecLng_Deutschland)" UnLng_Deutschland
SectionIn  1 2 3
  ; Resources
  Delete "$INSTDIR\hunspell\de_DE.dic"
  Delete "$INSTDIR\hunspell\de_DE.aff"
SectionEnd
Section "Un.$(SecLng_Polish)" UnLng_Polish
SectionIn  1 2 3
  ; Resources
  Delete "$INSTDIR\hunspell\pl_PL.dic"
  Delete "$INSTDIR\hunspell\pl_PL.aff"
SectionEnd
Section "Un.$(SecLng_Ukraine)" UnLng_Ukraine
SectionIn  1 2 3
  ; Resources
  Delete "$INSTDIR\hunspell\uk_UA.dic"
  Delete "$INSTDIR\hunspell\uk_UA.aff"
SectionEnd
SectionGroupEnd

/*
+TS
*/
;-------------------------------------
	!include eyecu-un-langv.nsi
;-------------------------------------

Section "Un.$(SecDoc)" UnDoc
SectionIn  1 2 3
; Resources
  RMDir /r "$INSTDIR\Docs"  
SectionEnd
  