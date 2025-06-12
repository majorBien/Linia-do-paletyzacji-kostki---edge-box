const wsUrl = 'ws://192.168.0.100:81/';
let ws;

function log(msg, err=false) {
  const el = document.getElementById('logs');
  const time = new Date().toLocaleTimeString();
  el.innerHTML = `[${time}] ${msg}<br>` + el.innerHTML;
  if (err) el.style.color = '#f00';
}

function updateClock() {
  document.getElementById('clock').textContent = new Date().toLocaleTimeString('pl-PL');
}
setInterval(updateClock,1000);
updateClock();

function sendCmd(type, data=null) {
  const msg = { type, data };
  if (ws && ws.readyState===WebSocket.OPEN) {
    ws.send(JSON.stringify(msg));
    log(`→ Wysłano: ${type}${data!=null? ' '+data:''}`);
  } else log('Brak połączenia WS!', true);
}

function setLamp(id, on) {
  document.getElementById('lamp_'+id).style.background = on? {'sensor1':'#388e3c','sensor3':'#388e3c','wrap_done':'#388e3c','robot':'#2962FF','inverter':'#2962FF'}[id]: '#ccc';
}

function handleMsg(msg) {
  const o = JSON.parse(msg);
  if (o.weight!==undefined) document.getElementById('weight').innerText = o.weight.toFixed(2);
  if (o.weightStatus) document.getElementById('weightStatus').innerText = o.weightStatus;
  ['sensor1','sensor3','wrap_done','robot','inverter'].forEach(id => {
    if (o[id]!==undefined) setLamp(id, o[id]);
  });
  if (o.wrapProgress!==undefined) {
    document.getElementById('progressBar').style.width = o.wrapProgress+'%';
    document.getElementById('progress').innerText = o.wrapProgress+'%';
    document.getElementById('wrapMsg').innerText = o.wrapProgress>=100 ? 'Owijanie zakończone' : 'Owijanie trwa...';
    document.getElementById('wrapStatus').innerText = o.wrapProgress>=100?'ZAKOŃCZONE':'OWIJA';
  }
  if (o.error) log('⚠️ ERROR: '+o.error, true);
}

function connectWS() {
  ws = new WebSocket(wsUrl);
  ws.onopen = ()=> log('🔌 połączono z ESP32');
  ws.onmessage = e=> handleMsg(e.data);
  ws.onclose = ()=> { log('🔌 rozłączono, ponawianie za 5s...', true); setTimeout(connectWS,5000); };
  ws.onerror = e=> log('Błąd WebSocket', true);
}
connectWS();