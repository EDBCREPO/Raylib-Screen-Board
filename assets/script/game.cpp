/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

#pragma once
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
    #include <raylib/raygui.h>
    #include <raylib/raymath.h>
}

/*────────────────────────────────────────────────────────────────────────────*/

namespace rl { 

    event_t<>      onClose;
    event_t<>      onInit;
    event_t<>      onDraw;
    event_t<float> onLoop;

    object_t Global; 
    
    /*─······································································─*/

    void SetAttr( string_t name, object_t value ) { 
        Global[name] = value;
    }
    
    object_t GetAttr( string_t name ) { 
        return Global[name]; 
    }

    bool HasAttr( string_t name ) {
        return Global.has( name );
    }
    
    object_t GetAttr() { 
        return Global; 
    }
    
    /*─······································································─*/

    void Close() { CloseWindow(); process::exit(1); }

    void Init( Vector2 size, uint fps, string_t title ) {

        InitWindow( size.x, size.y, title.get() ); 
        SetTargetFPS( fps ); process::add([=](){
        coStart
            while( !IsWindowReady() ){ coNext; } onInit.emit(); 
            while( !WindowShouldClose() ){ 
                onLoop.emit( GetFrameTime() ); BeginDrawing();
                onDraw.emit(); EndDrawing(); coNext;
            }   onClose.emit();
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

    event_t<>      onRemove;
    event_t<>      onDraw;
    event_t<float> onLoop;
    
    /*─······································································─*/

    template< class T, class... V >
    Item( T cb, V... args ) noexcept : obj( new NODE() ) {
        auto self = type::bind( this ); obj->state = 1;

        process::add([=](){ cb( self, args... ); return -1; });

        auto idl = rl::onLoop([=]( float delta ){ 
            if(!self->exists() || WindowShouldClose() )
              { self->free(); return; }
                self->onLoop.emit( delta );
        });

        auto idd = rl::onDraw([=](){ 
            if(!self->exists() || WindowShouldClose() )
              { return; } self->onDraw.emit(); 
        });

        auto idr = self->onRemove([=](){
            if( WindowShouldClose() ){ return; }
                rl::onDraw.off( idd );
                rl::onLoop.off( idl );
        });

        onClose([=](){ self->close(); });
        
    }

    Item() noexcept : obj( new NODE() ) {}

   ~Item(){ if( obj.count() > 1 ) { return; } free(); } 
    
    /*─······································································─*/

    void SetAttr( string_t name, object_t value ) const noexcept { 
        obj->attr[name] = value;
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

    void free() const noexcept { close(); }

    void remove() const noexcept { close(); }

    bool exists() const noexcept { return obj->state != 0; }

    void close() const noexcept { if( !exists() ){ return; } obj->state = 0; onRemove.emit(); }

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

    event_t<>      onRemove;
    event_t<>      onDraw;
    event_t<float> onLoop;
    
    /*─······································································─*/

    template< class T, class... V >
    Scene( T cb, V... args ) noexcept : obj( new NODE() ) {
        auto self = type::bind( this ); obj->state = 1;

        process::add([=](){ cb( self, args... ); return -1; });

        auto idl = rl::onLoop([=]( float delta ){ 
            if(!self->exists() || WindowShouldClose() )
              { self->free(); return; }
                self->onLoop.emit( delta );
        });

        auto idd = rl::onDraw([=](){ 
            if(!self->exists() || WindowShouldClose() )
              { return; } self->onDraw.emit(); 
        });

        auto idr = self->onRemove([=](){
            if( WindowShouldClose() ){ return; }
                rl::onDraw.off( idd ); 
                rl::onLoop.off( idl );
        });

        onClose([=](){ self->close(); });

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
        { name.resize(sizeof(item)); memcpy( name.get(), &item, sizeof(item) ); }
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

    void free() const noexcept { close(); }

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

rl::Vector2 operator^( rl::Vector2 v1, rl::Vector2 v2 ){ return rl::Vector2Multiply( v1, v2 ); }

rl::Vector2 operator-( rl::Vector2 v1, rl::Vector2 v2 ){ return rl::Vector2Subtract( v1, v2 ); }

rl::Vector2 operator-( rl::Vector2 v1, float f1 ){ return rl::Vector2SubtractValue( v1, f1 ); }

rl::Vector2 operator/( rl::Vector2 v1, rl::Vector2 v2 ){ return rl::Vector2Divide( v1, v2 ); }

float operator*( rl::Vector2 v1, rl::Vector2 v2 ){ return rl::Vector2DotProduct( v1, v2 ); }

rl::Vector2 operator+( rl::Vector2 v1, rl::Vector2 v2 ){ return rl::Vector2Add( v1, v2 ); }

rl::Vector2 operator+( rl::Vector2 v1, float f1 ){ return rl::Vector2AddValue( v1, f1 ); }

rl::Vector2 operator*( rl::Vector2 v1, float f1 ){ return rl::Vector2Scale( v1, f1 ); }

rl::Vector2 operator-( rl::Vector2 v1 ){ return rl::Vector2Negate( v1 ); }

/*────────────────────────────────────────────────────────────────────────────*/

rl::Vector3 operator^( rl::Vector3 v1, rl::Vector3 v2 ){ return rl::Vector3Multiply( v1, v2 ); }

rl::Vector3 operator-( rl::Vector3 v1, rl::Vector3 v2 ){ return rl::Vector3Subtract( v1, v2 ); }

rl::Vector3 operator-( rl::Vector3 v1, float f1 ){ return rl::Vector3SubtractValue( v1, f1 ); }

rl::Vector3 operator/( rl::Vector3 v1, rl::Vector3 v2 ){ return rl::Vector3Divide( v1, v2 ); }

float operator*( rl::Vector3 v1, rl::Vector3 v2 ){ return rl::Vector3DotProduct( v1, v2 ); }

rl::Vector3 operator+( rl::Vector3 v1, rl::Vector3 v2 ){ return rl::Vector3Add( v1, v2 ); }

rl::Vector3 operator+( rl::Vector3 v1, float f1 ){ return rl::Vector3AddValue( v1, f1 ); }

rl::Vector3 operator*( rl::Vector3 v1, float f1 ){ return rl::Vector3Scale( v1, f1 ); }

rl::Vector3 operator-( rl::Vector3 v1 ){ return rl::Vector3Negate( v1 ); }

/*────────────────────────────────────────────────────────────────────────────*/

rl::Vector4 operator^( rl::Vector4 v1, rl::Vector4 v2 ){ return rl::Vector4Multiply( v1, v2 ); }

rl::Vector4 operator-( rl::Vector4 v1, rl::Vector4 v2 ){ return rl::Vector4Subtract( v1, v2 ); }

rl::Vector4 operator-( rl::Vector4 v1, float f1 ){ return rl::Vector4SubtractValue( v1, f1 ); }

rl::Vector4 operator/( rl::Vector4 v1, rl::Vector4 v2 ){ return rl::Vector4Divide( v1, v2 ); }

float operator*( rl::Vector4 v1, rl::Vector4 v2 ){ return rl::Vector4DotProduct( v1, v2 ); }

rl::Vector4 operator+( rl::Vector4 v1, rl::Vector4 v2 ){ return rl::Vector4Add( v1, v2 ); }

rl::Vector4 operator+( rl::Vector4 v1, float f1 ){ return rl::Vector4AddValue( v1, f1 ); }

rl::Vector4 operator*( rl::Vector4 v1, float f1 ){ return rl::Vector4Scale( v1, f1 ); }

rl::Vector4 operator-( rl::Vector4 v1 ){ return rl::Vector4Negate( v1 ); }

/*────────────────────────────────────────────────────────────────────────────*/