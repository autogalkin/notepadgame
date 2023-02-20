﻿#include "world.h"


#include "notepader.h"
#include "../core/engine.h"
#include "../../extern/range-v3-0.12.0/include/range/v3/view/enumerate.hpp"
#include "../../extern/range-v3-0.12.0/include/range/v3/view/filter.hpp"

backbuffer::backbuffer(backbuffer&& other) noexcept: line_lenght_(other.line_lenght_),
                                                     lines_count_(other.lines_count_),
                                                     buffer(std::move(other.buffer)),
                                                     engine_(other.engine_),
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
    engine_ = other.engine_;
    scroll = std::move(other.scroll);
    return *this;
}

char backbuffer::at(const position& char_on_screen) const
{
    return (*buffer)[char_on_screen.line()].pin()[char_on_screen.index_in_line()];
}

void backbuffer::redraw(const position& src, const position& dest, const shape& sh)
{
    for(auto rows = sh.data.rowwise();
        auto [line, row] : rows | ranges::views::enumerate)
    {
        for(int ind_in_line{-1}; const auto ch : row
            | ranges::views::filter([&ind_in_line](const char c){++ind_in_line; return c != shape::whitespace;}))
        {
            position d = dest + position{static_cast<npi_t>(line), ind_in_line};
            position s = src + position{static_cast<npi_t>(line), ind_in_line};
            at(d) = ch;
            if(d != s) at(s) = shape::whitespace;
        }
    }
}

void backbuffer::erase(const position& src, const shape& sh)
{
    for(auto rows = sh.data.rowwise();
        auto [line, row] : rows | ranges::views::enumerate)
    {
        for(int ind_in_line{-1}; const auto ch : row
            | ranges::views::filter([&ind_in_line](const char c){++ind_in_line; return c != shape::whitespace;}))
        {
            position s = src + position{static_cast<npi_t>(line), ind_in_line};
            at(s) = shape::whitespace;
        }
    }
}


char& backbuffer::at(const position& char_on_screen)
{
    (*buffer)[char_on_screen.line()].mark_dirty();
    return (*buffer)[char_on_screen.line()].pin()[char_on_screen.index_in_line()];
}

backbuffer::~backbuffer() = default;

void backbuffer::init(const int window_width)
{
    const int char_width = engine_->get_char_width();
    line_lenght_ = window_width / char_width;
    lines_count_ = engine_->get_lines_on_screen();
    
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
    const auto pos = engine_->get_caret_index();
    bool buffer_is_changed = false;
    
    for(int enumerate{-1}; auto& line : *buffer // | ranges::views::enumerate
        | ranges::views::filter([this, &enumerate](auto& l){++enumerate; return l.is_changed() || scroll.is_changed() ;}))
    {
        const npi_t lnum = scroll.get().line() + enumerate;
        const npi_t fi = engine_->get_first_char_index_in_line(lnum);
        const npi_t ll = engine_->get_line_lenght(lnum); // include endl if exists
        npi_t endsel = scroll.get().index_in_line() + line_lenght_ + endl > ll - endl ? ll : line_lenght_ + scroll.get().index_in_line() + 1;
   
        char ch_end {'\0'};
        if(lnum >= engine_->get_lines_count()-1)    { ch_end = '\n';   }
        else                                       { endsel  -= endl; }
    
        *(line.pin().end() - 1/*past-the-end*/ - 1 /* '\0' */) = ch_end; 
        engine_->set_selection(fi+scroll.get().index_in_line(), fi + endsel);
    
        engine_->replace_selection({line.get().begin(), line.get().end()});
        line.reset_flag();
        buffer_is_changed = true;  
        
    }
    
    if(buffer_is_changed)
    {
        engine_->set_caret_index(pos);
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



world::~world() = default;

