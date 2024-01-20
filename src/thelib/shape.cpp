#include "shape.hpp"
#include "body.hpp"
#include <cassert>

#ifdef THELIB_DEBUG
#define ASSERT_BODY_INITIALIZED() assert(body.initialized)
#else
#define ASSERT_BODY_INITIALIZED()
#endif

namespace lib {
void shape_t::_add_to_space(space_t *space) TESTING_NOEXCEPT
{
    cpSpaceAddShape(reinterpret_cast<cpSpace *>(space), this);
}

float shape_t::friction() TESTING_NOEXCEPT { return cpShapeGetFriction(this); }

void shape_t::set_friction(float friction) TESTING_NOEXCEPT
{
    cpShapeSetFriction(this, friction);
};

bool shape_t::sensor() TESTING_NOEXCEPT { return cpShapeGetSensor(this); }

void shape_t::set_sensor(bool is_sensor) TESTING_NOEXCEPT
{
    cpShapeSetSensor(this, is_sensor);
}

float shape_t::density() TESTING_NOEXCEPT { return cpShapeGetDensity(this); }

void shape_t::set_density(float density) TESTING_NOEXCEPT
{
    cpShapeSetDensity(this, density);
}

cpShapeFilter shape_t::filter() TESTING_NOEXCEPT
{
    return cpShapeGetFilter(this);
}

void shape_t::set_filter(cpShapeFilter filter) TESTING_NOEXCEPT
{
    cpShapeSetFilter(this, filter);
}

float shape_t::elasticity() TESTING_NOEXCEPT
{
    return cpShapeGetElasticity(this);
}

void shape_t::set_elasticity(float elasticity) TESTING_NOEXCEPT
{
    cpShapeSetElasticity(this, elasticity);
}

cpDataPointer shape_t::user_data() TESTING_NOEXCEPT
{
    return cpShapeGetUserData(this);
}

void shape_t::set_user_data(cpDataPointer pointer) TESTING_NOEXCEPT
{
    cpShapeSetUserData(this, pointer);
}

vect_t shape_t::surface_velocity() TESTING_NOEXCEPT
{
    return cpShapeGetSurfaceVelocity(this);
}

void shape_t::set_surface_velocity(vect_t velocity) TESTING_NOEXCEPT
{
    cpShapeSetSurfaceVelocity(this, velocity);
}

cpCollisionType shape_t::collision_type() TESTING_NOEXCEPT
{
    return cpShapeGetCollisionType(this);
}

void shape_t::set_collision_type(cpCollisionType type) TESTING_NOEXCEPT
{
    cpShapeSetCollisionType(this, type);
}

void shape_t::free() TESTING_NOEXCEPT { cpShapeFree(this); }

body_t *shape_t::body() TESTING_NOEXCEPT
{
    return static_cast<body_t *>(cpShapeGetBody(this));
}

void shape_t::remove_from_space() TESTING_NOEXCEPT
{
    if (space != nullptr)
        cpSpaceRemoveShape(space, this);
}

rect_t shape_t::get_bounding_box() TESTING_NOEXCEPT
{
    return cpShapeGetBB(this);
}

int poly_shape_t::count() const TESTING_NOEXCEPT
{
    return cpPolyShapeGetCount(&shape);
}
float poly_shape_t::radius() const TESTING_NOEXCEPT
{
    return cpPolyShapeGetRadius(&shape);
}
vect_t poly_shape_t::vertex(int index) const TESTING_NOEXCEPT
{
    return cpPolyShapeGetVert(&shape, index);
}

poly_shape_t::poly_shape_t(lib::body_t &body,
                           const default_options_t &options) TESTING_NOEXCEPT
{
    static_assert(
        sizeof(cpVect) == sizeof(lib::vect_t),
        "Make sure lib::vect_t and cpVect are laid out identically in memory");
    cpPolyShapeInitRaw(
        this, &body, static_cast<int>(options.vertices.size()),
        reinterpret_cast<const cpVect *>(options.vertices.data()),
        options.radius);
    cpShapeSetCollisionType(&shape, options.collision_type);
}

poly_shape_t::poly_shape_t(lib::body_t &body,
                           const square_options_t &options) TESTING_NOEXCEPT
{
    cpBoxShapeInit2(this, &body, options.bounding, options.radius);
    cpShapeSetCollisionType(&shape, options.collision_type);
}

vect_t segment_shape_t::a() const TESTING_NOEXCEPT
{
    return cpSegmentShapeGetA(&shape);
}
vect_t segment_shape_t::b() const TESTING_NOEXCEPT
{
    return cpSegmentShapeGetB(&shape);
}
vect_t segment_shape_t::normal() const TESTING_NOEXCEPT
{
    return cpSegmentShapeGetNormal(&shape);
}

void segment_shape_t::set_neighbors(vect_t prev, vect_t next) TESTING_NOEXCEPT
{
    cpSegmentShapeSetNeighbors(&shape, prev, next);
}

segment_shape_t::segment_shape_t(lib::body_t &body,
                                 const options_t &options) TESTING_NOEXCEPT
{
    // TODO: figure out why reinterpret_cast is necessary here, body_t
    // publicly inherits from cpBody?
    cpSegmentShapeInit(this, reinterpret_cast<cpBody *>(&body), options.a,
                       options.b, options.radius);

    cpShapeSetCollisionType(&shape, options.collision_type);
}
} // namespace lib
