#pragma once

#include <string>
#include <vector>
#include <variant>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <functional>

namespace exd::interaction
{

// ── Data value variant ──

using DataValue = std::variant<double, int64_t, std::string, bool>;

// ── Data source ──

/// A named data source — series, matrix, table, scalar variable.
/// Sources live in the DataGraph and are addressed by path.
struct DataSource
{
    /// Fully qualified path (e.g. "/sources/revenue")
    std::string path;
    /// Human-readable label
    std::string label;
    /// Scalar value (for constants/settings)
    std::optional<DataValue> scalar;
    /// 1D series values (for bar/line/scatter data)
    std::vector<double> series;
    /// 2D matrix (for heatmap, adjacency, table data)
    std::vector<std::vector<double>> matrix;
    /// Whether this source has changed since last resolution
    bool dirty = true;
};

// ── Change listener callback ──

/// Called when a source value changes. The path identifies the source.
using DataChangeCallback = std::function<void(const std::string& path, const DataSource& source)>;

/// Per-listener state: stores the subscribed path prefix and the callback.
struct ListenerEntry
{
    std::string path;       ///< Subscribed path prefix (or "*" for all).
    DataChangeCallback callback;
};

// ── Data graph ──

/// Registry of data sources. Sources are addressed by stable path strings.
/// External code mutates sources; listeners are notified on change.
class DataGraph
{
public:
    DataGraph();
    ~DataGraph();

    /// Create or replace a source at the given path.
    DataSource& create(const std::string& path, const std::string& label = "");

    /// Get a source by path. Returns nullptr if not found.
    DataSource* get(const std::string& path);

    /// Delete a source and all bindings referencing it.
    bool remove(const std::string& path);

    /// Set a scalar value at a path. Creates the source if it doesn't exist.
    void setScalar(const std::string& path, double value);

    /// Set a series (replaces existing). Notifies listeners.
    void setSeries(const std::string& path, const std::vector<double>& values);

    /// Append a value to a series.
    void appendToSeries(const std::string& path, double value);

    /// Remove a point from a series.
    void removeFromSeries(const std::string& path, size_t index);

    /// Set a value in a matrix.
    void setMatrixValue(const std::string& path, size_t row, size_t col, double value);

    /// Get a scalar value from a path (utility for resolution).
    double getScalar(const std::string& path, double defaultValue = 0.0) const;

    /// Register a listener for changes to a path subtree.
    /// The path is a prefix: subscribing to "/sources/revenue" receives
    /// notifications for "/sources/revenue", "/sources/revenue/2024", etc.
    /// Use "*" to match all paths.
    /// Returns a token that can be used to unregister.
    uint64_t subscribe(const std::string& path, DataChangeCallback callback);

    /// Unregister a listener.
    void unsubscribe(uint64_t token);

    /// Notify listeners for a given path (called internally, exposed for
    /// external mutation via commands).
    void notify(const std::string& path);

    /// Returns all paths currently in the graph.
    std::vector<std::string> paths() const;

    /// Returns all currently dirty sources and clears dirty flags.
    std::vector<std::string> takeDirtyPaths();

private:
    std::unordered_map<std::string, DataSource> sources_;
    std::unordered_map<uint64_t, ListenerEntry> listeners_;
    uint64_t nextToken_ = 1;
};

}  // namespace exd::interaction
