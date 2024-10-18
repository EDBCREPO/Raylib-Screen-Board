#pragma once

namespace rl { namespace scene {

    void scene_1( ptr_t<Scene> self ) {

        struct item_header {
            array_t<Vector4> pos;
            Vector4          vec;
            Color            col;
            int              out;
            int              siz;
            int              mde;
        };  

        struct NODE {
            Color                color;
            ptr_t<Vector2 >      cursor;
            queue_t<item_header> item_queue;
        };  ptr_t<NODE> obj= new NODE();

        function_t<Color,string_t> get_color_hex ([]( string_t hex ){
            auto data = regex::match_all( hex, "\\w{2}" );
            uchar r = encoder::hex::set<uchar>(data[0]); 
            uchar g = encoder::hex::set<uchar>(data[1]);
            uchar b = encoder::hex::set<uchar>(data[2]);
            return Color({ r, g, b, 255 });
        });

        auto X  = GetCurrentMonitor();
        auto Sx = 300; // GetMonitorWidth(X);
        auto Sy = 325; // GetMonitorHeight(X);

        SetWindowSize( Sx, Sy ); /*
        SetWindowState( FLAG_WINDOW_MOUSE_PASSTHROUGH |
                            FLAG_WINDOW_UNDECORATED   |
                            FLAG_WINDOW_UNFOCUSED     | 
                            FLAG_FULLSCREEN_MODE      |
                            FLAG_WINDOW_TOPMOST       );
        */

        obj->color = BLACK;

        auto ide = server::onEvent([=]( string_t a, string_t b ){
            if( a == "opacity" ){ obj->color.a = string::to_int(b); }
            if( a == "color" )  { obj->color   = get_color_hex(b) ; }
            if( a == "cmd" && b == "clear" )
              { obj->item_queue.free(); return; }
        });

        auto ids = server::onMessage([=]( string_t raw ){
        try {

            auto data = json::parse( raw.get() );

            auto col = data["col"].as<array_t<object_t>>();
            auto end = data["end"].as<array_t<object_t>>();
            auto sta = data["sta"].as<array_t<object_t>>();
            auto scr = data["scr"].as<array_t<object_t>>();
            auto beg = data["beg"].as<array_t<object_t>>();

            auto ste = data["ste"].as<int>();
            auto siz = data["siz"].as<int>();
            auto mde = data["mde"].as<int>();
            auto out = data["out"].as<int>();

            auto scrX = scr[0].as<int>();
            auto scrY = scr[1].as<int>();

            auto colR = (uchar)( col[0].as<int>() );
            auto colG = (uchar)( col[1].as<int>() );
            auto colB = (uchar)( col[2].as<int>() );
            
            auto begX = string::to_float(beg[0].as<string_t>()) * Sx / scrX;
            auto begY = string::to_float(beg[1].as<string_t>()) * Sy / scrY;

            auto staX = string::to_float(sta[0].as<string_t>()) * Sx / scrX;
            auto staY = string::to_float(sta[1].as<string_t>()) * Sy / scrY;
            
            auto endX = string::to_float(end[0].as<string_t>()) * Sx / scrX;
            auto endY = string::to_float(end[1].as<string_t>()) * Sy / scrY;
            
            obj->cursor = new Vector2({ endX, endY });

            if( mde >= 4 ){ return; }

            if( obj->item_queue.empty() || ste == 0 ){
                item_header ihd; obj->cursor = nullptr;
                memset( &ihd, sizeof(ihd), 1 );
                obj->item_queue.push( ihd );
            }

            if( mde == 0 ){ auto ihd = obj->item_queue.last();

                ihd->data.vec = Vector4({ begX, begY, endX, endY });
                ihd->data.col = Color({ colR, colG, colB, 255 });
                ihd->data.pos.push({ staX, staY, endX, endY });
                ihd->data.out = out;
                ihd->data.siz = siz;
                ihd->data.mde = mde;

            } else { auto ihd = obj->item_queue.last();

                ihd->data.pos = array_t<Vector4>(1,{ staX, staY, endX, endY });
                ihd->data.vec = Vector4({ begX, begY, endX, endY });
                ihd->data.col = Color({ colR, colG, colB, 255 });
                ihd->data.out = out;
                ihd->data.siz = siz;
                ihd->data.mde = mde;

            }
        
        } catch(...) { return; }
        });

        self->onDraw([=](){ ClearBackground( obj->color );
            if( obj->item_queue.empty() ){ return; }

            for( auto &x: obj->item_queue.data() ){ 
            for( auto &y: x.pos ){

                if( x.mde == 0 ){

                    DrawLineEx( { y.x, y.y }, { y.z, y.w }, x.siz, x.col );

                } elif( x.mde == 1 ) { 

                    Vector4 z = x.vec; 
                    DrawLineEx({ z.x, z.y }, { z.z, z.w }, x.siz, x.col );

                } elif( x.mde == 2 ) { 
                    
                    Vector4 z = x.vec; 
                    auto r = sqrt( pow(z.x-z.z,2) + pow(z.y-z.w,2) );
                    if( x.out == 1 ){ DrawCircle( z.x, z.y, r, x.col ); } 
                    else            { DrawCircleLines( z.x, z.y, r, x.col ); }

                } elif( x.mde == 3 ) { 
                    
                    Vector4 z = x.vec;

                    if( z.x > z.z ){ type::swap( z.x, z.z ); }
                    if( z.y > z.w ){ type::swap( z.y, z.w ); }
                    
                    if( x.out == 1 ){
                        DrawRectangleV({ z.x, z.y }, { abs( z.x-z.z ), abs( z.y-z.w ) }, x.col );
                    } else {
                        DrawRectangleLinesEx({ z.x, z.y, abs( z.x-z.z ), abs( z.y-z.w ) }, x.siz, x.col );
                    }

                }

            }}
            
            if( obj->cursor != nullptr ){
                auto posX = obj->cursor->x; 
                auto posY = obj->cursor->y;
                DrawCircleLines( posX, posY, 10, WHITE );
            }

        });

        self->onRemove([=](){
            if( WindowShouldClose() ){ return; }
            process::clear( ids );
            process::clear( ide );
        });

    }

}}