var Socket;
var ecg
var duration;
var pulse = 1000;
var frm = null;

function mainToggle(btn){
  var xhttp = new XMLHttpRequest();
  xhttp.open('PUT', btn, false);
  xhttp.send(); Socket.send('u');
}

function mtr(x) {
  if (x === 0) {Socket.send('z');}    //Calibrate
  if (x === 1) {Socket.send('t');}    //Up reservoir
  if (x === 2) {Socket.send('g');}    //Up generator
  if (x === 3) {Socket.send('y');}    //Down reservoir
  if (x === 4) {Socket.send('h');}    //Down generator
  if (x === 5) {Socket.send('b');}    //Down both
  if (x === 6) {Socket.send('o');}    //Up both
}

function send_data() {
  let value1 = document.getElementById('textbox1').value;
  let value2 = document.getElementById('textbox2').value;
  let value3 = document.getElementById('textbox3').value;
  let value4 = document.getElementById('textbox4').value;
  let value5 = document.getElementById('textbox5').value;
  if (value1 == ''){value1 = '2000';}
  if (value2 == ''){value2 = '500';}
  if (value3 == ''){value3 = '100';}
  if (value4 == ''){value4 = '100';}
  if (value5 == ''){value5 = '1';}
  let result ='p'+value1+'p'+value2+'p'+value3+'p'+value4+'p'+value5;
  Socket.send(result);
}

function receive_data(event) {
  var wrapper = document.querySelector(".wrapper");
  var checkbox = document.getElementById("camera");
  var toggleCam = document.getElementById('toggle-cam');
  var toggleGen = document.getElementById('toggle-gen');
  const myArray = event.data.split(' ');
  if(myArray.length >= 2){
    //Incoming generator toggle data
    if(myArray[0]=='0'){
      toggleGen.innerHTML='Off';
      toggleGen.style.backgroundColor='red';
      wrapper.classList.add("start");
      ecg.speed = 0;
    } else {
      toggleGen.innerHTML='On';
      toggleGen.style.backgroundColor='green';
      wrapper.classList.remove("start");
      ecg.speed = duration;}

    //Incoming camera toggle data
    if(myArray[1]=='0'){
      toggleCam.innerHTML='Off';
      toggleCam.style.backgroundColor='red';
      checkbox.checked=false;
    } else {
      toggleCam.innerHTML='On';
      toggleCam.style.backgroundColor='green';
      checkbox.checked=true;}
    document.getElementById('plsw').innerHTML = myArray[2];
    document.getElementById('cmdel').innerHTML = myArray[3];
    document.getElementById('dpdel').innerHTML = myArray[4];
    document.getElementById('dpnum').innerHTML = myArray[5];
    document.getElementById('trvl').innerHTML = myArray[6];
    pulse = myArray[2];
    duration = myArray[4] / 1000;}
};

/*                Graph code              */
var ECGPulse = function() {
  this.demo = document.getElementById('graph');
  this.ctx = this.demo.getContext('2d');
  this.w = this.demo.getBoundingClientRect().width;
  this.h = this.demo.getBoundingClientRect().height;
  this.move = true;
  this.px = 0;
  this.opx = 0;
  this.py = this.h;
  this.opy = this.py;
  this.speed = 1;
  this.scanBarWidth = 15;
  this.frame = null;
  this.demo.width = this.w;
  this.demo.height = this.h;
  this.ctx.strokeStyle = '#000000';
  this.ctx.lineWidth = 2;
  this.ctx.beginPath();
  this.ctx.moveTo(10, this.h - 24);
  this.ctx.lineTo(this.w, this.h - 24);
  this.ctx.stroke();
  this.ctx.beginPath();
  this.ctx.moveTo(11, 25);
  this.ctx.lineTo(11, this.h - 25);
  this.ctx.stroke();
  this.ctx.fillStyle = 'black';
  this.ctx.font = '12px Arial';
  this.ctx.fillText('255', 0, 20);
  this.ctx.fillText('0', 5, this.h - 10); 
  this.ctx.fillText(pulse/1000 + ' µs', this.w - 30, this.h - 10);
  this.ctx.strokeStyle = '#f94449';
  this.ctx.lineWidth = 5;
  
  this.loop = function() {
      if(this.move){
      this.px += this.speed * 2.4243;  //length duration to seconds.
      this.ctx.clearRect(this.px + 10, 0, this.scanBarWidth, this.h - 195);
      this.ctx.beginPath();
      this.ctx.moveTo(this.opx + 12, this.opy - 28);
      this.ctx.lineTo(this.px + 12, this.py - 28);
      this.ctx.stroke();
      if(this.px >= 90 && this.px <= 100){
        this.py = 55;
        this.ctx.lineTo(this.px + 12, this.py - 30);
        this.ctx.stroke();
      }
      if(this.px >= 300 && this.px <= 310){
        this.py = 200;
        this.ctx.lineTo(this.px + 12, this.py - 26);
        this.ctx.stroke();
      }
      this.opx = this.px;
      this.opy = this.py;
      if(this.opx > this.w){this.px = this.opx = -this.speed; this.move = false;};
      this.frame = requestAnimationFrame(this.loop.bind(this));
    }
  };
};

window.onload = function(event){
  Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
  Socket.onopen = function(event) {Socket.send('u');};
  Socket.onmessage = function(event) {receive_data(event);};
  Socket.onerror = function(error) {};
  ecg = new ECGPulse();
  ecg.speed = 5;
  document.addEventListener('keydown', function(event) {
    if(event.key === 'w'){
      ecg.move = true;
      if(ecg.frame != null){ecg.frame.loop()} else {ecg.loop()};
    }
  }); 
}