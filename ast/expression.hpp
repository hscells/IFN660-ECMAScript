#pragma once
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include "node.hpp"
#include <string> 
#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <vector>
#include <sstream>
#include "narrations.h"


#include "../type/type.hpp"

using namespace std;



extern int global_var;
extern int codeScopeDepth;

inline unsigned int getNewRegister() {
	return global_var++;
}

class Expression:public Node{
public:
	virtual unsigned int genCode() = 0;
	virtual unsigned int genStoreCode()=0;
};

class DecimalIntegerLiteralExpression:public Expression{
private:
    int value;
    int refID;
public:
    DecimalIntegerLiteralExpression(int value){
        this->value = value;
    };

    int getValue() {
        return value;
    }

    void dump(int indent){
        label(indent, "IntegerLiteralExpression: %d\n", value);
    }
    	  
    unsigned int genCode() {
        return getNewRegister();
    }
	
	unsigned int genStoreCode() {
		unsigned int registerNumber = getNewRegister();		
		emit("\tESValue* r%d = new Number(%d);", registerNumber, this->getValue());
		return registerNumber;
	 };
};

class DecimalLiteralExpression:public Expression{
private:
    double value;
    
public:
    DecimalLiteralExpression(double value){
        this->value = value;
    };

    double getValue() {
        return value;
    }
    
    void dump(int indent){
        label(indent, "IntegerLiteralExpression: %d\n", value);
    }

    unsigned int genCode() {
        return getNewRegister();
    }
	
	unsigned int genStoreCode() {
		unsigned int registerNumber = getNewRegister();
		emit("\tESValue* r%d = new Reference(env, \"%d\");", registerNumber, this->getValue());
		return registerNumber;
	};
};

class IdentifierExpression:public Expression{
private:
    std::string name;
    Reference* reference;
    int refID;
public:
    IdentifierExpression(std::string name){
        this->name = name;
        this->reference = NULL;
    };
    
    IdentifierExpression() {};

    std::string getReferencedName() {
        return name;
    }
    

    void dump(int indent){
        label(indent, "IdentifierExpression: %s\n", name.c_str());
    }

	unsigned int genCode() {
		return getNewRegister();
	}

	unsigned int genStoreCode() 	{
		unsigned int registerNumber = getNewRegister();
		emit("\tESValue* r%d = new Reference(new String(\"%s\"));", registerNumber, this->getReferencedName().c_str());
		return registerNumber;
	}
};

class StringLiteralExpression: public Expression {
private:
	std::string val;
	const char* result;
public:
	StringLiteralExpression(const char* val) {
		this->val = std::string(val);
		this->result = val;
	};

    std::string getValue() {
        return val;
    }
    
    int getIntValue() {
		unsigned uintVar;
	  	std::istringstream in(val);
	   	in >> uintVar;
	   	return uintVar;
		
    }
    
	void dump(int indent) {
		label(indent, "StringLiteralExpression: %s\n", val.c_str());
	}

    unsigned int genCode() 	{
        return getNewRegister();
    }

	unsigned int genStoreCode() {
		unsigned int registerNumber = getNewRegister();
		emit("\tESValue* r%d = new Reference(env, \"%s\");", registerNumber, this->getValue().c_str());
		return registerNumber;
	};
};

class AssignmentExpression:public Expression {
private:
    Expression *lhs, *rhs;
    int lhsReg, rhsReg;
public:
    AssignmentExpression(Expression *lhs, Expression *rhs) {
        this->lhs = lhs;
        this->rhs = rhs;
    };
    
    AssignmentExpression() {};

    AssignmentExpression(Expression* expression){
        this->lhs = expression;
    }

    void dump(int indent) {
        label(indent, "AssignmentExpression\n");
        lhs->dump(indent + 1, "lhs");
        if(rhs != NULL){
            rhs->dump(indent + 1, "rhs");
        }
    }

    unsigned int genStoreCode() 	{
    	
    	unsigned int lhsRegisterNumber = lhs->genStoreCode();
		unsigned int rhsRegisterNumber = rhs->genStoreCode();
		unsigned int registerNumber = getNewRegister();	

		emit("\tESValue* r%d = Core::assign(r%d, r%d);", registerNumber, lhsRegisterNumber, rhsRegisterNumber);
		return registerNumber;
	}
   
    
    unsigned int genCode() {
        return getNewRegister();
    }
};

