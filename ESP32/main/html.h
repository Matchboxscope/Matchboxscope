#pragma once

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE HTML>
<html>
<h3>Anglerfish</h3>
<span style="font-family: arial, helvetica, sans-serif;"><img id="stream" />
<p>This is the GUI for the Anglerfish / Matchboxscope&nbsp;</p>
<p>If you have questions for its use, please have a look in the <a href="https://github.com/beniroquai/Matchboxscope/" target="_blank" rel="noopener">Github repository</a></p>
<p>If you wish to upload a new firmware (bin), please follow <a href="/" onclick="javascript:event.target.port=82">this link.</a> </p>
</span>
<p><span style="font-family: arial, helvetica, sans-serif;">
<script src="https://lib.imjoy.io/imjoy-loader.js"></script>
</span></p>
<figure>
<div id="stream-container" class="image-container hidden">
<div class="close" id="close-stream"><span style="font-family: arial, helvetica, sans-serif;">&times;</span></div>
<span style="font-family: arial, helvetica, sans-serif;"><img id="stream" /></span></div>
</figure>
<section class="main">
<section id="buttons">
<table style="height: 424px; width: 47.8344%;">
<tbody>
<tr style="height: 41px;">
<td align="center" style="height: 41px; width: 29.663%;"><span style="font-family: arial, helvetica, sans-serif;"><button id="get-still">Get Still</button></span></td>
<td style="height: 41px; width: 24.9576%;"></td>
<td align="center" style="height: 41px; width: 21.1088%;"><span style="font-family: arial, helvetica, sans-serif;"><button id="toggle-stream">Start Stream</button></span></td>
</tr>
<tr style="height: 41px;">
<td align="center" style="height: 41px; width: 29.663%;"><span style="font-family: arial, helvetica, sans-serif;"><button id="perform-stack">Stack</button></span></td>
<td style="height: 41px; width: 24.9576%;"></td>
<td align="center" style="height: 41px; width: 21.1088%;"><span style="font-family: arial, helvetica, sans-serif;"><button id="get-stack">Perform Stack</button></span></td>
</tr>
<tr style="height: 3px;">
<td style="height: 3px; width: 29.663%;"></td>
<td align="center" style="height: 3px; width: 24.9576%;"></td>
<td style="width: 21.1088%; height: 3px;"></td>
</tr>
<tr style="height: 31px;">
<td style="height: 31px; width: 29.663%;"><span style="font-family: arial, helvetica, sans-serif;">Flash</span></td>
<td align="center" style="height: 31px; width: 24.9576%;"><span style="font-family: arial, helvetica, sans-serif;"><input type="range" id="flash" min="0" max="255" value="0" onchange="try{fetch(document.location.origin+'/control?var=flash&amp;val='+this.value);}catch(e){}" /></span></td>
<td style="height: 31px; width: 21.1088%;"></td>
</tr>
<tr style="height: 31px;">
<td style="height: 31px; width: 29.663%;"><span style="font-family: arial, helvetica, sans-serif;">Lens/Pump</span></td>
<td align="center" style="height: 31px; width: 24.9576%;"><span style="font-family: arial, helvetica, sans-serif;"><input type="range" id="lens" min="0" max="255" value="0" onchange="try{fetch(document.location.origin+'/control?var=lens&amp;val='+this.value);}catch(e){}" /></span></td>
<td style="height: 31px; width: 21.1088%;"></td>
</tr>
<tr style="height: 31px;">
<td style="height: 31px; width: 29.663%;"><span style="font-family: arial, helvetica, sans-serif;">Resolution</span></td>
<td align="center" style="height: 31px; width: 24.9576%;"><span style="font-family: arial, helvetica, sans-serif;"><input type="range" id="framesize" min="0" max="9" value="5" onchange="try{fetch(document.location.origin+'/control?var=framesize&amp;val='+this.value);}catch(e){}" /></span></td>
<td style="height: 31px; width: 21.1088%;"></td>
</tr>
<tr style="height: 31px;">
<td style="height: 31px; width: 29.663%;"><span style="font-family: arial, helvetica, sans-serif;">Effect</span></td>
<td align="center" style="height: 31px; width: 24.9576%;"><span style="font-family: arial, helvetica, sans-serif;"><input type="range" id="effect" min="0" max="5" value="2" onchange="try{fetch(document.location.origin+'/control?var=effect&amp;val='+this.value);}catch(e){}" /></span></td>
<td style="height: 31px; width: 21.1088%;"></td>
</tr>
<tr style="height: 31px;">
<td style="height: 31px; width: 29.663%;"><span style="font-family: arial, helvetica, sans-serif;">Gain</span></td>
<td align="center" style="height: 31px; width: 24.9576%;"><span style="font-family: arial, helvetica, sans-serif;"><input type="range" id="gain" min="0" max="30" value="0" onchange="try{fetch(document.location.origin+'/control?var=gain&amp;val='+this.value);}catch(e){}" /></span></td>
<td style="height: 31px; width: 21.1088%;"></td>
</tr>
<tr style="height: 31px;">
<td style="height: 31px; width: 29.663%;"><span style="font-family: arial, helvetica, sans-serif;">Exposure Time</span></td>
<td align="center" style="height: 31px; width: 24.9576%;"><span style="font-family: arial, helvetica, sans-serif;"><input type="range" id="exposuretime" min="0" max="1600" value="100" onchange="try{fetch(document.location.origin+'/control?var=exposuretime&amp;val='+this.value);}catch(e){}" /></span></td>
<td style="height: 31px; width: 21.1088%;"></td>
</tr>
<tr style="height: 31px;">
<td style="height: 31px; width: 29.663%;"><span style="font-family: arial, helvetica, sans-serif;">Quality</span></td>
<td align="center" style="height: 31px; width: 24.9576%;"><span style="font-family: arial, helvetica, sans-serif;"><input type="range" id="quality" min="10" max="63" value="10" onchange="try{fetch(document.location.origin+'/control?var=quality&amp;val='+this.value);}catch(e){}" /></span></td>
<td style="height: 31px; width: 21.1088%;"></td>
</tr>
<tr style="height: 31px;">
<td style="height: 31px; width: 29.663%;"><span style="font-family: arial, helvetica, sans-serif;">Timelapse Interval (0..180s; 0 =&gt; off)</span></td>
<td align="center" style="height: 31px; width: 24.9576%;"><span style="font-family: arial, helvetica, sans-serif;"><input type="range" id="timelapse" min="0" max="180" value="0" onchange="try{fetch(document.location.origin+'/control?var=timelapseinterval&amp;val='+this.value);}catch(e){}" /></span></td>
<td style="height: 31px; width: 21.1088%;"></td>
</tr>
<tr style="height: 27px;">
<td align="center" style="height: 27px; width: 29.663%;"><span style="font-family: arial, helvetica, sans-serif;"><a href="#" onclick="sendToImageJ()"> <img alt="ImJoy" src="https://ij.imjoy.io/assets/badge/open-in-imagej-js-badge.svg" /></a></span></td>
<td style="width: 24.9576%; height: 27px;"></td>
<td style="width: 21.1088%; height: 27px;"></td>
</tr>
</tbody>
</table>
</section>
</section>
<div id="window-container"></div>
<div id="menu-container"></div>
<p><span style="font-family: arial, helvetica, sans-serif;">
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
                    o = document.getElementById('close-stream'),
                    p = () => {
                        window.stop(), m.innerHTML = 'Start Stream'
                    },
                    q = () => {
                        j.src = `${c+':81'}/stream.mjpeg`, f(k), m.innerHTML = 'Stop Stream'
                    },
                    r = document.getElementById('get-stack');
                    l.onclick = () => {
                    p()
                    j.src = `${c}/capture.jpeg?_cb=${Date.now()}`
                    downloadImage(c)
                    f(k)
                },
                o.onclick = () => {
                    p(), e(k)
                },
                m.onclick = () => { //toggle stream
                    const B = 'Stop Stream' === m.innerHTML;
                    B ? p() : q()
                },
                r.onclick = () => { // stack
                    getStack()
                }
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
              };

            async function getStack(){
                try{
                  fetch(document.location.origin+'/getstack');
                  }
                catch(e){}
              };
        </script>
