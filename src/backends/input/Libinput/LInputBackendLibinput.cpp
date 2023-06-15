#include <private/LCompositorPrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LKeyboardPrivate.h>
#include <LInputBackend.h>
#include <LLog.h>
#include <unordered_map>
#include <cstring>

using namespace Louvre;

struct BACKEND_DATA
{
    libinput *li = nullptr;
    udev *ud = nullptr;
    libinput_interface libinputInterface;
    LSeat *seat;
};

// Libseat devices
std::unordered_map<int,int>devices;

// Event common
static libinput_event *ev;
static libinput_event_type eventType;

// Keyboard related
static libinput_event_keyboard *keyEvent;
static libinput_key_state keyState;
static Int32 keyCode;

// Pointer related
static libinput_event_pointer *pointerEvent;
static libinput_button_state pointerButtonState;
static UInt32 pointerButton;
static Float64 x = 0.0, y = 0.0;

// For any axis event
static UInt32 axisSource;
static Float64 axisX = 0.0, axisY = 0.0;

// For discrete scroll events
static Float64 discreteX = 0.0, discreteY = 0.0;

// For 120 scroll events
static Float64 d120X = 0.0, d120Y = 0.0;

static Int32 openRestricted(const char *path, int flags, void *data)
{
    L_UNUSED(flags);

    LSeat *seat = (LSeat*)data;
    int id, fd;

    id = seat->openDevice(path, &fd);
    if (seat->imp()->initLibseat())
        devices[fd] = id;

    return fd;
}

static void closeRestricted(int fd, void *data)
{
    LSeat *seat = (LSeat*)data;

    if (seat->imp()->initLibseat())
        seat->closeDevice(devices[fd]);
    else
        seat->closeDevice(fd);
}

static Int32 processInput(int, unsigned int, void *userData)
{
    LSeat *seat = (LSeat*)userData;
    BACKEND_DATA *data = (BACKEND_DATA*)seat->imp()->inputBackendData;

    int ret = libinput_dispatch(data->li);

    if (ret != 0)
    {
        LLog::error("[Libinput Backend] Failed to dispatch libinput %s.", strerror(-ret));
        return 0;
    }

    while ((ev = libinput_get_event(data->li)) != NULL)
    {
        eventType = libinput_event_get_type(ev);

        if (eventType == LIBINPUT_EVENT_POINTER_MOTION)
        {
            if (!(seat->capabilities() & LSeat::Pointer))
                goto skip;

            pointerEvent = libinput_event_get_pointer_event(ev);

            x = libinput_event_pointer_get_dx(pointerEvent);
            y = libinput_event_pointer_get_dy(pointerEvent);

            seat->pointer()->pointerMoveEvent(x, y);
        }
        else if (eventType == LIBINPUT_EVENT_POINTER_BUTTON)
        {
            if (!(seat->capabilities() & LSeat::Pointer))
                goto skip;

            pointerEvent = libinput_event_get_pointer_event(ev);
            pointerButton = libinput_event_pointer_get_button(pointerEvent);
            pointerButtonState = libinput_event_pointer_get_button_state(pointerEvent);

            seat->pointer()->pointerButtonEvent(
                (LPointer::Button)pointerButton,
                (LPointer::ButtonState)pointerButtonState);
        }
        else if (eventType == LIBINPUT_EVENT_KEYBOARD_KEY)
        {
            if (!(seat->capabilities() & LSeat::Keyboard))
                goto skip;

            keyEvent = libinput_event_get_keyboard_event(ev);
            keyState = libinput_event_keyboard_get_key_state(keyEvent);
            keyCode = libinput_event_keyboard_get_key(keyEvent);

            if (seat->keyboard()->imp()->backendKeyEvent(keyCode, keyState))
            {
                libinput_event_destroy(ev);
                return 0;
            }
        }
        else if (eventType == LIBINPUT_EVENT_POINTER_SCROLL_FINGER)
        {
            if (!(seat->capabilities() & LSeat::Pointer))
                goto skip;

            pointerEvent = libinput_event_get_pointer_event(ev);

            if (libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL))
                axisX = libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);

            if (libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL))
                axisY = libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);

            seat->pointer()->pointerAxisEvent(axisX, axisY, axisX, axisY, 1);
        }
        else if (eventType == LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS)
        {
            if (!(seat->capabilities() & LSeat::Pointer))
                goto skip;

            pointerEvent = libinput_event_get_pointer_event(ev);

            if (libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL))
                axisX = libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);

            if (libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL))
                axisY = libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);

            seat->pointer()->pointerAxisEvent(axisX, axisY, axisX, axisY, 2);
        }
        else if (eventType == LIBINPUT_EVENT_POINTER_SCROLL_WHEEL)
        {
            if (!(seat->capabilities() & LSeat::Pointer))
                goto skip;

            pointerEvent = libinput_event_get_pointer_event(ev);

            if (libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL))
            {
                discreteX = libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
                d120X = libinput_event_pointer_get_scroll_value_v120(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
            }

            if (libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL))
            {
                discreteY = libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
                d120Y = libinput_event_pointer_get_scroll_value_v120(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
            }

            seat->pointer()->pointerAxisEvent(discreteX, discreteY, d120X, d120Y, 0);
        }

        skip:
        seat->backendNativeEvent(ev);
        libinput_event_destroy(ev);
        libinput_dispatch(data->li);
    }
    return 0;
}

