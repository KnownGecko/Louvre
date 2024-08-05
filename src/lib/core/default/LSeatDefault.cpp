#include <LIdleListener.h>
#include <LCompositor.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LCursor.h>
#include <LLog.h>

using namespace Louvre;

//! [nativeInputEvent]
void LSeat::nativeInputEvent(void *event)
{
    L_UNUSED(event);
}
//! [nativeInputEvent]

//! [enabledChanged]
void LSeat::enabledChanged()
{
    if (!enabled())
        return;

    cursor()->setVisible(false);
    cursor()->setVisible(true);
    cursor()->move(1, 1);
    compositor()->repaintAllOutputs();
}
//! [enabledChanged]

//! [outputPlugged]
void LSeat::outputPlugged(LOutput *output)
{
    output->setScale(output->dpi() >= 200 ? 2 : 1);

    if (compositor()->outputs().empty())
        output->setPos(0);
    else
        output->setPos(compositor()->outputs().back()->pos() + LPoint(compositor()->outputs().back()->size().w(), 0));

    compositor()->addOutput(output);
    compositor()->repaintAllOutputs();
}
//! [outputPlugged]

//! [outputUnplugged]
void LSeat::outputUnplugged(LOutput *output)
{
    compositor()->removeOutput(output);

    Int32 totalWidth { 0 };

    for (LOutput *o : compositor()->outputs())
    {
        o->setPos(LPoint(totalWidth, 0));
        totalWidth += o->size().w();
    }

    compositor()->repaintAllOutputs();
}
//! [outputUnplugged]

//! [inputDevicePlugged]
void LSeat::inputDevicePlugged(LInputDevice *device)
{
    L_UNUSED(device);
}
//! [inputDevicePlugged]

//! [inputDeviceUnplugged]
void LSeat::inputDeviceUnplugged(LInputDevice *device)
{
    L_UNUSED(device);
}
//! [inputDeviceUnplugged]

bool LSeat::isIdleStateInhibited() const
{
    for (LSurface *surface : idleInhibitorSurfaces())
        if (surface->mapped() && !surface->outputs().empty())
            return true;

    return false;
}

void LSeat::onIdleListenerTimeout(const LIdleListener &listener)
{
    if (isIdleStateInhibited())
        listener.resetTimer();

    /*
     * If the timer is not reset, the client will assume the user is idle.
     */
}

void LSeat::onEvent(const LEvent &event)
{
    L_UNUSED(event)

    /*
     * Resetting all timers each time an event occurs is not CPU-friendly,
     * as multiple events can be triggered in a single main loop iteration.
     * Instead, using the option below is more efficient.
     *
     * for (const LIdleListener *idleListener : idleListeners())
     *     idleListener->resetTimer();
     */

    /*
     * Setting this flag to false indicates the user wasn't idle during
     * this main loop iteration. If that's the case, Louvre will reset
     * all timers only once at the end of the iteration.
     */
    setIsUserIdleHint(false);
}
