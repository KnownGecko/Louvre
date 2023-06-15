#ifndef LPOPUPROLE_H
#define LPOPUPROLE_H

#include <LBaseSurfaceRole.h>
#include <LRect.h>

using namespace Louvre::Protocols;

/*!
 * @brief Popup role for surfaces
 *
 * The LPopupRole class is a role for surfaces commonly used by clients to display context menus and tooltips.\n
 * <center><img src="https://lh3.googleusercontent.com/6caGayutKKWqndpd6ogno2lPw8XGELxnFums4gvkWZKOJYO0762yVG3mHLrc1rw63r1eEJabEdW9F5AA2BDTFCtpB_hiPlTY4FkKlHfH1B-2MdLvXCD6RuwZOZvhOl6EhydtsOYGPw=w2400"></center>
 * 
 * Popup surfaces are always children of other surfaces (other Popups or Toplevels). 
 * They have complex positioning rules defined in their LPositioner instance. 
 * The default implementation of rolePosC() implements these rules, making it possible to restrict the area where the Popup should be positioned with setPositionerBoundsC().\n
 * 
 * The Popup role is part of the [XDG Shell](https://wayland.app/protocols/xdg-shell#xdg_popup) protocol.
 * The Wayland protocol also offers its own Popup role, but it is considered obsolete and therefore not included in the library.
 */
class Louvre::LPopupRole : public LBaseSurfaceRole
{
public:
    struct Params;

    /*!
     * @brief Constructor for LPopupRole class.
     * @param params Internal library parameters provided in the virtual LCompositor::createPopupRoleRequest() constructor.
     */
    LPopupRole(Params *params);

    /*!
     * @brief Destructor of LPopupRole class.
     *
     * Invoked after LCompositor::destroyPopupRequest().
     */
    virtual ~LPopupRole();

    LPopupRole(const LPopupRole&) = delete;
    LPopupRole& operator= (const LPopupRole&) = delete;

    /*!
     * @brief Window geometry in surface coordinates.
     *
     * The window geometry is a rectangle of the Popup that excludes its decorations (typically shadows).
     */
    const LRect &windowGeometryS() const;

    /*!
     * @brief Window geometry in compositor coordinates.
     *
     * The window geometry is a rectangle of the Popup that excludes its decorations (typically shadows).\n
     * Its components (x,y,width,height) are given by:
     *
     * <center><img src="https://lh3.googleusercontent.com/qYH--yLCkb39PBCqAGqNC8l-jL8YuDPCIcUOTaqXyvp0hUr8Rj6Ug8MS7Fb68-XuWOyhsmsOYb5rKaph3hO40w-3J9zRzISuvCRfU5pFf6dVJ8lgbH_JJ2FpkAYijbH0POUyiB7xDw=w2400" width="512px"></center>
     */
    const LRect &windowGeometryC() const;

    /*!
     * @brief Positioning rules.
     *
     * Rules for positioning the Popup.
     */
    const LPositioner &positioner() const;

    /*!
     * @brief Constraints the positioning area of the Popup.
     *
     * Constraints the area where the Popup can be positioned.
     *
     * @param bounds Constraint rectangle (x,y,w,h). Passing a rectangle with area 0 deactivates the constraint.
     */
    void setPositionerBoundsC(const LRect &bounds);

    /*!
     * @brief Constraint area for the Popup's position in surface coordinates.
     *
     * The constraint area for the Popup's position, set using setPositionerBoundsC().
     */
    const LRect &positionerBoundsS() const;

    /*!
     * @brief Popup position constraint area in compositor coordinates.
     *
     * Constraint area of the Popup in compositor coordinates, assigned with setPositionerBoundsC().
     */
    const LRect &positionerBoundsC() const;

    /// xdg_popup resource
    XdgShell::RXdgPopup *xdgPopupResource() const;

    /// xdg_surface resource
    XdgShell::RXdgSurface *xdgSurfaceResource() const;

/// @name Events
/// @{

    /*!
     * @brief Configures the Popup.
     *
     * Suggests a size and position for the Popup relative to its parent position.\n
     * The position and size of the rect refers to the window geometry of the Popup (ignoring its decoration).
     *
     * @param rect Suggested position and size.
     */
    void configureC(const LRect &rect);

    /*!
     * @brief Dimisses the Popup.
     *
     * Dismisses the Popup.
     */
    void sendPopupDoneEvent();

    /*!
     * @brief Acknowledge a repositioning request (since 3)
     *
     * Notifies the client that the Popup has been repositioned using the new LPositioner.
     *
     * @param token Token generated by repositionRequest().
     */
    void sendRepositionedEvent(UInt32 token);
///@}


/// @name Virtual Methods
/// @{
    /*!
     * @brief Position of the Popup surface according to the role.
     *
     * The default implementation of rolePosC() positions the Popup following the rules of its LPositioner and
     * restricting its area to positionerBoundsC().
     *
     * Reimplement this virtual method if you want to define your own logic for positioning the Popup.
     *
     * #### Default implementation
     * @snippet LPopupRoleDefault.cpp rolePosC
     */
    virtual const LPoint &rolePosC() const override;

    /*!
     * @brief Response to a Ping event.
     *
     * Client response to a ping() event.
     *
     * Reimplement this virtual method if you want to be notified when a client responds to a ping() event.
     *
     * @param serial The same serial passed in ping().
     *
     * #### Default implementation
     * @snippet LPopupRoleDefault.cpp pong
     */
    virtual void pong(UInt32 serial);

    /*!
     * @brief Geometry change.
     *
     * Change in the geometry of the Popup window accessible with windowGeometryS() or windowGeometryC().
     *
     * Reimplement this virtual method if you want to be notified when the Popup changes its window geometry
     * (typically in response to a configureC() event).
     *
     * #### Default implementation
     * @snippet LPopupRoleDefault.cpp geometryChanged
     */
    virtual void geometryChanged();

    /*!
     * @brief Request to acquire keyboard focus.
     *
     * Request from the Popup for its surface to acquire keyboard focus.
     *
     * Reimplement this virtual method if you want to be notified when the Popup requests to acquire
     * keyboard focus.
     *
     * #### Default implementation
     * @snippet LPopupRoleDefault.cpp grabSeatRequest
     */
    virtual void grabSeatRequest();

    /*!
     * @brief Configuration request.
     *
     * Request from a Popup for the compositor to suggest its position and size relative to its parent
     * according to the rules of its LPositioner.
     *
     * Reimplement this virtual method if you want to be notified when the Popup requests to be configured.
     *
     * #### Default implementation
     * @snippet LPopupRoleDefault.cpp configureRequest
     */
    virtual void configureRequest();

#if LOUVRE_XDG_WM_BASE_VERSION >= 3

    /*!
     * @brief Repositioning request.
     *
     * Request from a Popup for the compositor to reposition it using a new LPositioner.
     * Once repositioned, the sendRepositionedEvent() event should be sent, passing the same token given in the argument.
     * Reimplement this virtual method if you want to be notified when the Popup requests to be repositioned.
     *
     * #### Default implementation
     * @snippet LPopupRoleDefault.cpp repositionRequest
     */
    virtual void repositionRequest(UInt32 token);
#endif
///@}

    LPRIVATE_IMP(LPopupRole)

    void handleSurfaceCommit() override;
    void globalScaleChanged(Int32 oldScale, Int32 newScale) override;
};

#endif // LPOPUPROLE_H
