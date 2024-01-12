#pragma once
#include "chipmunk/chipmunk.h"
#include "thelib/rect.hpp"
#include "thelib/slice.hpp"
#include "thelib/vect.hpp"
#include <chipmunk/chipmunk_structs.h>

namespace lib {

class body_t;
class space_t;

class shape_t : public ::cpShape
{
  public:
    // youre not inteded to make one of these, its abstract
    shape_t() = delete;

    inline constexpr shape_t(const cpShape &original) TESTING_NOEXCEPT
        : ::cpShape(original)
    {
    }

    [[nodiscard]] float friction() TESTING_NOEXCEPT;
    void set_friction(float friction) TESTING_NOEXCEPT;
    [[nodiscard]] bool sensor() TESTING_NOEXCEPT;
    void set_sensor(bool is_sensor) TESTING_NOEXCEPT;
    [[nodiscard]] float density() TESTING_NOEXCEPT;
    void set_density(float density) TESTING_NOEXCEPT;
    [[nodiscard]] cpShapeFilter filter() TESTING_NOEXCEPT;
    void set_filter(cpShapeFilter filter) TESTING_NOEXCEPT;
    [[nodiscard]] float elasticity() TESTING_NOEXCEPT;
    void set_elasticity(float elasticity) TESTING_NOEXCEPT;
    [[nodiscard]] cpDataPointer user_data() TESTING_NOEXCEPT;
    void set_user_data(cpDataPointer pointer) TESTING_NOEXCEPT;
    [[nodiscard]] vect_t surface_velocity() TESTING_NOEXCEPT;
    void set_surface_velocity(vect_t velocity) TESTING_NOEXCEPT;
    [[nodiscard]] cpCollisionType collision_type() TESTING_NOEXCEPT;
    void set_collision_type(cpCollisionType type) TESTING_NOEXCEPT;

    [[nodiscard]] rect_t get_bounding_box() TESTING_NOEXCEPT;

    // not a destructor becasue we'd like to be able to cast cpShapes to this
    // wtihout invoking RAII
    void free() TESTING_NOEXCEPT;

    [[nodiscard]] body_t *body() TESTING_NOEXCEPT;

    // initialize separately

    /// Circle constructor
    // void Init(ShapeType type, const Body &body, float radius, vect_t offset);
    // /// Segment constructor
    // void Init(ShapeType type, const Body &body, vect_t a, vect_t b,
    //           float radius);
    // /// Poly shape constructor
    // void Init(ShapeType type, const Body &body, vect_t sides[],
    //           const int number_of_sides, cpTransform transform, float
    //           radius);
    // /// Poly shape constructor with default radius of 1
    // void Init(ShapeType type, const Body &body, vect_t sides[],
    //           const int number_of_sides, cpTransform transform);
    // /// Poly shape which is a box
    // void Init(ShapeType type, const Body &body, float width, float height,
    //           float radius);
    // copy assignment op

    void remove_from_space() TESTING_NOEXCEPT;

  private:
    void _add_to_space(space_t *space) TESTING_NOEXCEPT;
    friend space_t;
};

class poly_shape_t : public ::cpPolyShape
{
  public:
    // cannot be default constructed because initializing a shape requires a
    // body
    poly_shape_t() = delete;

    struct default_options_t
    {
        cpCollisionType collision_type;
        slice_t<vect_t> vertices;
        float radius;
    };

    inline constexpr poly_shape_t(const cpPolyShape &original) TESTING_NOEXCEPT
        : ::cpPolyShape(original)
    {
    }

    poly_shape_t(lib::body_t &body,
                 const default_options_t &options) TESTING_NOEXCEPT;

    struct square_options_t
    {
        cpCollisionType collision_type;
        lib::rect_t bounding;
        float radius;
    };

    poly_shape_t(lib::body_t &body,
                 const square_options_t &options) TESTING_NOEXCEPT;

    [[nodiscard]] int count() const TESTING_NOEXCEPT;
    [[nodiscard]] float radius() const TESTING_NOEXCEPT;
    [[nodiscard]] vect_t vertex(int index) const TESTING_NOEXCEPT;
    inline constexpr lib::shape_t *parent_cast() TESTING_NOEXCEPT
    {
        return static_cast<lib::shape_t *>(&shape);
    }
};

class segment_shape_t : public ::cpSegmentShape
{
  public:
    segment_shape_t() = delete;

    inline constexpr segment_shape_t(const cpSegmentShape &original)
        TESTING_NOEXCEPT : ::cpSegmentShape(original)
    {
    }

    struct options_t
    {
        cpCollisionType collision_type;
        lib::vect_t a;
        lib::vect_t b;
        float radius;
    };

    segment_shape_t(lib::body_t &body,
                    const options_t &options) TESTING_NOEXCEPT;

    [[nodiscard]] vect_t a() const TESTING_NOEXCEPT;
    [[nodiscard]] vect_t b() const TESTING_NOEXCEPT;
    [[nodiscard]] vect_t normal() const TESTING_NOEXCEPT;

    void set_neighbors(vect_t prev, vect_t next) TESTING_NOEXCEPT;
    inline constexpr lib::shape_t *parent_cast() TESTING_NOEXCEPT
    {
        return static_cast<lib::shape_t *>(&shape);
    }
};

} // namespace lib
