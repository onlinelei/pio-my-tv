#ifndef CONFIG_PORTAL_HTML_H
#define CONFIG_PORTAL_HTML_H

#include <Arduino.h>

// Web 配置页面 HTML（PROGMEM 存储在 Flash 中）- 中文界面
static const char CONFIG_PORTAL_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1">
<title>MyTV 设备配置</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,'PingFang SC','Microsoft YaHei',sans-serif;
  background:#0a0a0a;color:#e0e0e0;min-height:100vh;display:flex;
  justify-content:center;align-items:flex-start;padding:20px}
.card{background:#1a1a1a;border-radius:16px;padding:32px;width:100%;max-width:420px;
  box-shadow:0 8px 32px rgba(0,0,0,.5)}
h1{text-align:center;font-size:24px;margin-bottom:8px;color:#fff}
.subtitle{text-align:center;font-size:13px;color:#888;margin-bottom:28px}
.section{margin-bottom:20px}
label{display:block;font-size:13px;color:#aaa;margin-bottom:6px;font-weight:500}
input[type=text],input[type=password],select{width:100%;padding:12px 14px;
  background:#2a2a2a;border:1px solid #444;border-radius:10px;color:#fff;
  font-size:15px;outline:none;transition:border .2s}
input:focus,select:focus{border-color:#4a9eff}
.scan-btn{width:100%;padding:10px;background:#2a2a2a;border:1px solid #555;
  border-radius:10px;color:#4a9eff;font-size:13px;cursor:pointer;margin-top:6px;
  transition:background .2s}
.scan-btn:hover{background:#333}
.scan-btn:disabled{color:#666;cursor:not-allowed}
.pwd-wrap{position:relative}
.pwd-toggle{position:absolute;right:12px;top:50%;transform:translateY(-50%);
  background:none;border:none;color:#888;cursor:pointer;font-size:18px;padding:4px}
.slider-wrap{display:flex;align-items:center;gap:12px}
input[type=range]{flex:1;-webkit-appearance:none;height:6px;background:#333;
  border-radius:3px;outline:none}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:22px;height:22px;
  background:#4a9eff;border-radius:50%;cursor:pointer}
.slider-val{min-width:40px;text-align:center;font-size:15px;font-weight:600;color:#4a9eff}
.switch-wrap{display:flex;align-items:center;gap:12px}
.switch{position:relative;width:48px;height:26px;cursor:pointer}
.switch input{opacity:0;width:0;height:0}
.slider-track{position:absolute;top:0;left:0;right:0;bottom:0;background:#333;
  border-radius:13px;transition:.3s}
.slider-track:before{content:'';position:absolute;width:20px;height:20px;
  left:3px;bottom:3px;background:#888;border-radius:50%;transition:.3s}
.switch input:checked+.slider-track{background:#1a5a2a}
.switch input:checked+.slider-track:before{transform:translateX(22px);background:#4caf50}
.save-btn{width:100%;padding:14px;background:linear-gradient(135deg,#4a9eff,#2d7cd6);
  border:none;border-radius:12px;color:#fff;font-size:16px;font-weight:600;
  cursor:pointer;margin-top:8px;transition:opacity .2s}
.save-btn:hover{opacity:.9}
.save-btn:disabled{opacity:.5;cursor:not-allowed}
.status{text-align:center;margin-top:16px;padding:12px;border-radius:10px;
  font-size:14px;display:none}
.status.success{display:block;background:#1a3a1a;color:#4caf50;border:1px solid #2a5a2a}
.status.error{display:block;background:#3a1a1a;color:#f44336;border:1px solid #5a2a2a}
.status.scanning{display:block;background:#1a2a3a;color:#4a9eff;border:1px solid #2a4a6a}
.loading{display:inline-block;width:16px;height:16px;border:2px solid #4a9eff;
  border-top-color:transparent;border-radius:50%;animation:spin .8s linear infinite;
  vertical-align:middle;margin-right:6px}
@keyframes spin{to{transform:rotate(360deg)}}
select option{background:#2a2a2a;color:#fff}
.wifi-item{display:flex;justify-content:space-between;align-items:center}
.wifi-rssi{font-size:11px;color:#888}
</style>
</head>
<body>
<div class="card">
  <h1>MyTV 设备配置</h1>
  <p class="subtitle">首次使用向导</p>

  <div class="section">
    <label>WiFi 网络</label>
    <select id="ssid"><option value="">-- 请选择或在下方手动输入 --</option></select>
    <button class="scan-btn" id="scanBtn" onclick="scanWifi()">扫描附近 WiFi</button>
    <input type="text" id="ssidManual" placeholder="或手动输入 WiFi 名称"
      style="margin-top:8px" autocomplete="off">
  </div>

  <div class="section">
    <label>WiFi 密码</label>
    <div class="pwd-wrap">
      <input type="password" id="password" placeholder="请输入 WiFi 密码" autocomplete="off">
      <button class="pwd-toggle" onclick="togglePwd()">&#128065;</button>
    </div>
  </div>

  <div class="section">
    <label>设备名称</label>
    <input type="text" id="devName" value="MyTV" placeholder="例如：客厅电视"
      autocomplete="off">
  </div>

  <div class="section">
    <label>屏幕亮度</label>
    <div class="slider-wrap">
      <input type="range" id="brightness" min="1" max="100" value="80"
        oninput="document.getElementById('briVal').textContent=this.value+'%'">
      <span class="slider-val" id="briVal">80%</span>
    </div>
  </div>

  <div class="section">
    <label>时区</label>
    <select id="timezone">
      <option value="-12">UTC-12 (贝克岛)</option>
      <option value="-8">UTC-8 (洛杉矶)</option>
      <option value="-5">UTC-5 (纽约)</option>
      <option value="0">UTC+0 (伦敦)</option>
      <option value="1">UTC+1 (柏林)</option>
      <option value="3">UTC+3 (莫斯科)</option>
      <option value="5.5">UTC+5:30 (孟买)</option>
      <option value="8" selected>UTC+8 (上海/北京)</option>
      <option value="9">UTC+9 (东京)</option>
      <option value="10">UTC+10 (悉尼)</option>
      <option value="12">UTC+12 (奥克兰)</option>
    </select>
  </div>

  <div class="section">
    <label>OTA 自动更新</label>
    <div class="switch-wrap">
      <label class="switch">
        <input type="checkbox" id="otaEnabled" checked>
        <span class="slider-track"></span>
      </label>
      <span id="otaLabel">已启用</span>
    </div>
  </div>

  <button class="save-btn" id="saveBtn" onclick="saveConfig()">保存并重启</button>
  <div class="status" id="status"></div>
</div>

<script>
const $=id=>document.getElementById(id);

$('ssidManual').addEventListener('input',function(){
  $('ssid').value='';
});
$('ssid').addEventListener('change',function(){
  if(this.value)$('ssidManual').value='';
});
$('otaEnabled').addEventListener('change',function(){
  $('otaLabel').textContent=this.checked?'已启用':'已禁用';
});

function togglePwd(){
  const p=$('password');
  p.type=p.type==='password'?'text':'password';
}

function setStatus(msg,cls){
  const s=$('status');
  s.className='status '+cls;
  s.innerHTML=msg;
}

async function scanWifi(){
  const btn=$('scanBtn');
  btn.disabled=true;
  btn.textContent='正在扫描...';
  setStatus('<span class="loading"></span>正在扫描附近的 WiFi 网络...','scanning');
  try{
    const r=await fetch('/scan');
    const nets=await r.json();
    const sel=$('ssid');
    sel.innerHTML='<option value="">-- 请选择网络 --</option>';
    nets.sort((a,b)=>b.rssi-a.rssi);
    nets.forEach(n=>{
      const opt=document.createElement('option');
      opt.value=n.ssid;
      const bars=n.rssi>-50?'&#9679;&#9679;&#9679;':n.rssi>-70?'&#9679;&#9679;':'&#9679;';
      opt.innerHTML=n.ssid+' <span class="wifi-rssi">'+n.rssi+'dB '+bars+'</span>';
      sel.appendChild(opt);
    });
    $('status').style.display='none';
  }catch(e){
    setStatus('扫描失败: '+e.message,'error');
  }
  btn.disabled=false;
  btn.textContent='扫描附近 WiFi';
}

async function saveConfig(){
  const ssid=$('ssid').value||$('ssidManual').value.trim();
  const pwd=$('password').value;
  if(!ssid){setStatus('请选择或输入 WiFi 名称','error');return;}

  $('saveBtn').disabled=true;
  setStatus('<span class="loading"></span>正在保存配置...','scanning');

  const data={
    ssid:ssid,
    password:pwd,
    deviceName:$('devName').value.trim()||'MyTV',
    brightness:parseInt($('brightness').value),
    timezone:parseInt($('timezone').value),
    otaEnabled:$('otaEnabled').checked
  };

  try{
    const r=await fetch('/save',{
      method:'POST',
      headers:{'Content-Type':'application/json'},
      body:JSON.stringify(data)
    });
    if(r.ok){
      setStatus('配置已保存! 设备将在 3 秒后重启...','success');
      setTimeout(()=>{setStatus('正在重启... 请连接到您的 WiFi 网络。','success');},3000);
    }else{
      setStatus('保存失败: HTTP '+r.status,'error');
      $('saveBtn').disabled=false;
    }
  }catch(e){
    setStatus('保存失败: '+e.message,'error');
    $('saveBtn').disabled=false;
  }
}
</script>
</body>
</html>
)rawliteral";

#endif // CONFIG_PORTAL_HTML_H
#ifndef CONFIG_PORTAL_HTML_H
#define CONFIG_PORTAL_HTML_H

#include <Arduino.h>

// Web 配置页面 HTML（PROGMEM 存储在 Flash 中）
static const char CONFIG_PORTAL_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1">
<title>MyTV Setup</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;
  background:#0a0a0a;color:#e0e0e0;min-height:100vh;display:flex;
  justify-content:center;align-items:flex-start;padding:20px}
.card{background:#1a1a1a;border-radius:16px;padding:32px;width:100%;max-width:420px;
  box-shadow:0 8px 32px rgba(0,0,0,.5)}
h1{text-align:center;font-size:24px;margin-bottom:8px;color:#fff}
.subtitle{text-align:center;font-size:13px;color:#888;margin-bottom:28px}
.section{margin-bottom:20px}
label{display:block;font-size:13px;color:#aaa;margin-bottom:6px;font-weight:500}
input[type=text],input[type=password],select{width:100%;padding:12px 14px;
  background:#2a2a2a;border:1px solid #444;border-radius:10px;color:#fff;
  font-size:15px;outline:none;transition:border .2s}
input:focus,select:focus{border-color:#4a9eff}
.scan-btn{width:100%;padding:10px;background:#2a2a2a;border:1px solid #555;
  border-radius:10px;color:#4a9eff;font-size:13px;cursor:pointer;margin-top:6px;
  transition:background .2s}
.scan-btn:hover{background:#333}
.scan-btn:disabled{color:#666;cursor:not-allowed}
.pwd-wrap{position:relative}
.pwd-toggle{position:absolute;right:12px;top:50%;transform:translateY(-50%);
  background:none;border:none;color:#888;cursor:pointer;font-size:18px;padding:4px}
.slider-wrap{display:flex;align-items:center;gap:12px}
input[type=range]{flex:1;-webkit-appearance:none;height:6px;background:#333;
  border-radius:3px;outline:none}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:22px;height:22px;
  background:#4a9eff;border-radius:50%;cursor:pointer}
.slider-val{min-width:40px;text-align:center;font-size:15px;font-weight:600;color:#4a9eff}
.switch-wrap{display:flex;align-items:center;gap:12px}
.switch{position:relative;width:48px;height:26px;cursor:pointer}
.switch input{opacity:0;width:0;height:0}
.slider-track{position:absolute;top:0;left:0;right:0;bottom:0;background:#333;
  border-radius:13px;transition:.3s}
.slider-track:before{content:'';position:absolute;width:20px;height:20px;
  left:3px;bottom:3px;background:#888;border-radius:50%;transition:.3s}
.switch input:checked+.slider-track{background:#1a5a2a}
.switch input:checked+.slider-track:before{transform:translateX(22px);background:#4caf50}
.save-btn{width:100%;padding:14px;background:linear-gradient(135deg,#4a9eff,#2d7cd6);
  border:none;border-radius:12px;color:#fff;font-size:16px;font-weight:600;
  cursor:pointer;margin-top:8px;transition:opacity .2s}
.save-btn:hover{opacity:.9}
.save-btn:disabled{opacity:.5;cursor:not-allowed}
.status{text-align:center;margin-top:16px;padding:12px;border-radius:10px;
  font-size:14px;display:none}
.status.success{display:block;background:#1a3a1a;color:#4caf50;border:1px solid #2a5a2a}
.status.error{display:block;background:#3a1a1a;color:#f44336;border:1px solid #5a2a2a}
.status.scanning{display:block;background:#1a2a3a;color:#4a9eff;border:1px solid #2a4a6a}
.loading{display:inline-block;width:16px;height:16px;border:2px solid #4a9eff;
  border-top-color:transparent;border-radius:50%;animation:spin .8s linear infinite;
  vertical-align:middle;margin-right:6px}
@keyframes spin{to{transform:rotate(360deg)}}
select option{background:#2a2a2a;color:#fff}
.wifi-item{display:flex;justify-content:space-between;align-items:center}
.wifi-rssi{font-size:11px;color:#888}
</style>
</head>
<body>
<div class="card">
  <h1>MyTV Setup</h1>
  <p class="subtitle">Device Configuration Wizard</p>

  <div class="section">
    <label>WiFi Network</label>
    <select id="ssid"><option value="">-- Select or type below --</option></select>
    <button class="scan-btn" id="scanBtn" onclick="scanWifi()">Scan Networks</button>
    <input type="text" id="ssidManual" placeholder="Or enter SSID manually"
      style="margin-top:8px" autocomplete="off">
  </div>

  <div class="section">
    <label>WiFi Password</label>
    <div class="pwd-wrap">
      <input type="password" id="password" placeholder="Enter password" autocomplete="off">
      <button class="pwd-toggle" onclick="togglePwd()">&#128065;</button>
    </div>
  </div>

  <div class="section">
    <label>Device Name</label>
    <input type="text" id="devName" value="MyTV" placeholder="e.g. LivingRoom-TV"
      autocomplete="off">
  </div>

  <div class="section">
    <label>Screen Brightness</label>
    <div class="slider-wrap">
      <input type="range" id="brightness" min="1" max="100" value="80"
        oninput="document.getElementById('briVal').textContent=this.value+'%'">
      <span class="slider-val" id="briVal">80%</span>
    </div>
  </div>

  <div class="section">
    <label>Timezone</label>
    <select id="timezone">
      <option value="-12">UTC-12 (Baker Island)</option>
      <option value="-8">UTC-8 (Los Angeles)</option>
      <option value="-5">UTC-5 (New York)</option>
      <option value="0">UTC+0 (London)</option>
      <option value="1">UTC+1 (Berlin)</option>
      <option value="3">UTC+3 (Moscow)</option>
      <option value="5.5">UTC+5:30 (Mumbai)</option>
      <option value="8" selected>UTC+8 (Shanghai)</option>
      <option value="9">UTC+9 (Tokyo)</option>
      <option value="10">UTC+10 (Sydney)</option>
      <option value="12">UTC+12 (Auckland)</option>
    </select>
  </div>

  <div class="section">
    <label>OTA Auto Update</label>
    <div class="switch-wrap">
      <label class="switch">
        <input type="checkbox" id="otaEnabled" checked>
        <span class="slider-track"></span>
      </label>
      <span id="otaLabel">Enabled</span>
    </div>
  </div>

  <button class="save-btn" id="saveBtn" onclick="saveConfig()">Save & Restart</button>
  <div class="status" id="status"></div>
</div>

<script>
const $=id=>document.getElementById(id);

$('ssidManual').addEventListener('input',function(){
  $('ssid').value='';
});
$('ssid').addEventListener('change',function(){
  if(this.value)$('ssidManual').value='';
});
$('otaEnabled').addEventListener('change',function(){
  $('otaLabel').textContent=this.checked?'Enabled':'Disabled';
});

function togglePwd(){
  const p=$('password');
  p.type=p.type==='password'?'text':'password';
}

function setStatus(msg,cls){
  const s=$('status');
  s.className='status '+cls;
  s.innerHTML=msg;
}

async function scanWifi(){
  const btn=$('scanBtn');
  btn.disabled=true;
  btn.textContent='Scanning...';
  setStatus('<span class="loading"></span>Scanning for WiFi networks...','scanning');
  try{
    const r=await fetch('/scan');
    const nets=await r.json();
    const sel=$('ssid');
    sel.innerHTML='<option value="">-- Select network --</option>';
    nets.sort((a,b)=>b.rssi-a.rssi);
    nets.forEach(n=>{
      const opt=document.createElement('option');
      opt.value=n.ssid;
      const bars=n.rssi>-50?'&#9679;&#9679;&#9679;':n.rssi>-70?'&#9679;&#9679;':'&#9679;';
      opt.innerHTML=n.ssid+' <span class="wifi-rssi">'+n.rssi+'dB '+bars+'</span>';
      sel.appendChild(opt);
    });
    $('status').style.display='none';
  }catch(e){
    setStatus('Scan failed: '+e.message,'error');
  }
  btn.disabled=false;
  btn.textContent='Scan Networks';
}

async function saveConfig(){
  const ssid=$('ssid').value||$('ssidManual').value.trim();
  const pwd=$('password').value;
  if(!ssid){setStatus('Please select or enter WiFi SSID','error');return;}

  $('saveBtn').disabled=true;
  setStatus('<span class="loading"></span>Saving configuration...','scanning');

  const data={
    ssid:ssid,
    password:pwd,
    deviceName:$('devName').value.trim()||'MyTV',
    brightness:parseInt($('brightness').value),
    timezone:parseInt($('timezone').value),
    otaEnabled:$('otaEnabled').checked
  };

  try{
    const r=await fetch('/save',{
      method:'POST',
      headers:{'Content-Type':'application/json'},
      body:JSON.stringify(data)
    });
    if(r.ok){
      setStatus('Configuration saved! Device will restart in 3 seconds...','success');
      setTimeout(()=>{setStatus('Restarting... Please reconnect to your WiFi.','success');},3000);
    }else{
      setStatus('Save failed: HTTP '+r.status,'error');
      $('saveBtn').disabled=false;
    }
  }catch(e){
    setStatus('Save failed: '+e.message,'error');
    $('saveBtn').disabled=false;
  }
}
</script>
</body>
</html>
)rawliteral";

#endif // CONFIG_PORTAL_HTML_H
