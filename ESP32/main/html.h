static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE HTML>
<html>
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width,initial-scale=1">
        <title>ESP32 OV2460</title>
        <style>          body{font-family:Arial,Helvetica,sans-serif;background:#181818;color:#EFEFEF;font-size:16px}h2{font-size:18px}section.main{display:flex}#menu,section.main{flex-direction:column}#menu{display:none;flex-wrap:nowrap;min-width:340px;background:#363636;padding:8px;border-radius:4px;margin-top:-10px;margin-right:10px}#content{display:flex;flex-wrap:wrap;align-items:stretch}figure{padding:0;margin:0;-webkit-margin-before:0;margin-block-start:0;-webkit-margin-after:0;margin-block-end:0;-webkit-margin-start:0;margin-inline-start:0;-webkit-margin-end:0;margin-inline-end:0}figure img{display:block;width:100%;height:auto;border-radius:4px;margin-top:8px}@media (min-width: 800px) and (orientation:landscape){#content{display:flex;flex-wrap:nowrap;align-items:stretch}figure img{display:block;max-width:100%;max-height:calc(100vh - 40px);width:auto;height:auto}figure{padding:0;margin:0;-webkit-margin-before:0;margin-block-start:0;-webkit-margin-after:0;margin-block-end:0;-webkit-margin-start:0;margin-inline-start:0;-webkit-margin-end:0;margin-inline-end:0}}section#buttons{display:flex;flex-wrap:nowrap;justify-content:space-between}#nav-toggle{cursor:pointer;display:block}#nav-toggle-cb{outline:0;opacity:0;width:0;height:0}#nav-toggle-cb:checked+#menu{display:flex}.input-group{display:flex;flex-wrap:nowrap;line-height:22px;margin:5px 0}.input-group>label{display:inline-block;padding-right:10px;min-width:47%}.input-group input,.input-group select{flex-grow:1}.range-max,.range-min{display:inline-block;padding:0 5px}button{display:block;margin:5px;padding:0 12px;border:0;line-height:28px;cursor:pointer;color:#fff;background:#ff3034;border-radius:5px;font-size:16px;outline:0}button:hover{background:#ff494d}button:active{background:#f21c21}button.disabled{cursor:default;background:#a0a0a0}input[type=range]{-webkit-appearance:none;width:100%;height:22px;background:#363636;cursor:pointer;margin:0}input[type=range]:focus{outline:0}input[type=range]::-webkit-slider-runnable-track{width:100%;height:2px;cursor:pointer;background:#EFEFEF;border-radius:0;border:0 solid #EFEFEF}input[type=range]::-webkit-slider-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer;-webkit-appearance:none;margin-top:-11.5px}input[type=range]:focus::-webkit-slider-runnable-track{background:#EFEFEF}input[type=range]::-moz-range-track{width:100%;height:2px;cursor:pointer;background:#EFEFEF;border-radius:0;border:0 solid #EFEFEF}input[type=range]::-moz-range-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer}input[type=range]::-ms-track{width:100%;height:2px;cursor:pointer;background:0 0;border-color:transparent;color:transparent}input[type=range]::-ms-fill-lower{background:#EFEFEF;border:0 solid #EFEFEF;border-radius:0}input[type=range]::-ms-fill-upper{background:#EFEFEF;border:0 solid #EFEFEF;border-radius:0}input[type=range]::-ms-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer;height:2px}input[type=range]:focus::-ms-fill-lower{background:#EFEFEF}input[type=range]:focus::-ms-fill-upper{background:#363636}.switch{display:block;position:relative;line-height:22px;font-size:16px;height:22px}.switch input{outline:0;opacity:0;width:0;height:0}.slider{width:50px;height:22px;border-radius:22px;cursor:pointer;background-color:grey}.slider,.slider:before{display:inline-block;transition:.4s}.slider:before{position:relative;content:"";border-radius:50%;height:16px;width:16px;left:4px;top:3px;background-color:#fff}input:checked+.slider{background-color:#ff3034}input:checked+.slider:before{-webkit-transform:translateX(26px);transform:translateX(26px)}select{border:1px solid #363636;font-size:14px;height:22px;outline:0;border-radius:5px}.image-container{position:relative;min-width:160px}.close{position:absolute;right:5px;top:5px;background:#ff3034;width:16px;height:16px;border-radius:100px;color:#fff;text-align:center;line-height:18px;cursor:pointer}.hidden{display:none}
        </style>
        <script src="https://lib.imjoy.io/imjoy-loader.js"></script>
    </head>
    <body>
    <figure>
      <div id="stream-container" class="image-container hidden">
        <div class="close" id="close-stream">×</div>
        <img id="stream" src="">
      </div>
    </figure>
        <section class="main">
            <section id="buttons">
                <table>
                <tr><td align="center"><button id="get-still">Get Still</button></td><td><td>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp</td><td align="center"><button id="toggle-stream">Start Stream</button></td></tr>
                <tr><td align="center"><button id="perform-stack">Stack</button></td><td><td>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp</td><td align="center"><button id="get-stack">Perform Stack</button></td></tr>
                <tr><td></td><td align="center">&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<td></td><td></td></tr>
                <tr><td align="center">&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp</td><td align="center">&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<td></td><td align="center">&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp</td></tr>
                <tr><td></td><td align="center" colspan="2"></br></tr>
                <tr><td></td><td align="center" colspan="2"></br></tr>
                <tr><td></td><td align="center" colspan="2"></br></tr>
                <tr><td>Flash</td><td align="center" colspan="2"><input type="range" id="flash" min="0" max="32768" value="0" onchange="try{fetch(document.location.origin+'/control?var=flash&val='+this.value);}catch(e){}"><td></td></tr>
                <tr><td>LENS</td><td align="center" colspan="2"><input type="range" id="lens" min="0" max="32768" value="0" onchange="try{fetch(document.location.origin+'/control?var=lens&val='+this.value);}catch(e){}"><td></td></tr>
                <tr><td>Resolution</td><td align="center" colspan="2"><input type="range" id="framesize" min="0" max="9" value="5" onchange="try{fetch(document.location.origin+'/control?var=framesize&val='+this.value);}catch(e){}"><td></td></tr>
                <tr><td>Effect</td><td align="center" colspan="2"><input type="range" id="effect" min="0" max="5" value="2" onchange="try{fetch(document.location.origin+'/control?var=effect&val='+this.value);}catch(e){}"><td></td></tr>
                <tr><td>Gain</td><td align="center" colspan="2"><input type="range" id="gain" min="0" max="30" value="0" onchange="try{fetch(document.location.origin+'/control?var=gain&val='+this.value);}catch(e){}"><td></td></tr>
                <tr><td>Exposure Time</td><td align="center" colspan="2"><input type="range" id="exposuretime" min="0" max="1600" value="100" onchange="try{fetch(document.location.origin+'/control?var=exposuretime&val='+this.value);}catch(e){}"><td></td></tr>
                <tr><td>Quality</td><td align="center" colspan="2"><input type="range" id="quality" min="10" max="63" value="10" onchange="try{fetch(document.location.origin+'/control?var=quality&val='+this.value);}catch(e){}"><td></td></tr>              
                <tr><td>Timelapse Interval (0..180s; 0 => off) </td><td align="center" colspan="2"><input type="range" id="timelapse" min="0" max="180" value="0" onchange="try{fetch(document.location.origin+'/control?var=timelapseinterval&val='+this.value);}catch(e){}"><td></td></tr>
                <tr><td align="center"><a href="#" onclick="sendToImageJ()"> <img alt="ImJoy" src="https://ij.imjoy.io/assets/badge/open-in-imagej-js-badge.svg" alt="Send To ImJoy"></td></tr>
                </table>
            </section>         
        </section>

        <div id="window-container"></div>
        <div id="menu-container"></div>
        <script>
            loadImJoyBasicApp({
                process_url_query: true,
                show_window_title: false,
                show_progress_bar: true,
                show_empty_window: true,
                menu_style: { position: "absolute", right: 0, top: "2px" },
                window_style: {width: '100%', height: '100%'},
                main_container: null,
                menu_container: "menu-container",
                window_manager_container: "window-container",
                imjoy_api: { } // override some imjoy API functions here
            }).then(async app => {
                // get the api object from the root plugin
                const api = app.imjoy.api;
                // if you want to let users to load new plugins, add a menu item
                app.addMenuItem({
                    label: "➕ Load Plugin",
                    callback() {
                    const uri = prompt(
                        `Please type a ImJoy plugin URL`,
                        "https://github.com/imjoy-team/imjoy-plugins/blob/master/repository/ImageAnnotator.imjoy.html"
                    );
                    if (uri) app.loadPlugin(uri);
                    },
                });
                
                window.sendToImageJ = async function(){
                    const imageURL = document.location.origin
                    const response = await fetch(`${imageURL}/capture.jpeg?_cb=${Date.now()}`);
                    const bytes = await response.arrayBuffer();
                    // if you want a windows displayed in a draggable rezisable grid layout
                    let ij = await api.getWindow("ImageJ.JS")
                    if(!ij){
                        ij = await api.createWindow({
                            src: "https://ij.imjoy.io",
                            name: "ImageJ.JS",
                            fullscreen: true,
                            // window_id: "imagej-window", // if you want to display imagej in a specific window, place a div with this id in the html
                        });
                    }
                    else{
                        await ij.show();
                    }
                    // https://github.com/imjoy-team/imagej.js#viewimageimg_array-config
                    await ij.viewImage(bytes, {name: 'image.jpeg'})
                }
            });
        </script>
        <script>
            document.addEventListener('DOMContentLoaded', function() {
                function b(B) {
                    let C;
                    switch (B.type) {
                        case 'checkbox':
                            C = B.checked ? 1 : 0;
                            break;
                        case 'range':
                        case 'select-one':
                            C = B.value;
                            break;
                        case 'button':
                        case 'submit':
                            C = '1';
                            break;
                        default:
                            return;
                    }
                    const D = `${c}/control?var=${B.id}&val=${C}`;
                    fetch(D).then(E => {
                        console.log(`request to ${D} finished, status: ${E.status}`)
                    })
                }
                var c = document.location.origin;
                const e = B => {
                        B.classList.add('hidden')
                    },
                    f = B => {
                        B.classList.remove('hidden')
                    },
                    g = B => {
                        B.classList.add('disabled'), B.disabled = !0
                    },
                    h = B => {
                        B.classList.remove('disabled'), B.disabled = !1
                    },
                    i = (B, C, D) => {
                        D = !(null != D) || D;
                        let E;
                        'checkbox' === B.type ? (E = B.checked, C = !!C, B.checked = C) : (E = B.value, B.value = C), D && E !== C ? b(B) : !D && ('aec' === B.id ? C ? e(v) : f(v) : 'agc' === B.id ? C ? (f(t), e(s)) : (e(t), f(s)) : 'awb_gain' === B.id ? C ? f(x) : e(x) : 'face_recognize' === B.id && (C ? h(n) : g(n)))
                    };
                document.querySelectorAll('.close').forEach(B => {
                    B.onclick = () => {
                        e(B.parentNode)
                    }
                }), fetch(`${c}/status`).then(function(B) {
                    return B.json()
                }).then(function(B) {
                    document.querySelectorAll('.default-action').forEach(C => {
                        i(C, B[C.id], !1)
                    })
                });
                const j = document.getElementById('stream'),
                    k = document.getElementById('stream-container'),
                    l = document.getElementById('get-still'),
                    m = document.getElementById('toggle-stream'),
                    n = document.getElementById('face_enroll'),
                    o = document.getElementById('close-stream'),
                    p = () => {
                        window.stop(), m.innerHTML = 'Start Stream'
                    },
                    q = () => {
                        j.src = `${c+':81'}/stream.mjpeg`, f(k), m.innerHTML = 'Stop Stream'
                    },
                    getStack = () =>{
                      try{
                        fetch(document.location.origin+'/getStack');
                        }
                      catch(e){}
                    },
                    r = document.getElementById('get-stack'),;
                l.onclick = () => {
                    p()
                    j.src = `${c}/capture.jpeg?_cb=${Date.now()}`
                    downloadImage(c)
                    f(k)
                }, o.onclick = () => {
                    p(), e(k)
                }, m.onclick = () => { //toggle stream
                    const B = 'Stop Stream' === m.innerHTML;
                    B ? p() : q()
                }, r.onclick = () => { // stack
                    getStack() 
                }, n.onclick = () => {
                    b(n)
                }, document.querySelectorAll('.default-action').forEach(B => {
                    B.onchange = () => b(B)
                });
                const r = document.getElementById('agc'),
                    s = document.getElementById('agc_gain-group'),
                    t = document.getElementById('gainceiling-group');
                r.onchange = () => {
                    b(r), r.checked ? (f(t), e(s)) : (e(t), f(s))
                };
                const u = document.getElementById('aec'),
                    v = document.getElementById('aec_value-group');
                u.onchange = () => {
                    b(u), u.checked ? e(v) : f(v)
                };
                const w = document.getElementById('awb_gain'),
                    x = document.getElementById('wb_mode-group');
                w.onchange = () => {
                    b(w), w.checked ? f(x) : e(x)
                };
                const A = document.getElementById('framesize');
                A.onchange = () => {
                    b(A), 5 < A.value && (i(y, !1), i(z, !1))
                };
            });

            async function downloadImage(c){
                // Using fetch
                const image = await fetch(`${c}/capture.jpeg?_cb=${Date.now()}`)
                const imageBlog = await image.blob()
                const imageURL = URL.createObjectURL(imageBlog)
          
                const link = document.createElement('a')
                link.href = imageURL
                link.download = "micrograph_"+Date.now()+".jpg"
                document.body.appendChild(link)
                link.click()
                document.body.removeChild(link)
          
              }
          
        </script>
    </body>
</html>
)rawliteral";
