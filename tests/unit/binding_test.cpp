#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <exd/interaction/interaction.hpp>

using namespace exd::interaction;

TEST_CASE("BindingRegistry: add and resolve scalar")
{
    DataGraph graph;
    graph.setScalar("/data/width", 200.0);

    BindingRegistry registry(graph);
    Binding binding;
    binding.fieldPath = "size.width";
    binding.sourcePath = "/data/width";
    binding.transform.kind = TransformKind::None;

    uint64_t token = registry.add(1, binding);
    CHECK(token > 0);

    auto fields = registry.resolveForSource("/data/width");
    REQUIRE(fields.size() == 1);
    CHECK(fields[0].fieldPath == "size.width");
    REQUIRE(std::holds_alternative<double>(fields[0].value));
    CHECK(std::get<double>(fields[0].value) == doctest::Approx(200.0));
}

TEST_CASE("BindingRegistry: resolve series")
{
    DataGraph graph;
    graph.setSeries("/data/values", { 1.0, 2.0, 3.0, 4.0, 5.0 });

    BindingRegistry registry(graph);
    Binding binding;
    binding.fieldPath = "series[0].values";
    binding.sourcePath = "/data/values";
    binding.index = -1;  // full series

    registry.add(1, binding);

    auto fields = registry.resolveForSource("/data/values");
    REQUIRE(fields.size() == 1);
    REQUIRE(std::holds_alternative<std::vector<double>>(fields[0].value));
    auto& series = std::get<std::vector<double>>(fields[0].value);
    CHECK(series.size() == 5);
    CHECK(series[0] == doctest::Approx(1.0));
    CHECK(series[4] == doctest::Approx(5.0));
}

TEST_CASE("BindingRegistry: resolve indexed")
{
    DataGraph graph;
    graph.setSeries("/data/values", { 10.0, 20.0, 30.0 });

    BindingRegistry registry(graph);
    Binding binding;
    binding.fieldPath = "position.y";
    binding.sourcePath = "/data/values";
    binding.index = 1;  // second element

    registry.add(1, binding);

    auto fields = registry.resolveForSource("/data/values");
    REQUIRE(fields.size() == 1);
    REQUIRE(std::holds_alternative<double>(fields[0].value));
    CHECK(std::get<double>(fields[0].value) == doctest::Approx(20.0));
}

TEST_CASE("BindingRegistry: affectedVisuals")
{
    DataGraph graph;
    graph.setScalar("/a", 1.0);
    graph.setScalar("/b", 2.0);

    BindingRegistry registry(graph);
    registry.add(10, { "field1", "/a", -1, { TransformKind::None }, true });
    registry.add(20, { "field2", "/a", -1, { TransformKind::None }, true });
    registry.add(30, { "field3", "/b", -1, { TransformKind::None }, true });

    auto affectedByA = registry.affectedVisuals("/a");
    CHECK(affectedByA.size() == 2);
    CHECK(affectedByA[0] == 10);
    CHECK(affectedByA[1] == 20);

    auto affectedByB = registry.affectedVisuals("/b");
    CHECK(affectedByB.size() == 1);
    CHECK(affectedByB[0] == 30);
}

TEST_CASE("BindingRegistry: remove visual")
{
    DataGraph graph;
    graph.setScalar("/x", 1.0);

    BindingRegistry registry(graph);
    registry.add(1, { "f", "/x" });
    registry.add(2, { "g", "/x" });
    registry.removeVisual(1);

    auto affected = registry.affectedVisuals("/x");
    CHECK(affected.size() == 1);
    CHECK(affected[0] == 2);
}
