#include <gtest/gtest.h>

#include <cpptoolkit/mvvm/mvvm.h>

using namespace cpptoolkit::mvvm;

TEST(UndoStack, ExecuteUndoRedo) {
    UndoStack stack;
    int value = 0;
    stack.Execute("inc", [&] { ++value; }, [&] { --value; });
    EXPECT_EQ(value, 1);
    EXPECT_TRUE(stack.CanUndo.Get());
    EXPECT_EQ(stack.UndoText.Get(), "inc");
    stack.Undo();
    EXPECT_EQ(value, 0);
    EXPECT_TRUE(stack.CanRedo.Get());
    EXPECT_FALSE(stack.CanUndo.Get());
    stack.Redo();
    EXPECT_EQ(value, 1);
    EXPECT_FALSE(stack.CanRedo.Get());
}

TEST(UndoStack, NewActionClearsRedo) {
    UndoStack stack;
    int v = 0;
    stack.Execute("a", [&] { ++v; }, [&] { --v; });
    stack.Undo();
    stack.Execute("b", [&] { v += 10; }, [&] { v -= 10; });
    EXPECT_FALSE(stack.CanRedo.Get());
}

TEST(UndoStack, DepthLimitDropsOldest) {
    UndoStack stack(2);
    int v = 0;
    for (int i = 0; i < 5; ++i) stack.Execute("i", [&] { ++v; }, [&] { --v; });
    EXPECT_EQ(stack.UndoDepth(), 2u);
    EXPECT_THROW(UndoStack(0), std::invalid_argument);
}

TEST(UndoStack, GroupUndoesAsOneInReverse) {
    UndoStack stack;
    std::string log;
    {
        UndoStack::Group g(stack, "both");
        stack.Execute("1", [&] { log += "d1"; }, [&] { log += "u1"; });
        stack.Execute("2", [&] { log += "d2"; }, [&] { log += "u2"; });
    }
    EXPECT_EQ(stack.UndoDepth(), 1u);
    EXPECT_EQ(stack.UndoText.Get(), "both");
    log.clear();
    stack.Undo();
    EXPECT_EQ(log, "u2u1");
}

TEST(UndoStack, PropertyChangesMergeUntilBreak) {
    UndoStack stack;
    ObservableProperty<float> p(nullptr, "Gain", 0.f);
    for (float f : {1.f, 2.f, 3.f}) {
        const float old = p.Get();
        p.Set(f);
        RecordChange(stack, p, old, f, "Gain", true);
    }
    EXPECT_EQ(stack.UndoDepth(), 1u);
    stack.BreakMerge();
    const float old = p.Get();
    p.Set(9.f);
    RecordChange(stack, p, old, 9.f, "Gain", true);
    EXPECT_EQ(stack.UndoDepth(), 2u);
    stack.Undo();
    EXPECT_EQ(p.Get(), 3.f);
    stack.Undo();
    EXPECT_EQ(p.Get(), 0.f);
    stack.Redo();
    EXPECT_EQ(p.Get(), 3.f);
}

TEST(UndoStack, ChangesMadeWhileApplyingAreNotRecorded) {
    UndoStack stack;
    ObservableProperty<int> a(nullptr, "A", 0);
    ObservableProperty<int> b(nullptr, "B", 0);
    auto sub = a.OnChanged([&](const int& v) { SetUndoable(stack, b, v * 2); });
    SetUndoable(stack, a, 5);
    EXPECT_EQ(b.Get(), 10);
    EXPECT_EQ(stack.UndoDepth(), 1u);
}

TEST(UndoStack, CommandsFollowState) {
    UndoStack stack;
    ObservableProperty<int> p(nullptr, "P", 0);
    EXPECT_FALSE(stack.UndoCommand.CanExecute());
    SetUndoable(stack, p, 4);
    EXPECT_TRUE(stack.UndoCommand.CanExecute());
    stack.UndoCommand.Execute();
    EXPECT_EQ(p.Get(), 0);
    EXPECT_TRUE(stack.RedoCommand.CanExecute());
    EXPECT_FALSE(SetUndoable(stack, p, 0));
}
