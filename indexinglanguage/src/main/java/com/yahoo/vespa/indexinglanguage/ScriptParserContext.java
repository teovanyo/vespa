// Copyright Vespa.ai. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.indexinglanguage;

import com.yahoo.language.Linguistics;
import com.yahoo.language.process.Chunker;
import com.yahoo.language.process.Embedder;
import com.yahoo.language.process.FieldGenerator;
import com.yahoo.vespa.indexinglanguage.linguistics.AnnotatorConfig;
import com.yahoo.vespa.indexinglanguage.parser.CharStream;

import java.util.Collections;
import java.util.Map;

/**
 * @author Simon Thoresen Hult
 */
public class ScriptParserContext {

    private AnnotatorConfig annotatorConfig = new AnnotatorConfig();
    private Linguistics linguistics;
    private final Map<String, Chunker> chunkers;
    private final Map<String, Embedder> embedders;
    private final Map<String, FieldGenerator> generators;
    private String defaultFieldName = null;
    private CharStream inputStream = null;

    public ScriptParserContext(Linguistics linguistics,
                               Map<String, Chunker> chunkers,
                               Map<String, Embedder> embedders,
                               Map<String, FieldGenerator> generators) {
        this.linguistics = linguistics;
        this.chunkers = chunkers;
        this.embedders = embedders;
        this.generators = generators;
    }

    public AnnotatorConfig getAnnotatorConfig() {
        return annotatorConfig;
    }

    public ScriptParserContext setAnnotatorConfig(AnnotatorConfig config) {
        annotatorConfig = new AnnotatorConfig(config);
        return this;
    }

    public Linguistics getLinguistcs() {
        return linguistics;
    }

    public ScriptParserContext setLinguistics(Linguistics linguistics) {
        this.linguistics = linguistics;
        return this;
    }

    public Map<String, Chunker> getChunkers() {
        return Collections.unmodifiableMap(chunkers);
    }

    public Map<String, Embedder> getEmbedders() {
        return Collections.unmodifiableMap(embedders);
    }
    
    public Map<String, FieldGenerator> getGenerators() {
        return Collections.unmodifiableMap(generators);
    }

    public String getDefaultFieldName() {
        return defaultFieldName;
    }

    public ScriptParserContext setDefaultFieldName(String name) {
        defaultFieldName = name;
        return this;
    }

    public CharStream getInputStream() {
        return inputStream;
    }

    public ScriptParserContext setInputStream(CharStream stream) {
        inputStream = stream;
        return this;
    }

}
