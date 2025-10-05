#pragma once

#include "dependencies/glm/glm.hpp"

namespace mball {
    struct IsoPoint {
        float density;
    };

    class IsoSurface {
        
        float length;
        float width;
        float height;
    };
}