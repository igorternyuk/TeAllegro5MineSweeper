TEMPLATE = app
CONFIG -= console c++14
CONFIG -= app_bundle
CONFIG -= qt
DEFINES += DEBUG
#Можно по отдельности подключать
#LIBS += -lallegro
#LIBS += -lallegro_dialog
#LIBS += -lallegro_image
#А можно пакетом
LIBS += `pkg-config --libs allegro-5.0 allegro_dialog-5.0 allegro_ttf-5.0 allegro_font-5.0 allegro_primitives-5.0 allegro_image-5.0 allegro_audio-5.0 allegro_acodec-5.0`
SOURCES += main.cpp \
    model.cpp \
    controller.cpp \
    view.cpp
QMAKE_CXXFLAGS += -std=c++14
#QMAKE_CXXFLAGS += -static

HEADERS += \
    model.h \
    controller.h \
    view.h \
    modellistener.h
