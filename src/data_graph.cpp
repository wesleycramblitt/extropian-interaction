#include <exd/interaction/data_graph.hpp>

#include <cmath>
#include <algorithm>

namespace exd::interaction
{

DataGraph::DataGraph() = default;
DataGraph::~DataGraph() = default;

DataSource& DataGraph::create(const std::string& path, const std::string& label)
{
    auto& s = sources_[path];
    s.path = path;
    s.label = label;
    s.dirty = true;
    return s;
}

DataSource* DataGraph::get(const std::string& path)
{
    auto it = sources_.find(path);
    return it != sources_.end() ? &it->second : nullptr;
}

bool DataGraph::remove(const std::string& path)
{
    return sources_.erase(path) > 0;
}

void DataGraph::setScalar(const std::string& path, double value)
{
    auto& s = create(path);
    s.scalar = value;
    s.dirty = true;
    notify(path);
}

void DataGraph::setSeries(const std::string& path, const std::vector<double>& values)
{
    auto& s = create(path);
    s.series = values;
    s.dirty = true;
    notify(path);
}

void DataGraph::appendToSeries(const std::string& path, double value)
{
    auto& s = create(path);
    s.series.push_back(value);
    s.dirty = true;
    notify(path);
}

void DataGraph::removeFromSeries(const std::string& path, size_t index)
{
    auto* s = get(path);
    if (!s || index >= s->series.size()) return;
    s->series.erase(s->series.begin() + static_cast<ptrdiff_t>(index));
    s->dirty = true;
    notify(path);
}

void DataGraph::setMatrixValue(const std::string& path, size_t row, size_t col, double value)
{
    auto& s = create(path);
    if (row >= s.matrix.size()) s.matrix.resize(row + 1);
    if (col >= s.matrix[row].size()) s.matrix[row].resize(col + 1);
    s.matrix[row][col] = value;
    s.dirty = true;
    notify(path);
}

double DataGraph::getScalar(const std::string& path, double defaultValue) const
{
    auto it = sources_.find(path);
    if (it == sources_.end()) return defaultValue;
    if (auto& s = it->second.scalar)
        return std::get<double>(*s);
    return defaultValue;
}

uint64_t DataGraph::subscribe(const std::string& path, DataChangeCallback callback)
{
    uint64_t token = nextToken_++;
    listeners_[token] = ListenerEntry{path, std::move(callback)};
    return token;
}

void DataGraph::unsubscribe(uint64_t token)
{
    listeners_.erase(token);
}

void DataGraph::notify(const std::string& path)
{
    auto it = sources_.find(path);
    if (it == sources_.end()) return;

    for (auto& [token, entry] : listeners_)
    {
        // "*" matches everything; otherwise check prefix.
        if (entry.path == "*" || path.rfind(entry.path, 0) == 0)
            entry.callback(path, it->second);
    }
}

std::vector<std::string> DataGraph::paths() const
{
    std::vector<std::string> result;
    for (const auto& [path, _] : sources_)
        result.push_back(path);
    return result;
}

std::vector<std::string> DataGraph::takeDirtyPaths()
{
    std::vector<std::string> dirty;
    for (auto& [path, src] : sources_)
    {
        if (src.dirty)
        {
            dirty.push_back(path);
            src.dirty = false;
        }
    }
    return dirty;
}

}  // namespace exd::interaction
