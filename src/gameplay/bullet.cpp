#include <gameplay/bullet.hpp>

constexpr size_t initial_bullet_reservation = 100;

namespace cw::gameplay::bullet {

static lib::opt_t<bullet_allocator> bullets;

bullet_t::~bullet_t() noexcept
{
    physics::delete_body(body);
    physics::delete_polygon_shape(shape);
}

lib::vect_t bullet_t::position() const noexcept
{
    return physics::get_body(body).position();
}

lib::opt_t<void *> bullet_t::user_data() const noexcept
{
    return physics::get_user_data(body);
}

/// Do not use this constructor unless you know what you're doing
bullet_t::bullet_t(const bullet_creation_options_t &options) noexcept
    : body(physics::create_body(game_id_e::Bullet,
                                lib::body_t::body_options_t{
                                    .type = lib::body_t::Type::DYNAMIC,
                                    .mass = global_mass,
                                    .moment = INFINITY,
                                })),
      shape(physics::create_box_shape(body, global_square_hitbox_options))
{
    lib::body_t &actual = physics::get_body(body);
    actual.set_position(options.position);
    actual.set_velocity(options.initial_velocity);

    // set the id of the shape to be bullet, for good measure
    auto &shape_actual = physics::get_polygon_shape(shape);
    set_physics_id(*shape_actual.parent_cast(), game_id_e::Bullet);
}

void init() noexcept { bullets.emplace(initial_bullet_reservation); }
void cleanup() noexcept { bullets.reset(); }

raw_bullet_t spawn(const bullet_creation_options_t &options) noexcept
{
    auto maybe_spawned = bullets.value().alloc_new(options);

    if (maybe_spawned.okay())
        return maybe_spawned.release();

    using code = bullet_allocator::alloc_err_code_e;
    switch (maybe_spawned.status()) {
    case code::OOM:
        LN_FATAL("Got OOM when trying to spawn bullet");
    default:
        LN_FATAL("Programmer error in bullet::spawn.");
    }
    std::abort();
    return maybe_spawned.release();
}

raw_bullet_t spawn(const bullet_creation_options_t &options,
                   void *user_data) noexcept
{
    auto handle = spawn(options);
    auto res = bullets.value().get(handle);
    if (!res.okay()) {
        LN_FATAL("A bullet that was literally just successfully allocated "
                 "failed when passed to bullets.get(). Aborting. How does that "
                 "even happen?");
        std::abort();
    }

    auto &bullet = res.release();

    physics::set_user_data_and_id(bullet.body, game_id_e::Bullet, user_data);

    return handle;
}

lib::opt_t<bullet_t &> try_get(raw_bullet_t handle) noexcept
{
    auto res = bullets.value().get(handle);
    if (!res.okay())
        // discard the status code, just turn it into an optional
        return {};
    else
        return res.release();
}

bool try_destroy(raw_bullet_t handle) noexcept
{
    return bullets.value().free(handle).okay();
}

bool is_body_bullet(cpBody &maybe_bullet)
{
    if (auto id = physics::get_id(*lib::body_t::from_chipmunk(&maybe_bullet))) {
        return id.value() == game_id_e::Bullet;
    }
    return false;
}
} // namespace cw::bullet
