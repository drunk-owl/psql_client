CONFIG -= qt
TEMPLATE = lib
TARGET = psql_client
VERSION = 0.0.1

include(./lib.prf)

HEADERS += \
    psql_conn_pool.h \
    property.h \
    psql_conn.h \
    psql_result.h \
    psql_tcp_conn.h \
    psql_uds_conn.h

SOURCES += \
    psql_conn_pool.cpp \
    psql_conn.cpp \
    psql_result.cpp \
    psql_tcp_conn.cpp \
    psql_uds_conn.cpp

QMAKE_CXXFLAGS += -std=gnu++11

LIBS += -lboost_system -lpq

CONFIG += link_prl
QMAKE_LFLAGS += -Wl,--as-needed

#TODO: Implement binary format
