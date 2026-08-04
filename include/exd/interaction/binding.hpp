#pragma once

#include <exd/interaction/data_graph.hpp>
#include <exd/interaction/event.hpp>

#include <string>
#include <vector>
#include <variant>
#include <cstdint>
#include <optional>
#include <functional>

namespace exd::interaction
{

// ── Transform functions ──

enum class TransformKind : uint8_t
{
    None,           // passthrough
    Scale,          // value * factor
    Clamp,          // min, max clamping
    Normalize,      // map [inMin, inMax] → [outMin, outMax]
    ElementWise,    // map per-element of series
    Aggregate,      // sum, avg, min, max across series
};

/// Parameters for a transform.
struct TransformParams
{
    TransformKind kind = TransformKind::None;
    std::vector<double> args;  // interpretation depends on kind
};

// ── Binding descriptor ──

/// Declares that a visual descriptor field is bound to a data source path.
/// The resolver evaluates the binding on source change and patches the descriptor.
struct Binding
{
    /// The field being bound (e.g. "series[0].values", "position.x", "size.width").
    std::string fieldPath;

    /// The data source path (e.g. "/sources/revenue/values").
    std::string sourcePath;

    /// Index into a series or row/col into a matrix (if subset binding).
    /// -1 means "all" (bind the full series/matrix).
    int index = -1;

    /// Transform to apply when resolving the value.
    TransformParams transform;

    /// If true, changes to the source immediately mark the bound visual dirty.
    bool immediate = true;
};

// ── Resolved field value ──

/// The result of evaluating a binding: a value or series to patch into a descriptor.
struct ResolvedField
{
    std::string fieldPath;
    std::variant<double, std::vector<double>, std::string> value;
};

// ── Binding registry ──

/// Maps visual descriptor fields to data source paths.
/// When a source changes, bindings referencing it are resolved and
/// the resulting ResolvedFields are emitted for the app to patch.
class BindingRegistry
{
public:
    BindingRegistry(DataGraph& graph);
    ~BindingRegistry();

    /// Add a binding. Returns a token to remove it later.
    uint64_t add(VisualId visualId, const Binding& binding);

    /// Remove a binding by token.
    void remove(uint64_t token);

    /// Remove all bindings for a visual.
    void removeVisual(VisualId visualId);

    /// Resolve all bindings that depend on a given source path.
    /// Returns resolved fields to be patched into visuals.
    std::vector<ResolvedField> resolveForSource(const std::string& sourcePath);

    /// Resolve all bindings for a specific visual.
    std::vector<ResolvedField> resolveForVisual(VisualId visualId);

    /// Get all bindings for a visual.
    std::vector<Binding> bindingsForVisual(VisualId visualId) const;

    /// Returns visual IDs whose bindings are affected by a source change.
    std::vector<VisualId> affectedVisuals(const std::string& sourcePath) const;

private:
    struct BindingEntry
    {
        uint64_t token;
        VisualId visualId;
        Binding binding;
    };

    DataGraph& graph_;
    std::vector<BindingEntry> bindings_;
    uint64_t nextToken_ = 1;
};

// ── Transform application ──

/// Apply a scalar transform to a single value.
double applyTransform(TransformKind kind, const std::vector<double>& args, double value);

/// Apply a transform to a full series (element-wise or aggregate).
std::vector<double> applyTransformSeries(TransformKind kind, const std::vector<double>& args,
                                         const std::vector<double>& series);

}  // namespace exd::interaction
