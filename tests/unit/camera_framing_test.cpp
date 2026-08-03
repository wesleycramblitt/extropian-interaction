#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <exd/interaction/interaction.hpp>

using namespace exd::interaction;

TEST_CASE("CameraFraming: basic frame")
{
    exd::core::Bounds bounds;
    bounds.min = { -10.0f, -5.0f, 0.0f };
    bounds.max = {  10.0f,  5.0f, 0.0f };

    auto pose = frameBounds(bounds, 16.0f / 9.0f, 1.0f, false);

    // Target should be at center
    CHECK(pose.target.x == doctest::Approx(0.0f));
    CHECK(pose.target.y == doctest::Approx(0.0f));

    // Position should be above center (looking down in Z)
    CHECK(pose.position.x == doctest::Approx(0.0f));
    CHECK(pose.position.y == doctest::Approx(0.0f));
    CHECK(pose.position.z > 0.0f);
}

TEST_CASE("CameraFraming: smoothFrame interpolates")
{
    exd::core::Bounds bounds;
    bounds.min = { 0.0f, 0.0f, 0.0f };
    bounds.max = { 10.0f, 10.0f, 0.0f };

    CameraPose current;
    current.position = { 0.0f, 0.0f, 100.0f };
    current.target = { 0.0f, 0.0f, 0.0f };

    auto target = frameBounds(bounds);

    // t=0 should be current
    auto s0 = smoothFrame(current, target, 0.0f);
    CHECK(s0.position.z == doctest::Approx(100.0f));

    // t=0.5 should be halfway
    auto s5 = smoothFrame(current, target, 0.5f);
    float halfway = 100.0f + (target.position.z - 100.0f) * 0.5f;
    CHECK(s5.position.z == doctest::Approx(halfway));

    // t=1 should be target
    auto s1 = smoothFrame(current, target, 1.0f);
    CHECK(s1.position.z == doctest::Approx(target.position.z));
}

TEST_CASE("CameraFraming: panCamera")
{
    exd::core::Bounds bounds;
    bounds.min = { 0.0f, 0.0f, 0.0f };
    bounds.max = { 100.0f, 100.0f, 0.0f };

    CameraPose pose = frameBounds(bounds, 16.0f / 9.0f, 1.0f, false);

    // Pan right by 100 screen pixels on 1920-wide viewport
    auto panned = panCamera(pose, { 100.0f, 0.0f, 0.0f }, 1920.0f, 1080.0f);

    // Position + target should both have moved right (X decreased for rightward pan)
    CHECK(panned.target.x < pose.target.x);
    CHECK(panned.position.x < pose.position.x);
}
