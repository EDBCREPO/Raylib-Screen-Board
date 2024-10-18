#pragma once

namespace rl { namespace scene {

    void scene_0( ptr_t<Scene> self ) {

        auto url = GetAttr("url").as<string_t>();
        auto a   = ptr_t<uint>({ 0, 360, 0 });

        SetWindowSize( 300, 350 ); /*
        ClearWindowState( FLAG_WINDOW_MOUSE_PASSTHROUGH |
                              FLAG_WINDOW_UNDECORATED   | 
                              FLAG_WINDOW_TOPMOST       |
                              FLAG_FULLSCREEN_MODE      |
                              FLAG_WINDOW_UNFOCUSED     );
        */

        self->onDraw([=](){
            ClearBackground({ 40, 40, 40, 255 });
            DrawCircleLines( 150, 150, 100, WHITE );
            DrawCircleLines( 150, 150, a[2], WHITE );
            DrawText( url.get(), 30, 10, 20, WHITE );
            DrawText( "waiting new device", 60, 265, 20, WHITE );
            DrawCircleSectorLines( { 150, 150 }, 100, a[0], a[1], 30, WHITE );
        });

        self->onLoop([=]( float delta ){

            a[0] += 100 * delta; a[0] %= 360;
            a[1] += 100 * delta; a[1] %= 360;
            a[2] += 100 * delta; 
            
            a[2] =  clamp( a[2], 0u, 100u );

            [=](){
            coStart
                coDelay( 3000 ); a[2] = 0;
            coStop
            }();

        });

    }

}}