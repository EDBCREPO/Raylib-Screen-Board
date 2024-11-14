/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

#pragma once
#define _SHADER_(...)(string_t("#version 100\n")+_STRING_(__VA_ARGS__)).get()
//      RAYGUI_IMPLEMENTATION

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/encoder.h>
#include <nodepp/timer.h>
#include <nodepp/event.h>
#include <nodepp/json.h>
#include <nodepp/map.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace rl {
    #include <raylib/raylib.h>
    /*---------------------*/
    #include <raylib/raygl.h>
    #include <raylib/raygui.h>
    #include <raylib/raymath.h>
}

/*────────────────────────────────────────────────────────────────────────────*/

namespace rl { 

    ptr_t<Camera2D> GlobalCam2D;
    ptr_t<Camera3D> GlobalCam3D;
    object_t        Global;

    ulong           Waiting=0;

    event_t<>       on3DDraw;
    event_t<>       on2DDraw;
    event_t<>       onClose;
    event_t<>       onInit;
    event_t<>       onDraw;
    event_t<float>  onLoop;
    event_t<>       onNext;
    
    /*─······································································─*/

    void SetAttr( string_t name, object_t value ){ Global[name] = value; }

    void RemoveAttr( string_t name ){ Global.erase( name ); }
    
    object_t GetAttr( string_t name ){ return Global[name]; }

    bool HasAttr( string_t name ){ return Global.has(name); }
    
    object_t GetAttr(){ return Global; }
    
    /*─······································································─*/

    void Close() { 
        static bool b=0; if(b){ return; } b=1;
        onClose.emit(); CloseWindow();
        process::exit(1);
    }

    void BeginMode3D( Camera3D& cam, float near, float far ){
        rlDrawRenderBatchActive();        // Update and draw internal render batch
        rlMatrixMode(RL_PROJECTION);      // Switch to projection matrix
        rlPushMatrix(); rlLoadIdentity(); // Reset current matrix (projection)

        float aspect = GetRenderWidth() * 1.016f / GetRenderHeight();

        if ( cam.projection == CAMERA_PERSPECTIVE ) {

            double top   = near * tan(cam.fovy * 0.5 * DEG2RAD);
            double right = top  * aspect;
            rlFrustum(-right, right, -top, top, near, far);

        } else {

            double top = cam.fovy / 2.0;
            double right = top * aspect;
            rlOrtho(-right, right, -top, top, near, far);

        }

        rlMatrixMode(RL_MODELVIEW);     // Switch back to modelview matrix
        rlLoadIdentity();               // Reset current matrix (modelview)

        Matrix matView = MatrixLookAt(cam.position, cam.target, cam.up);
        rlMultMatrixf(MatrixToFloat(matView));

        rlEnableDepthTest(); // Enable DEPTH_TEST for 3D
    }

    void Init( int width, int height, uint fps, string_t title ) {
        InitWindow( width, height, title.get() ); SetTargetFPS( fps ); 
        process::onSIGEXIT([](){ Close(); }); process::add([=](){
        coStart
            onInit.emit(); while( !WindowShouldClose() ){
            while( !IsWindowReady() || Waiting!=0 ){ coNext; } 
                onLoop.emit( GetFrameTime() );  BeginDrawing(); 
                    if( GlobalCam3D!=nullptr ){ BeginMode3D( *GlobalCam3D, 0.4f, 1000.0f ); on3DDraw.emit(); EndMode3D(); }
                    if( GlobalCam2D!=nullptr ){ BeginMode2D( *GlobalCam2D );                on2DDraw.emit(); EndMode2D(); }
                    if( !onDraw.empty() )     { onDraw.emit(); }
                EndDrawing(); coNext; onNext.emit();
            }   Close();
        coStop
        });
    }

}

/*────────────────────────────────────────────────────────────────────────────*/

namespace rl { class Item {
protected:

    struct NODE {
        bool    state = 0;
        object_t attr;
    }; ptr_t<NODE> obj;

public:

    event_t<>      on3DDraw;
    event_t<>      on2DDraw;
    event_t<>      onRemove;
    event_t<>      onDraw;
    event_t<float> onLoop;
    
    /*─······································································─*/

