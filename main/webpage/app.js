// app.js

const API_HMI_URL    = '/api/hmi';
const API_STATUS_URL = '/api/status';

function log(msg, err = false) {
  const el   = document.getElementById('logs');
  const time = new Date().toLocaleTimeString();
  const line = `[${time}] ${msg}<br>`;
  el.innerHTML = line + el.innerHTML;
  if (err) el.style.color = '#f00';
}

function updateClock() {
  document.getElementById('clock').textContent =
    new Date().toLocaleTimeString('pl-PL');
}
setInterval(updateClock, 1000);
updateClock();

/**
 * Send a command to the ESP32 via HTTP POST
 * @param {string} type – command name
 * @param {number|null} data – optional numeric payload
 */
function sendCmd(type, data = null) {
  const payload = JSON.stringify({ type, data });
  fetch(API_HMI_URL, {
    method:  'POST',
    headers: { 'Content-Type': 'application/json' },
    body:     payload
  })
  .then(resp => {
    if (resp.ok) {
      log(`→ Sent: ${type}${data != null ? ' ' + data : ''}`);
    } else {
      log(`HTTP ${resp.status} sending ${type}`, true);
    }
  })
  .catch(() => log('Fetch error sending HMI command', true));
}

/**
 * Set lamp background color
 * @param {string} id  – one of sensor1,sensor3,wrap_done,robot,inverter
 * @param {boolean} on
 */
function setLamp(id, on) {
  const colors = {
    sensor1:   '#388e3c',
    sensor3:   '#388e3c',
    wrap_done: '#388e3c',
    robot:     '#2962FF',
    inverter:  '#2962FF'
  };
  document.getElementById('lamp_' + id).style.background =
    on ? colors[id] : '#ccc';
}

/**
 * Apply a status object to the UI
 * @param {object} o
 */
function handleMsg(o) {
  if (o.weight !== undefined) {
    document.getElementById('weight').innerText = o.weight.toFixed(2);
  }
  if (o.weightStatus) {
    document.getElementById('weightStatus').innerText = o.weightStatus;
  }
  ['sensor1','sensor3','wrap_done','robot','inverter'].forEach(id => {
    if (o[id] !== undefined) setLamp(id, o[id]);
  });
  if (o.wrapProgress !== undefined) {
    document.getElementById('progressBar').style.width = o.wrapProgress + '%';
    document.getElementById('progress').innerText      = o.wrapProgress + '%';
    document.getElementById('wrapMsg').innerText      =
      o.wrapProgress >= 100 ? 'Owijanie zakończone' : 'Owijanie trwa...';
    document.getElementById('wrapStatus').innerText   =
      o.wrapProgress >= 100 ? 'ZAKOŃCZONE' : 'OWIJA';
  }
  if (o.error) {
    log('⚠️ ERROR: ' + o.error, true);
  }
}

/**
 * Poll the ESP32 for current status
 */
function fetchStatus() {
  fetch(API_STATUS_URL)
    .then(resp => {
      if (!resp.ok) throw new Error(resp.status);
      return resp.json();
    })
    .then(data => handleMsg(data))
    .catch(() => log('Fetch error getting status', true));
}
setInterval(fetchStatus, 1000);
fetchStatus();
