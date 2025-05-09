// Copyright Vespa.ai. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.indexinglanguage.expressions;

import com.yahoo.document.DataType;

/**
 * @author Simon Thoresen Hult
 */
public final class SetVarExpression extends Expression {

    private final String varName;

    public SetVarExpression(String varName) {
        this.varName = varName;
    }

    @Override
    public boolean isMutating() { return false; }

    public String getVariableName() { return varName; }

    @Override
    public DataType setInputType(DataType inputType, TypeContext context) {
        context.setVariableType(varName, inputType);
        return super.setInputType(inputType, context);
    }

    @Override
    public DataType setOutputType(DataType outputType, TypeContext context) {
        if (outputType == null) return null;
        context.setVariableType(varName, leastGeneralNonNullOf(outputType, context.getVariableType(varName)));
        return super.setOutputType(outputType, context);
    }

    private void setVariableType(DataType newType, TypeContext context) {
        DataType existingType = context.getVariableType(varName);
        DataType mostGeneralType = newType;
        if (existingType != null) {
            if (existingType.isAssignableTo(newType))
                mostGeneralType = newType;
            else if (newType.isAssignableTo(existingType))
                mostGeneralType = existingType;
            else
                throw new VerificationException(this, "Cannot set variable '" + varName + "' to type " + newType.getName() +
                                                      ": It is already set to type " + existingType.getName());
        }
        context.setVariableType(varName, mostGeneralType);
    }

    @Override
    protected void doExecute(ExecutionContext context) {
        context.setVariable(varName, context.getCurrentValue());
    }

    @Override
    public String toString() {
        return "set_var " + varName;
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof SetVarExpression rhs)) return false;
        if (!varName.equals(rhs.varName)) return false;
        return true;
    }

    @Override
    public int hashCode() {
        return getClass().hashCode() + varName.hashCode();
    }

}
