#include <exd/interaction/command.hpp>

namespace exd::interaction
{

CommandStack::CommandStack(size_t maxDepth)
    : maxDepth_(maxDepth)
{}
CommandStack::~CommandStack() = default;

void CommandStack::execute(std::unique_ptr<Command> cmd)
{
    cmd->execute();
    undoStack_.push_back(std::move(cmd));
    redoStack_.clear();  // new action invalidates redo history

    while (undoStack_.size() > maxDepth_)
        undoStack_.erase(undoStack_.begin());
}

std::optional<std::string> CommandStack::undo()
{
    if (undoStack_.empty()) return {};
    auto cmd = std::move(undoStack_.back());
    undoStack_.pop_back();
    std::string label = cmd->label();
    cmd->undo();
    redoStack_.push_back(std::move(cmd));
    return label;
}

std::optional<std::string> CommandStack::redo()
{
    if (redoStack_.empty()) return {};
    auto cmd = std::move(redoStack_.back());
    redoStack_.pop_back();
    std::string label = cmd->label();
    cmd->execute();
    undoStack_.push_back(std::move(cmd));
    return label;
}

std::string CommandStack::undoLabel() const
{
    if (undoStack_.empty()) return "";
    return undoStack_.back()->label();
}

std::string CommandStack::redoLabel() const
{
    if (redoStack_.empty()) return "";
    return redoStack_.back()->label();
}

void CommandStack::clear()
{
    undoStack_.clear();
    redoStack_.clear();
}

void CommandStack::setMaxDepth(size_t depth)
{
    maxDepth_ = depth;
    while (undoStack_.size() > maxDepth_)
        undoStack_.erase(undoStack_.begin());
}

}  // namespace exd::interaction
