#include <exd/interaction/binding.hpp>

#include <algorithm>

namespace exd::interaction
{

BindingRegistry::BindingRegistry(DataGraph& graph)
    : graph_(graph)
{}

BindingRegistry::~BindingRegistry() = default;

uint64_t BindingRegistry::add(VisualId visualId, const Binding& binding)
{
    uint64_t token = nextToken_++;
    bindings_.push_back({ token, visualId, binding });
    return token;
}

void BindingRegistry::remove(uint64_t token)
{
    bindings_.erase(std::remove_if(bindings_.begin(), bindings_.end(),
        [token](const BindingEntry& e) { return e.token == token; }),
        bindings_.end());
}

void BindingRegistry::removeVisual(VisualId visualId)
{
    bindings_.erase(std::remove_if(bindings_.begin(), bindings_.end(),
        [visualId](const BindingEntry& e) { return e.visualId == visualId; }),
        bindings_.end());
}

std::vector<ResolvedField> BindingRegistry::resolveForSource(const std::string& sourcePath)
{
    std::vector<ResolvedField> results;
    for (const auto& entry : bindings_)
    {
        if (entry.binding.sourcePath != sourcePath) continue;

        auto* src = graph_.get(sourcePath);
        if (!src) continue;

        ResolvedField field;
        field.fieldPath = entry.binding.fieldPath;

        if (!src->series.empty())
        {
            if (entry.binding.index < 0)
                field.value = src->series;
            else if (static_cast<size_t>(entry.binding.index) < src->series.size())
                field.value = src->series[static_cast<size_t>(entry.binding.index)];
        }
        else if (src->scalar)
        {
            field.value = std::get<double>(*src->scalar);
        }

        results.push_back(std::move(field));
    }
    return results;
}

std::vector<ResolvedField> BindingRegistry::resolveForVisual(VisualId visualId)
{
    std::vector<ResolvedField> results;
    for (const auto& entry : bindings_)
    {
        if (entry.visualId != visualId) continue;

        auto* src = graph_.get(entry.binding.sourcePath);
        if (!src) continue;

        ResolvedField field;
        field.fieldPath = entry.binding.fieldPath;

        if (!src->series.empty())
        {
            if (entry.binding.index < 0)
                field.value = src->series;
            else if (static_cast<size_t>(entry.binding.index) < src->series.size())
                field.value = src->series[static_cast<size_t>(entry.binding.index)];
        }
        else if (src->scalar)
        {
            field.value = std::get<double>(*src->scalar);
        }

        results.push_back(std::move(field));
    }
    return results;
}

std::vector<Binding> BindingRegistry::bindingsForVisual(VisualId visualId) const
{
    std::vector<Binding> result;
    for (const auto& entry : bindings_)
    {
        if (entry.visualId == visualId) result.push_back(entry.binding);
    }
    return result;
}

std::vector<VisualId> BindingRegistry::affectedVisuals(const std::string& sourcePath) const
{
    std::vector<VisualId> result;
    for (const auto& entry : bindings_)
    {
        if (entry.binding.sourcePath == sourcePath)
            result.push_back(entry.visualId);
    }
    return result;
}

}  // namespace exd::interaction
