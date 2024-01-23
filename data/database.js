var Socket;
var running = false;
var dropsLeft = 0;
var timerID = null;
var intervalID = null;
const defaultValues = ['2000', '500', '100', '100', '1', '0', '0', '0'];
const elementIds = ['plsw', 'cmdel', 'dpdel', 'dpnum', 'trvl', 'tilt', 'resPos', 'genPos'];

window.onload = function(event){
  Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
  Socket.onopen = function(event) {};
  Socket.onmessage = function(event) {receive_data(event)};
  Socket.onerror = function(error) {};
}

function updateDrops(){
  var dropNum = document.getElementById('dpnum').innerHTML;
  dropNum = parseInt(dropNum.innerHTML, 10) - 1;
  dropsLeft--;
}

function onMove(isGo){
  if(!running && isGo){
    send_data();
    running = true;
    changeBtnState('stop', 'go', '#FF0000');
    setTimeout(function() {changeBtnState('go', 'stop', '#4CAF50');}, 3000);
    sendBtnData("/go");
  } else {
    sendBtnData("/stop");
    if(timerID != null){clearTimeout(timerID);}
    changeBtnState('go', 'stop', '#4CAF50');
    running = false;
  }
}

function changeBtnState(onButton, offButton, color){
  var btn1 = document.getElementById(onButton);
  var btn2 = document.getElementById(offButton);
  btn1.style.backgroundColor = color;
  btn1.style.cursor = 'default';
  btn1.disabled = false;
  btn2.style.backgroundColor = '#ADAEB3';
  btn2.style.cursor = 'not-allowed';
  btn2.disabled = true;
}

function sendBtnData(btn){
  var xhttp = new XMLHttpRequest();
  xhttp.open('PUT', btn, false);
  xhttp.send();
}

function send_data() {
  let result = '';
  for (let i = 1 ; i <= defaultValues.length ; i++) {
    let value = document.getElementById(`textbox${i}`).value;
    result += 'p' + (/\d/.test(value) ? value : defaultValues[i - 1]);
  }
  Socket.send(result);
}

function receive_data(event) {
  if(event.data === 'reached'){ 
    running = false;
    changeBtnState('go', 'stop');
  } else {
    const myArray = event.data.split(' ');
    var wrapper = document.querySelector(".wrapper");
    var checkbox = document.getElementById("camera");
    var toggleCam = document.getElementById('toggle-cam');
    var toggleGen = document.getElementById('toggle-gen');
    if (myArray.length >= 2) {
      
      // Update Generator Toggle Data
      const isGenOn = myArray[0] === '1';
      toggleGen.innerHTML = isGenOn ? 'On' : 'Off';
      toggleGen.style.backgroundColor = isGenOn ? 'green' : 'red';
      wrapper.classList.toggle("start", isGenOn);

      // Update Camera Toggle Data
      const isCamOn = myArray[1] === '1';
      toggleCam.innerHTML = isCamOn ? 'On' : 'Off';
      toggleCam.style.backgroundColor = isCamOn ? 'green' : 'red';
      checkbox.checked = isCamOn;

      // Update Other Elements
      elementIds.forEach((id, index) => {
        document.getElementById(id).innerHTML = myArray[index + 2];
      });
      let difference = document.getElementById('genPos').innerHTML - 
                       document.getElementById('resPos').innerHTML;
      document.getElementById('mainPos').innerHTML = Math.abs(difference);
    }
    const dropDelay = document.getElementById('dpdel').innerHTML;
    if(toggleGen.innerHTML === 'On') {
      if(intervalID === null){
        dropsLeft = myArray[3];
        setInterval(updateDrops, parseInt((dropDelay) + 10), dropNum);
      }
    } else if (dropsLeft <= 0 || toggleGen.innerHTML === 'Off'){
      if(intervalID !== null){
        clearInterval(intervalID);
        intervalID = null;
      }
    }
  }
};

/*                Graph code              */
