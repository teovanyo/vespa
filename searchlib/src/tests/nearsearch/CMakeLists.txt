# Copyright Vespa.ai. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
vespa_add_executable(searchlib_nearsearch_test_app TEST
    SOURCES
    nearsearch_test.cpp
    DEPENDS
    searchlib_queryeval_test
    vespa_searchlib
    GTest::gtest
)
vespa_add_test(NAME searchlib_nearsearch_test_app COMMAND searchlib_nearsearch_test_app)
