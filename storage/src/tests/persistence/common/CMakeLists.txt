# Copyright Vespa.ai. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
vespa_add_library(storage_testpersistence_common TEST
    SOURCES
    filestortestfixture.cpp
    persistenceproviderwrapper.cpp
    DEPENDS
    GTest::gtest
    vespa_persistence
    storage_testcommon
)
