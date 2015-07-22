# Translations to build and install
TRANS_LANGS  = en ru pl de uk ja nl es

# Translation file name
isEmpty(TARGET_SHORT): TRANS_TARGET = $$member(TARGET, 0)
else: TRANS_TARGET = $$member(TARGET_SHORT, 0)

# Translations to update from source
TRANS_SOURCE = $$member(TARGET,0)
TRANSLATIONS = \
			   $${TRANS_SOURCE_ROOT}/translations/en/$${TRANS_SOURCE}.ts \
			   $${TRANS_SOURCE_ROOT}/translations/ru/$${TRANS_SOURCE}.ts

include(../make/translations.inc)
