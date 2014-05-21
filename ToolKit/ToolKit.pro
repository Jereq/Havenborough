#-------------------------------------------------
#
# Project created by QtCreator 2014-04-28T08:50:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ToolKit
TEMPLATE = app
DESTDIR = "$(SolutionDir)Client/Bin"

DXSDK_DIR = $$(DXSDK_DIR)

win32 {
    TEMPLATE = vcapp
    INCLUDEPATH = "$${DXSDK_DIR}/Include"
    LIBS += -L"$${DXSDK_DIR}Lib/x86"

    QMAKE_CXXFLAGS_DEBUG += -wd4996
    QMAKE_CXXFLAGS_RELEASE += -wd4996
}

INCLUDEPATH += "$(SolutionDir)Common/Source"
debug: LIBS += -L"$(SolutionDir)Common/Test"
release: LIBS += -L"$(SolutionDir)Common/Bin"

INCLUDEPATH += "$(SolutionDir)Graphics/include"
debug: LIBS += -L"$(SolutionDir)Graphics/Test"
release: LIBS += -L"$(SolutionDir)Graphics/Bin"

INCLUDEPATH += "$(SolutionDir)Physics/include"
debug: LIBS += -L"$(SolutionDir)Physics/Test"
release: LIBS += -L"$(SolutionDir)Physics/Bin"

INCLUDEPATH += "$(SolutionDir)Common/3rd party"
INCLUDEPATH += "$(BOOST_INC_DIR)"
LIBS += -L"$(BOOST_LIB_DIR)"

SOURCES += Source/main.cpp\
        Source/MainWindow.cpp \
    Source/TreeItem.cpp \
    Source/TreeFilter.cpp \
    Source/Camera.cpp \
    Source/KeyboardControl.cpp \
    Source/FlyControl.cpp \
    Source/ObjectManager.cpp \
    Source/DXWidget.cpp \
    Source/MyDX11Widget.cpp \
    Source/Tree.cpp \
    Source/XMLLevel.cpp \
    Source/qFileSystemModelDialog.cpp \
    Source/ToolManager.cpp \
    Source/CameraInterpolation.cpp \

HEADERS  += Source/MainWindow.h \
    Source/TreeItem.h \
    Source/TreeFilter.h \
    Source/DXWidget.h \
    Source/MyDX11Widget.h \
    Source/Camera.h \
    Source/KeyboardControl.h \
    Source/FlyControl.h \
    Source/ObjectManager.h \
    Source/Tree.h \
    Source/XMLLevel.h \
    Source/qFileSystemModelDialog.h \
    Source/ToolManager.h \
    Source/Tools.h \
    Source/CameraInterpolation.h \
    Source/ToolManager.h \

FORMS    += Source/MainWindow.ui

RESOURCES += \
    Resources.qrc
