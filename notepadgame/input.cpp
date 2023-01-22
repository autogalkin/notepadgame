﻿#include "input.h"



void input::receive_input()
{
    // wParam is a keycode VK_...
    // The repeat count, scan code, extended-key flag, context code, https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-keydown
    if(0ul == curr_input_code_) return;

    switch (static_cast<key>(curr_input_code_))
    {
    case key::w: signals_.on_w(curr_direction_); break;
    case key::a: signals_.on_a(curr_direction_); break;
    case key::s: signals_.on_s(curr_direction_); break;
    case key::d: signals_.on_d(curr_direction_); break;
    }
    curr_input_code_ = 0;
    curr_direction_ = direction::undefined;
}

void input::set_input_message(const LPMSG msg)
{
    switch (msg->message)
    {
    case WM_KEYDOWN: 
    case WM_SYSKEYDOWN: curr_direction_ = direction::down; break;
    case WM_KEYUP:   
    case WM_SYSKEYUP:curr_direction_ = direction::up; break;
    default: return;
    }
    curr_input_code_ = msg->wParam;

}
