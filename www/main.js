
let state = {
    col: [ 255, 255, 255 ],
    beg: [ 0, 0 ],
    scr: [ 0, 0 ],
    sta: [ 0, 0 ],
    end: [ 0, 0 ],
    out: 0,
    siz: 2,
    mde: 0,
    ste: 0
};

function onSend( state, server ) {
    server.send( JSON.stringify( state ) );
}

function init( server ) {

    let Event = new app.event();

    app.addEvent( $("[screen]"), "touchstart", (ev)=>{
        ev.preventDefault(); state.end = [
            Number(ev.touches[0].clientX.toFixed(6)), 
            Number(ev.touches[0].clientY.toFixed(6)) 
        ];  state.scr = [
            $("[screen]").clientWidth,
            $("[screen]").clientHeight,
        ];  state.sta = state.end;
            state.beg = state.end; state.ste = 1;
        Event.emit( "Send", state, server );
    });

    app.addEvent( $("[screen]"), "touchmove", (ev)=>{
        ev.preventDefault(); state.end = [
            Number(ev.touches[0].clientX.toFixed(6)), 
            Number(ev.touches[0].clientY.toFixed(6)) 
        ];  state.ste = 1;
    });

    app.addEvent( $("[screen]"), "touchend", (ev)=>{
        ev.preventDefault(); state.ste = 0; 
        Event.emit( "Send", state, server );
    });

    setInterval(()=>{
        if( state.ste == 0 ){ return; }
            Event.emit( "Send", state, server );
            state.sta = state.end;
    }, 50 );

    Event.on( "Send", onSend );

}

window.onload = ()=>{

    let server = new WebSocket( window.origin.replace("http","ws") );

    server.onopen = ()=>{
        console.log("connected");
        init( server );
    };

    server.onmessage = ({data})=>{
        if( data == "updated" ){
            let ctx     = $("canvas").getContext("2d");
            let img     = new Image;
                img.src = `${window.origin}/image.png`;
            img.onload = ()=>{ ctx.reset();
                ctx.webkitImageSmoothingEnabled = false;
                ctx.mozImageSmoothingEnabled = false;
                ctx.imageSmoothingEnabled = false;
                ctx.drawImage( img, 0, 0, $("canvas").width, $("canvas").height );
            }
        }   console.log( data );
    };

    server.onclose = ()=>{
        console.log("disconnected");
        setTimeout(()=>{
            window.location.reload();
        },10000);
    };

    server.onerror = ()=>{
        console.log("something went wrong");
        setTimeout(()=>{
            window.location.reload();
        },10000);
    }

};