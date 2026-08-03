#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <exd/interaction/interaction.hpp>

using namespace exd::interaction;

TEST_CASE("GestureRecognizer: tap")
{
    GestureRecognizer gr;
    gr.setDragThreshold(10.0f);

    PointerEvent down;
    down.type = PointerEvent::Type::Down;
    down.position = { 100.0f, 200.0f, 0.0f };

    auto result = gr.feed(down, 0.016);
    CHECK(!result.has_value());
    CHECK(gr.phase() == GesturePhase::Possible);

    PointerEvent up;
    up.type = PointerEvent::Type::Up;
    up.position = { 100.0f, 200.0f, 0.0f };  // no movement = tap

    result = gr.feed(up, 0.050);
    REQUIRE(result.has_value());
    CHECK(*result == GestureKind::Tap);
    CHECK(gr.phase() == GesturePhase::Ended);
}

TEST_CASE("GestureRecognizer: drag")
{
    GestureRecognizer gr;
    gr.setDragThreshold(5.0f);

    PointerEvent down;
    down.type = PointerEvent::Type::Down;
    down.position = { 0.0f, 0.0f, 0.0f };
    gr.feed(down, 0.0);

    // Move just under threshold
    PointerEvent move1;
    move1.type = PointerEvent::Type::Move;
    move1.position = { 3.0f, 0.0f, 0.0f };
    auto r1 = gr.feed(move1, 0.016);
    CHECK(!r1.has_value());

    // Move past threshold
    PointerEvent move2;
    move2.type = PointerEvent::Type::Move;
    move2.position = { 10.0f, 0.0f, 0.0f };
    auto r2 = gr.feed(move2, 0.016);
    REQUIRE(r2.has_value());
    CHECK(*r2 == GestureKind::Drag);
    CHECK(gr.phase() == GesturePhase::Began);

    // Continue drag
    PointerEvent move3;
    move3.type = PointerEvent::Type::Move;
    move3.position = { 20.0f, 5.0f, 0.0f };
    auto r3 = gr.feed(move3, 0.016);
    CHECK(!r3.has_value());  // no new gesture, just changed phase
    CHECK(gr.phase() == GesturePhase::Changed);

    // End drag
    PointerEvent up;
    up.type = PointerEvent::Type::Up;
    up.position = { 20.0f, 5.0f, 0.0f };
    auto r4 = gr.feed(up, 0.016);
    REQUIRE(r4.has_value());
    CHECK(*r4 == GestureKind::Drag);
    CHECK(gr.phase() == GesturePhase::Ended);
}

TEST_CASE("GestureRecognizer: double tap")
{
    GestureRecognizer gr;
    PointerEvent down, up;

    down.type = PointerEvent::Type::Down;
    down.position = { 50, 50, 0 };
    gr.feed(down, 0.0);
    up.type = PointerEvent::Type::Up;
    up.position = { 50, 50, 0 };
    auto r1 = gr.feed(up, 0.050);
    REQUIRE(r1.has_value());
    CHECK(*r1 == GestureKind::Tap);

    down.position = { 50, 50, 0 };
    gr.feed(down, 0.100);
    up.position = { 50, 50, 0 };
    auto r2 = gr.feed(up, 0.150);
    REQUIRE(r2.has_value());
    CHECK(*r2 == GestureKind::DoubleTap);
}

TEST_CASE("GestureRecognizer: reset")
{
    GestureRecognizer gr;
    PointerEvent down;
    down.type = PointerEvent::Type::Down;
    gr.feed(down, 0.0);
    CHECK(gr.phase() == GesturePhase::Possible);

    gr.reset();
    CHECK(gr.phase() == GesturePhase::Idle);
}
