#include <exd/interaction/gestures.hpp>

#include <cmath>

namespace exd::interaction
{

GestureRecognizer::GestureRecognizer() = default;
GestureRecognizer::~GestureRecognizer() = default;

std::optional<GestureKind> GestureRecognizer::feed(const PointerEvent& event, double dt)
{
    totalTimeMs_ += dt;
    elapsedMs_ += dt;

    // LongPress timeout: if still in Possible and holding within drag threshold
    if (phase_ == GesturePhase::Possible && elapsedMs_ > longPressTimeoutMs_ && event.type != PointerEvent::Type::Down)
    {
        float dist = (event.position - startPosition_).length();
        if (dist <= dragThreshold_)
        {
            lastPosition_ = event.position;
            currentGesture_ = GestureKind::LongPress;
            phase_ = GesturePhase::Ended;
            tapCount_ = 0;
            return currentGesture_;
        }
    }

    switch (event.type)
    {
    case PointerEvent::Type::Down:
        // Reset tap count if double-tap window has elapsed since the last tap
        if (totalTimeMs_ - lastTapTimeMs_ > doubleTapWindowMs_)
            tapCount_ = 0;
        phase_ = GesturePhase::Possible;
        startPosition_ = event.position;
        lastPosition_ = event.position;
        moved_ = false;
        elapsedMs_ = 0.0;
        return {};  // gesture not yet determined

    case PointerEvent::Type::Move:
        if (phase_ == GesturePhase::Possible || phase_ == GesturePhase::Began || phase_ == GesturePhase::Changed)
        {
            float dist = (event.position - startPosition_).length();
            if (dist > dragThreshold_)
            {
                if (currentGesture_ != GestureKind::Drag)
                {
                    currentGesture_ = GestureKind::Drag;
                    phase_ = GesturePhase::Began;
                    return currentGesture_;
                }
                phase_ = GesturePhase::Changed;
            }
            lastPosition_ = event.position;
        }
        return {};

    case PointerEvent::Type::Up:
        if (phase_ == GesturePhase::Possible)
        {
            tapCount_++;
            currentGesture_ = (tapCount_ >= 2) ? GestureKind::DoubleTap : GestureKind::Tap;
            phase_ = GesturePhase::Ended;
            lastTapTimeMs_ = totalTimeMs_;
            return currentGesture_;
        }
        if (phase_ == GesturePhase::Began || phase_ == GesturePhase::Changed)
        {
            phase_ = GesturePhase::Ended;
            return currentGesture_;
        }
        phase_ = GesturePhase::Idle;
        return {};

    default:
        return {};
    }
}

void GestureRecognizer::reset()
{
    phase_ = GesturePhase::Idle;
    currentGesture_ = GestureKind::None;
    startPosition_ = {};
    lastPosition_ = {};
    elapsedMs_ = 0.0;
    totalTimeMs_ = 0.0;
    lastTapTimeMs_ = -1000.0;
    tapCount_ = 0;
    moved_ = false;
}

math::Vec3f GestureRecognizer::delta() const
{
    return lastPosition_ - startPosition_;
}

double GestureRecognizer::accumulatedDistance() const
{
    return static_cast<double>((lastPosition_ - startPosition_).length());
}

}  // namespace exd::interaction
