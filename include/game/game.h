#ifndef GAME_GAME
#define GAME_GAME

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>

#include <nodepp/optional.h>
#include <nodepp/observer.h>
#include <nodepp/object.h>
#include <nodepp/map.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace rl {
#include <raylib/raylib.h>
#include <raylib/raymath.h>
}

/*────────────────────────────────────────────────────────────────────────────*/

namespace game { 
    using namespace nodepp;

    event_t<>      onClose;
    event_t<float> onLoop;
    event_t<>      onDraw;
    
    /*─······································································─*/

    object_t global;
    
    /*─······································································─*/

    class item { public:

        ptr_t<int> state = new int(1);
        event_t<>  onRemove;
        object_t   attr;

        template< class T >
        item ( T cb ){ cb(*this); }
        item () noexcept = default;

        void remove() const noexcept {
        if(*state == 0 ) { return; } 
           *state =  0; onRemove.emit(); 
        }

    ~item(){ 
        if( state.count() > 1 )
          { return; } remove();
        } 

    }; 
    map_t<string_t,optional_t<item>> objects;
    
    /*─······································································─*/

    void append_item( string_t name, function_t<void,item&> cb ){
         objects[name] = optional_t<item>( cb );
    }

    optional_t<item> get_item( string_t name ){
         return objects[name];
    }

    void remove_item( string_t name ){
         objects.erase( name );
    }

    void remove_all(){ 
         objects.erase();
    }
    
    /*─······································································─*/

    void start( int w, int h, int fps, string_t name ){
         rl::SetWindowState( rl::FLAG_WINDOW_TRANSPARENT );
         rl::InitWindow( w, h, name.get() );
         rl::SetTargetFPS( fps );
    }

    void stop(){
        process::add([](){
            if( rl::WindowShouldClose() )
              { onClose.emit(); }
        coStart

            onLoop.emit( rl::GetFrameTime() ); coNext;

            rl::BeginDrawing(); 
            onDraw.emit(); coNext;
            rl::EndDrawing();
            
            coGoto(0);

        coStop
        });
    }
    
}

/*────────────────────────────────────────────────────────────────────────────*/

#endif