#include "bmpfont.hpp"
#include "vectors.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <vector>
#include <algorithm>

#include "main.hpp"

std::ostream& operator<<(std::ostream& o, const std::array<double, 2>& a)
{
    o << "[" << a[0] << "," << a[1] << "]";
    return o;
}

void draw_o(std::shared_ptr<SDL_Renderer> r, std::array<double, 2> p, std::shared_ptr<SDL_Texture> tex, double w, double h, double a, bool flip=false)
{
    SDL_Rect dst_rect = {(int)(p[0] - w / 2), (int)(p[1] - h / 2), (int)w, (int)h};
    SDL_RendererFlip sdl_flip = (flip) ? SDL_RendererFlip::SDL_FLIP_HORIZONTAL : SDL_RendererFlip::SDL_FLIP_NONE;
    SDL_RenderCopyEx(r.get(), tex.get(), NULL, &dst_rect, a, NULL, sdl_flip);
}

void draw_obstacle(std::shared_ptr<SDL_Renderer> r, std::array<double, 2> p, std::shared_ptr<SDL_Texture> tex, double w, double h, double a)
{
    SDL_Rect dst_rect = {(int)p[0], (int)p[1], (int)w, (int)h};
    SDL_RenderCopyEx(r.get(), tex.get(), NULL, &dst_rect, a, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
}

void initialize_players(game_c &game)
{
    game.players.push_back(player_c({4, 30}));
    //game.players.push_back(player_c({1.5, 10}));

    // player keyboard mapping
    // player 0
    game.keyboard_map.push_back({{"right", SDL_SCANCODE_RIGHT},
        {"left", SDL_SCANCODE_LEFT},
        {"gun_up", SDL_SCANCODE_UP},
        {"gun_down", SDL_SCANCODE_DOWN},
        {"up", SDL_SCANCODE_SPACE},
        {"shoot", SDL_SCANCODE_LSHIFT},
        {"down", SDL_SCANCODE_LCTRL}
    });
    // player 1
//     game.keyboard_map.push_back({{"right", SDL_SCANCODE_D},
//         {"left", SDL_SCANCODE_A},
//         {"up", SDL_SCANCODE_W},
//         {"down", SDL_SCANCODE_S}});
}

void gen_obstacle_rect(game_c &game, double x, double y, int width, int height, std::string texture)
{
    for (int i=0; i<width; i++) {
        for (int j=0; j<height; j++) {
            obstacle_c o;
            o.position = {x+i, y+j};
            o.size = {1,1};
            o.texture = texture;
            game.obstacles.push_back(o);
        }
    }
}

void initialize_obstacles(game_c &game)
{
    // outer walls
    gen_obstacle_rect(game, -1, -1, 64, 1, "block1");
    gen_obstacle_rect(game, -1, 0, 1, 36, "block1");
    //gen_obstacle_rect(game, 64, 0, 1, 36, "block1");

    //ground
    gen_obstacle_rect(game, 0, 33, 64, 3, "block1");


    // floor 1
    gen_obstacle_rect(game, 0, 23, 50, 2, "block1");
    gen_obstacle_rect(game, 55, 23, 9, 2, "block1");

    gen_obstacle_rect(game, 52, 28, 3, 1, "block1");

    // floor 2
    gen_obstacle_rect(game, 0, 13, 9, 2, "block1");
    gen_obstacle_rect(game, 14, 13, 50, 2, "block1");

    gen_obstacle_rect(game, 9, 18, 3, 1, "block1");


    gen_obstacle_rect(game, 10, 29, 5, 1, "block1");
    gen_obstacle_rect(game, 15, 30, 5, 1, "block1");

    gen_obstacle_rect(game, 20, 29, 1, 4, "block1");
}

void initialize_emitters(game_c &game)
{

    emitter_c emitter1;
    emitter1.position = {-5, 20};
    emitter1.friction = 0;
    emitter1.acceleration = {0, 0};
    emitter1.velocity = {0, 0};
    emitter1.emit_delay = 1;
    emitter1.emit_to_emit = 0;
    game.emitters.push_back(emitter1);

    emitter_c emitter2;
    emitter2.position = {65, 31};
    emitter2.friction = 0;
    emitter2.acceleration = {0, 0};
    emitter2.velocity = {0, 0};
    emitter2.emit_delay = 1;
    emitter2.emit_to_emit = 0;
    game.emitters.push_back(emitter2);

}

game_c initialize_all()
{
    game_c game;
    /// SDL
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);

    /// WINDOW
    game.window_p = std::shared_ptr<SDL_Window>(
        SDL_CreateWindow("GOTY", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE),
        [](auto* window) { SDL_DestroyWindow(window); });

    game.renderer_p = std::shared_ptr<SDL_Renderer>(
        SDL_CreateRenderer(game.window_p.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
        [](auto* renderer) {
            SDL_DestroyRenderer(renderer);
        });

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_RenderSetLogicalSize(game.renderer_p.get(), 640, 360);

    /// MEDIA
    for (int i = 0; i < 3; i++) {
        game.textures["player[" + std::to_string(i) + "]"] = std::shared_ptr<SDL_Texture>(IMG_LoadTexture(game.renderer_p.get(), (std::string("data/player") + std::to_string(i) + ".png").c_str()),
            [](auto* tex) { SDL_DestroyTexture(tex); });
    }
    for (int i = 0; i < 3; i++) {
        game.textures["bullet[" + std::to_string(i) + "]"] = std::shared_ptr<SDL_Texture>(IMG_LoadTexture(game.renderer_p.get(), (std::string("data/bullet") + std::to_string(i) + ".png").c_str()),
            [](auto* tex) { SDL_DestroyTexture(tex); });
    }
    for (int i = 0; i < 1; i++) {
        game.textures["emitter[" + std::to_string(i) + "]"] = std::shared_ptr<SDL_Texture>(IMG_LoadTexture(game.renderer_p.get(), (std::string("data/emitter") + std::to_string(i) + ".png").c_str()),
            [](auto* tex) { SDL_DestroyTexture(tex); });
    }
    game.textures["font_10"] = std::shared_ptr<SDL_Texture>(IMG_LoadTexture(game.renderer_p.get(), "data/oqls65n.png"),
        [](auto* tex) { SDL_DestroyTexture(tex); });

    game.textures["font_10_red"] = std::shared_ptr<SDL_Texture>(IMG_LoadTexture(game.renderer_p.get(), "data/oqls65n_red.png"),
        [](auto* tex) { SDL_DestroyTexture(tex); });

    game.textures["font_10_blue"] = std::shared_ptr<SDL_Texture>(IMG_LoadTexture(game.renderer_p.get(), "data/oqls65n_blue.png"),
        [](auto* tex) { SDL_DestroyTexture(tex); });

    game.textures["block1"] = std::shared_ptr<SDL_Texture>(IMG_LoadTexture(game.renderer_p.get(), "data/block1.png"),
        [](auto* tex) { SDL_DestroyTexture(tex); });

    game.textures["gun"] = std::shared_ptr<SDL_Texture>(IMG_LoadTexture(game.renderer_p.get(), "data/gun.png"),
        [](auto* tex) { SDL_DestroyTexture(tex); });

    game.textures["guy"] = std::shared_ptr<SDL_Texture>(IMG_LoadTexture(game.renderer_p.get(), "data/guy.png"),
        [](auto* tex) { SDL_DestroyTexture(tex); });


    /// PLAYERS
    initialize_players(game);

    /// OBSTACLES
    initialize_obstacles(game);

    /// EMITTERS
    initialize_emitters(game);

    /// physics details
    game.dt = std::chrono::milliseconds(15);

    return game;
}


int process_input(game_c& game)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            return false;
    }
    auto kbdstate = SDL_GetKeyboardState(NULL);
    if (kbdstate[SDL_SCANCODE_ESCAPE]) return false;
    if (kbdstate[SDL_SCANCODE_Q]) {
        game.players[0].position = {4, 10};
    }
    if (kbdstate[SDL_SCANCODE_R]) {
        game.players[0].position = {4, 30};
    }
    for (unsigned i = 0; i < game.players.size(); i++) {
        for (auto [k, v] : game.keyboard_map.at(i)) {
            if (kbdstate[v]) game.players.at(i).intentions[k] = 1;
        }
    }
    return true;
}

