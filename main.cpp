#include <nodepp/nodepp.h>

using namespace nodepp;

#include "assets/script/game.cpp"
#include "assets/script/server.cpp"
#include "assets/scenes/scene_0.cpp"
#include "assets/scenes/scene_1.cpp"

void onMain() {
    
    rl::SetConfigFlags( rl::FLAG_WINDOW_MOUSE_PASSTHROUGH |
                        rl::FLAG_WINDOW_UNDECORATED       |
                        rl::FLAG_WINDOW_TRANSPARENT       |
                        rl::FLAG_WINDOW_UNFOCUSED         | 
                        rl::FLAG_FULLSCREEN_MODE          |
                        rl::FLAG_WINDOW_TOPMOST       
    );
    
    /*─······································································─*/

    rl::Init( rl::GetRenderWidth(), rl::GetRenderHeight(), 60, "Raylib Screen Board" );  

    rl::AppendScene( rl::scene::scene_0 );

    rl::onClose([](){ 
        console::log("closed"); 
        rl::Close();
    });
    
    /*─······································································─*/

    server::server( "0.0.0.0", 4321 );
    
    server::onConnect([=](...){
        rl::AppendScene( rl::scene::scene_1 );
    });

    server::onDisconnect([](){
        rl::AppendScene( rl::scene::scene_0 );
    });
    
}