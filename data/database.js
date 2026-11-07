// DropGen web UI. Buttons call HTTP endpoints; the device pushes its full state over a WebSocket
// after every change (and ~5x a second while the motors move), and this page just mirrors it.

const FIELDS = ['plsw', 'cmdel', 'dpdel', 'dpnum', 'trvl', 'tilt'];  // shown values, in protocol order
let socket = null;
let state = null;

function connect() {
  socket = new WebSocket('ws://' + window.location.hostname + ':81/');
  socket.onopen = () => setStatus('connected', true);
  socket.onmessage = (event) => receiveData(event.data);
  socket.onclose = () => {
    setStatus('disconnected, retrying…', false);
    setTimeout(connect, 2000);
  };
  socket.onerror = () => socket.close();
}

window.addEventListener('load', () => {
  connect();
  window.addEventListener('resize', drawGraph);
  drawGraph();
});

function setStatus(text, ok) {
  const el = document.getElementById('status');
  if (!el) return;
  el.textContent = text;
  el.style.color = ok ? '#2E722E' : '#CC0000';
}

function sendBtnData(path) {
  fetch(path, { method: 'PUT' }).catch(() => setStatus('request failed', false));
}

// Submit sends every setting; an empty box keeps the device's current value rather than a default.
function send_data(isGo = false) {
  if (!state || !socket || socket.readyState !== WebSocket.OPEN) {
    setStatus('not connected', false);
    return;
  }
  const current = [state.pulse, state.camDelay, state.dropDelay, state.dropNum, state.travel, state.tilt];
  let msg = '';
  for (let i = 0; i < 6; i++) {
    const v = parseFloat(document.getElementById(`textbox${i + 1}`).value);
    msg += 'p' + (Number.isFinite(v) ? v : current[i]);
  }
  const res = parseFloat(document.getElementById('textbox7').value);
  const gen = parseFloat(document.getElementById('textbox8').value);
  msg += 'p' + (isGo && Number.isFinite(res) ? res : state.resPos).toFixed(2);
  msg += 'p' + (isGo && Number.isFinite(gen) ? gen : state.genPos).toFixed(2);
  socket.send(msg);
}

function calculateTime(isGo) {
  if (isGo) {
    send_data(true);
    // the settings message travels on the socket; give it a moment to land before moving
    setTimeout(() => sendBtnData('/go'), 100);
  } else {
    sendBtnData('/stop');
  }
}

function styleToggle(button, on) {
  button.innerHTML = on ? 'On' : 'Off';
  button.style.backgroundColor = on ? '#4CAF50' : '#FF0000';
  button.style.boxShadow = on ? ' 0 9px #2E722E' : ' 0 9px #CC0000';
}

function setEnabled(id, enabled, color, shadow) {
  const btn = document.getElementById(id);
  btn.disabled = !enabled;
  btn.style.backgroundColor = enabled ? color : '#ADAEB3';
  btn.style.boxShadow = enabled ? shadow : ' 0 9px #8A8B8F';
  btn.style.cursor = enabled ? 'pointer' : 'not-allowed';
}

// "drop camera pulse camDelay dropDelay dropNum travel tilt resPos genPos dropsLeft moving"
function receiveData(text) {
  const f = text.trim().split(/\s+/).map(Number);
  if (f.length < 12 || f.some(Number.isNaN)) return;
  state = {
    drop: f[0] === 1, camera: f[1] === 1, pulse: f[2], camDelay: f[3], dropDelay: f[4], dropNum: f[5],
    travel: f[6], tilt: f[7], resPos: f[8], genPos: f[9], dropsLeft: f[10], moving: f[11] === 1,
  };

  styleToggle(document.getElementById('toggle-gen'), state.drop);
  document.querySelector('.wrapper').classList.toggle('start', state.drop);
  styleToggle(document.getElementById('toggle-cam'), state.camera);
  document.getElementById('camera').checked = state.camera;

  [state.pulse, state.camDelay, state.dropDelay, state.dropNum, state.travel, state.tilt].forEach((v, i) => {
    document.getElementById(FIELDS[i]).innerHTML = v;
  });
  document.getElementById('resPos').innerHTML = state.resPos.toFixed(2);
  document.getElementById('genPos').innerHTML = state.genPos.toFixed(2);
  document.getElementById('mainPos').innerHTML = Math.abs(state.genPos - state.resPos).toFixed(2);

  const left = document.getElementById('dropsLeft');
  if (left) left.textContent = state.drop ? `${state.dropsLeft} of ${state.dropNum} droplets left` : 'idle';

  setEnabled('go', !state.moving, '#4CAF50', ' 0 9px #2E722E');
  setEnabled('stop', state.moving, '#FF0000', ' 0 9px #CC0000');
  drawGraph();
}

/* Piezo drive waveform: +V for the pulse width, then held at -V until the next droplet.
   The pulse is microseconds and the period milliseconds, so the pulse is drawn wider than to
   scale; the labels carry the real numbers. A tick marks the camera trigger when it's on. */
function drawGraph() {
  const canvas = document.getElementById('graph');
  if (!canvas) return;
  const dpr = window.devicePixelRatio || 1;
  const w = canvas.clientWidth, h = canvas.clientHeight;
  canvas.width = w * dpr;
  canvas.height = h * dpr;
  const g = canvas.getContext('2d');
  g.scale(dpr, dpr);
  g.clearRect(0, 0, w, h);

  const pulse = state ? state.pulse : 1000, period = state ? state.dropDelay : 500;
  const top = 18, bottom = h - 22, mid = (top + bottom) / 2, left = 8, right = w - 8;
  const cycles = 3, cycleW = (right - left) / cycles;
  const pulseW = Math.max(6, Math.min(cycleW * 0.4, cycleW * (pulse / 1000) / period * 40));

  g.strokeStyle = '#DDDDDD';
  g.beginPath();
  g.moveTo(left, mid);
  g.lineTo(right, mid);
  g.stroke();

  g.strokeStyle = '#F94449';
  g.lineWidth = 2;
  g.beginPath();
  g.moveTo(left, bottom);
  for (let c = 0; c < cycles; c++) {
    const x = left + c * cycleW + 4;
    g.lineTo(x, bottom);
    g.lineTo(x, top);
    g.lineTo(x + pulseW, top);
    g.lineTo(x + pulseW, bottom);
  }
  g.lineTo(right, bottom);
  g.stroke();

  if (state && state.camera) {
    g.strokeStyle = '#4C8DFF';
    g.lineWidth = 1.5;
    for (let c = 0; c < cycles; c++) {
      const x = left + c * cycleW + 4 + Math.min(cycleW - 8, pulseW + (state.camDelay / period) * cycleW);
      g.beginPath();
      g.moveTo(x, top - 6);
      g.lineTo(x, top + 8);
      g.stroke();
    }
  }

  g.fillStyle = '#555555';
  g.font = '11px Arial';
  g.fillText(`${pulse} µs`, left + 4, top - 5);
  const label = `${period} ms`;
  g.fillText(label, left + cycleW - g.measureText(label).width / 2, h - 6);
}
