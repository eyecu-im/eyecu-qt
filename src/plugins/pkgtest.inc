#Check availability of optional featires
message(FEATURE_DIR=$$FEATURE_DIR)
greaterThan(QT_MAJOR_VERSION, 4): QT_CONFIG *= serialport multimedia webkit script
else {
	exists($$QMAKESPEC): FEATURE_DIR=$$QMAKESPEC/../features
	else: exists($(QMAKESPEC)): FEATURE_DIR=$(QMAKESPEC)/../features
	else: exists($(QTDIR)): FEATURE_DIR=$(QTDIR)/mkspecs/features
	else: exists($(UNIXROOT)/usr/share/qt4/mkspecs/features): FEATURE_DIR=$(UNIXROOT)/usr/share/qt4/mkspecs/features

	FEATURES = serialport mobility
	for(feature, FEATURES): exists($$FEATURE_DIR/$${feature}.prf): QT_CONFIG *= $${feature}
}
