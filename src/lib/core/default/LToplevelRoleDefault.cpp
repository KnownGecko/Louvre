#include <protocols/XdgShell/RXdgSurface.h>
#include <protocols/XdgShell/RXdgToplevel.h>
#include <LToplevelMoveSession.h>
#include <LToplevelResizeSession.h>
#include <LCompositor.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LTouch.h>
#include <LTouchPoint.h>
#include <LTouchDownEvent.h>

using namespace Louvre;

//! [rolePos]
const LPoint &LToplevelRole::rolePos() const
{
    m_rolePos = surface()->pos() - windowGeometry().topLeft();
    return m_rolePos;
}
//! [rolePos]

//! [startMoveRequest]
void LToplevelRole::startMoveRequest(const LEvent &triggeringEvent)
{
    if (fullscreen())
        return;

    if (triggeringEvent.type() == LEvent::Type::Touch)
    {
        if (triggeringEvent.subtype() != LEvent::Subtype::Down)
            return;

        if (!cursor()->output())
            return;

        const LTouchDownEvent& touchDownEvent { static_cast<const LTouchDownEvent&>(triggeringEvent) };
        LTouchPoint *touchPoint { seat()->touch()->findTouchPoint(touchDownEvent.id()) };

        if (!touchPoint)
            return;

        if (touchPoint->surface() != surface())
            return;

        const LPoint initDragPoint { LTouch::toGlobal(cursor()->output(), touchPoint->pos()) };

        moveSession().start(triggeringEvent, initDragPoint);
    }
    else if (surface()->hasPointerFocus())
        moveSession().start(triggeringEvent, cursor()->pos());
}
//! [startMoveRequest]

//! [startResizeRequest]
void LToplevelRole::startResizeRequest(const LEvent &triggeringEvent, ResizeEdge edge)
{
    if (fullscreen())
        return;

    if (triggeringEvent.type() == LEvent::Type::Touch)
    {
        if (triggeringEvent.subtype() != LEvent::Subtype::Down)
            return;

        if (!cursor()->output())
            return;

        const LTouchDownEvent &touchDownEvent { static_cast<const LTouchDownEvent&>(triggeringEvent) };
        LTouchPoint *touchPoint { seat()->touch()->findTouchPoint(touchDownEvent.id()) };

        if (!touchPoint)
            return;

        if (touchPoint->surface() != surface())
            return;

        const LPoint initDragPoint { LTouch::toGlobal(cursor()->output(), touchPoint->pos()) };

        resizeSession().start(triggeringEvent, edge, initDragPoint);
    }
    else if (surface()->hasPointerFocus())
        resizeSession().start(triggeringEvent, edge, cursor()->pos());
}
//! [startResizeRequest]

//! [configureRequest]
void LToplevelRole::configureRequest()
{
    // Using (0,0) allows the client to decide the size
    configureSize(0,0);
    configureState(pending().state | Activated);
    configureDecorationMode(ClientSide);
}
//! [configureRequest]

void LToplevelRole::configurationChanged(LBitset<ConfigurationChanges> changes)
{
    surface()->repaintOutputs();

    if (!changes.check(StateChanged))
        return;

    const LBitset<State> stateChanges { current().state ^ previous().state };

    if (stateChanges.check(Activated))
    {
        if (activated())
            seat()->keyboard()->setFocus(surface());
    }

    if (stateChanges.check(Maximized))
    {
        if (maximized())
        {
            if (cursor()->output())
            {
                surface()->raise();
                surface()->setPos(cursor()->output()->pos());
                surface()->setMinimized(false);
            }
            else
            {
                configureSize(0, 0);
                configureState(pending().state & ~Maximized);
            }
        }

        return;
    }

    if (stateChanges.check(Fullscreen))
    {
        if (fullscreen())
        {
            if (cursor()->output())
            {
                surface()->setPos(cursor()->output()->pos());
                surface()->raise();
            }
            else
            {
                configureSize(0, 0);
                configureState(pending().state & ~Fullscreen);
            }
        }

        return;
    }

}

//! [titleChanged]
void LToplevelRole::titleChanged()
{
    /* No default implementation */
}
//! [titleChanged]

//! [appIdChanged]
void LToplevelRole::appIdChanged()
{
    /* No default implementation */
}
//! [appIdChanged]

//! [preferredDecorationModeChanged]
void LToplevelRole::preferredDecorationModeChanged()
{
    /* No default implementation */
}
//! [preferredDecorationModeChanged]

//! [setMaximizedRequest]
void LToplevelRole::setMaximizedRequest()
{
    if (!cursor()->output())
        return;

    configureSize(cursor()->output()->size());
    configureState(Activated | Maximized);
}
//! [setMaximizedRequest]

//! [unsetMaximizedRequest]
void LToplevelRole::unsetMaximizedRequest()
{
    configureState(pending().state &~ Maximized);
}
//! [unsetMaximizedRequest]

//! [setFullscreenRequest]
void LToplevelRole::setFullscreenRequest(LOutput *preferredOutput)
{
    const LOutput *output { preferredOutput != nullptr ? preferredOutput : cursor()->output()};

    if (!output)
        return;

    configureSize(output->size());
    configureState(Activated | Fullscreen);
}
//! [setFullscreenRequest]

//! [unsetFullscreenRequest]
void LToplevelRole::unsetFullscreenRequest()
{
    configureState(pending().state &~ Fullscreen);
}
//! [unsetFullscreenRequest]

//! [setMinimizedRequest]
void LToplevelRole::setMinimizedRequest()
{
    surface()->setMinimized(true);

    if (surface()->hasPointerFocus())
        seat()->pointer()->setFocus(nullptr);

    if (surface()->hasKeyboardFocus())
        seat()->keyboard()->setFocus(nullptr);

    moveSession().stop();
    resizeSession().stop();
}
//! [setMinimizedRequest]

//! [showWindowMenuRequest]
void LToplevelRole::showWindowMenuRequest(const LEvent &triggeringEvent, Int32 x, Int32 y)
{
    L_UNUSED(triggeringEvent);
    L_UNUSED(x);
    L_UNUSED(y);

    /* Here the compositor should render a context menu showing
     * the minimize, maximize and fullscreen options */
}
//! [showWindowMenuRequest]
