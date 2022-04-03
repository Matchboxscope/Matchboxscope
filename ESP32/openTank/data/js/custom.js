// Use this for tests on local machine without websocket server
var test = true;

var targetUrl = "ws://" + window.location.host + ":82";

// Keyboard
var pressedKeys = {};
window.onkeyup = function (e) { pressedKeys[e.keyCode] = false; }
window.onkeydown = function (e) { pressedKeys[e.keyCode] = true; }

var websocket;
window.addEventListener('load', onLoad);

const view = document.getElementById('stream');
const buttonDwnl = document.getElementById('buttonDownload')

var setr = [0, 0];
let joystick;

var d = new Date();
var time = d.getMilliseconds();

function onLoad() {
  //joystick = new JoystickController("stick", 64, 8);
  initializeSockets();
}

function initializeSockets() {
  console.log('Opening WebSocket connection to OpenTank ESP32...');

  if (!test) {
    websocket = new WebSocket(targetUrl);

    websocket.onopen = event => {
      console.log('Starting connection to server..');
    };

    websocket.onmessage = message => {
      if (message.data instanceof Blob) {
        // console.log("WebSocket image received")
        var urlObject = URL.createObjectURL(message.data);
        view.src = urlObject;
      }

      d = new Date();
      var ping = d.getMilliseconds() - time;
    
      document.getElementById('stat').innerHTML = 'ping: ' + ping + 'ms';
    };

    websocket.onclose = message => {
      console.log('camWebSocket Closing connection to server..');
      setTimeout(initializeSockets, 2000);
    };
  }

  setInterval(loop, 100);
}

function loop() {
  // Get data from joystick
  setr[0] = joystick.value.x * 100;
  setr[1] = -joystick.value.y * 100;

  // Get data from keyboard ONE KEY PER TIME
  // Keyboard control override joystick
  // WASD = 87, 65, 83, 68
  if (pressedKeys[87] == true) { // W
    setr[1] = 100;
  }
  else if (pressedKeys[83] == true) { // S
    setr[1] = -100;
  }

  if (pressedKeys[65] == true) { // A
    setr[0] = -100;
  }
  else if (pressedKeys[68] == true) { // D
    setr[0] = 100;
  }

  d = new Date();
  time = d.getMilliseconds();

  // Send data to tank
  if (!test) {
    websocket.send(setr);
  }
}
