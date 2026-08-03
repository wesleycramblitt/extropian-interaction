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

void ViewSync::setBrushRange(const std::string& /*viewId*/,
                              const std::string& /*axisId*/,
                              double /*min*/, double /*max*/)
{
    // TODO: set brush range, resolve which visuals are within range
}

const std::unordered_set<VisualId>& ViewSync::brushedVisuals(const std::string& /*viewId*/) const
{
    static const std::unordered_set<VisualId> empty;
    return empty;
}

void ViewSync::setSharedSelection(const std::unordered_set<VisualId>& selected)
{
    sharedSelection_ = selected;
}

void ViewSync::setSharedTime(double normalizedTime)
{
    sharedTime_ = normalizedTime;
}

void ViewSync::setViewFilter(const std::string& /*viewId*/, FilterFn /*filter*/)
{
    // TODO: store and apply filter
}

void ViewSync::clearViewFilter(const std::string& /*viewId*/)
{
    // TODO: clear filter
}

std::vector<std::string> ViewSync::sync()
{
    std::vector<std::string> changed;
    for (auto& [viewId, config] : views_)
    {
        if (config.shareSelection)
            changed.push_back(viewId);
        if (config.shareTime)
            changed.push_back(viewId);
    }
    return changed;
}

void ViewSync::reset()
{
    sharedSelection_.clear();
    sharedTime_ = 0.0;
    views_.clear();
}

}  // namespace exd::interaction
