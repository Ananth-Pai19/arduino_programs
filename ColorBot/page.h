#pragma once
// Phone control page (const -> stored in flash on the R4).
const char PAGE_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
<title>ColourBot</title>
<style>
  :root { --red:#e5484d; --green:#30a46c; --blue:#3e63dd; --bg:#111418; --card:#1c2026; --line:#2c323a; --text:#eceef1; --dim:#8b949e; }
  * { box-sizing:border-box; -webkit-tap-highlight-color:transparent; }
  body { margin:0; font-family:system-ui,-apple-system,sans-serif; background:var(--bg); color:var(--text); }
  main { max-width:460px; margin:0 auto; padding:14px 14px 40px; }
  header { display:flex; justify-content:space-between; align-items:center; margin-bottom:12px; }
  h1 { font-size:20px; margin:0; }
  .conn { font-size:12px; padding:4px 10px; border-radius:99px; background:#2a1d1d; color:#f2a5a5; }
  .conn.ok { background:#16261d; color:#8fd5a6; }
  .card { background:var(--card); border:1px solid var(--line); border-radius:14px; padding:14px; margin-bottom:12px; }
  .label { font-size:12px; color:var(--dim); text-transform:uppercase; letter-spacing:.06em; margin-bottom:8px; }
  .status-msg { font-size:18px; font-weight:600; min-height:24px; }
  .chips { display:flex; gap:8px; margin-top:10px; flex-wrap:wrap; }
  .chip { font-size:12px; padding:4px 10px; border-radius:99px; background:#262b33; color:var(--dim); }
  .chip.on { background:#3a2f12; color:#f5cf6b; }
  .steps { display:flex; gap:4px; margin-top:12px; }
  .steps div { flex:1; height:6px; border-radius:3px; background:#2c323a; }
  .steps div.done { background:#8fd5a6; } .steps div.now { background:#f5cf6b; }
  .steplabels { display:flex; gap:4px; margin-top:4px; font-size:10px; color:var(--dim); }
  .steplabels span { flex:1; text-align:center; }
  .row { display:grid; grid-template-columns:repeat(3,1fr); gap:8px; }
  .row + .label { margin-top:14px; }
  button { font:inherit; border:0; border-radius:12px; padding:16px 8px; font-size:16px; font-weight:600; color:#fff; cursor:pointer; transition:transform .05s, opacity .15s; }
  button:active { transform:scale(.97); }
  button:disabled { opacity:.35; }
  .c { background:#262b33; }
  .c.red { color:var(--red); } .c.green { color:var(--green); } .c.blue { color:var(--blue); }
  .c.sel.red { background:var(--red); color:#fff; } .c.sel.green { background:var(--green); color:#fff; } .c.sel.blue { background:var(--blue); color:#fff; }
  .start { width:100%; margin-top:14px; background:#3e63dd; font-size:18px; }
  .stop { width:100%; background:var(--red); font-size:22px; padding:22px; letter-spacing:.08em; margin-bottom:12px; }
  .err { color:#f2a5a5; font-size:14px; margin-top:8px; min-height:18px; }
</style>
</head>
<body>
<main>
  <header><h1>ColourBot</h1><span id="conn" class="conn">connecting…</span></header>

  <div class="card">
    <div class="label">Status</div>
    <div class="status-msg" id="msg">—</div>
    <div class="steps" id="steps"><div></div><div></div><div></div><div></div><div></div></div>
    <div class="steplabels"><span>find</span><span>pick</span><span>carry</span><span>drop</span><span>home</span></div>
    <div class="chips">
      <span class="chip" id="jobChip">no mission</span>
      <span class="chip" id="magChip">magnet off</span>
    </div>
    <div class="err" id="err"></div>
  </div>

  <button class="stop" onclick="api('/stop')">STOP</button>

  <div class="card">
    <div class="label">1 · Pick up from</div>
    <div class="row" id="pickRow"></div>
    <div class="label">2 · Drop at</div>
    <div class="row" id="dropRow"></div>
    <button class="start" id="startBtn" onclick="startMission()">Start mission</button>
  </div>
</main>

<script>
const COLOURS = ['red','green','blue'];
let pick = null, drop = null, busy = false, lastOk = 0;

function makeRow(id, onTap) {
  const row = document.getElementById(id);
  COLOURS.forEach(c => {
    const b = document.createElement('button');
    b.className = 'c ' + c; b.textContent = c[0].toUpperCase() + c.slice(1); b.dataset.c = c;
    b.onclick = () => onTap(c);
    row.appendChild(b);
  });
}
makeRow('pickRow', c => { pick = c; if (drop === c) drop = null; render(); });
makeRow('dropRow', c => { drop = c; if (pick === c) pick = null; render(); });

function render() {
  document.querySelectorAll('#pickRow .c').forEach(b => b.classList.toggle('sel', b.dataset.c === pick));
  document.querySelectorAll('#dropRow .c').forEach(b => b.classList.toggle('sel', b.dataset.c === drop));
  document.getElementById('startBtn').disabled = busy || !pick || !drop;
}

function startMission() { api('/mission?pick=' + pick + '&drop=' + drop); }

async function api(path) {
  const err = document.getElementById('err');
  if (navigator.vibrate) navigator.vibrate(20);
  try {
    const r = await fetch(path, { method: 'POST' });
    const j = await r.json();
    if (!r.ok) { err.textContent = j.error || 'Error'; return; }
    err.textContent = ''; show(j);
  } catch (e) { err.textContent = 'No response from bot'; }
}

const STEP = { sweep_pick:0, pick:1, sweep_drop:2, drop:3, home:4 };
function show(s) {
  lastOk = Date.now();
  busy = s.busy;
  document.getElementById('msg').textContent = s.msg;
  document.getElementById('jobChip').textContent = s.pick ? (s.pick + ' → ' + s.drop).toLowerCase() : 'no mission';
  const mc = document.getElementById('magChip');
  mc.textContent = s.magnet ? 'magnet ON' : 'magnet off'; mc.classList.toggle('on', s.magnet);
  const n = STEP[s.state];
  [...document.getElementById('steps').children].forEach((d, i) => {
    d.className = n === undefined ? (s.msg.startsWith('Delivered') ? 'done' : '') : (i < n ? 'done' : i === n ? 'now' : '');
  });
  render();
}

async function poll() {
  try { const r = await fetch('/status'); show(await r.json()); } catch (e) {}
  // The arm sometimes pauses ~3 s (homing), so allow 5 s before calling it disconnected.
  const ok = Date.now() - lastOk < 5000, c = document.getElementById('conn');
  c.textContent = ok ? 'connected' : 'disconnected'; c.classList.toggle('ok', ok);
  setTimeout(poll, 500);
}
render(); poll();
</script>
</body>
</html>
)rawliteral";