    template< class T, class... V >
    Item( T cb, V... args ) noexcept : obj( new NODE() ) {
        auto self = type::bind( this ); obj->state = 1;
        array_t<void*> id; Waiting++;

        id.push( rl::onLoop([=]( float delta ){ 
            if( !self->exists() ){ self->free(); return; } 
                self->onLoop.emit( delta );
        }) );

        id.push( rl::onDraw([=](){ 
            if( !self->exists() ){ return; } 
                self->onDraw.emit(); 
        }) );

        id.push( rl::on3DDraw([=](){ 
            if( !self->exists() ){ return; } 
                self->on3DDraw.emit(); 
        }) );

        id.push( rl::on2DDraw([=](){ 
            if( !self->exists() ){ return; } 
                self->on2DDraw.emit(); 
        }) );

        auto idr = self->onRemove.once([=](){
             if( WindowShouldClose() ){ return; }
            for( auto &x: id ){ process::clear(x); }
        });

        rl::onClose.once([=](){ self->close(); });
        process::add([=](){ cb( self, args... ); Waiting--; return -1; });
        
    }

    Item() noexcept : obj( new NODE() ) {}

   ~Item(){ if( obj.count() > 1 ) { return; } free(); } 
    
    /*─······································································─*/

    void SetAttr( string_t name, object_t value ) const noexcept { 
        obj->attr[name] = value;
    }

    void RemoveAttr( string_t name ) const noexcept { 
        obj->attr.erase( name );
    }
    
    object_t GetAttr( string_t name ) const noexcept { 
        return obj->attr[name]; 
    }

    bool HasAttr( string_t name ) const noexcept {
        return obj->attr.has( name );
    }
    
    object_t GetAttr() const noexcept { 
        return obj->attr; 
    }

    /*─······································································─*/

    void free()   const noexcept { close(); }

    void remove() const noexcept { close(); }

    bool exists() const noexcept { return obj->state != 0; }

