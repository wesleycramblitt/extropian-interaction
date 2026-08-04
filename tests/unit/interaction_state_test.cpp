#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <exd/interaction/interaction.hpp>

using namespace exd::interaction;

TEST_CASE("InteractionState: hover enter/leave")
{
    InteractionState state;
    InteractionConfig cfg;
    cfg.hoverable = true;

    PointerEvent move;
    move.type = PointerEvent::Type::Move;
    move.position = { 10.0f, 20.0f, 0.0f };

    auto result = state.process(move, GestureKind::None, 1, cfg);
    REQUIRE(result.has_value());
    CHECK(std::holds_alternative<VisualHovered>(*result));
    CHECK(std::get<VisualHovered>(*result).visualId == 1);
    CHECK(state.hoveredVisual() == 1);
}

TEST_CASE("InteractionState: hover leave on clear")
{
    InteractionState state;
    InteractionConfig cfg;
    cfg.hoverable = true;

    PointerEvent move;
    move.type = PointerEvent::Type::Move;
    state.process(move, GestureKind::None, 1, cfg);
    CHECK(state.hoveredVisual() == 1);

    auto result = state.clearHover(0);
    REQUIRE(result.has_value());
    CHECK(std::holds_alternative<VisualUnhovered>(*result));
    CHECK(state.hoveredVisual() == kInvalidVisualId);
}

TEST_CASE("InteractionState: selection toggle")
{
    InteractionState state;
    InteractionConfig cfg;
    cfg.selectable = true;

    PointerEvent down;
    down.type = PointerEvent::Type::Down;

    auto r1 = state.process(down, GestureKind::Tap, 42, cfg);
    REQUIRE(r1.has_value());
    CHECK(std::holds_alternative<VisualSelected>(*r1));
    CHECK(state.selectedVisuals().count(42));

    auto r2 = state.process(down, GestureKind::Tap, 42, cfg);
    REQUIRE(r2.has_value());
    CHECK(std::holds_alternative<VisualDeselected>(*r2));
    CHECK(!state.selectedVisuals().count(42));
}

TEST_CASE("InteractionState: drag begin/end")
{
    InteractionState state;
    InteractionConfig cfg;
    cfg.draggable = true;

    // ── Down: begins drag, no event emitted ──
    PointerEvent down;
    down.type = PointerEvent::Type::Down;
    down.position = { 0.0f, 0.0f, 0.0f };

    auto r1 = state.process(down, GestureKind::Drag, 5, cfg);
    CHECK(!r1.has_value());
    CHECK(state.isDragging());

    // ── Move: emits VisualDragged with delta ──
    PointerEvent move;
    move.type = PointerEvent::Type::Move;
    move.position = { 10.0f, 0.0f, 0.0f };
    move.delta = { 10.0f, 0.0f, 0.0f };

    auto r2 = state.process(move, GestureKind::Drag, 5, cfg);
    REQUIRE(r2.has_value());
    CHECK(std::holds_alternative<VisualDragged>(*r2));
    auto& dragged = std::get<VisualDragged>(*r2);
    CHECK(dragged.visualId == 5);
    CHECK(dragged.delta.x == 10.0f);
    CHECK(dragged.delta.y == 0.0f);
    CHECK(dragged.start.x == 0.0f);
    CHECK(dragged.start.y == 0.0f);
    CHECK(dragged.current.x == 10.0f);
    CHECK(dragged.current.y == 0.0f);

    // ── Up: emits final VisualDragged, ends drag ──
    PointerEvent up;
    up.type = PointerEvent::Type::Up;
    up.position = { 15.0f, 5.0f, 0.0f };
    up.delta = { 5.0f, 5.0f, 0.0f };

    auto r3 = state.process(up, GestureKind::None, 5, cfg);
    REQUIRE(r3.has_value());
    CHECK(std::holds_alternative<VisualDragged>(*r3));
    auto& finalDrag = std::get<VisualDragged>(*r3);
    CHECK(finalDrag.visualId == 5);
    CHECK(finalDrag.delta.x == 5.0f);
    CHECK(finalDrag.delta.y == 5.0f);
    CHECK(!state.isDragging());
}

TEST_CASE("InteractionState: reset")
{
    InteractionState state;
    InteractionConfig cfg;
    cfg.hoverable = true;
    cfg.selectable = true;

    PointerEvent move;
    move.type = PointerEvent::Type::Move;
    state.process(move, GestureKind::None, 1, cfg);

    PointerEvent down;
    down.type = PointerEvent::Type::Down;
    state.process(down, GestureKind::Tap, 2, cfg);

    CHECK(state.hoveredVisual() == 1);
    CHECK(state.selectedVisuals().size() == 1);

    state.reset();
    CHECK(state.hoveredVisual() == kInvalidVisualId);
    CHECK(state.selectedVisuals().empty());
}
