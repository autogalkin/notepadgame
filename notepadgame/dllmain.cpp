
#include "core/gamelog.h"
#include "core/notepader.h"
#include "core/engine.h"

#include "entities/atmosphere.h"
#include "core/base_types.h"


#include <Windows.h>

#include "ecs_processors/movement.h"
#include "core/world.h"
#include "entities/factories.h"


BOOL APIENTRY DllMain(const HMODULE h_module, const DWORD  ul_reason_for_call, LPVOID lp_reserved)
{
    //  the h_module is notepadgame.dll and the np_module is notepad.exe
    
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        // Ignore thread notifications
        DisableThreadLibraryCalls(h_module);

        gamelog::get(); // alloc a console for the cout
        
        // run the notepad singleton
        notepader& np = notepader::get();
        const auto np_module = GetModuleHandleW(nullptr); // get the module handle of the notepad.exe
        
        [[maybe_unused]] constexpr uint8_t start_options =
            0
            //  notepader::hide_selection
            //| notepader::kill_focus
            //| notepader::disable_mouse
            | notepader::show_eol
            | notepader::show_spaces;
        
        np.connect_to_notepad(np_module, start_options); // start initialization and a game loop
        
        np.get_on_open().connect([]
        {
            
            const auto& w = notepader::get().get_engine()->get_world();
            auto& exec =  w->get_ecs_executor();
            
            exec.emplace_back(std::make_unique<input_passer>(w.get(), notepader::get().get_input_manager().get()));
            exec.emplace_back(std::make_unique<collision::query>(w.get(), w->get_registry()));
            exec.emplace_back(std::make_unique<movement>(w.get()));
            
            w->spawn_actor([](entt::registry& reg, const entt::entity entity)
            {
                reg.emplace<shape>(entity, shape::initializer_from_data{"f", 1, 1});
                character::make(reg, entity, location{3, 3});
                reg.emplace<input_passer::down_signal>(entity, &character::process_input<>);
                
            });
            
            w->spawn_actor([](entt::registry& reg, const entt::entity entity)
            {
                reg.emplace<shape>(entity, shape::initializer_from_data{"d", 1, 1});
                character::make(reg, entity, location{5, 8});
                reg.emplace<input_passer::down_signal>(entity,
                    &character::process_input<input::key::up
                                            , input::key::left
                                            , input::key::down
                                            , input::key::right>);
                
            });
            
        });
    }
    return TRUE;
}
