defineReplace(prependAll) {
    for(a,$$1):result += $$2$${a}$$3
    return($$result)
}

qtPrepareTool(QMAKE_LRELEASE, lrelease)
TRANSLATIONS = $$prependAll(LANGUAGES, $$PWD/translations/$${TARGET}_,.ts)

qm.depends = $${TRANSLATIONS}
qm.input = TRANSLATIONS
qm.output = $$OUT_PWD/../package/root/translations/${QMAKE_FILE_BASE}.qm
qm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
qm.name = LRELEASE ${QMAKE_FILE_IN}
qm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += qm
PRE_TARGETDEPS += compiler_qm_make_all
