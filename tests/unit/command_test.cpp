#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <exd/interaction/interaction.hpp>

using namespace exd::interaction;

TEST_CASE("CommandStack: execute, undo, redo")
{
    CommandStack stack;
    CHECK(!stack.canUndo());
    CHECK(!stack.canRedo());

    // A simple test command that mutates an int
    struct TestCommand : public Command
    {
        int& value;
        int oldValue;
        int newValue;
        TestCommand(int& v, int nv) : value(v), oldValue(v), newValue(nv) {}
        void execute() override { value = newValue; }
        void undo() override { value = oldValue; }
        std::string label() const override { return "Test"; }
    };

    int x = 0;
    stack.execute(std::make_unique<TestCommand>(x, 42));
    CHECK(x == 42);
    CHECK(stack.canUndo());
    CHECK(!stack.canRedo());
    CHECK(stack.undoLabel() == "Test");

    stack.undo();
    CHECK(x == 0);
    CHECK(!stack.canUndo());
    CHECK(stack.canRedo());
    CHECK(stack.redoLabel() == "Test");

    stack.redo();
    CHECK(x == 42);
    CHECK(stack.canUndo());
}

TEST_CASE("CommandStack: redo cleared by new command")
{
    CommandStack stack;
    struct TestCommand : public Command
    {
        int& value;
        int oldValue;
        int newValue;
        TestCommand(int& v, int nv) : value(v), oldValue(v), newValue(nv) {}
        void execute() override { value = newValue; }
        void undo() override { value = oldValue; }
        std::string label() const override { return "Test"; }
    };

    int x = 0;
    stack.execute(std::make_unique<TestCommand>(x, 10));
    stack.undo();
    CHECK(stack.canRedo());
    stack.execute(std::make_unique<TestCommand>(x, 20));
    CHECK(x == 20);
    CHECK(!stack.canRedo());
}

TEST_CASE("CommandStack: max depth")
{
    CommandStack stack(3);
    struct TestCommand : public Command
    {
        int& value;
        int oldValue;
        int newValue;
        TestCommand(int& v, int nv) : value(v), oldValue(v), newValue(nv) {}
        void execute() override { value = newValue; }
        void undo() override { value = oldValue; }
        std::string label() const override { return "Test"; }
    };

    int x = 0;
    for (int i = 1; i <= 5; i++)
        stack.execute(std::make_unique<TestCommand>(x, i));

    CHECK(x == 5);

    // undo 3 times (max depth), 4th should fail
    for (int i = 0; i < 3; i++)
        CHECK(stack.undo().has_value());
    CHECK(!stack.undo().has_value());
    CHECK(x == 2);  // oldest kept was 2
}
