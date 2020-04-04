#include "Common.h"
#include "test_runner.h"

#include <sstream>

using namespace std;

string Print(const Expression* e) {
    if (!e) {
        return "Null expression provided";
    }
    stringstream output;
    output << e->ToString() << " = " << e->Evaluate();
    return output.str();
}

class ExpressionValue : public Expression {
public:
    ExpressionValue(int value)
        : value(value)
    {}

    virtual int Evaluate() const override {
        return value;
    }

    virtual string ToString() const override {
        return to_string(value);
    }

private:
    int value;
};

class ExpressionSum : public Expression {
public:
    ExpressionSum(unique_ptr<Expression>& ptr_left, unique_ptr<Expression>& ptr_right)
        : ptr_left(move(ptr_left))
        , ptr_right(move(ptr_right))
    {}

    virtual int Evaluate() const override {
        auto value_left = ptr_left->Evaluate();
        auto value_right = ptr_right->Evaluate();
        return value_left + value_right;
    }

    virtual string ToString() const override {
        auto str_left = ptr_left->ToString();
        auto str_right = ptr_right->ToString();
        return "(" + str_left + ")+(" + str_right + ")";
    }

private:
    unique_ptr<Expression> ptr_left, ptr_right;
};

class ExpressionMult: public Expression {
public:
    ExpressionMult(unique_ptr<Expression>& ptr_left, unique_ptr<Expression>& ptr_right)
        : ptr_left(move(ptr_left))
        , ptr_right(move(ptr_right))
    {}

    virtual int Evaluate() const override {
        auto value_left = ptr_left->Evaluate();
        auto value_right = ptr_right->Evaluate();
        return value_left * value_right;
    }

    virtual string ToString() const override {
        auto str_left = ptr_left->ToString();
        auto str_right = ptr_right->ToString();
        return "(" + str_left + ")*(" + str_right + ")";
    }

private:
    unique_ptr<Expression> ptr_left, ptr_right;
};


ExpressionPtr Value(int value) {
    return make_unique<ExpressionValue>(value);
}

ExpressionPtr Sum(ExpressionPtr left, ExpressionPtr right) {
    return make_unique<ExpressionSum>(left, right);
}

ExpressionPtr Product(ExpressionPtr left, ExpressionPtr right) {
    return make_unique<ExpressionMult>(left, right);
}

void Test() {
    ExpressionPtr e1 = Product(Value(2), Sum(Value(3), Value(4)));
    ASSERT_EQUAL(Print(e1.get()), "(2)*((3)+(4)) = 14");

    ExpressionPtr e2 = Sum(move(e1), Value(5));
    ASSERT_EQUAL(Print(e2.get()), "((2)*((3)+(4)))+(5) = 19");

    ASSERT_EQUAL(Print(e1.get()), "Null expression provided");
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, Test);
    return 0;
}