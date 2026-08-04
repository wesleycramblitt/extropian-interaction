#include <exd/interaction/binding.hpp>

#include <algorithm>
#include <cmath>
#include <numeric>

namespace exd::interaction
{

// ── Transform application ──

double applyTransform(TransformKind kind, const std::vector<double>& args, double value)
{
    switch (kind)
    {
        case TransformKind::None:
            return value;

        case TransformKind::Scale:
            if (args.empty()) return value;
            return value * args[0];

        case TransformKind::Clamp:
            if (args.size() < 2) return value;
            return std::clamp(value, args[0], args[1]);

        case TransformKind::Normalize:
            if (args.size() < 4) return value;
            {
                double inMin  = args[0];
                double inMax  = args[1];
                double outMin = args[2];
                double outMax = args[3];
                double range  = inMax - inMin;
                if (range == 0.0) return outMin;
                return outMin + (value - inMin) / range * (outMax - outMin);
            }

        case TransformKind::ElementWise:
        case TransformKind::Aggregate:
            // These only make sense on series; passthrough for scalar.
            return value;
    }
    return value;
}

std::vector<double> applyTransformSeries(TransformKind kind, const std::vector<double>& args,
                                          const std::vector<double>& series)
{
    switch (kind)
    {
        case TransformKind::None:
            return series;

        case TransformKind::Scale:
        case TransformKind::Clamp:
        case TransformKind::Normalize:
            // Apply scalar transform per-element.
            {
                std::vector<double> result;
                result.reserve(series.size());
                for (double v : series)
                    result.push_back(applyTransform(kind, args, v));
                return result;
            }

        case TransformKind::ElementWise:
            // args[0] = sub-transform kind (as double), args[1..N] = sub-transform args.
            if (args.empty()) return series;
            {
                auto subKind = static_cast<TransformKind>(static_cast<int>(args[0]));
                std::vector<double> subArgs(args.begin() + 1, args.end());
                std::vector<double> result;
                result.reserve(series.size());
                for (double v : series)
                    result.push_back(applyTransform(subKind, subArgs, v));
                return result;
            }

        case TransformKind::Aggregate:
            {
                if (series.empty()) return {};
                if (args.empty()) return {};
                int op = static_cast<int>(args[0]);
                double result = 0.0;
                switch (op)
                {
                    case 0: // sum
                        result = std::accumulate(series.begin(), series.end(), 0.0);
                        break;
                    case 1: // avg
                        result = std::accumulate(series.begin(), series.end(), 0.0) / static_cast<double>(series.size());
                        break;
                    case 2: // min
                        result = *std::min_element(series.begin(), series.end());
                        break;
                    case 3: // max
                        result = *std::max_element(series.begin(), series.end());
                        break;
                    default:
                        result = series[0];
                        break;
                }
                return { result };
            }
    }
    return series;
}

// ── BindingRegistry ──

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
            {
                field.value = applyTransformSeries(
                    entry.binding.transform.kind,
                    entry.binding.transform.args,
                    src->series);
            }
            else if (static_cast<size_t>(entry.binding.index) < src->series.size())
            {
                double v = applyTransform(
                    entry.binding.transform.kind,
                    entry.binding.transform.args,
                    src->series[static_cast<size_t>(entry.binding.index)]);
                field.value = v;
            }
        }
        else if (src->scalar && std::holds_alternative<double>(*src->scalar))
        {
            double v = applyTransform(
                entry.binding.transform.kind,
                entry.binding.transform.args,
                std::get<double>(*src->scalar));
            field.value = v;
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
            {
                field.value = applyTransformSeries(
                    entry.binding.transform.kind,
                    entry.binding.transform.args,
                    src->series);
            }
            else if (static_cast<size_t>(entry.binding.index) < src->series.size())
            {
                double v = applyTransform(
                    entry.binding.transform.kind,
                    entry.binding.transform.args,
                    src->series[static_cast<size_t>(entry.binding.index)]);
                field.value = v;
            }
        }
        else if (src->scalar && std::holds_alternative<double>(*src->scalar))
        {
            double v = applyTransform(
                entry.binding.transform.kind,
                entry.binding.transform.args,
                std::get<double>(*src->scalar));
            field.value = v;
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
