#include <exd/interaction/interaction.hpp>

#include <iostream>
#include <string>

int main()
{
    using namespace exd::interaction;

    std::cout << "=== Extropian Interaction Demo ===\n\n";

    // ── DataGraph ──
    DataGraph graph;
    graph.setScalar("/sources/revenue/2024", 150000.0);
    graph.setSeries("/sources/revenue/quarterly", { 30000.0, 42000.0, 38000.0, 40000.0 });

    std::cout << "DataGraph: " << graph.paths().size() << " sources\n";
    std::cout << "  Revenue 2024: " << graph.getScalar("/sources/revenue/2024") << "\n";
    auto* series = graph.get("/sources/revenue/quarterly");
    if (series)
        std::cout << "  Quarterly: " << series->series.size() << " data points\n";

    // ── Command stack ──
    CommandStack commands(100);
    int editValue = 0;

    struct DemoCommand : public Command
    {
        int& val; int old, nw;
        std::string label_;
        DemoCommand(int& v, int n, std::string lab) : val(v), old(v), nw(n), label_(std::move(lab)) {}
        void execute() override { val = nw; }
        void undo() override { val = old; }
        std::string label() const override { return label_; }
    };

    commands.execute(std::make_unique<DemoCommand>(editValue, 42, "Set Answer"));
    std::cout << "  After execute: " << editValue << "\n";
    std::cout << "  Can undo: " << commands.canUndo() << ", Can redo: " << commands.canRedo() << "\n";

    commands.undo();
    std::cout << "  After undo: " << editValue << "\n";

    commands.redo();
    std::cout << "  After redo: " << editValue << "\n";

    // ── Gesture Recognizer ──
    GestureRecognizer gr;
    gr.setDragThreshold(5.0f);

    PointerEvent down{ PointerEvent::Type::Down, {10.0f, 10.0f, 0.0f} };
    auto g0 = gr.feed(down, 0.0);
    std::cout << "  Down: phase=" << static_cast<int>(gr.phase());

    PointerEvent up{ PointerEvent::Type::Up, {10.0f, 10.0f, 0.0f} };
    auto g1 = gr.feed(up, 0.050);
    std::cout << " Up: gesture=" << (g1.has_value() ? static_cast<int>(*g1) : -1) << "\n";

    // ── Binding Registry ──
    BindingRegistry bindings(graph);
    Binding b;
    b.fieldPath = "chart.series";
    b.sourcePath = "/sources/revenue/quarterly";
    b.index = -1;
    bindings.add(1, b);

    auto fields = bindings.resolveForSource("/sources/revenue/quarterly");
    std::cout << "  Resolved " << fields.size() << " binding(s) for quarterly\n";

    // ── Context Focus ──
    ContextFocus focus;
    ContextEntry root{ 0, "Home", { {0,0,0}, {100,100,0} } };
    focus.setRoot(root);
    focus.drillIn({ 1, "Chart: Revenue", { {10,10,0}, {90,90,0} } });
    std::cout << "  Focus depth: " << focus.depth() << ", active: " << focus.active().label << "\n";
    std::cout << "  Ghosts: " << focus.ghostedAncestors().size() << "\n";

    focus.drillOut();
    std::cout << "  After drillOut -> active: " << focus.active().label << "\n";

    std::cout << "\n=== All systems OK ===\n";
    return 0;
}
