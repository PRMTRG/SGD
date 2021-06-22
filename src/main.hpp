#ifndef ___MAIN_CLASS_FOR_BULLETHELL_HPP__
#define ___MAIN_CLASS_FOR_BULLETHELL_HPP__

class physical_c
{
public:
    std::array<double, 2> position;
    std::array<double, 2> velocity;
    std::array<double, 2> acceleration;
    double friction;

    // basic physics
    void update(double dt_f)
    {
        using namespace tp::operators;
        auto new_acceleration = acceleration - velocity * length(velocity) * friction;
        auto new_velocity = velocity + new_acceleration * dt_f;
        auto new_position = position + new_velocity * dt_f + new_acceleration * dt_f * dt_f * 0.5;
        position = new_position;
        velocity = new_velocity;
        acceleration = new_acceleration;
    }
};

class bullet_c : public physical_c
{
public:
    std::string type;
    std::string behavior;
    bool damages_player = false;
    bool blocked_by_obstacles = false;
    bool destroyed_on_contact = false;
    bool expired = false;
    double time = 0.2;
    void update(double dt_f)
    {
        using namespace tp::operators;

        if (behavior == "a") {
            if (time > 0.5) {
                acceleration = {30, 100};
            }
            else {
                acceleration = {30, -100};
            }
            if (time > 1) {
                time = 0;
            }
            time += dt_f;
        }
        else if (behavior == "b") {
            acceleration = {0, 50};
            time += dt_f;
            if (time > 4) expired = true;
        }
        else if (behavior == "c") {
            acceleration = {-20, 0};
        }

        auto new_acceleration = acceleration - velocity * length(velocity) * friction;
        auto new_velocity = velocity + new_acceleration * dt_f;
        auto new_position = position + new_velocity * dt_f + new_acceleration * dt_f * dt_f * 0.5;
        position = new_position;
        velocity = new_velocity;
        acceleration = new_acceleration;

        if (behavior == "a") {
            if (position[1] < 15.5) position[1] = 15.5;
            if (position[1] > 22) position[1] = 22;
        }

    }
};

class emitter_c : public physical_c
{
public:
    double emit_to_emit;
    double emit_delay;
};

class player_c : public physical_c
{
public:
    std::map<std::string, int> intentions;

    double health;
    double points;
    double max_hspeed;
    double jump_time = 0.25;
    bool jump_available = true;
    double jump_time_left = jump_time;
    bool crouching = false;
    int gun_angle = 0;
    bool last_move_left = false;
    bool on_ground = false;

    player_c(std::array<double, 2> position_ = {10, 10}, std::array<double, 2> velocity_ = {0, 0}, std::array<double, 2> acceleration_ = {0, 0}, double friction_ = 0.03,
             double max_hspeed_ = 15)
    {
        position = position_;
        velocity = velocity_;
        acceleration = acceleration_;
        friction = friction_;
        max_hspeed = max_hspeed_;

        health = 100;
        points = 0;
    }

    void apply_intent()
    {

    }

    void update(double dt_f)
    {
        //apply_intent();
        using namespace tp::operators;

        if (intentions.count("gun_up")) gun_angle = std::min(70, gun_angle + 2);
        if (intentions.count("gun_down")) gun_angle = std::max(-10, gun_angle - 2);

        acceleration = {0, 50};
        if (intentions.count("right")) {
            acceleration[0] += 100;
            last_move_left = false;
        }

        if (intentions.count("left")) {
            acceleration[0] += -100;
            last_move_left = true;
        }

        if (intentions.count("down")) {
            if (!crouching) {
                position[1] += 0.35;
            }
            crouching = true;
        }
        else {
            if (crouching && on_ground) position[1] -= 0.6;
            crouching = false;
        }

        if (on_ground) {
            jump_available = true;
            jump_time_left = jump_time;
        }
        else if (!intentions.count("up")) jump_available = false;

        if (intentions.count("up") && jump_available) {
            acceleration[1] += -170;
            jump_time_left -= dt_f;
            if (jump_time_left <= 0) jump_available = false;
        }

        bool breaking = (on_ground && !intentions.count("left") && !intentions.count("right"));
        std::array<double, 2> new_acceleration;
        if (breaking) {
            if (velocity[0] * velocity[0] > 10)
                new_acceleration = acceleration - velocity * length(velocity) * friction * 10;
            else
                new_acceleration = acceleration - velocity * length(velocity) * friction * 40;
        }
        else {
            new_acceleration = acceleration - velocity * length(velocity) * friction;
        }
        auto new_velocity = velocity + new_acceleration * dt_f;
        auto new_position = position + new_velocity * dt_f + new_acceleration * dt_f * dt_f * 0.5;
        position = new_position;
        velocity = new_velocity;
        if (velocity[0] < -max_hspeed) velocity[0] = -max_hspeed;
        if (velocity[0] > max_hspeed) velocity[0] = max_hspeed;
        if (breaking) {
            velocity = {(velocity[0] * velocity[0] > 2.5) ? velocity[0] : 0.0, 0};
        }
        acceleration = new_acceleration;
        intentions.clear();
        on_ground = false;
    }

//     bool is_safe_place()
//     {
//         return false;
//         return ((position[0] < 4.0) && (position[1] > 30) && (position[0] > 0.0) && (position[1] < 33));
//     }
};

class obstacle_c {
public:
    std::array<double, 2> position;
    std::array<double, 2> size;
    std::string texture;
};


class game_c
{
public:
    std::shared_ptr<SDL_Window> window_p;
    std::shared_ptr<SDL_Renderer> renderer_p;
    std::map<std::string, std::shared_ptr<SDL_Texture>> textures;
    std::vector<player_c> players;
    std::vector<bullet_c> bullets;
    std::vector<emitter_c> emitters;

    std::vector<obstacle_c> obstacles;


    std::chrono::milliseconds dt;

    std::vector<std::map<std::string, int>> keyboard_map;


};

#endif
