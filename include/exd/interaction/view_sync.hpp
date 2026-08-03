#pragma once

#include <exd/interaction/event.hpp>

#include <unordered_set>
#include <vector>
#include <string>
#include <functional>

namespace exd::interaction
{

// ── Coordinated view configuration ──

struct ViewConfig
{
    std::string viewId;
    bool synchronized = false;    // participate in brush/link
    bool shareSelection = false;  // shared selection across linked views
    bool shareTime = false;       // shared time cursor
    bool shareAbstraction = false;// linked abstraction level
};

// ── View sync state ──

/// Manages brushed/linked selection across multiple views.
/// Implements renderer‑plan §10: brushing and linking, coordinated views.
class ViewSync
{
public:
    ViewSync();
    ~ViewSync();

    /// Register a view.
    void addView(const ViewConfig& config);
    void removeView(const std::string& viewId);

    /// Set the brushed range for a view (axis brush).
    void setBrushRange(const std::string& viewId, const std::string& axisId,
                       double min, double max);

    /// Get entities selected by the brush in a view.
    const std::unordered_set<VisualId>& brushedVisuals(const std::string& viewId) const;

    /// Shared selected set (cross‑view selection, propagated to synced views).
    void setSharedSelection(const std::unordered_set<VisualId>& selected);
    const std::unordered_set<VisualId>& sharedSelection() const { return sharedSelection_; }

    /// Shared time cursor (scrubbed position, 0‑1).
    void setSharedTime(double normalizedTime);
    double sharedTime() const { return sharedTime_; }

    /// Filter active visuals by attribute value.
    using FilterFn = std::function<bool(VisualId)>;
    void setViewFilter(const std::string& viewId, FilterFn filter);
    void clearViewFilter(const std::string& viewId);

    /// Call after selection/filter changes to propagate to synced views.
    /// Returns the viewIds that need updating.
    std::vector<std::string> sync();

    /// Clear all state.
    void reset();

private:
    std::unordered_map<std::string, ViewConfig> views_;
    std::unordered_set<VisualId> sharedSelection_;
    double sharedTime_ = 0.0;
};

}  // namespace exd::interaction
