var Socket;

window.onload = function(event){
  Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
  Socket.onopen = function(event) {sendBtnData("/dropgen")};
  Socket.onmessage = function(event) {receive_data(event)};
  Socket.onerror = function(error) {};
}

function updateDrops(){
  var dropNum = document.getElementById('dpnum');
  dropNum.innerHTML = parseInt(dropNum.innerHTML, 10) - 1;
}

function sendBtnData(btn){
  var xhttp = new XMLHttpRequest();
  xhttp.open('PUT', btn, false);
  xhttp.send();
}

function send_data() {

  let result = '';
  const defaultValues = ['2000', '500', '100', '100', '1', '0', '0', '0'];
  for (let i = 1 ; i <= defaultValues.length ; i++) {
    let value = document.getElementById(`textbox${i}`).value;
    result += 'p' + (value || defaultValues[i - 1]);
  }
  Socket.send(result);
}

function receive_data(event) {
  var intervalID = null;
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
    const elementIds = ['plsw', 'cmdel', 'dpdel', 'dpnum',
                        'trvl', 'tilt', 'resPos', 'genPos'];
    elementIds.forEach((id, index) => {
      document.getElementById(id).innerHTML = myArray[index + 2];
    });
    let difference = document.getElementById('resPos').value - 
                     document.getElementById('genPos').value;
    document.getElementById('mainPos').innerHTML = Math.abs(difference);
  }
  const dropDelay = document.getElementById('dpdel').innerHTML;
  if(toggleGen.innerHTML === 'On') {
    if(intervalID === null){
      setInterval(updateDrops(), parseInt((dropDelay) + 10), 10);
    }
  } else {
    if(intervalID !== null){
      clearInterval(intervalID);
      intervalID = null;
    }
  }
};

/*                Graph code              */

