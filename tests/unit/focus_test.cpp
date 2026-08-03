#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <exd/interaction/interaction.hpp>

using namespace exd::interaction;

TEST_CASE("ContextFocus: drill in and out")
{
    ContextFocus focus;

    ContextEntry root;
    root.visualId = 0;
    root.label = "Root";
    focus.setRoot(root);
    CHECK(focus.depth() == 1);
    CHECK(focus.active().label == "Root");
    CHECK(focus.isAtRoot());

    ContextEntry child;
    child.visualId = 1;
    child.label = "Chart";
    focus.drillIn(child);
    CHECK(focus.depth() == 2);
    CHECK(focus.active().label == "Chart");
    CHECK(!focus.isAtRoot());

    auto popped = focus.drillOut();
    REQUIRE(popped.has_value());
    CHECK(popped->label == "Chart");
    CHECK(focus.active().label == "Root");
    CHECK(focus.isAtRoot());
}

TEST_CASE("ContextFocus: ghosted ancestors")
{
    ContextFocus focus;
    ContextEntry root; root.visualId = 0; root.label = "Root";
    focus.setRoot(root);
    focus.drillIn({ 1, "Level 1" });
    focus.drillIn({ 2, "Level 2" });

    auto ghosts = focus.ghostedAncestors();
    CHECK(ghosts.size() == 2);
    CHECK(ghosts[0].label == "Root");
    CHECK(ghosts[1].label == "Level 1");
}

TEST_CASE("ContextFocus: reset to root")
{
    ContextFocus focus;
    ContextEntry root; root.visualId = 0; root.label = "Root";
    focus.setRoot(root);
    focus.drillIn({ 1, "Mid" });
    focus.drillIn({ 2, "Deep" });
    focus.resetToRoot();
    CHECK(focus.depth() == 1);
    CHECK(focus.active().label == "Root");
}

TEST_CASE("ContextFocus: drillToLevel")
{
    ContextFocus focus;
    ContextEntry root; root.visualId = 0; root.label = "Root";
    focus.setRoot(root);
    focus.drillIn({ 1, "A" });
    focus.drillIn({ 2, "B" });

    auto result = focus.drillToLevel(1);
    CHECK(result.label == "A");
    CHECK(focus.depth() == 2);  // level 1 = depth 2 (0=root, 1=A)
}
