#ifndef LOBJECT_H
#define LOBJECT_H

#include <LNamespaces.h>

/*!
 * @brief Base class for Louvre objects.
 */
class Louvre::LObject
{
public:
    LObject();

    /*!
     * The global compositor instance.
     */
    static LCompositor *compositor();

    /*!
     * The global seat instance.
     */
    static LSeat *seat();

    /*!
     * Returns a pointer to the cursor implementation provided by the library.\n
     * Must be accessed within or after the LCompositor::initialized() or LCompositor::cursorInitialized() virtual methods are invoked.\n
     * If the cursor has not yet been initialized, this method returns nullptr.
     */
    static LCursor *cursor();

    LPRIVATE_IMP(LObject);
};

#endif // LOBJECT_H
