# unit test of the text written by 7-Zip, rar...: built and run by tests/run_tests.py
QT       += core
QT       -= gui
CONFIG   += console c++11
CONFIG   -= app_bundle
TARGET    = process_text_test
TEMPLATE  = app
INCLUDEPATH += ../../src/utils
SOURCES  += process_text_test.cpp ../../src/utils/ProcessText.cpp
HEADERS  += ../../src/utils/ProcessText.h