class ObjectLiteralExpression : public Expression {
private:
    vector<Expression*> *propertyDefinitionList;
public:
    //No parameter constructor
    ObjectLiteralExpression(){};
    ObjectLiteralExpression(vector<Expression*> *propertyDefinitionList) {
        this->propertyDefinitionList = propertyDefinitionList;
    };

    void dump(int indent) {
        label(indent, "ObjectLiteralExpression\n");

        if(propertyDefinitionList != NULL) {
            for (vector<Expression*>::iterator iter = propertyDefinitionList->begin(); iter != propertyDefinitionList->end(); ++iter)
                (*iter)->dump(indent+1);
        }
    }

    unsigned int genCode() {
        return getNewRegister();
    }
	
	unsigned int genStoreCode() {
		return global_var;
	};

};

class PropertyDefinitionExpression : public Expression {
private:
    Expression *key;
    Expression *value;

public:
    PropertyDefinitionExpression(Expression *key, Expression *value) {
        this->key = key;
        this->value = value;
    };

    void dump(int indent) {
        label(indent, "PropertyDefinitionExpression\n");
        indent++;
        label(indent, "Key\n");
        key->dump(indent + 1);
        label(indent, "Value\n");
        if (value != NULL) {
            value->dump(indent + 1);
        } else {
            label(indent + 1, "[UNDEFINED]\n");
        }
    }

    unsigned int genCode() {
        return getNewRegister();
    }
	
	unsigned int genStoreCode() {
		return global_var;
	};
	

};

class LiteralPropertyNameExpression : public Expression {
    Expression *literalExpression;
public:
    LiteralPropertyNameExpression(Expression *literalExpression) {
        this->literalExpression = literalExpression;
    };

    void dump (int indent) {
        label(indent, "LiteralPropertyNameExpression\n");
        indent++;
        literalExpression->dump(indent);
    }

    unsigned int genCode() {
        return getNewRegister();
    }
	
	unsigned int genStoreCode() {
		return global_var;
	};
};

class ComputedPropertyNameExpression : public Expression {
private:
    Expression *computedExpression;
public:
    ComputedPropertyNameExpression(Expression *computedExpression) {
        this->computedExpression = computedExpression;
    };

    void dump(int indent) {
        label(indent, "ComputedPropertyNameExpression\n");
        indent++;
        computedExpression->dump(indent);
    }

    unsigned int genCode() 	{
        return getNewRegister();
    }
	
	unsigned int genStoreCode() {
		return global_var;
	};

};

/* See ECMA Specifications http://www.ecma-international.org/ecma-262/6.0/#sec-unary-operators */
/* Unary operator + * / - % */
class UnaryExpression : public Expression {


private:
	 Expression *unaryExpression;
	 
public:
	UnaryExpression(Expression *unaryExpression) {
		this->unaryExpression = unaryExpression;
	};
	
	unsigned int genCode() {
        return getNewRegister();
    }

    unsigned int genStoreCode() {
        return getNewRegister();
    }
        
    void dump(int indent) {
        unaryExpression->dump(indent + 1, "rhs");
    }
    
    /* Called by all subclasses of UnaryExpression for file output of register operation
     * Must call in explicit ordering to get correct file output - cannot call in emit(..)
     * Operation defined in narrations.h
     */
    unsigned int fileEmit(const char* operation) {
        unsigned int rhsRegister = unaryExpression->genStoreCode();
        unsigned int registerNumber = getNewRegister();
        emit("\tESValue* r%d = Core::%s(r%d);", registerNumber, operation,  rhsRegister);
        return registerNumber;
    }
};

/* Add operator Unary Expression - Subclass Unary Expression*/
class UnaryAddExpression : public UnaryExpression {

private:
    Expression* rhs;

public:
    UnaryAddExpression(Expression* rhs) : UnaryExpression(rhs) {
        this->rhs = rhs;
    }
    
    unsigned int genStoreCode() {
        return fileEmit(ADDITION_U);
    }
    
    void dump(int indent) {
        label(indent, "AdditionUnaryExpression: +\n");
        UnaryExpression::dump(indent);
    }
};


