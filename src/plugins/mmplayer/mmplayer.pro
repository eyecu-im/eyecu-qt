TARGET = mmplayer 

include(mmplayer.pri)
include(../plugins.inc)
include(../pkgtest.inc)

contains(QT_CONFIG, multimediakit): {
CONFIG += mobility
MOBILITY += multimedia
}
else: contains(QT_CONFIG, multimedia): QT += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += ffmpeg util
else: CONFIG += ffmpeg util
