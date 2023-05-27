#include <protocols/XdgDecoration/private/RXdgToplevelDecorationPrivate.h>

void RXdgToplevelDecoration::RXdgToplevelDecorationPrivate::resource_destroy(wl_resource *resource)
{
    RXdgToplevelDecoration *rXdgToplevelDecoration = (RXdgToplevelDecoration*)wl_resource_get_user_data(resource);
    delete rXdgToplevelDecoration;
}

void RXdgToplevelDecoration::RXdgToplevelDecorationPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void RXdgToplevelDecoration::RXdgToplevelDecorationPrivate::set_mode(wl_client *client, wl_resource *resource, UInt32 mode)
{
    /* Does it really matter what the client wants? */

    L_UNUSED(client);
    L_UNUSED(resource);
    L_UNUSED(mode);
}

void RXdgToplevelDecoration::RXdgToplevelDecorationPrivate::unset_mode(wl_client *client, wl_resource *resource)
{
    /* Does it really matter what the client wants? */

    L_UNUSED(client);
    L_UNUSED(resource);
}
