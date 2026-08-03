#pragma once

#include <exd/interaction/event.hpp>
#include <exd/geometry/types.hpp>  // geometry::Bounds

#include <vector>
#include <string>
#include <optional>

namespace exd::interaction
{

// ── Context focus entry ──

/// A breadcrumb entry in the context stack.
struct ContextEntry
{
    VisualId visualId = kInvalidVisualId;   // the visual that owns this context
    std::string label;                      // display label
    geometry::Bounds bounds = {};           // bounding region (for camera framing)
    std::vector<VisualId> childVisuals;     // visuals revealed at this level
};

// ── Context focus stack ──

/// Manages drill‑in / drill‑out navigation through hierarchical visual contexts.
/// Maintains a breadcrumb stack; the active context is the top of the stack.
/// Previous contexts are "ghosted" (reduced opacity) per renderer‑plan §10.3.
class ContextFocus
{
public:
    ContextFocus();
    ~ContextFocus();

    /// Drill into a new context (push onto stack). Returns the new depth.
    size_t drillIn(const ContextEntry& context);

    /// Drill out to the parent context. Returns the newly active entry, or nullopt
    /// if already at root (root cannot be popped).
    std::optional<ContextEntry> drillOut();

    /// Jump directly to a specific depth level (clears entries above it).
    /// Returns the newly active entry.
    ContextEntry drillToLevel(size_t depth);

    /// Active context (top of stack).
    const ContextEntry& active() const;

    /// Full breadcrumb stack (active at back).
    const std::vector<ContextEntry>& breadcrumbs() const { return stack_; }

    /// Ghosted ancestors (everything below the active level, for reduced‑opacity
    /// rendering). Useful for "persistent context" background rendering.
    std::vector<ContextEntry> ghostedAncestors() const;

    /// Current nesting depth.
    size_t depth() const { return stack_.size(); }

    /// Whether the stack is at the root.
    bool isAtRoot() const { return stack_.size() <= 1; }

    /// Clear to root (keep only the root entry).
    void resetToRoot();

    /// Set the root entry (replaces entire stack).
    void setRoot(const ContextEntry& root);

private:
    std::vector<ContextEntry> stack_;
};

}  // namespace exd::interaction