/* Subtract operator Unary Expression - Subclass Unary Expression*/
class UnarySubtractExpression : public UnaryExpression {

private:
    Expression* rhs;

public:
    UnarySubtractExpression(Expression* rhs) : UnaryExpression(rhs) {
        this->rhs = rhs;
    }
    
    unsigned int genStoreCode() {
        return fileEmit(SUBTRACTION_U);
    }
    
    void dump(int indent) {
        label(indent, "SubtractionUnaryExpression: -\n");
        UnaryExpression::dump(indent);
    }
};

/* To be removed - replace with BinaryExpression */
class PlusAditiveExpression : public Expression {

private:
	char operand; 
	Expression *lhs;
	Expression *rhs;
	int opRegL, opRegR;
	 
	public:
	PlusAditiveExpression(Expression *lhs,  Expression *rhs) {
		this->lhs = lhs;
		this->rhs = rhs;
	};
	 
	 
	void dump(int indent) {
		label(indent, "PlusAdditiveExpression\n");
		lhs->dump(++indent);		
		rhs->dump(indent);   
    }
    
	unsigned int genCode() 	{
        return getNewRegister();
    }
	
	
	unsigned int genStoreCode() {
		unsigned int lhsRegisterNumber = lhs->genStoreCode();
		unsigned int rhsRegisterNumber =  rhs->genStoreCode();
		unsigned int registerNumber = getNewRegister();	
		emit("\tESValue* r%d = Core::plus(r%d, r%d);", registerNumber, lhsRegisterNumber, rhsRegisterNumber );
		return registerNumber;
		
	};
};

class Arguments : public Expression {
private:
    std::vector<AssignmentExpression*>* argumentList;
public:
    Arguments(vector<AssignmentExpression*>* argumentList){
        this->argumentList = argumentList;
    };

    void dump(int indent) {
        label(indent, "Arguments\n");
        for (vector<AssignmentExpression*>::iterator iter = argumentList->begin(); iter != argumentList->end(); ++iter)
          (*iter)->dump(indent+1);
    }

  unsigned int genCode() {
      return getNewRegister();
    }
};


class CallExpression : public Expression {
private:
    Expression* expression;
    Arguments* arguments;
public:
    CallExpression(Expression *expression) {
        this->expression = expression;
    };

    void dump(int indent) {
        label(indent, "CallExpression\n");
        indent++;
        expression->dump(indent);
        if(arguments != NULL) {
            arguments->dump(indent+1);
        }
    }


    unsigned int genCode() {
        return getNewRegister();
    }
};

class ArrayLiteralExpression : public Expression {
private:
    vector<Expression*> *elementList;
public:
    //No parameter constructor
    ArrayLiteralExpression(){};
    ArrayLiteralExpression(vector<Expression*> *elementList) {
        this->elementList = elementList;
    };

    void dump(int indent) {
        label(indent, "ArrayLiteralExpression\n");

        if(elementList != NULL) {
            for (vector<Expression*>::iterator iter = elementList->begin(); iter != elementList->end(); ++iter)
                (*iter)->dump(indent+1);
        }
    }

    unsigned int genCode() {
        return getNewRegister();
    }
	
	unsigned int genStoreCode() {
		return global_var;
	};

};

/* Each Binary Expression will inherit from BinaryExpression
 * Operators for Binary Expression -->'+', '-', '*', '/' 
 * Each operator will be implemented in subclass of BinaryExpression
*/
class BinaryExpression : public Expression {
private:
    Expression* lhs;
    Expression* rhs;

public:
    BinaryExpression(Expression* lhs, Expression* rhs) {
        this->lhs = lhs;
        this->rhs = rhs;
    };
    
    unsigned int genCode() {
		return getNewRegister();
	}

    unsigned int genStoreCode() {
		return getNewRegister();
	}
        
    void dump(int indent) {
        lhs->dump(indent + 1, "lhs");
        if(rhs != NULL){
            rhs->dump(indent + 1, "rhs");
        }
    }
    
