#include <exd/interaction/binding_resolver.hpp>

namespace exd::interaction
{

BindingResolver::BindingResolver(DataGraph& graph)
    : graph_(graph)
{}

BindingResolver::~BindingResolver()
{
    detach();
}

void BindingResolver::attach(BindingRegistry& registry)
{
    detach();
    registry_ = &registry;

    // Subscribe to the data graph for all current paths
    graphToken_ = graph_.subscribe("*", [this](const std::string& path, const DataSource& /*src*/)
    {
        if (!registry_) return;
        auto affected = registry_->affectedVisuals(path);
        for (auto vid : affected)
        {
            if (dirtyNotifier_) dirtyNotifier_(vid);
        }
    });
}

void BindingResolver::setDirtyNotifier(DirtyNotifier notifier)
{
    dirtyNotifier_ = std::move(notifier);
}

void BindingResolver::resolveAll()
{
    if (!registry_ || !dirtyNotifier_) return;

    // Resolve bindings for all visuals known to the registry
    // (registry_->bindingsForVisual requires a visual id; we'd need
    //  a list of all visuals. In practice, the app layer drives this.)
    // For now, resolve dirty sources:
    auto dirty = graph_.takeDirtyPaths();
    for (const auto& path : dirty)
    {
        auto affected = registry_->affectedVisuals(path);
        for (auto vid : affected)
        {
            if (dirtyNotifier_) dirtyNotifier_(vid);
        }
    }
}

void BindingResolver::detach()
{
    if (graphToken_ != 0)
    {
        graph_.unsubscribe(graphToken_);
        graphToken_ = 0;
    }
    registry_ = nullptr;
}

}  // namespace exd::interaction
