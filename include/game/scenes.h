#ifndef GAME_SCENES
#define GAME_SCENES

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>
#include <nodepp/encoder.h>
#include <nodepp/fs.h>
#include <json/json.h>
#include <game/game.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace game   {
namespace scene1 { using namespace nodepp;

    void unload(){ remove_item("scene"); }

    void load(){
        append_item("scene",[]( item& self ){
            auto url = global["url"].as<string_t>();
            auto a   = ptr_t<uint>({ 0, 360, 0 });

            rl::SetWindowSize( 300, 300 );
            fs::remove_file("./www/image.jpg");
            rl::ClearWindowState( rl::FLAG_WINDOW_MOUSE_PASSTHROUGH |
                                  rl::FLAG_WINDOW_UNDECORATED       | 
                                  rl::FLAG_WINDOW_TOPMOST           |
                                  rl::FLAG_FULLSCREEN_MODE          |
                                  rl::FLAG_WINDOW_UNFOCUSED         );

            auto idd = onDraw([=](){
                 rl::ClearBackground({ 40, 40, 40, 255 });
                 rl::DrawCircleLines( 150, 150, 100, WHITE );
                 rl::DrawCircleLines( 150, 150, a[2], WHITE );
                 rl::DrawText( url.get(), 30, 10, 20, WHITE );
                 rl::DrawText( "waiting new device", 60, 265, 20, WHITE );
                 rl::DrawCircleSectorLines( { 150, 150 }, 100, a[0], a[1], 30, WHITE );
            });

            auto idl = onLoop([=]( float delta ){
                 a[0] += 100 * delta; a[0] %= 360;
                 a[1] += 100 * delta; a[1] %= 360;
                 a[2] += 100 * delta; 
                 a[2] =  clamp( a[2], 0u, 100u );
            });

            auto idt = timer::interval([=](){
                 a[2] = 0;
            }, 3000 );

            self.onRemove([=](){ 
                timer::clear( idt );
                onDraw.off( idd ); 
                onLoop.off( idl );    
            });
        });
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace game   {
namespace scene2 { using namespace nodepp;
                   using namespace tiny; 

    struct item_header {
        array_t<rl::Vector4> pos;
        rl::Vector4          vec;
        rl::Color            col;
        int                  out;
        int                  siz;
        int                  mde;
    };  

    ptr_t<rl::RenderTexture2D> rend;
    ptr_t<rl::Color>           color;
    ptr_t<rl::Vector2>         cursor;
    queue_t<item_header>       item_queue;

    namespace {
    
    void get_color_hex( rl::Color* col, string_t& hex ){
         uchar r=0, g=0, b=0; 
         auto data = regex::match_all( hex, "\\w{2}" );
         color->r = encoder::hex::set<uchar>(data[0]); 
         color->g = encoder::hex::set<uchar>(data[1]);
         color->b = encoder::hex::set<uchar>(data[2]);
    }
    
    void updateImage(){
    /*
        rl::BeginTextureMode( *rend );
            onDraw.emit();
        rl::EndTextureMode();

        int size = 0; cursor = nullptr;
        auto img = rl::LoadImageFromTexture( rend->texture );
                   rl::ImageFlipVertical( &img );
        auto dta = rl::ExportImageToMemory(img,".png",&size);
                   rl::UnloadImage( img );

        file_t out ( "./www/image.png", "w" );
        out.write(string_t((char*)dta,size ));
        server::onWrite.emit( "updated" ); 
    */
    }

    }

    void unload(){ remove_item("scene"); }

    void load(){
        append_item("scene",[]( item& self ){

            auto X  = rl::GetCurrentMonitor();
            auto Sy = rl::GetMonitorHeight(X);
            auto Sx = rl::GetMonitorWidth(X);

            rl::SetWindowSize( Sx, Sy );
            rl::SetWindowState( rl::FLAG_WINDOW_MOUSE_PASSTHROUGH |
                                rl::FLAG_WINDOW_UNDECORATED       |
                                rl::FLAG_WINDOW_UNFOCUSED         | 
                                rl::FLAG_FULLSCREEN_MODE          |
                                rl::FLAG_WINDOW_TOPMOST           );

            rend  = type::bind( rl::LoadRenderTexture( Sx, Sy ) );
            color = type::bind( rl::Color({ 0, 0, 0, 0 }) );

            console::log( Sx, Sy );

            auto ide = server::onEvent([=]( string_t a, string_t b ){
                 if( a == "opacity" )
                   { color->a = string::to_int(b); }
                 if( a == "color" )
                   { get_color_hex( &color, b ); }
                 if( a == "cmd" && b == "clear" )
                   { item_queue.free(); return; }
                   updateImage();
            });

            auto ids = server::onMessage([=]( string_t data ){
                TinyJson json; json.ReadJson(data.get());
                if( data[data.last()] != '}' || 
                    data[0]           != '{' 
                  ){ return; }

                auto ste = json.Get<int>("ste");
                auto siz = json.Get<int>("siz");
                auto mde = json.Get<int>("mde");
                auto out = json.Get<int>("out");

                auto col = json.Get<xarray>("col");
                auto end = json.Get<xarray>("end");
                auto sta = json.Get<xarray>("sta"); 
                auto scr = json.Get<xarray>("scr");
                auto beg = json.Get<xarray>("beg");

                scr.Enter(0); auto scrX = scr.Get<int>();
                scr.Enter(1); auto scrY = scr.Get<int>();

                col.Enter(0); auto colR = (uchar) col.Get<int>();
                col.Enter(1); auto colG = (uchar) col.Get<int>();
                col.Enter(2); auto colB = (uchar) col.Get<int>();
                
                beg.Enter(0); auto begX = beg.Get<float>() * Sx / scrX;
                beg.Enter(1); auto begY = beg.Get<float>() * Sy / scrY;

                sta.Enter(0); auto staX = sta.Get<float>() * Sx / scrX;
                sta.Enter(1); auto staY = sta.Get<float>() * Sy / scrY;
                
                end.Enter(0); auto endX = end.Get<float>() * Sx / scrX;
                end.Enter(1); auto endY = end.Get<float>() * Sy / scrY;
                
                cursor = new rl::Vector2({ endX, endY });

                if( mde >= 4 ){ goto CURSOR; }

                if( item_queue.empty() || ste == 0 ){
                    item_header ihd; cursor = nullptr;
                    memset( &ihd, sizeof(ihd), 1 );
                    item_queue.push( ihd );
                }

                if( mde == 0 ){ auto ihd = item_queue.last();

                    ihd->data.vec = rl::Vector4({ begX, begY, endX, endY });
                    ihd->data.col = rl::Color({ colR, colG, colB, 255 });
                    ihd->data.pos.push({ staX, staY, endX, endY });
                    ihd->data.out = out;
                    ihd->data.siz = siz;
                    ihd->data.mde = mde;

                } else { auto ihd = item_queue.last();

                    ihd->data.pos = array_t<rl::Vector4>(1,{ staX, staY, endX, endY });
                    ihd->data.vec = rl::Vector4({ begX, begY, endX, endY });
                    ihd->data.col = rl::Color({ colR, colG, colB, 255 });
                    ihd->data.out = out;
                    ihd->data.siz = siz;
                    ihd->data.mde = mde;

                }

                CURSOR:
                if( item_queue.empty() || ste == 0 ){
                    updateImage();
                }

            });

            auto idd = onDraw([=](){
                 rl::ClearBackground( *color );
                 if( item_queue.empty() ){ return; }

                 for( auto &x: item_queue.data() ){ for( auto &y: x.pos ){
                  if( x.mde == 0 ){
                      rl::DrawLineEx( { y.x, y.y }, { y.z, y.w }, x.siz, x.col );
                  } elif( x.mde == 1 ) { 
                      rl::Vector4 z = x.vec; 
                      rl::DrawLineEx({ z.x, z.y },{ z.z, z.w },x.siz, x.col );
                  } elif( x.mde == 2 ) { rl::Vector4 z = x.vec; 
                      auto r = sqrt( pow(z.x-z.z,2) + pow(z.y-z.w,2) );
                      if( x.out == 1 ){
                        rl::DrawCircle( z.x, z.y, r, x.col );
                      } else {
                        rl::DrawCircleLines( z.x, z.y, r, x.col );
                      }
                  } elif( x.mde == 3 ) { rl::Vector4 z = x.vec; 
                      if( z.x > z.z ){ type::swap( z.x, z.z ); }
                      if( z.y > z.w ){ type::swap( z.y, z.w ); }
                      if( x.out == 1 ){
                        rl::DrawRectangleV({ z.x, z.y }, { abs( z.x-z.z ), abs( z.y-z.w ) }, x.col );
                      } else {
                        rl::DrawRectangleLinesEx({ z.x, z.y, abs( z.x-z.z ), abs( z.y-z.w ) }, x.siz, x.col );
                      }
                  }
                 }}
                
                 if( cursor != nullptr ){
                    auto posX = cursor->x;
                    auto posY = cursor->y;
                    rl::DrawCircleLines( posX, posY, 10, WHITE );
                 }
            });

            self.onRemove([=](){
                if( rend.has_value() )
                    rl::UnloadRenderTexture( *rend );
                server::onMessage.off( ids );
                server::onEvent.off( ide );
                onDraw.off( idd );
                item_queue.free();
            });

        });
    }

}}

#endif