void process_events(game_c& game)
{
    using namespace tp::operators;
    double dt_f = game.dt.count() / 1000.0;
    /// apply safe place and hit points
//     for (unsigned i = 0; i < game.players.size(); i++) {
//         auto& p = game.players[i];
//         if ((p.is_safe_place())) {
//             p.health += dt_f * 10.0;
//             if (p.health > 100.0) p.health = 100;
//         } else {
//             p.points += dt_f * 10.0;
//         }
//     }

    // EMITTERS
    for (unsigned i = 0; i < game.emitters.size(); i++) {
        auto& e = game.emitters[i];
        e.emit_to_emit -= dt_f;

        if (i == 0) {
            if (e.emit_to_emit <= 0.0) {
                e.emit_to_emit = e.emit_delay;
                //game.bullets.push_back(bullet_c({e.position, {0, 0}, {0.0, 0.0},0.1,"bullet[0]","a"}));
                bullet_c bullet;
                bullet.position = e.position;
                bullet.velocity = {0, 0};
                bullet.acceleration = {0.0, 0.0};
                bullet.friction = 0.1;
                bullet.type = "bullet[1]";
                bullet.behavior = "a";
                bullet.damages_player = true;
                game.bullets.push_back(bullet);
            }
        }
        else if (i == 1) {
            if (e.emit_to_emit <= 0.0) {
                e.emit_to_emit = e.emit_delay;
                //game.bullets.push_back(bullet_c({e.position, {0, 0}, {0.0, 0.0},0.1,"bullet[0]","a"}));
                bullet_c bullet;
                bullet.position = e.position;
                bullet.velocity = {0, 0};
                bullet.acceleration = {0.0, 0.0};
                bullet.friction = 0.0;
                bullet.type = "bullet[1]";
                bullet.behavior = "c";
                bullet.damages_player = true;
                bullet.blocked_by_obstacles = true;
                bullet.destroyed_on_contact = true;
                game.bullets.push_back(bullet);
            }
        }

    }

    // PLAYER SHOOTING
    if (game.players[0].intentions.count("shoot")) {
        double angle = game.players[0].gun_angle + 90;
        if (game.players[0].last_move_left) {
            angle = -angle;
        }
        double rad = angle * M_PI / 180;
        std::array<double, 2> acceleration = { sin(rad) * 30, cos(rad) * 30 };

        bullet_c bullet;
        rad = -angle * M_PI / 180;
        bullet.position = {game.players[0].position[0] - sin(rad) * 2.0, game.players[0].position[1] - 0.2 + cos(rad) * 2.0};
        bullet.velocity = acceleration;
        bullet.velocity[0] += game.players[0].velocity[0];
        bullet.velocity[1] += game.players[0].velocity[1];
        bullet.acceleration = {0, 0};
        bullet.friction = 0.0;
        bullet.type = "bullet[0]";
        bullet.behavior = "b";
        bullet.blocked_by_obstacles = true;
        bullet.destroyed_on_contact = false;
        game.bullets.push_back(bullet);
    }

    // BULLETS WHICH DAMAGE PLAYER
    std::vector<bullet_c> new_bullets;
    for (unsigned i = 0; i < game.players.size(); i++) {
//         if (!game.players[i].is_safe_place())
        for (unsigned j = 0; j < game.bullets.size(); j++) {
            if (game.bullets[j].damages_player && length(game.players[i].position - game.bullets[j].position) < 1.3) {
                game.players[i].health -= 10;
                if (game.players[i].health <= 0) {
                    game.players[i].position = {4, 30};
                    game.players[i].health = 100;
                }
                if (game.bullets[j].behavior == "c") {
                    game.players[i].velocity[0] = -80;
                }
            } else {
                new_bullets.push_back(game.bullets[j]);
            }
        }
    }
    std::swap(new_bullets,game.bullets);
}

