#ifndef LPOSITIONERPRIVATE_H
#define LPOSITIONERPRIVATE_H

#include <LPositioner.h>

using namespace Louvre;

class LPositioner::LPositionerPrivate
{
    struct PositionerData
    {
        LCompositor *compositor                                     = nullptr;

        LSize sizeS, sizeC;
        LRect anchorRectS, anchorRectC;
        LPoint offsetS, offsetC;

        UInt32 anchor                                               = Anchor::NoAnchor;
        UInt32 gravity                                              = Gravity::NoGravity;
        UInt32 constraintAdjustment                                 = ConstraintAdjustment::NoAdjustment;

    #if LOUVRE_XDG_WM_BASE_VERSION >= 3
        bool isReactive                                             = false;
        LSize parentSizeS, parentSizeC;
        UInt32 parentConfigureSerial;
    #endif
    };

public:
    LPositionerPrivate()                                        = default;
    ~LPositionerPrivate()                                       = default;

    LPositionerPrivate(const LPositionerPrivate&)               = delete;
    LPositionerPrivate& operator= (const LPositionerPrivate&)   = delete;
    PositionerData data;

    void updateGlobalScale();
};

#endif // LPOSITIONERPRIVATE_H
