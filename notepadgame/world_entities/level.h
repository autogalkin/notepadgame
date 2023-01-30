﻿#pragma once

#include <memory>
#include <vector>
#include "actor.h"
#include "character.h"




class gamelog;

template<typename T>
struct dirty_flag
{
    [[nodiscard]] const T& get() const noexcept {return value_.second;}
    [[nodiscard]] T& pin()  noexcept
    {
        mark_dirty();
        return value_.second;
    }
    void mark_dirty() noexcept { value_.first = true;} 
    void reset_flag() noexcept {value_.first = false;}
    [[nodiscard]] bool is_changed() const noexcept {return value_.first;}
private:
    std::pair<bool, T> value_;
};

class world;

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

    explicit backbuffer(world* owner): world_(owner){}
    virtual ~backbuffer() = default;
    
    void init(const int window_width);
    
    // tick operations
    void send();
    void get() const;
    
    [[nodiscard]] virtual char& at(translation char_on_screen);
    [[nodiscard]] virtual char at(translation char_on_screen) const;
    [[nodiscard]] int get_line_size() const noexcept { return line_lenght_;}
    [[nodiscard]] const translation& get_scroll() const noexcept {return scroll.get();}
    
    [[nodiscard]] translation global_position_to_buffer_position(const translation& position) const noexcept
    {
        return {position.line() - get_scroll().line(), position.index_in_line() - get_scroll().index_in_line() };
    }
    
    [[nodiscard]] bool is_in_buffer(const translation& position) const noexcept
    {
        return position.line() > get_scroll().line() || position.index_in_line() > get_scroll().index_in_line();
    }
    
private:
    static constexpr int endl = 1;
    int line_lenght_{0};
    int lines_count_{0};
    std::unique_ptr< std::vector< dirty_flag< std::vector<char> > > > buffer;
    world* world_;
    
    dirty_flag<translation> scroll;
};


class collision
{
public: 
    
    struct query_combiner
    {
        typedef std::vector<actor::collision_response> result_type;
        template <typename InputIterator>
        std::vector<actor::collision_response> operator()(InputIterator first, InputIterator last) const
        {
            
            std::vector<actor::collision_response> v;
            while (first != last)
            {
                if((*first).has_value())
                {
                    v.push_back((*first).value());
                }
                ++first;
            }
            return v;
        }
    };
    
    using signal_type = boost::signals2::signal< std::optional<actor::collision_response> (const actor::tag_type asker, const translation& position), query_combiner>;
    const std::unique_ptr<signal_type>& get_query() const {return query;}
    
private:
    std::unique_ptr<signal_type> query = std::make_unique<signal_type>();
    
};


class level final : public backbuffer
{
    
public:
    explicit level(world* owner) noexcept
       : backbuffer(owner)
    {
    }
    
    std::optional<actor*> find_actor(const actor::tag_type tag)
    {
        const auto t = actors.find(tag);
        if (t == actors.end()) return std::nullopt;
        return t->second.get();
    }
    
    // the factory method for create a new actor
    template <std::derived_from<actor> T, typename ...Args>
    requires requires(Args&& ... args){ T(spawner{}, std::forward<Args>(args)...);}
    T* spawn_actor(const translation& spawn_location, Args&&... args)
    {
        std::unique_ptr<actor> spawned =std::make_unique<T>(spawner{}, std::forward<Args>(args)...);
        spawned->set_position(spawn_location);
        spawned->set_level(this);
        auto [It, success] = actors.emplace(std::make_pair(spawned->get_id(), std::move( spawned )));
        if(success){}
        //{
        //}  // TODO  exeption if fail
        return static_cast<T*>(It->second.get());
    }
    
    void destroy_actor(const actor::tag_type tag);
    bool set_actor_location(const actor::tag_type tag, const translation new_position);
    collision& get_collision() noexcept {return collision_;}

protected:
private:
    std::unordered_map< actor::tag_type, std::unique_ptr<actor>, actor::hasher > actors{};
    collision collision_{};

};



