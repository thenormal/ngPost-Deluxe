# unit test of SettingsTransfer (export and import of the settings and the profiles): built and run by tests/run_tests.py
QT       += core
QT       -= gui
CONFIG   += console c++11
CONFIG   -= app_bundle
TARGET    = settings_transfer_test
TEMPLATE  = app
INCLUDEPATH += ../../src/utils
SOURCES  += settings_transfer_test.cpp ../../src/utils/SettingsTransfer.cpp ../../src/utils/PostProfile.cpp
HEADERS  += ../../src/utils/SettingsTransfer.h ../../src/utils/PostProfile.h
