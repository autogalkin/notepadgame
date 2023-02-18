﻿#pragma once

#include <queue>

#include "boost/signals2.hpp"
#include "base_types.h"
#include "tick.h"
#include <../../extern/entt-3.11.1/single_include/entt/entt.hpp>

#define NOMINMAX
#include "gamelog.h"
#include "Windows.h"
#undef NOMINMAX


class input final : public tickable
{
public:
    
    // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    enum class key : WPARAM {
          w = 0x57
        , a = 0x41
        , s = 0x53
        , d = 0x44
        , space = VK_SPACE
        , up = VK_UP
        , right = VK_RIGHT
        , left = VK_LEFT
        , down = VK_DOWN
    };
    
    using key_state_type = std::vector<input::key>;
    using signal_type = boost::signals2::signal<void (const key_state_type& key_state)>;
    
    explicit input() = default;
    ~input() override;

    input(input& other) noexcept = delete;
    input(input&& other) noexcept = delete;
    input& operator=(const input& other) = delete;
    input& operator=(input&& other) noexcept = delete;
    
    virtual void tick() override;
    void send_input();
    void receive(const LPMSG msg);
    [[nodiscard]] const key_state_type& get_down_key_state() const { return down_key_state_;}
    [[nodiscard]] input::signal_type& get_down_signal() {return on_down; }
    void clear_input()
    {
        down_key_state_.clear();
    }
private:
    
    signal_type on_down;
    signal_type on_up;
    key_state_type down_key_state_;
    key_state_type up_key_state_;

};

class input_passer final : public ecs_processor
{
public:
    explicit input_passer(world* w, input* i): ecs_processor(w), input_(i){}
    struct down_signal{
        std::function<void(entt::registry&, entt::entity, const input::key_state_type&)> callback{};
    };
    struct up_signal : down_signal {};
    
    void execute(entt::registry& reg) override
    {
        const input::key_state_type& state = input_->get_down_key_state();
        if(state.empty()) return;
        
        for(const auto view = reg.view<const input_passer::down_signal>();
            const auto entity: view){
            
            view.get<input_passer::down_signal>(entity).callback(reg, entity, state);
            //view.get<input_passer::up_signal>(entity).callback(reg, entity, state);
            }
    }
    input* input_{nullptr};
};

