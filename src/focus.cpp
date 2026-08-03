#include <exd/interaction/focus.hpp>

#include <stdexcept>

namespace exd::interaction
{

ContextFocus::ContextFocus() = default;
ContextFocus::~ContextFocus() = default;

size_t ContextFocus::drillIn(const ContextEntry& context)
{
    stack_.push_back(context);
    return stack_.size();
}

std::optional<ContextEntry> ContextFocus::drillOut()
{
    if (stack_.size() <= 1) return {};
    auto entry = stack_.back();
    stack_.pop_back();
    return entry;
}

ContextEntry ContextFocus::drillToLevel(size_t depth)
{
    if (depth == 0 || depth >= stack_.size()) return active();
    stack_.resize(depth + 1);
    return active();
}

const ContextEntry& ContextFocus::active() const
{
    if (stack_.empty()) throw std::runtime_error("ContextFocus: no root context set");
    return stack_.back();
}

std::vector<ContextEntry> ContextFocus::ghostedAncestors() const
{
    if (stack_.size() <= 1) return {};
    return { stack_.begin(), stack_.end() - 1 };
}

void ContextFocus::resetToRoot()
{
    if (stack_.size() <= 1) return;
    auto root = stack_.front();
    stack_.clear();
    stack_.push_back(std::move(root));
}

void ContextFocus::setRoot(const ContextEntry& root)
{
    stack_.clear();
    stack_.push_back(root);
}

}  // namespace exd::interaction
