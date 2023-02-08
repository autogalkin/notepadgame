﻿#include "world.h"


#include "../core/engine.h"
#include <stdexcept>

#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>

#include "../core/gamelog.h"

backbuffer::backbuffer(backbuffer&& other) noexcept: line_lenght_(other.line_lenght_),
                                                     lines_count_(other.lines_count_),
                                                     buffer(std::move(other.buffer)),
                                                     world_(other.world_),
                                                     scroll(std::move(other.scroll))
{
}

backbuffer& backbuffer::operator=(backbuffer&& other) noexcept
{
    if (this == &other)
        return *this;
    line_lenght_ = other.line_lenght_;
    lines_count_ = other.lines_count_;
    buffer = std::move(other.buffer);
    world_ = other.world_;
    scroll = std::move(other.scroll);
    return *this;
}

char backbuffer::at(const location& char_on_screen) const
{
    return (*buffer)[char_on_screen.line()].pin()[char_on_screen.index_in_line];
}



char& backbuffer::at(const location& char_on_screen)
{
    (*buffer)[char_on_screen.line].mark_dirty();
    return (*buffer)[char_on_screen.line].pin()[char_on_screen.index_in_line];
}

backbuffer::~backbuffer() = default;

void backbuffer::init(const int window_width)
{
    const int char_width = world_->get_char_width();
    line_lenght_ = window_width / char_width;
    lines_count_ = world_->get_lines_on_screen();
    
    if(!buffer) buffer = std::make_unique<std::vector< dirty_flag< std::vector<char> > > >(lines_count_);
    else buffer->resize(lines_count_);
    for(auto& line  : *buffer)
    {
        line.pin().resize(line_lenght_+2, ' '); // 2 for \n and \0
        line.pin().back() = '\0';
    }

}

void backbuffer::send()
{
    const auto pos = world_->get_caret_index();
    bool buffer_is_changed = false;
    
    for(int enumerate{-1}; auto& line : *buffer // | ranges::views::enumerate
        | ranges::views::filter([this, &enumerate](auto& l){++enumerate; return l.is_changed() || scroll.is_changed() ;}))
    {
        const npi_t lnum = scroll.get().line + enumerate;
        const npi_t fi = world_->get_first_char_index_in_line(lnum);
        const npi_t ll = world_->get_line_lenght(lnum); // include endl if exists
        npi_t endsel = scroll.get().index_in_line + line_lenght_+endl > ll-endl ? ll : line_lenght_ + scroll.get().index_in_line+1;
   
        char ch_end {'\0'};
        if(lnum >= world_->get_lines_count()-1)    { ch_end = '\n';   }
        else                                       { endsel  -= endl; }
    
        *(line.pin().end() - 1/*past-the-end*/ - 1 /* '\0' */) = ch_end; 
        world_->set_selection(fi+scroll.get().index_in_line,  fi  + endsel);
    
        world_->replace_selection({line.get().begin(), line.get().end()});
        line.reset_flag();
        buffer_is_changed = true;  
        
    }
    
    if(buffer_is_changed)
    {
        world_->set_caret_index(pos);
        scroll.reset_flag();
    } 
}

void backbuffer::get() const
{
    /*
    const auto pos = world_->get_caret_index();
    const int64_t fl = world_->get_first_visible_line();
    const int64_t h_scroll = world_->get_horizontal_scroll_offset() / world_->get_char_width();
    
    for(const auto [i, linedata] : *buffer | boost::adaptors::indexed(0))
    {
        if(auto&  line = linedata; line.is_changed())
        {
            const int64_t lnum = fl + i;
            const int64_t fi = world_->get_first_char_index_in_line(lnum);
            
            world_->set_selection(fi+h_scroll,  fi  + line_lenght_ + h_scroll);
            world_->get_selection_text(line.pin());
            line.reset_flag();
        }
    }
    world_->set_caret_index(pos);
    */
}


level::~level() = default;

bool level::destroy_actor(const actor::tag_type tag)
{
    if(auto& pos = actors.at(tag)->get_pivot(); is_in_buffer(pos))
    {
        at(global_position_to_buffer_position(pos)) = shape::whitespace;
    }
    
    if(actors.erase(tag)) return true;
    return false;
}

bool level::set_actor_location(const actor::tag_type tag, const location new_position)
{
    // TODO bug collision does not work!!!
    const auto opt = find_actor(tag);
    if(!opt)
    {
        throw std::logic_error{"attempt to access to a not existing actor, the actor::tag_type is invalid"};
    }
    actor* act = *opt;
    uint8_t move_block_flag = 0;
    
    //for(const std::vector<actor::collision_response> responces = collision_.get_query()(act->get_id(), act->get_pivot());
       // auto& res : responces)
    {
        
       // actor* other = actors.at(res).get();
       // act->on_hit(other);
        //if(other && other->on_hit(act))
        //{
        //    return false ;
        //}
    }
    
    if(const auto bpos =global_position_to_buffer_position(new_position);
        is_in_buffer(bpos))
    {
        at(bpos) = act->getmesh();
    }
    if(const auto b_old = global_position_to_buffer_position(act->get_pivot());
        is_in_buffer(b_old))
    {
        at(b_old) = shape::whitespace;
    }
    act->set_position(new_position);
        
    return true;
        
}