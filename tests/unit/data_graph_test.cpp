#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <exd/interaction/interaction.hpp>

using namespace exd::interaction;

TEST_CASE("DataGraph: create and get")
{
    DataGraph graph;
    auto& src = graph.create("/sources/revenue", "Revenue");
    CHECK(src.path == "/sources/revenue");
    CHECK(src.label == "Revenue");
    CHECK(src.dirty == true);

    auto* found = graph.get("/sources/revenue");
    REQUIRE(found != nullptr);
    CHECK(found->label == "Revenue");
}

TEST_CASE("DataGraph: setScalar")
{
    DataGraph graph;
    graph.setScalar("/settings/threshold", 42.0);
    CHECK(graph.getScalar("/settings/threshold") == doctest::Approx(42.0));
    CHECK(graph.getScalar("/nonexistent", 99.0) == doctest::Approx(99.0));
}

TEST_CASE("DataGraph: setSeries and append")
{
    DataGraph graph;
    graph.setSeries("/data/temperatures", { 20.0, 22.0, 25.0 });
    auto* src = graph.get("/data/temperatures");
    REQUIRE(src != nullptr);
    CHECK(src->series.size() == 3);
    CHECK(src->series[0] == doctest::Approx(20.0));

    graph.appendToSeries("/data/temperatures", 30.0);
    CHECK(src->series.size() == 4);
    CHECK(src->series[3] == doctest::Approx(30.0));

    graph.removeFromSeries("/data/temperatures", 1);
    CHECK(src->series.size() == 3);
    CHECK(src->series[1] == doctest::Approx(25.0));
}

TEST_CASE("DataGraph: notify subscribers")
{
    DataGraph graph;
    int callCount = 0;
    std::string receivedPath;
    graph.subscribe("*", [&](const std::string& path, const DataSource&) {
        callCount++;
        receivedPath = path;
    });

    graph.setScalar("/test/value", 10.0);
    CHECK(callCount == 1);
    CHECK(receivedPath == "/test/value");
}

TEST_CASE("DataGraph: paths")
{
    DataGraph graph;
    graph.setScalar("/a/x", 1.0);
    graph.setScalar("/b/y", 2.0);
    auto p = graph.paths();
    CHECK(p.size() == 2);
}

TEST_CASE("DataGraph: takeDirtyPaths clears dirty")
{
    DataGraph graph;
    graph.setScalar("/a", 1.0);
    graph.setScalar("/b", 2.0);
    auto dirty = graph.takeDirtyPaths();
    CHECK(dirty.size() == 2);
    auto dirty2 = graph.takeDirtyPaths();
    CHECK(dirty2.empty());
}
