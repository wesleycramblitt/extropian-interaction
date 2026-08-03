#pragma once

#include <exd/interaction/command.hpp>
#include <exd/interaction/event.hpp>

#include <string>
#include <vector>
#include <cstdint>

namespace exd::interaction
{

// ── Document & data model interfaces (injected by app layer) ──

/// Lightweight document interface — edit ops mutate visual data through this.
/// Implemented by the app's VisualDocument or equivalent.
class IDocument
{
public:
    virtual ~IDocument() = default;
    virtual void setText(VisualId id, const std::string& text) = 0;
    virtual void resizeVisual(VisualId id, float w, float h) = 0;
    virtual void moveVisual(VisualId id, float x, float y) = 0;
    virtual void setParent(VisualId child, VisualId parent) = 0;
    virtual void markDirty(VisualId id) = 0;
};

// ── Concrete edit commands ──

/// Change text of a visual (text labels, text input, text cells).
class EditTextCommand : public Command
{
public:
    EditTextCommand(IDocument& doc, VisualId visualId, std::string newText);
    void execute() override;
    void undo() override;
    std::string label() const override { return "Edit text"; }
    bool canMerge(const Command& other) const override;
    void merge(Command& other) override;

private:
    IDocument& doc_;
    VisualId visualId_;
    std::string newText_;
    std::string oldText_;
    uint64_t mergeTimestamp_ = 0;
};

/// Apply a visual patch (transform, style, layout) to a visual.
class PatchVisualCommand : public Command
{
public:
    PatchVisualCommand(IDocument& doc, VisualId visualId,
                       float x, float y, float w, float h);
    void execute() override;
    void undo() override;
    std::string label() const override { return "Patch visual"; }

private:
    IDocument& doc_;
    VisualId visualId_;
    float newX_, newY_, newW_, newH_;
    float oldX_, oldY_, oldW_, oldH_;
};

/// Set a value in the data graph at a source path.
class SetValueCommand : public Command
{
public:
    SetValueCommand(class DataGraph& graph, const std::string& path, double value);
    void execute() override;
    void undo() override;
    std::string label() const override { return "Set value"; }
    bool canMerge(const Command& other) const override;
    void merge(Command& other) override;

private:
    class DataGraph& graph_;
    std::string path_;
    double newValue_ = 0.0;
    double oldValue_ = 0.0;
    bool hadOldValue_ = false;
};

/// Append a data point to a chart series.
class AppendDataPointCommand : public Command
{
public:
    AppendDataPointCommand(IDocument& doc, VisualId visualId,
                           uint32_t seriesIndex, double x, double y);
    void execute() override;
    void undo() override;
    std::string label() const override { return "Add data point"; }

private:
    IDocument& doc_;
    VisualId visualId_;
    uint32_t seriesIndex_;
    double x_, y_;
};

/// Remove a data point from a chart series.
class RemoveDataPointCommand : public Command
{
public:
    RemoveDataPointCommand(IDocument& doc, VisualId visualId,
                           uint32_t seriesIndex, uint32_t pointIndex);
    void execute() override;
    void undo() override;
    std::string label() const override { return "Remove data point"; }

private:
    IDocument& doc_;
    VisualId visualId_;
    uint32_t seriesIndex_;
    uint32_t pointIndex_;
};

/// Reorder chart series.
class ReorderSeriesCommand : public Command
{
public:
    ReorderSeriesCommand(IDocument& doc, VisualId visualId,
                         uint32_t fromIndex, uint32_t toIndex);
    void execute() override;
    void undo() override;
    std::string label() const override { return "Reorder series"; }

private:
    IDocument& doc_;
    VisualId visualId_;
    uint32_t fromIndex_;
    uint32_t toIndex_;
};

}  // namespace exd::interaction
