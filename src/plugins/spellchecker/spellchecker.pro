TARGET = spellchecker
os2:TARGET_SHORT = spellchk
include(spellchecker.pri)
include(../plugins.inc)

USE_ENCHANT {
  include(enchantchecker.inc)
} else:USE_ASPELL {
  include(aspellchecker.inc)
} else:USE_MACSPELL {
  include(macspellchecker.inc)
} else {
  include(hunspellchecker.inc)
}
