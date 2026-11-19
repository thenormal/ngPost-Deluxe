# unit test of the speed limit (slots of time given to the Articles): built and run by tests/run_tests.py
QT       += core
QT       -= gui
CONFIG   += console c++11
CONFIG   -= app_bundle
TARGET    = speed_limiter_test
TEMPLATE  = app
INCLUDEPATH += ../../src/utils
SOURCES  += speed_limiter_test.cpp ../../src/utils/SpeedLimiter.cpp
HEADERS  += ../../src/utils/SpeedLimiter.h
