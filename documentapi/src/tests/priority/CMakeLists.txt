# Copyright Vespa.ai. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
vespa_add_executable(documentapi_priority_test_app TEST
    SOURCES
    priority_test.cpp
    DEPENDS
    vespa_documentapi
    GTest::gtest
)
vespa_add_test(NAME documentapi_priority_test_app COMMAND documentapi_priority_test_app)
