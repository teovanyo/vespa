# Copyright Vespa.ai. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
vespa_add_executable(config_configfetcher_test_app TEST
    SOURCES
    configfetcher_test.cpp
    DEPENDS
    vespa_config
    GTest::gtest
)
vespa_add_test(NAME config_configfetcher_test_app COMMAND config_configfetcher_test_app)
vespa_generate_config(config_configfetcher_test_app ../../test/resources/configdefinitions/my.def)