    /* Called by all subclasses of BinaryExpression for file output of register operation
     * Must call in explicit ordering to get correct file output - cannot call in emit(..)
     * Operation defined in narrations.h
     */
    unsigned int fileEmit(const char* operation) {
    	unsigned int lhsRegister = lhs->genStoreCode();
    	unsigned int rhsRegister = rhs->genStoreCode();
    	unsigned int registerNumber = getNewRegister();
		emit("\tESValue* r%d = Core::%s(r%d, r%d);", registerNumber, operation,  lhsRegister, rhsRegister);
		return registerNumber;
	}
    
};

/* Plus additive operator Binary Expression */
class AdditiveBinaryExpression : public BinaryExpression {

private:
	Expression* lhs;
	Expression* rhs;

public:
	AdditiveBinaryExpression(Expression* lhs, Expression* rhs) : BinaryExpression(lhs, rhs) {
		this->lhs = lhs;
		this->rhs = rhs;
	}
	
    unsigned int genStoreCode() {
    	return fileEmit(ADDITION);
	}
	
	void dump(int indent) {
		label(indent, "AdditiveBinaryExpression: +\n");
		BinaryExpression::dump(indent);
	}
};

/* Subtraction operator Binary Expression */
class SubtractionBinaryExpression : public BinaryExpression {

private:
	Expression* lhs;
	Expression* rhs;

public:
	SubtractionBinaryExpression(Expression* lhs, Expression* rhs) : BinaryExpression(lhs, rhs) {
		this->lhs = lhs;
		this->rhs = rhs;
	}
	
    unsigned int genStoreCode() {
    	return fileEmit(SUBTRACTION);
	}
	
	void dump(int indent) {
		label(indent, "SubtractionBinaryExpression: -\n");
		BinaryExpression::dump(indent);
	}
};


/* Multiplicative operator Binary Expression */
class MultiplicativeBinaryExpression : public BinaryExpression {

private:
	Expression* lhs;
	Expression* rhs;

public:
	MultiplicativeBinaryExpression(Expression* lhs, Expression* rhs) : BinaryExpression(lhs, rhs) {
		this->lhs = lhs;
		this->rhs = rhs;
	}
	
    unsigned int genStoreCode() {
    	return fileEmit(MULTIPLICATION);
	}
	
	void dump(int indent) {
		label(indent, "MultiplicativeBinaryExpression: *\n");
		BinaryExpression::dump(indent);
	}
};


/* Division operator Binary Expression */
class DivisionBinaryExpression : public BinaryExpression {

private:
	Expression* lhs;
	Expression* rhs;

public:
	DivisionBinaryExpression(Expression* lhs, Expression* rhs) : BinaryExpression(lhs, rhs) {
		this->lhs = lhs;
		this->rhs = rhs;
	}
	
    unsigned int genStoreCode() {
    	return fileEmit(DIVISION);
	}
	
	void dump(int indent) {
		label(indent, "DivisionBinaryExpression: /\n");
		BinaryExpression::dump(indent);
	}
};

/* Each Equality Expression will inherit from EqualityExpression class
 * Operators for Binary Expression -->'==', '<', '>', '!=' 
 * Each operator will be implemented in subclass of BinaryExpression
*/
class EqualityExpression : public Expression {
private:
    Expression* lhs;
    Expression* rhs;

public:
    EqualityExpression(Expression* lhs, Expression* rhs) {
        this->lhs = lhs;
        this->rhs = rhs;
    };
    
    unsigned int genCode() {
		return getNewRegister();
	}

    unsigned int genStoreCode() {
		return getNewRegister();
	}
        
    void dump(int indent) {
        lhs->dump(indent + 1, "lhs");
        if(rhs != NULL){
            rhs->dump(indent + 1, "rhs");
        }
    }
    
    /* Called by all subclasses of EqualityExpression for file output of register operation
     * Must call in explicit ordering to get correct file output - cannot call in emit(..)
     * Operation defined in narrations.h
     */
    unsigned int fileEmit(const char* evaluation) {
    	unsigned int lhsRegister = lhs->genStoreCode();
    	unsigned int rhsRegister = rhs->genStoreCode();
    	unsigned int registerNumber = getNewRegister();
		emit("\tbool r%d = Core::%s(r%d, r%d);", registerNumber, evaluation,  lhsRegister, rhsRegister);
		return registerNumber;
	}
    
};

