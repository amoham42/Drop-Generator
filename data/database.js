var Socket;
var dropsLeft = 0;
var timerID = null;
var intervalID = null;
const defaultValues = ['2000', '500', '100', '100', '1', '0'];
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

function calculateTime(isGo){
  if(isGo){
    let resMotPos = parseFloat(document.getElementById('resPos').innerHTML);
    let genMotPos = parseFloat(document.getElementById('genPos').innerHTML);
    let resMotInp = parseFloat(document.getElementById('textbox7').value);
    let genMotInp = parseFloat(document.getElementById('textbox8').value);
    let resDif = Math.abs(resMotPos - resMotInp);
    let genDif = Math.abs(genMotPos - genMotInp);
    resDif = !isNaN(resDif) ? resDif * 100 : 0.0;
    genDif = !isNaN(genDif) ? genDif * 100 : 0.0;

    return Math.max(resDif, genDif);
  }
}

function onMove(isGo){
  let timeout = calculateTime(true);
  if(isGo && timeout > 0.0){
    send_data(true);
    changeBtnState('stop', 'go', '#FF0000', ' 0 9px #CC0000');
    setTimeout(function() {changeBtnState('go', 'stop', '#4CAF50', ' 0 9px #2E722E');}, timeout);
    sendBtnData("/go");
  } else {
    sendBtnData("/stop");
    if(timerID != null){clearTimeout(timerID);}
    changeBtnState('go', 'stop', '#4CAF50', ' 0 9px #2E722E');
  }
}

function changeBtnState(onButton, offButton, color, shadow){
  var btn1 = document.getElementById(onButton);
  var btn2 = document.getElementById(offButton);
  btn1.style.backgroundColor = color;
  btn1.style.boxShadow = shadow;
  btn1.style.cursor = 'default';
  btn1.disabled = false;
  btn2.style.backgroundColor = '#ADAEB3';
  btn2.style.boxShadow = ' 0 9px #8A8B8F';
  btn2.style.cursor = 'not-allowed';
  btn2.disabled = true;
}

function sendBtnData(btn){
  var xhttp = new XMLHttpRequest();
  xhttp.open('PUT', btn, false);
  xhttp.send();
}

function send_data(isGo = false) {
  let result = '';
  for (let i = 1 ; i <= defaultValues.length ; i++) {
    let value = document.getElementById(`textbox${i}`).value;
    result += 'p' + (/\d/.test(value) ? value : defaultValues[i - 1]);
  }
  if(isGo){
    let resMotPos = document.getElementById('resPos').innerHTML;
    let genMotPos = document.getElementById('genPos').innerHTML;
    result += 'p' + (/\d/.test(resMotPos) ? resMotPos : '0.0');
    result += 'p' + (/\d/.test(genMotPos) ? genMotPos : '0.0');
  } else {
    let resMotPos = document.getElementById('textbox7').value;
    let genMotPos = document.getElementById('textbox8').value;
    result += 'p' + (/\d/.test(resMotPos) ? resMotPos : '0.0');
    result += 'p' + (/\d/.test(genMotPos) ? genMotPos : '0.0');
  }
  
  Socket.send(result);
}

function receive_data(event) {
  
  const myArray = event.data.split(' ');
  var wrapper = document.querySelector(".wrapper");
  var checkbox = document.getElementById("camera");
  var toggleCam = document.getElementById('toggle-cam');
  var toggleGen = document.getElementById('toggle-gen');
  if (myArray.length >= 2) {
    
    // Update Generator Toggle Data
    const isGenOn = myArray[0] === '1';
    toggleGen.innerHTML = isGenOn ? 'On' : 'Off';
    toggleGen.style.backgroundColor = isGenOn ? '#4CAF50' : '#FF0000';
    toggleGen.style.boxShadow = isGenOn ? ' 0 9px #2E722E' : ' 0 9px #CC0000';
    wrapper.classList.toggle("start", isGenOn);

    // Update Camera Toggle Data
    const isCamOn = myArray[1] === '1';
    toggleCam.innerHTML = isCamOn ? 'On' : 'Off';
    toggleCam.style.backgroundColor = isCamOn ? '#4CAF50' : '#FF0000';
    toggleCam.style.boxShadow = isCamOn ? ' 0 9px #2E722E' : ' 0 9px #CC0000';
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
  
};

/*                Graph code              */

