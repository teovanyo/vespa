# Copyright Vespa.ai. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
vespa_add_executable(streamingvisitors_searchvisitor_test_app TEST
    SOURCES
    searchvisitor_test.cpp
    DEPENDS
    vespa_streamingvisitors
    GTest::gtest
)
vespa_add_test(NAME streamingvisitors_searchvisitor_test_app COMMAND streamingvisitors_searchvisitor_test_app)
