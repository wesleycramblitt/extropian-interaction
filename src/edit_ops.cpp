#include <exd/interaction/edit_ops.hpp>
#include <exd/interaction/data_graph.hpp>

namespace exd::interaction
{

// ── EditTextCommand ──

EditTextCommand::EditTextCommand(IDocument& doc, VisualId visualId, std::string newText)
    : doc_(doc), visualId_(visualId), newText_(std::move(newText))
{}

void EditTextCommand::execute()  { doc_.setText(visualId_, newText_); doc_.markDirty(visualId_); }
void EditTextCommand::undo()     { doc_.setText(visualId_, oldText_); doc_.markDirty(visualId_); }

bool EditTextCommand::canMerge(const Command& other) const
{
    auto* e = dynamic_cast<const EditTextCommand*>(&other);
    return e && e->visualId_ == visualId_;
}

void EditTextCommand::merge(Command& other)
{
    auto& e = static_cast<EditTextCommand&>(other);
    newText_ = e.newText_;
}

// ── PatchVisualCommand ──

PatchVisualCommand::PatchVisualCommand(IDocument& doc, VisualId visualId,
                                       float x, float y, float w, float h)
    : doc_(doc), visualId_(visualId), newX_(x), newY_(y), newW_(w), newH_(h)
    , oldX_(x), oldY_(y), oldW_(w), oldH_(h)
{}

void PatchVisualCommand::execute()
{
    doc_.moveVisual(visualId_, newX_, newY_);
    doc_.resizeVisual(visualId_, newW_, newH_);
    doc_.markDirty(visualId_);
}

void PatchVisualCommand::undo()
{
    doc_.moveVisual(visualId_, oldX_, oldY_);
    doc_.resizeVisual(visualId_, oldW_, oldH_);
    doc_.markDirty(visualId_);
}

// ── SetValueCommand ──

SetValueCommand::SetValueCommand(DataGraph& graph, const std::string& path, double value)
    : graph_(graph), path_(path), newValue_(value)
{
    auto* src = graph_.get(path_);
    if (src)
    {
        if (auto& s = src->scalar)
            oldValue_ = std::get<double>(*s);
        hadOldValue_ = true;
    }
}

void SetValueCommand::execute() { graph_.setScalar(path_, newValue_); }
void SetValueCommand::undo()
{
    if (hadOldValue_) graph_.setScalar(path_, oldValue_);
}

bool SetValueCommand::canMerge(const Command& other) const
{
    auto* s = dynamic_cast<const SetValueCommand*>(&other);
    return s && s->path_ == path_;
}

void SetValueCommand::merge(Command& other)
{
    auto& s = static_cast<SetValueCommand&>(other);
    newValue_ = s.newValue_;
}

// ── AppendDataPointCommand ──

AppendDataPointCommand::AppendDataPointCommand(
    IDocument& doc, VisualId visualId, uint32_t seriesIndex, double x, double y)
    : doc_(doc), visualId_(visualId), seriesIndex_(seriesIndex), x_(x), y_(y)
{}

void AppendDataPointCommand::execute() { /* TODO: patch chart descriptor */ doc_.markDirty(visualId_); }
void AppendDataPointCommand::undo()    { /* TODO: remove last point */ doc_.markDirty(visualId_); }

// ── RemoveDataPointCommand ──

RemoveDataPointCommand::RemoveDataPointCommand(
    IDocument& doc, VisualId visualId, uint32_t seriesIndex, uint32_t pointIndex)
    : doc_(doc), visualId_(visualId), seriesIndex_(seriesIndex), pointIndex_(pointIndex)
{}

void RemoveDataPointCommand::execute() { /* TODO: remove from chart series */ doc_.markDirty(visualId_); }
void RemoveDataPointCommand::undo()    { /* TODO: re‑insert */ doc_.markDirty(visualId_); }

// ── ReorderSeriesCommand ──

ReorderSeriesCommand::ReorderSeriesCommand(
    IDocument& doc, VisualId visualId, uint32_t fromIndex, uint32_t toIndex)
    : doc_(doc), visualId_(visualId), fromIndex_(fromIndex), toIndex_(toIndex)
{}

void ReorderSeriesCommand::execute() { /* TODO: reorder chart series */ doc_.markDirty(visualId_); }
void ReorderSeriesCommand::undo()    { /* TODO: swap back */ doc_.markDirty(visualId_); }

}  // namespace exd::interaction
