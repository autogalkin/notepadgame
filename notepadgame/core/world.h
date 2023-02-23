﻿#pragma once

#include <memory>
#include <vector>




#include "../core/base_types.h"

#include "../ecs_processors/collision.h"
#include "tick.h"




class engine;

// the array of chars on the screen
class backbuffer 
{
    friend class world;
    friend class notepader; // for update the scroll
public:
    backbuffer(const backbuffer& other) = delete;
    backbuffer(backbuffer&& other) noexcept;
    backbuffer& operator=(const backbuffer& other) = delete;
    backbuffer& operator=(backbuffer&& other) noexcept;

    explicit backbuffer(engine* owner): engine_(owner){}
    virtual ~backbuffer();
    
    void init(const int window_width);
    
    // tick operations
    void send();
    void get() const;
    
    [[nodiscard]] virtual char& at(const position& char_on_screen);
    [[nodiscard]] virtual char at(const position& char_on_screen) const;
    [[nodiscard]] int get_line_size() const noexcept { return line_lenght_;}
    [[nodiscard]] const position& get_scroll() const noexcept {return scroll.get();}
    
    [[nodiscard]] position global_position_to_buffer_position(const position& position) const noexcept{
        return {position.line() - get_scroll().line(), position.index_in_line() - get_scroll().index_in_line()};
    }
    
    [[nodiscard]] bool is_in_buffer(const position& position) const noexcept{
        return position.line() > get_scroll().line() || position.index_in_line() > get_scroll().index_in_line();
    }
    void draw(const position& pivot, const shape::sprite& sh);
    void erase(const position& pos, const shape::sprite& sh);
private:
    static constexpr int endl = 1;
    int line_lenght_{0};
    int lines_count_{0};
    using line_type = dirty_flag< std::vector<char> >;
    std::unique_ptr< std::vector< line_type > > buffer{};
    engine* engine_;
    
    dirty_flag<position> scroll{};
};

class ecs_processors_executor
{
    enum class insert_order : int8_t{
        before = 0
      , after  = 1
    };
    
public:
    explicit ecs_processors_executor(world* w): w_(w){}
    bool insert_processor_at(std::unique_ptr<ecs_processor> who
        , const std::type_info& near_with /* typeid(ecs_processor) */
        , const insert_order where=insert_order::before)
    {
        if(auto it = std::ranges::find_if(data_
                                          , [&near_with]( const std::unique_ptr<ecs_processor>& n)
                                          {
                                              return  near_with == typeid(*n);
                                          });
        it != data_.end())
        {
            switch (where)
            {
            case insert_order::before:        break;
            case insert_order::after:  ++it ; break;
            }
            data_.insert(it, std::move(who));
            return true;
        }
        return false;
    }
    void execute(entt::registry& reg, const gametime::duration delta) const
    {
        for(const auto& i : data_){
            i->execute(reg, delta);
        }
    }
    
    template<typename T, typename ...Args>
    requires std::derived_from<T, ecs_processor> && std::is_constructible_v<T, world*, Args...>
    void push(Args&&... args){
        data_.emplace_back(std::make_unique<T>(w_, std::forward<Args>(args)...));
    }
private:
   std::vector< std::unique_ptr<ecs_processor>> data_;
    world* w_;
};


class world final : public backbuffer, public tickable  // NOLINT(cppcoreguidelines-special-member-functions)
{
    
public:
    // TODO запретить перемещение и копирование
    
    explicit world(engine* owner) noexcept: backbuffer(owner)
    {
        //scroll_changed_connection = owner->get_on_scroll_changed().connect([this](const location& new_scroll){scroll = new_scroll;});
        // declare dependencies
        //reg_.on_construct<velocity>().connect<&entt::registry::emplace<movement_force>>();
    }

    ~world() override;

    void tick(gametime::duration delta) override{
        executor_.execute(reg_, delta);
    }
    ecs_processors_executor& get_ecs_executor() {return executor_;}
    entt::registry& get_registry() {return reg_;}
    void spawn_actor(const std::function<void(entt::registry&, const entt::entity)>& for_add_components)
    {
        const auto entity = reg_.create();
        for_add_components(reg_, entity);
    }
private:
    
    boost::signals2::scoped_connection scroll_changed_connection;
    ecs_processors_executor executor_{this};
    entt::registry reg_;
    
};