bool LInputBackend::initialize(const LSeat *seat)
{
    seat->imp()->initLibseat();

    BACKEND_DATA *data = new BACKEND_DATA;
    data->seat = (LSeat*)seat;
    seat->imp()->inputBackendData = data;
    data->ud = udev_new();

    if (!data->ud)
        goto fail;

    data->libinputInterface.open_restricted = &openRestricted;
    data->libinputInterface.close_restricted = &closeRestricted;
    data->li = libinput_udev_create_context(&data->libinputInterface, data->seat, data->ud);

    if (!data->li)
        goto fail;

    if (seat->imp()->initLibseat())
        libinput_udev_assign_seat(data->li, libseat_seat_name(seat->libseatHandle()));
    else
        libinput_udev_assign_seat(data->li, "seat0");

    libinput_dispatch(data->li);
    LCompositor::addFdListener(libinput_get_fd(data->li), (LSeat*)seat, &processInput);

    return true;

    fail:
    uninitialize(seat);
    return false;
}

UInt32 LInputBackend::getCapabilities(const LSeat *seat)
{
    L_UNUSED(seat);
    return LSeat::InputCapabilities::Pointer | LSeat::InputCapabilities::Keyboard;
}

void *LInputBackend::getContextHandle(const LSeat *seat)
{
    BACKEND_DATA *data = (BACKEND_DATA*)seat->imp()->inputBackendData;
    return data->li;
}

void LInputBackend::suspend(const LSeat *seat)
{
    BACKEND_DATA *data = (BACKEND_DATA*)seat->imp()->inputBackendData;
    libinput_suspend(data->li);
}

void LInputBackend::forceUpdate(const LSeat *seat)
{
    processInput(0, 0, (LSeat*)seat);
}

void LInputBackend::resume(const LSeat *seat)
{
    BACKEND_DATA *data = (BACKEND_DATA*)seat->imp()->inputBackendData;
    libinput_resume(data->li);
}

void LInputBackend::uninitialize(const LSeat *seat)
{
    BACKEND_DATA *data = (BACKEND_DATA*)seat->imp()->inputBackendData;

    if (!data)
        return;

    if (data->li)
        libinput_unref(data->li);

    if (data->ud)
        udev_unref(data->ud);

    delete data;
    seat->imp()->inputBackendData = nullptr;
}

LInputBackendInterface API;

extern "C" LInputBackendInterface *getAPI()
{
    API.initialize = &LInputBackend::initialize;
    API.uninitialize = &LInputBackend::uninitialize;
    API.getCapabilities = &LInputBackend::getCapabilities;
    API.getContextHandle = &LInputBackend::getContextHandle;
    API.suspend = &LInputBackend::suspend;
    API.forceUpdate = &LInputBackend::forceUpdate;
    API.resume = &LInputBackend::resume;
    return &API;
}