void process_physics(game_c& game)
{
    using namespace tp::operators;
    double dt_f = game.dt.count() / 1000.0;


    auto old_players = game.players;
    // update moves
    for (auto& player : game.players) {
        player.update(dt_f);
    }


    // update bullets
    auto old_bullets = game.bullets;
    std::vector<bullet_c> bullets_new;
    for (unsigned i = 0; i < game.bullets.size(); i++) {
        auto &bullet = game.bullets[i];
        bullet.update(dt_f);

        if (bullet.expired) continue;

        if (!((bullet.position[0] > -10.0) && (bullet.position[0] < 74) && (bullet.position[0] > -1000.0) && (bullet.position[1] < 74))) {
            continue;
        }

        bool ok = true;
        if (bullet.blocked_by_obstacles) {
            for (auto &o : game.obstacles) {
                double ss = 0.4;
                if (!(((bullet.position[0]+ss) < o.position[0]) ||
                    ((bullet.position[0]-ss) > (o.position[0]+o.size[0])) ||
                    ((bullet.position[1]+ss) < o.position[1]) ||
                    ((bullet.position[1]-ss) > (o.position[1]+o.size[1])))) {
                     if (bullet.destroyed_on_contact) {
                         ok = false;
                     }
                     bool contact_left = ((old_bullets[i].position[0]+ss) < o.position[0]);
                     bool contact_right = ((old_bullets[i].position[0]-ss) > (o.position[0]+o.size[0]));
                     bool contact_top = ((old_bullets[i].position[1]+ss) < o.position[1]);
                     bool contact_bottom = ((old_bullets[i].position[1]-ss) > (o.position[1]+o.size[1]));
                     if ((contact_left || contact_right) && (!contact_top && !contact_bottom)) {
                         bullet.position[0] = old_bullets[i].position[0];
                         bullet.velocity[0] = 0;
                     }
                     if ((contact_top || contact_bottom) && (!contact_left && !contact_right)) {
                         bullet.position[1] = old_bullets[i].position[1];
                         bullet.velocity[1] = 0;
                         bullet.velocity[0] *= 0.97;
                     }
                }
            }
        }
        if (ok) bullets_new.push_back(bullet);
    }
    std::swap(bullets_new, game.bullets);
    
    // COLLISIONS BETWEEN PLAYERS
    for (unsigned i = 0; i < game.players.size(); i++) {
        for (unsigned j = i + 1; j < game.players.size(); j++) {
            if (length(game.players[i].position - game.players[j].position) < 1.0) {
                game.players[i].position = old_players[i].position;
                game.players[j].position = old_players[j].position;
                auto vec = game.players[i].position - game.players[j].position;
                vec = vec * (1.0 / length(vec));
                game.players[i].velocity = vec; //old_players[i].position;
                game.players[j].velocity = vec * -1.0;
            }
        }
    }

    // PLAYER COLLISIONS WITH OBSTACLES
    for (unsigned i = 0; i < game.players.size(); i++) {
        auto &p = game.players[i];
        double halfheight = 0;
        if (p.crouching)
            halfheight = 0.7;
        else
            halfheight = 1.2;
//         bool contact_top1 = false;
//         bool contact_bottom1 = false;
        for (auto &o : game.obstacles) {
            if (!(((p.position[0]+0.7) < o.position[0]) ||
                ((p.position[0]-0.7) > (o.position[0]+o.size[0])) ||
                ((p.position[1]+halfheight) < o.position[1]) ||
                ((p.position[1]-halfheight) > (o.position[1]+o.size[1])))) {
                bool contact_left = ((old_players[i].position[0]+0.7) < o.position[0]);
                bool contact_right = ((old_players[i].position[0]-0.7) > (o.position[0]+o.size[0]));
                bool contact_top = ((old_players[i].position[1]+halfheight) < o.position[1]);
                bool contact_bottom = ((old_players[i].position[1]-halfheight) > (o.position[1]+o.size[1]));
//                 if (contact_top) {
//                     contact_top1 = true;
//                 }
//                 if (contact_bottom) contact_bottom1 = true;
//                 if (contact_top1 && contact_bottom1) {
//                     game.players[i].intentions["must_crouch"] = 1;
//                 }
                if ((contact_left || contact_right) && (!contact_top && !contact_bottom)) {
                    p.position[0] = old_players[i].position[0];
                    p.velocity[0] = 0;
                }
                if ((contact_top || contact_bottom) && (!contact_left && !contact_right)) {
                    p.position[1] = old_players[i].position[1];
                    p.velocity[1] = 0;
                    //game.players[i].velocity = {(game.players[i].velocity[0] * game.players[i].velocity[0] > 2.5) ? game.players[i].velocity[0] : 0.0, 0};
                    if (contact_top) {
                        game.players[i].on_ground = true;
                    }

                }

            }
        }
    }



}