    void close()  const noexcept { if( !exists() ){ return; } obj->state = 0; onRemove.emit(); }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace rl { class Scene {
protected:

    struct NODE {
        map_t<string_t,Item> items;
        object_t attr;
        bool state = 0;
    }; ptr_t<NODE> obj;

public:

    event_t<>      on3DDraw;
    event_t<>      on2DDraw;
    event_t<>      onRemove;
    event_t<>      onDraw;
    event_t<float> onLoop;
    
    /*─······································································─*/

    template< class T, class... V >
    Scene( T cb, V... args ) noexcept : obj( new NODE() ) {
        auto self = type::bind( this ); obj->state = 1;
        array_t<void*> id; Waiting++;

        id.push( rl::onLoop([=]( float delta ){ 
            if( !self->exists() ){ self->free(); return; } 
                self->onLoop.emit( delta );
        }) );

        id.push( rl::onDraw([=](){ 
            if( !self->exists() ){ return; } 
                self->onDraw.emit(); 
        }) );

        id.push( rl::on3DDraw([=](){ 
            if( !self->exists() ){ return; } 
                self->on3DDraw.emit(); 
        }) );

        id.push( rl::on2DDraw([=](){ 
            if( !self->exists() ){ return; } 
                self->on2DDraw.emit(); 
        }) );

        auto idr = self->onRemove.once([=](){
             if( WindowShouldClose() ){ return; }
            for( auto &x: id ){ process::clear(x); }
        });

        rl::onClose.once([=](){ self->close(); });
        process::add([=](){ cb( self, args... ); Waiting--; return -1; });

    }

    Scene() noexcept : obj( new NODE() ){}

   ~Scene(){ if( obj.count() > 1 ) { return; } free(); } 
    
    /*─······································································─*/

    void SetAttr( string_t name, object_t value ) const noexcept { 
        obj->attr[name] = value;
    }
    
    object_t GetAttr( string_t name ) const noexcept { 
        return obj->attr[name]; 
    }

    void RemoveAttr( string_t name ) const noexcept { 
        obj->attr.erase( name ); 
    }

    bool HasAttr( string_t name ) const noexcept {
        return obj->attr.has( name );
    }
    
    object_t GetAttr() const noexcept { 
        return obj->attr;
    }
    
    /*─······································································─*/

    template< class T, class... V >
    Item& AppendItem( string_t name, T cb, V... args ) const noexcept {
        auto item = Item( cb, args... ); if( name == nullptr )
        { name.resize(sizeof(item)); memcpy( name.get(), (void*)&item, sizeof(item) ); }
          obj->items[name] = item; return obj->items[name];
    }

    void RemoveItem( string_t name ) const noexcept {
        obj->items[name].close();
    }

    Item& GetItem( string_t name ) const noexcept {
        return obj->items[name];
    }

    bool HasItem( string_t name ) const noexcept {
        return obj->items.has( name );
    }

    void RemoveItem() const noexcept {
        for( auto &x: obj->items.data() )
           { x.second.close(); }
    }

    /*─······································································─*/

    void free()   const noexcept { close(); }

    void remove() const noexcept { close(); }

    bool exists() const noexcept { return obj->state != 0; }

    /*─······································································─*/

    void close() const noexcept {
        if( !exists() ){ return; } obj->state = 0; 
            onRemove.emit(); RemoveItem();
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace rl { 
    
    ptr_t<Scene> Room = new Scene();

    void RemoveScene(){ Room->close(); }

    Scene& GetScene() { return *Room; }

    void SetScene( Scene& Scene ) { *Room = Scene; }

    template< class T, class... V >
    Scene& AppendScene( T cb, V... args ){ RemoveScene(); 
        Room = type::bind( Scene( cb, args... ) );
        return *Room;
    }

}

/*────────────────────────────────────────────────────────────────────────────*/

float       operator^ ( rl::Vector2 v1, rl::Vector2 v2 ){ return rl::Vector2DotProduct( v1, v2 ); }
rl::Vector2 operator* ( rl::Vector2 v1, rl::Vector2 v2 ){ return rl::Vector2Multiply( v1, v2 ); }
rl::Vector2 operator- ( rl::Vector2 v1, rl::Vector2 v2 ){ return rl::Vector2Subtract( v1, v2 ); }
rl::Vector2 operator- ( rl::Vector2 v1, float f1 ){ return rl::Vector2SubtractValue( v1, f1 ); }
rl::Vector2 operator/ ( rl::Vector2 v1, rl::Vector2 v2 ){ return rl::Vector2Divide( v1, v2 ); }
rl::Vector2 operator+ ( rl::Vector2 v1, rl::Vector2 v2 ){ return rl::Vector2Add( v1, v2 ); }
rl::Vector2 operator+ ( rl::Vector2 v1, float f1 ){ return rl::Vector2AddValue( v1, f1 ); }
rl::Vector2 operator* ( rl::Vector2 v1, float f1 ){ return rl::Vector2Scale( v1, f1 ); }
rl::Vector2 operator- ( rl::Vector2 v1 ){ return rl::Vector2Negate( v1 ); }

void operator-=( rl::Vector2& v1, float f1 )      { v1 = rl::Vector2SubtractValue( v1, f1 ); }
void operator==( rl::Vector2& v1, rl::Vector2 v2 ){ memcmp( &v1, &v2, sizeof(rl::Vector2) ); }
void operator-=( rl::Vector2& v1, rl::Vector2 v2 ){ v1 = rl::Vector2Subtract( v1, v2 ); }
void operator^=( rl::Vector2& v1, rl::Vector2 v2 ){ v1 = rl::Vector2Multiply( v1, v2 ); }
void operator/=( rl::Vector2& v1, rl::Vector2 v2 ){ v1 = rl::Vector2Divide( v1, v2 ); }
void operator+=( rl::Vector2& v1, rl::Vector2 v2 ){ v1 = rl::Vector2Add( v1, v2 ); }
void operator+=( rl::Vector2& v1, float f1 ){ v1 = rl::Vector2AddValue( v1, f1 ); }
void operator*=( rl::Vector2& v1, float f1 ){ v1 = rl::Vector2Scale( v1, f1 ); }

/*────────────────────────────────────────────────────────────────────────────*/

float       operator^ ( rl::Vector3 v1, rl::Vector3 v2 ){ return rl::Vector3DotProduct( v1, v2 ); }
rl::Vector3 operator* ( rl::Vector3 v1, rl::Vector3 v2 ){ return rl::Vector3Multiply( v1, v2 ); }
rl::Vector3 operator- ( rl::Vector3 v1, rl::Vector3 v2 ){ return rl::Vector3Subtract( v1, v2 ); }
rl::Vector3 operator- ( rl::Vector3 v1, float f1 ){ return rl::Vector3SubtractValue( v1, f1 ); }
rl::Vector3 operator/ ( rl::Vector3 v1, rl::Vector3 v2 ){ return rl::Vector3Divide( v1, v2 ); }
rl::Vector3 operator+ ( rl::Vector3 v1, rl::Vector3 v2 ){ return rl::Vector3Add( v1, v2 ); }
rl::Vector3 operator+ ( rl::Vector3 v1, float f1 ){ return rl::Vector3AddValue( v1, f1 ); }
rl::Vector3 operator* ( rl::Vector3 v1, float f1 ){ return rl::Vector3Scale( v1, f1 ); }
rl::Vector3 operator- ( rl::Vector3 v1 ){ return rl::Vector3Negate( v1 ); }

void operator-=( rl::Vector3& v1, float f1 )      { v1 = rl::Vector3SubtractValue( v1, f1 ); }
void operator==( rl::Vector3& v1, rl::Vector3 v2 ){ memcmp( &v1, &v2, sizeof(rl::Vector3) ); }
void operator-=( rl::Vector3& v1, rl::Vector3 v2 ){ v1 = rl::Vector3Subtract( v1, v2 ); }
void operator^=( rl::Vector3& v1, rl::Vector3 v2 ){ v1 = rl::Vector3Multiply( v1, v2 ); }
void operator/=( rl::Vector3& v1, rl::Vector3 v2 ){ v1 = rl::Vector3Divide( v1, v2 ); }
void operator+=( rl::Vector3& v1, rl::Vector3 v2 ){ v1 = rl::Vector3Add( v1, v2 ); }
void operator+=( rl::Vector3& v1, float f1 ){ v1 = rl::Vector3AddValue( v1, f1 ); }
void operator*=( rl::Vector3& v1, float f1 ){ v1 = rl::Vector3Scale( v1, f1 ); }

/*────────────────────────────────────────────────────────────────────────────*/

float       operator^ ( rl::Vector4 v1, rl::Vector4 v2 ){ return rl::Vector4DotProduct( v1, v2 ); }
rl::Vector4 operator* ( rl::Vector4 v1, rl::Vector4 v2 ){ return rl::Vector4Multiply( v1, v2 ); }
rl::Vector4 operator- ( rl::Vector4 v1, rl::Vector4 v2 ){ return rl::Vector4Subtract( v1, v2 ); }
rl::Vector4 operator- ( rl::Vector4 v1, float f1 ){ return rl::Vector4SubtractValue( v1, f1 ); }
rl::Vector4 operator/ ( rl::Vector4 v1, rl::Vector4 v2 ){ return rl::Vector4Divide( v1, v2 ); }
rl::Vector4 operator+ ( rl::Vector4 v1, rl::Vector4 v2 ){ return rl::Vector4Add( v1, v2 ); }
rl::Vector4 operator+ ( rl::Vector4 v1, float f1 ){ return rl::Vector4AddValue( v1, f1 ); }
rl::Vector4 operator* ( rl::Vector4 v1, float f1 ){ return rl::Vector4Scale( v1, f1 ); }
rl::Vector4 operator- ( rl::Vector4 v1 ){ return rl::Vector4Negate( v1 ); }

void operator-=( rl::Vector4& v1, float f1 )      { v1 = rl::Vector4SubtractValue( v1, f1 ); }
void operator==( rl::Vector4& v1, rl::Vector4 v2 ){ memcmp( &v1, &v2, sizeof(rl::Vector4) ); }
void operator-=( rl::Vector4& v1, rl::Vector4 v2 ){ v1 = rl::Vector4Subtract( v1, v2 ); }
void operator^=( rl::Vector4& v1, rl::Vector4 v2 ){ v1 = rl::Vector4Multiply( v1, v2 ); }
void operator/=( rl::Vector4& v1, rl::Vector4 v2 ){ v1 = rl::Vector4Divide( v1, v2 ); }
void operator+=( rl::Vector4& v1, rl::Vector4 v2 ){ v1 = rl::Vector4Add( v1, v2 ); }
void operator+=( rl::Vector4& v1, float f1 ){ v1 = rl::Vector4AddValue( v1, f1 ); }
void operator*=( rl::Vector4& v1, float f1 ){ v1 = rl::Vector4Scale( v1, f1 ); }

/*────────────────────────────────────────────────────────────────────────────*/