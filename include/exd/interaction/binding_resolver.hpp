#pragma once

#include <exd/interaction/binding.hpp>
#include <exd/interaction/data_graph.hpp>

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <functional>

namespace exd::interaction
{

// ── Dirty notifier interface (app layer implements) ──

/// Called when a visual needs recompilation due to a bound field change.
using DirtyNotifier = std::function<void(VisualId visualId)>;

// ── Binding resolver ──

/// Subscribes to the DataGraph and resolves bindings automatically.
/// When a source changes, affected visuals are resolved and dirty‑notified.
class BindingResolver
{
public:
    BindingResolver(DataGraph& graph);
    ~BindingResolver();

    /// Attach the binding registry and begin auto‑resolving.
    void attach(BindingRegistry& registry);

    /// Register a callback for when a visual needs re‑meshing.
    void setDirtyNotifier(DirtyNotifier notifier);

    /// Manually resolve and apply all bindings (e.g. on initial load).
    void resolveAll();

    /// Tear down subscriptions (called on shutdown or when registries change).
    void detach();

private:
    DataGraph& graph_;
    BindingRegistry* registry_ = nullptr;
    DirtyNotifier dirtyNotifier_;
    uint64_t graphToken_ = 0;
};

}  // namespace exd::interaction