/// player size i 10 x 10
void draw_scene(game_c& game)
{
    using namespace tp::operators;
//135, 206, 235  	135, 156, 235  	16, 34, 99
    SDL_SetRenderDrawColor(game.renderer_p.get(), 16, 34, 99, 255);
    SDL_RenderClear(game.renderer_p.get());
    SDL_SetRenderDrawColor(game.renderer_p.get(), 255, 100, 200, 255);

    // DRAW OBSTACLES
    for (auto &o: game.obstacles) {
        draw_obstacle(game.renderer_p, o.position * 10.0, game.textures.at(o.texture), o.size[0]*10, o.size[1]*10, 0);
    }

    // DRAW ALL EMITTERS
//     for (unsigned i = 0; i < game.emitters.size(); i++) {
//         auto& emitter = game.emitters[i];
//         draw_o(game.renderer_p, emitter.position * 10.0, game.textures.at("emitter[0]"), 16, 16, 0.0);
//     }
    // DRAW ALL BULLETS
    for (unsigned i = 0; i < game.bullets.size(); i++) {
        auto& bullet = game.bullets[i];
        draw_o(game.renderer_p, bullet.position * 10.0, game.textures.at(bullet.type), 8, 8, 0);
    }

    // DRAW PLAYER
    for (unsigned i = 0; i < game.players.size(); i++) {
        auto& player = game.players[i];
        int height = 16;
        if (!player.crouching) height += 10;
        draw_o(game.renderer_p, player.position * 10.0, game.textures.at("guy"), 16, height, 0);

        if (player.last_move_left) {
            draw_o(game.renderer_p, player.position * 10.0, game.textures.at("gun"), 50, 50, player.gun_angle, true);
        }
        else {
            draw_o(game.renderer_p, player.position * 10.0, game.textures.at("gun"), 50, 50, -player.gun_angle, false);
        }

//         if (player.is_safe_place())
//             draw_o(game.renderer_p, player.position * 10.0, game.textures.at("player[" + std::to_string(i) + "]"), 16 + 4, 16 + 4, player.position[0] * 36 + player.position[1] * 5);

        tp::draw_text(game.renderer_p, 10 + i * 130, 10, game.textures["font_10_red"], std::to_string((int)player.health));
        //tp::draw_text(game.renderer_p, 10 + i * 130 + 40, 340, game.textures["font_10_blue"], std::to_string((int)player.points));
    }

    SDL_RenderPresent(game.renderer_p.get());
}


int main(int, char**)
{
    using namespace std;
    using namespace std::chrono;

    auto game = initialize_all();
    steady_clock::time_point current_time = steady_clock::now(); // remember current time
    for (bool game_active = true; game_active;) {
        game_active = process_input(game);
        process_events(game);
        process_physics(game);
        draw_scene(game);

        this_thread::sleep_until(current_time = current_time + game.dt);
    }
    SDL_Quit();
    return 0;
}
