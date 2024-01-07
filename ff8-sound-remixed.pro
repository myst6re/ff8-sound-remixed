QT += core gui widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    core/io/audiofmt.cpp \
    core/ff8installation.cpp \
    core/sound.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    core/io/audiofmt.h \
    core/ff8installation.h \
    core/sound.h \
    mainwindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32 {
    DEFINES += PROG_VERSION=\"\\\"1.0.0\\\"\" RC_PRODUCT_VERSION=1,0,0,0 PROG_NAME=\"\\\"FF8 Sound Reloaded\\\"\" RC_COMPANY_NAME_STR=\"\\\"myst6re\\\"\"
    RC_FILE = Resources.rc
    LIBS += -ladvapi32 -lMsacm32 -lWinmm
}

OTHER_FILES += Resources.rc \
    README.md