/* Equal equality expression == */
class EqualityEqualExpression : public EqualityExpression {

private:
	Expression* lhs;
	Expression* rhs;
	
public:
	EqualityEqualExpression(Expression* lhs, Expression* rhs) : EqualityExpression(lhs, rhs) {
		this->lhs = lhs;
		this->rhs = rhs;
	}
	
	unsigned int genStoreCode() {
    	return fileEmit(EVALUATE_EE);
	}
	
	void dump(int indent) {
		label(indent, "EqualityExpression: ==\n");
		EqualityExpression::dump(indent);
	}

};

/* Equal equality expression < */
class EqualityLessThanExpression : public EqualityExpression {

private:
	Expression* lhs;
	Expression* rhs;
	
public:
	EqualityLessThanExpression(Expression* lhs, Expression* rhs) : EqualityExpression(lhs, rhs) {
		this->lhs = lhs;
		this->rhs = rhs;
	}
	
	unsigned int genStoreCode() {
    	return fileEmit(EVALUATE_LT);
	}
	
	void dump(int indent) {
		label(indent, "EqualityExpression: <\n");
		EqualityExpression::dump(indent);
	}

};

/* Equal equality expression > */
class EqualityGreaterThanlExpression : public EqualityExpression {

private:
	Expression* lhs;
	Expression* rhs;
	
public:
	EqualityGreaterThanlExpression(Expression* lhs, Expression* rhs) : EqualityExpression(lhs, rhs) {
		this->lhs = lhs;
		this->rhs = rhs;
	}
	
	unsigned int genStoreCode() {
    	return fileEmit(EVALUATE_GT);
	}
	
	void dump(int indent) {
		label(indent, "EqualityExpression: >\n");
		EqualityExpression::dump(indent);
	}

};


/* Equal equality expression != */
class EqualityNotEqualExpression : public EqualityExpression {

private:
	Expression* lhs;
	Expression* rhs;
	
public:
	EqualityNotEqualExpression(Expression* lhs, Expression* rhs) : EqualityExpression(lhs, rhs) {
		this->lhs = lhs;
		this->rhs = rhs;
	}
	
	unsigned int genStoreCode() {
    	return fileEmit(EVALUATE_NE);
	}
	
	void dump(int indent) {
		label(indent, "EqualityExpression: !=\n");
		EqualityExpression::dump(indent);
	}

};

/* Equal equality expression <= */
class EqualityLessThanEqualToExpression : public EqualityExpression {

private:
	Expression* lhs;
	Expression* rhs;
	
public:
	EqualityLessThanEqualToExpression(Expression* lhs, Expression* rhs) : EqualityExpression(lhs, rhs) {
		this->lhs = lhs;
		this->rhs = rhs;
	}
	
	unsigned int genStoreCode() {
    	return fileEmit(EVALUATE_LTET);
	}
	
	void dump(int indent) {
		label(indent, "EqualityExpression: <=\n");
		EqualityExpression::dump(indent);
	}

};

/* Equal equality expression >= */
class EqualityGreaterThanEqualToExpression : public EqualityExpression {

private:
	Expression* lhs;
	Expression* rhs;
	
public:
	EqualityGreaterThanEqualToExpression(Expression* lhs, Expression* rhs) : EqualityExpression(lhs, rhs) {
		this->lhs = lhs;
		this->rhs = rhs;
	}
	
	unsigned int genStoreCode() {
    	return fileEmit(EVALUATE_GTET);
	}
	
	void dump(int indent) {
		label(indent, "EqualityExpression: >=\n");
		EqualityExpression::dump(indent);
	}

};

/* Equal equality expression === */
class EqualityExactlyEqualExpression : public EqualityExpression {

private:
    Expression* lhs;
    Expression* rhs;
    
public:
    EqualityExactlyEqualExpression(Expression* lhs, Expression* rhs) : EqualityExpression(lhs, rhs) {
        this->lhs = lhs;
        this->rhs = rhs;
    }
    
    unsigned int genStoreCode() {
        return fileEmit(EVALUATE_EEE);
    }
    
    void dump(int indent) {
        label(indent, "EqualityExpression: ===\n");
        EqualityExpression::dump(indent);
    }

};

