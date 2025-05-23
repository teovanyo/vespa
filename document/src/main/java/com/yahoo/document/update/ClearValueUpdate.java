// Copyright Vespa.ai. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.document.update;

import com.yahoo.document.DataType;
import com.yahoo.document.datatypes.FieldValue;
import com.yahoo.document.serialization.DocumentUpdateWriter;

/**
 * A value update that removes the current value of the field.
 *
 * @author Einar M R Rosenvinge
 */
public class ClearValueUpdate extends ValueUpdate {

    public ClearValueUpdate() {
        super(ValueUpdateClassID.CLEAR);
    }

    @Override
    public FieldValue applyTo(FieldValue fval) {
        return null;
    }

    @Override
    protected void checkCompatibility(DataType fieldType) {
        // empty
    }

    @Override
    public FieldValue getValue() {
        return null;
    }

    @Override
    public void setValue(FieldValue value) {
        // empty
    }

    @Override
    public void serialize(DocumentUpdateWriter data, DataType superType) {
        data.write(this, superType);
    }
}
