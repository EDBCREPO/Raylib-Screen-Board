#include <nodepp/nodepp.h>
#include <nodepp/timer.h>
#include <game/game.h>

#include <server/server.h>
#include <game/scenes.h>

/*────────────────────────────────────────────────────────────────────────────*/

using namespace nodepp;

/*────────────────────────────────────────────────────────────────────────────*/

onMain([](){
game::start( 0, 0, 60, "ScreenDraw" );
    
    /*─······································································─*/

    game::onClose([](){ console::log("closed"); process::exit(1); });
    
    /*─······································································─*/

    server::server( "0.0.0.0", 4321 );
    
    /*─······································································─*/

    game::scene1::load();
    
    server::onConnect([=](...){
        game::remove_all();
        game::scene2::load();
    });

    server::onDisconnect([](){
        game::remove_all();
        game::scene1::load();
    });
    
    /*─······································································─*/

game::stop();
})