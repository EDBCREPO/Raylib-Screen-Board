#pragma once

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>
#include <nodepp/event.h>
#include <nodepp/http.h>
#include <nodepp/path.h>
#include <nodepp/dns.h>
#include <nodepp/ws.h>
#include <nodepp/fs.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace server { using namespace nodepp;

    event_t<>                  onDisconnect;
    event_t<ws_t>              onConnect;
    event_t<string_t>          onMessage;
    event_t<string_t>          onWrite;
    event_t<except_t>          onError;
    event_t<string_t,string_t> onEvent;
    
    /*─······································································─*/

    void WebHandler( http_t cli ){

        for( auto &x: cli.query.data() ){
             onEvent.emit( x.first, x.second );
             cli.write_header( 200, header_t());
             cli.write("done"); return;
        }

        string_t dir = "www/index.html";
        if( cli.path != "/" )
            dir = path::join( "www", cli.path );

        console::log( cli.path, cli.get_fd() );

        if( !fs::exists_file(dir) ){
            cli.write_header( 404, header_t({ { "content-type", "text/plain" } }) );
            cli.write( "Opps 404 Error" ); 
            cli.close(); return;
        }

        auto str = fs::readable( dir );

        cli.write_header( 200, header_t({
            { "Content-Length", string::to_string(str.size()) },
            { "Cache-Control", "public, max-age=3600" },
            { "Content-Type",   path::mimetype(dir) }
        }));

        stream::pipe( str, cli );

    }
    
    /*─······································································─*/
    
    void server( string_t host, uint port ){

        auto server = http::server( WebHandler ); 
                        ws::server( server );

        server.onConnect([=]( ws_t cli ){

            console::log("connected");
            onConnect.emit( cli );

            auto idw = onWrite([=]( string_t data ){
                 cli.write( data );
            });

            cli.onData([=]( string_t data ){
                onMessage.emit( data );
            });
        
            cli.onClose([=](){
                console::log("diconencted");
                onDisconnect.emit();
                onWrite.off( idw );
            });

        });

        server.onError([]( except_t err ){ onError.emit( err ); });

        server.listen( host, port, [=](...){
            rl::SetAttr( "url",
                string::format("http://%s:%d",dns::get_hostname().get(), port )
            );  console::log( "Started->", "http://localhost:4321" );
        });
    }

}

/*────────────────────────────────────────────────────────────────────────────*/