</span></p>
</html>
)rawliteral";




static const char PROGMEM otaindex[] = R"rawliteral(
<!DOCTYPE HTML>
<html>
<p><span style="font-family: tahoma, arial, helvetica, sans-serif;">
<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js"></script>
</span></p>
<form method="POST" action="#" enctype="multipart/form-data" id="upload_form">
<h2><span style="font-family: tahoma, arial, helvetica, sans-serif;"><strong>OTA Updater for the Anglerfish</strong></span></h2>
<p><span style="font-family: tahoma, arial, helvetica, sans-serif;">Please go to the<a href="https://github.com/beniroquai/Matchboxscope/" title="URL" target="_blank" rel="noopener"> Github repository </a>of the Anglerfish and download the latest ".bin" file.&nbsp;&nbsp;</span><span style="font-family: tahoma, arial, helvetica, sans-serif;"></span></p>
<p><span style="font-family: tahoma, arial, helvetica, sans-serif;"><input type="file" name="update" /> <input type="submit" value="Update" /></span></p>
</form>
<div id="prg"><span style="font-family: tahoma, arial, helvetica, sans-serif;"><em>Progress</em>: 0%</span></div>
<p><span style="font-family: tahoma, arial, helvetica, sans-serif;">
<script>
  $('form').submit(function(e){
  e.preventDefault();
  var form = $('#upload_form')[0];
  var data = new FormData(form);
   $.ajax({
  url: '/update',
  type: 'POST',
  data: data,
  contentType: false,
  processData:false,
  xhr: function() {
  var xhr = new window.XMLHttpRequest();
  xhr.upload.addEventListener('progress', function(evt) {
  if (evt.lengthComputable) {
  var per = evt.loaded / evt.total;
  $('#prg').html('progress: ' + Math.round(per*100) + '%');
  }
  }, false);
  return xhr;
  },
  success:function(d, s) {
  console.log('success!')
 },
 error: function (a, b, c) {
 }
 });
 });
 </script>
</span></p>
</html>
)rawliteral";
