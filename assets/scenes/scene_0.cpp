#pragma once

namespace rl { namespace scene {

    void scene_0( ptr_t<Scene> self ) {

        auto url = GetAttr("url").as<string_t>();
        auto a   = ptr_t<uint>({ 0, 360, 0 });
        auto Sx  = GetRenderWidth(), Sy = GetRenderHeight();

        self->onDraw([=](){
            ClearBackground( BLANK );

            DrawText( url.get(), 20, 10, 20, WHITE );
            DrawText( "waiting new device", 60, 265, 20, WHITE );

            for( auto x=20; x<100; x+=20 )
               { DrawCircleLines( 150, 150, x, DARKGRAY ); }

            DrawCircle( 150, 150, 5, WHITE );
            DrawCircleLines( 150, 150, 100, WHITE );
            DrawCircleLines( 150, 150, a[2], WHITE );

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