#include <exd/interaction/view_sync.hpp>

namespace exd::interaction
{

ViewSync::ViewSync() = default;
ViewSync::~ViewSync() = default;

void ViewSync::addView(const ViewConfig& config)
{
    views_[config.viewId] = config;
}

void ViewSync::removeView(const std::string& viewId)
{
    views_.erase(viewId);
}

void ViewSync::setBrushRange(const std::string& viewId,
                              const std::string& axisId,
                              double min, double max)
{
    brushRanges_[viewId][axisId] = {min, max};
}

const std::unordered_set<VisualId>& ViewSync::brushedVisuals(const std::string& viewId) const
{
    static const std::unordered_set<VisualId> empty;
    auto it = brushedVisuals_.find(viewId);
    return (it != brushedVisuals_.end()) ? it->second : empty;
}

void ViewSync::setBrushedVisuals(const std::string& viewId, const std::unordered_set<VisualId>& visuals)
{
    brushedVisuals_[viewId] = visuals;
}

std::optional<std::pair<double, double>> ViewSync::getBrushRange(const std::string& viewId, const std::string& axisId) const
{
    auto vit = brushRanges_.find(viewId);
    if (vit == brushRanges_.end())
        return std::nullopt;
    auto ait = vit->second.find(axisId);
    if (ait == vit->second.end())
        return std::nullopt;
    return std::make_pair(ait->second.min, ait->second.max);
}

void ViewSync::setSharedSelection(const std::unordered_set<VisualId>& selected)
{
    sharedSelection_ = selected;
}

void ViewSync::setSharedTime(double normalizedTime)
{
    sharedTime_ = normalizedTime;
}

void ViewSync::setViewFilter(const std::string& viewId, FilterFn filter)
{
    viewFilters_[viewId] = std::move(filter);
}

void ViewSync::clearViewFilter(const std::string& viewId)
{
    viewFilters_.erase(viewId);
}

bool ViewSync::hasViewFilter(const std::string& viewId) const
{
    return viewFilters_.contains(viewId);
}

bool ViewSync::applyViewFilter(const std::string& viewId, VisualId visualId) const
{
    auto it = viewFilters_.find(viewId);
    if (it == viewFilters_.end())
        return true;  // no filter means pass-through
    return it->second(visualId);
}

std::vector<std::string> ViewSync::sync()
{
    std::vector<std::string> changed;
    for (auto& [viewId, config] : views_)
    {
        bool needsUpdate = config.shareSelection
                        || config.shareTime
                        || brushRanges_.contains(viewId)
                        || viewFilters_.contains(viewId);
        if (needsUpdate)
            changed.push_back(viewId);
    }
    return changed;
}

void ViewSync::reset()
{
    sharedSelection_.clear();
    sharedTime_ = 0.0;
    views_.clear();
    brushRanges_.clear();
    brushedVisuals_.clear();
    viewFilters_.clear();
}

}  // namespace exd::interaction
