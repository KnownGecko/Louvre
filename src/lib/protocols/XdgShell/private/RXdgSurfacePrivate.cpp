#include <protocols/XdgShell/private/RXdgSurfacePrivate.h>
#include <protocols/XdgShell/private/RXdgToplevelPrivate.h>
#include <protocols/XdgShell/private/RXdgPopupPrivate.h>
#include <protocols/XdgShell/xdg-shell.h>
#include <protocols/XdgShell/RXdgPositioner.h>
#include <private/LSurfacePrivate.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LPopupRolePrivate.h>
#include <LPositioner.h>
#include <LLog.h>

void RXdgSurface::RXdgSurfacePrivate::resource_destroy(wl_resource *resource)
{
    RXdgSurface *rXdgSurface = (RXdgSurface*)wl_resource_get_user_data(resource);
    delete rXdgSurface;
}

void RXdgSurface::RXdgSurfacePrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    RXdgSurface *rXdgSurface = (RXdgSurface*)wl_resource_get_user_data(resource);

    if (rXdgSurface->lSurface()->role())
    {
        wl_resource_post_error(resource, 0, "xdg_surface must be destroyed after its specific role");
        return;
    }

    wl_resource_destroy(resource);
}

void RXdgSurface::RXdgSurfacePrivate::get_toplevel(wl_client *client, wl_resource *resource, UInt32 id)
{
    L_UNUSED(client);

    RXdgSurface *rXdgSurface = (RXdgSurface*)wl_resource_get_user_data(resource);

    if (rXdgSurface->lSurface()->imp()->hasBufferOrPendingBuffer())
    {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED, "Given wl_surface already has a buffer attached.");
        return;
    }

    if (rXdgSurface->lSurface()->imp()->hasRoleOrPendingRole())
    {
        wl_resource_post_error(resource, XDG_WM_BASE_ERROR_ROLE, "Given wl_surface has another role.");
        return;
    }

    new RXdgToplevel(rXdgSurface, id);
}

void RXdgSurface::RXdgSurfacePrivate::get_popup(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *parent, wl_resource *positioner)
{
    L_UNUSED(client);

    RXdgPositioner *rXdgPositioner = (RXdgPositioner*)wl_resource_get_user_data(positioner);

    if (!rXdgPositioner->isValid())
        return;

    RXdgSurface *rXdgSurface = (RXdgSurface*)wl_resource_get_user_data(resource);

    if (rXdgSurface->lSurface()->imp()->hasBufferOrPendingBuffer())
    {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED, "Given wl_surface already has a buffer attached.");
        return;
    }

    if (rXdgSurface->lSurface()->imp()->hasRoleOrPendingRole())
    {
        wl_resource_post_error(resource, XDG_WM_BASE_ERROR_ROLE, "Given wl_surface has another role.");
        return;
    }

    if (!parent)
    {
        wl_resource_post_error(positioner, XDG_WM_BASE_ERROR_INVALID_POPUP_PARENT, "xdg_popup's without parent not supported");
        return;
    }

    RXdgSurface *rXdgParentSurface = nullptr;
    rXdgParentSurface = (RXdgSurface*)wl_resource_get_user_data(parent);

    if (rXdgSurface->lSurface()->imp()->isInChildrenOrPendingChildren(rXdgParentSurface->lSurface()))
    {
        wl_resource_post_error(positioner,
                               XDG_WM_BASE_ERROR_INVALID_POPUP_PARENT,
                               "Parent can not be child or equal to surface.");
        return;
    }

    new RXdgPopup(rXdgSurface, rXdgParentSurface, rXdgPositioner, id);
}

void RXdgSurface::RXdgSurfacePrivate::set_window_geometry(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);

    RXdgSurface *rXdgSurface = (RXdgSurface*)wl_resource_get_user_data(resource);

    if (!rXdgSurface->rXdgPopup() && !rXdgSurface->rXdgToplevel())
    {
        wl_resource_post_error(resource, 0, "Can not set window geometry with no role.");
        return;
    }

    if (width <= 0 || height <= 0)
    {
        wl_resource_post_error(resource, 0, "Invalid window geometry size.");
        return;
    }

    rXdgSurface->imp()->pendingWindowGeometryS = LRect(x, y, width, height);
    rXdgSurface->imp()->windowGeometrySet = true;
    rXdgSurface->imp()->hasPendingWindowGeometry = true;
}

void RXdgSurface::RXdgSurfacePrivate::ack_configure(wl_client *client, wl_resource *resource, UInt32 serial)
{
    L_UNUSED(client);

    RXdgSurface *rXdgSurface = (RXdgSurface*)wl_resource_get_user_data(resource);

    if (!rXdgSurface->rXdgPopup() && !rXdgSurface->rXdgToplevel())
    {
        wl_resource_post_error(resource, 0, "Can not ack xdg_surface with no role.");
        return;
    }

    if (rXdgSurface->lSurface()->roleId() == LSurface::Role::Toplevel)
    {
        LToplevelRole *toplevel = rXdgSurface->lSurface()->toplevel();

        while(!toplevel->imp()->sentConfs.empty())
        {
            if (toplevel->imp()->sentConfs.front().serial == serial)
            {
                toplevel->imp()->currentConf = toplevel->imp()->sentConfs.front();
                toplevel->imp()->sentConfs.pop_front();
                break;
            }

            toplevel->imp()->sentConfs.pop_front();
        }

        if (toplevel->imp()->xdgDecoration && toplevel->imp()->pendingDecorationMode != 0 && toplevel->imp()->lastDecorationModeConfigureSerial <= serial)
        {
            LLog::debug("DECORATION CHANGED");
            toplevel->imp()->decorationMode = (LToplevelRole::DecorationMode)toplevel->imp()->pendingDecorationMode;
            toplevel->decorationModeChanged();
            toplevel->imp()->pendingDecorationMode = 0;
            return;
        }
    }
    else if (rXdgSurface->lSurface()->roleId() == LSurface::Role::Popup)
    {

    }
    else
    {
        wl_resource_post_error(
            rXdgSurface->resource(),
            XDG_SURFACE_ERROR_NOT_CONSTRUCTED,
            "wl_surface does not have a role yet.");
    }
}