/* Equal equality expression !== */
class EqualityNotExactlyEqualExpression : public EqualityExpression {

private:
    Expression* lhs;
    Expression* rhs;
    
public:
    EqualityNotExactlyEqualExpression(Expression* lhs, Expression* rhs) : EqualityExpression(lhs, rhs) {
        this->lhs = lhs;
        this->rhs = rhs;
    }
    
    unsigned int genStoreCode() {
        return fileEmit(EVALUATE_NEE);
    }
    
    void dump(int indent) {
        label(indent, "EqualityExpression: !==\n");
        EqualityExpression::dump(indent);
    }

};


/* Bit Shifting operation x << y = x * 2^y 
 * This operator shifts the first operand the specified number of bits to the left. 
 * Excess bits shifted off to the left are discarded. 
 * Zero bits are shifted in from the right.*/
class LeftShiftExpression : public Expression {

private:
    Expression* lhs;
    Expression* rhs;

public:
    LeftShiftExpression(Expression* lhs, Expression* rhs) {
        this->lhs = lhs;
        this->rhs = rhs;
    }

    void dump (int indent) {
        label(indent, "BitwiseShiftExpression\n");
        label(++indent, "LeftShiftOperation: <<\n");
        label(++indent, "lhs:\n");
        lhs->dump(++indent);
        label(--indent, "rhs:\n");
        rhs->dump(++indent);
    }

    unsigned int genCode() {
        return getNewRegister();
    }
    
    unsigned int genStoreCode() {
        unsigned int lhsRegister = lhs->genStoreCode();
        unsigned int rhsRegister = rhs->genStoreCode();
        unsigned int registerNumber = getNewRegister();
        emit("\tESValue* r%d = Core::%s(r%d, r%d);", registerNumber, SHIFT_LEFT,  lhsRegister, rhsRegister);
        return registerNumber;
    };
};


/* Bit Shifting operation x >> y 
 * This operator shifts the first operand the specified number of bits to the right. 
 * Excess bits shifted off to the right are discarded. 
 * Copies of the leftmost bit are shifted in from the left.  */
class RightShiftExpression : public Expression {

private:
    Expression* lhs;
    Expression* rhs;

public:
    RightShiftExpression(Expression* lhs, Expression* rhs) {
        this->lhs = lhs;
        this->rhs = rhs;
    }

    void dump (int indent) {
        label(indent, "BitwiseShiftExpression\n");
        label(++indent, "RightShiftOperation: >>\n");
        label(++indent, "lhs:\n");
        lhs->dump(++indent);
        label(--indent, "rhs:\n");
        rhs->dump(++indent);
    }

    unsigned int genCode() {
        return getNewRegister();
    }
    
    unsigned int genStoreCode() {
        unsigned int lhsRegister = lhs->genStoreCode();
        unsigned int rhsRegister = rhs->genStoreCode();
        unsigned int registerNumber = getNewRegister();
        emit("\tESValue* r%d = Core::%s(r%d, r%d);", registerNumber, SHIFT_RIGHT,  lhsRegister, rhsRegister);
        return registerNumber;
    };
};


/* Bit Shifting operation x >>> y 
 * Ooperator shifts the first operand the specified number of bits to the right. 
 * Excess bits shifted off to the right are discarded. 
 * Zero bits are shifted in from the left. The sign bit becomes 0, so the result is always non-negative.
  */
class ZeroRightFillShiftExpression : public Expression {

private:
    Expression* lhs;
    Expression* rhs;

public:
    ZeroRightFillShiftExpression(Expression* lhs, Expression* rhs) {
        this->lhs = lhs;
        this->rhs = rhs;
    }

    void dump (int indent) {
        label(indent, "BitwiseShiftExpression\n");
        label(++indent, "RightShiftOperation: >>>\n");
        label(++indent, "lhs:\n");
        lhs->dump(++indent);
        label(--indent, "rhs:\n");
        rhs->dump(++indent);
    }

    unsigned int genCode() {
        return getNewRegister();
    }
    
    unsigned int genStoreCode() {
        unsigned int lhsRegister = lhs->genStoreCode();
        unsigned int rhsRegister = rhs->genStoreCode();
        unsigned int registerNumber = getNewRegister();
        emit("\tESValue* r%d = Core::%s(r%d, r%d);", registerNumber, ZERO_FILL_SHIFT_RIGHT,  lhsRegister, rhsRegister);
        return registerNumber;
    };
};
