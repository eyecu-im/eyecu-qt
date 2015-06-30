TRANS_LANGS  = ru pl de uk ja nl es
TRANS_SOURCE = $$member(TARGET,0)
isEmpty(TARGET_SHORT): TRANS_TARGET = $$member(TARGET, 0)
else: TRANS_TARGET = $$member(TARGET_SHORT, 0)
TRANSLATIONS = \
			   $${TRANS_SOURCE_ROOT}/translations/en/$${TRANS_SOURCE}.ts \
			   $${TRANS_SOURCE_ROOT}/translations/ru/$${TRANS_SOURCE}.ts \
			   $${TRANS_SOURCE_ROOT}/translations/pl/$${TRANS_SOURCE}.ts \
			   $${TRANS_SOURCE_ROOT}/translations/de/$${TRANS_SOURCE}.ts \
			   $${TRANS_SOURCE_ROOT}/translations/uk/$${TRANS_SOURCE}.ts \
			   $${TRANS_SOURCE_ROOT}/translations/ja/$${TRANS_SOURCE}.ts \
			   $${TRANS_SOURCE_ROOT}/translations/nl/$${TRANS_SOURCE}.ts \
			   $${TRANS_SOURCE_ROOT}/translations/es/$${TRANS_SOURCE}.ts
include(../make/translations.inc)
