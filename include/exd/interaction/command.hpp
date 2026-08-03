#pragma once

#include <exd/interaction/event.hpp>

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <functional>

namespace exd::interaction
{

// ── Command (abstract base) ──

/// A reversible edit. Execute applies the mutation; undo reverses it.
/// Commands operate on a document / data model (injected, not owned by command).
class Command
{
public:
    virtual ~Command() = default;

    /// Apply the edit.
    virtual void execute() = 0;
    /// Reverse the edit.
    virtual void undo() = 0;
    /// Human‑readable label for undo/redo UI.
    virtual std::string label() const = 0;

    /// Whether this command can be merged with a subsequent command of the same
    /// type (e.g. consecutive drag deltas or continuous scrub values).
    virtual bool canMerge(const Command& other) const { (void)other; return false; }
    /// Merge another command into this one (other must be of same type).
    virtual void merge(Command& other) { (void)other; }
};

// ── Undo/Redo stack ──

/// Manages a bounded undo/redo history.
class CommandStack
{
public:
    CommandStack(size_t maxDepth = 1000);
    ~CommandStack();

    /// Execute a command and push it onto the undo stack. Clears redo history.
    void execute(std::unique_ptr<Command> cmd);

    /// Undo the most recent command. Returns its label, or nullopt if empty.
    std::optional<std::string> undo();
    /// Redo the most recently undone command. Returns its label, or nullopt if empty.
    std::optional<std::string> redo();

    /// Whether undo/redo are available.
    bool canUndo() const { return !undoStack_.empty(); }
    bool canRedo() const { return !redoStack_.empty(); }

    /// The label of the next undo command (for UI).
    std::string undoLabel() const;
    /// The label of the next redo command.
    std::string redoLabel() const;

    /// Clear all history.
    void clear();

    /// Change the maximum stack depth.
    void setMaxDepth(size_t depth);

private:
    std::vector<std::unique_ptr<Command>> undoStack_;
    std::vector<std::unique_ptr<Command>> redoStack_;
    size_t maxDepth_;
};

}  // namespace exd::interaction
