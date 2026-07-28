// ===========================================================================
// 🔵🔴🙏 RAMESHWAR_MAKKHAN_EDITION_PRO (4 ACTIVE SLOTS + FULL SAVE) 🙏🔴🔵
// PART_1- DATE_27_7_2026  — STOP/UN-STOP TOGGLE + STABILITY FIX EDITION
// ===========================================================================

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include <HardwareSerial.h>
#include "DFRobotDFPlayerMini.h"
#include <Preferences.h>         
#include <ArduinoJson.h> 
#include <Firebase_ESP_Client.h> 
#include <time.h>                
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

const char* ssid = "Sarvoparidham_1"; 
const char* password = "Sarvoparidham@2396";

#define API_KEY "AIzaSyDCH0BCvuR1VafR1HaJ_JedVED1pP2ri-k" 
#define FIREBASE_AUTH "TAIC90FZ8B0mbTNC14JaPDT9egCp3R1itRYfGAAP"
#define DATABASE_URL "rameshwar-smaran-default-rtdb.firebaseio.com"

#define ESP_RX 26      
#define ESP_TX 25      
#define DF_BUSY_PIN 33
#define btnPin 27      
#define ledPin 32

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2); 
WebServer server(80);
Preferences prefs;

FirebaseData fbdoCmd;
FirebaseData fbdoStat;
FirebaseAuth fbAuth;
FirebaseConfig fbConfig;

HardwareSerial mySerial(2); 
DFRobotDFPlayerMini myDFPlayer;

#define TOTAL_F4_TRACKS 40
const char* f4N[TOTAL_F4_TRACKS] = {
  "ORDA-SARADHAR","ORA AAVO-GURUJI","ORDA-KUNDAL","MHATMAY CHINTA","SUKH CHINTA MANI","MURTI CHINTA MANI","BLANCK","TERE DWAR apadyo","NVAL SNEHI nathji","Shriji daya karo","Aekalda kem rahevay","Prabhu tam vina","Tum mere swami","Kyare have dekhu","Dukhi divas ne rat","Mara nath mara nath","Aakhadiyo aklay","Dhun_Amrut swami","DHUN VED mandir","JAMO JAMO mara","CHESHTA","RAM & CHESHTA","Track 23","Track 24","Track 25","Track 26","Track 27","Track 28","Track 29","Track 30","Track 31","Track 32","Track 33","Track 34","Track 35","Track 36","Track 37","Track 38","Track 39","Track 40"};

String dW[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

bool rtcOK = false, dfOK = false, trkPly = false, dhunManStop = false, f4Rep = false, f4Shuf = false;
int curVol = 20, svVol = 0, isMut = 0, curF4 = 1, curFld = 0, curTrkNum = 1;
int activeCmd = 0; 
String curMsg = "READY";
unsigned long trkSt = 0;
bool btnTriggered = false;
bool stoppedManually = false;   // 🟢 STOP/UN-STOP ટોગલ માટે 🟢
unsigned long lastNtpSync = 0; 
unsigned long volShowTimer = 0; 

// --- FAST TIME CACHING VARIABLES ---
int g_h = 0, g_m = 0, g_w = 0, g_s = 0;

// સેટિંગ્સ વેરીએબલ્સ
int gPH=18, gPM=50, gPEH=19, gPEM=5, enGD=1;
int sunGH=18, sunGM=50, enSunG=0;
int mDhSH=8, mDhSM=30, mDhEH=12, mDhEM=0, enMDh=1;
int eDhSH=16, eDhSM=0, eDhEH=18, eDhEM=49, enEDh=1;
int dhSH=16, dhSM=0, dhEH=17, dhEM=30, enDS=1;

int hc1SH=7, hc1SM=0, hc1EH=7, hc1EM=15, enHC1=0;
int hc2SH=19, hc2SM=0, hc2EH=19, hc2EM=15, enHC2=0;

// પીન 27 ટચ બટન માટેના 4 ACTIVE સ્લોટ્સ
int un1SH=5, un1SM=0, un1EH=8, un1EM=0, enUn1=1;
int un2SH=18, un2SM=0, un2EH=21, un2EM=0, enUn2=1;
int un3SH=0, un3SM=0, un3EH=0, un3EM=0, enUn3=0;
int un4SH=0, un4SM=0, un4EH=0, un4EM=0, enUn4=0;

TaskHandle_t CloudTask;
volatile bool cloudCmdPending = false;
volatile int activeCloudCmd = 0;
volatile int activeCloudFolder = 0;
volatile int activeCloudTrack = 0;
volatile bool needCloudSync = false;

SemaphoreHandle_t dataMutex;
String sharedStatusJson = "{}";

// ==============================================================================
// 🔴 અહીં તમારો મૂર્તિનો BASE64 કોડ મૂકવો (iconBase64) 🔴
// ==============================================================================
String iconBase64 = "data:image/jpeg;base64,";

const char* offlineHTML PROGMEM = R"=====(
<!DOCTYPE html><html lang="gu"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=0"><title>RAMESHWAR OFFLINE</title>
<style>
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
body{font-family:'Segoe UI',sans-serif;background:#121212;color:#e0e0e0;margin:0;padding-bottom:20px;text-align:center;user-select:none;touch-action:manipulation}
.c{padding:5px;max-width:600px;margin:auto}
.nav-tabs{background:#1a1a1a;padding:12px;display:flex;justify-content:center;gap:15px;position:sticky;top:0;z-index:100;box-shadow:0 4px 10px rgba(0,0,0,0.8)}
.nt-btn{padding:10px 25px;border-radius:20px;font-weight:bold;font-size:14px;background:#2a2a2a;color:#888;border:none;cursor:pointer;transition:all 0.3s}
.nt-btn.act{color:#00e5ff;background:#111;box-shadow:inset 2px 2px 5px #000;}
.card{background:#1e1e1e;border-radius:15px;padding:12px;margin-bottom:12px;box-shadow:0 4px 8px rgba(0,0,0,0.5);border:1px solid #333}
.stat-hdr{font-size: 22px;font-weight: bold;color: #ffffff;text-align:center;text-shadow:0px 0px 15px rgba(255, 50, 100, 0.9),0px 0px 5px rgba(255, 255, 255, 0.6);}
.stat-row{display:flex;justify-content:space-between;align-items:center;margin-bottom:5px;margin-top:5px;padding:0 5px;}
.time-text { font-size: 28px; font-weight: bold; white-space: nowrap; background: linear-gradient(to right, #B38728 0%, #FBF5B7 25%, #B38728 50%, #FBF5B7 75%, #B38728 100%); -webkit-background-clip: text; -webkit-text-fill-color: transparent; filter: drop-shadow(0px 4px 6px rgba(0,0,0,0.6)); }
.god-img { width: 100px; height: 100px; border-radius: 50%; object-fit: cover; border: 2px solid #FFD700; box-shadow: 0px 0px 15px rgba(255, 215, 0, 0.7); }
.badge{font-size:10px;padding:4px 8px;border-radius:5px;font-weight:bold;background:#4caf50;color:#fff;}
.badge.offline{background:#f44336}
.status-msg{color:#00e676;font-weight:bold;font-size:15px;margin-top:8px;text-transform:uppercase; text-align: left; padding-left: 5px; background:#111; padding:8px; border-radius:8px;}
.q-grid{display:grid;grid-template-columns:1fr 1fr;gap:12px;margin-bottom:12px}
.q-btn{border:none;border-radius:12px;padding:12px 5px;color:#fff;font-weight:bold;width:100%;cursor:pointer;font-size:14px;background:#242424; transition:0.2s; box-shadow:0 4px 6px rgba(0,0,0,0.4)}
.b-blu{border-bottom:5px solid #2196f3}.b-org{border-bottom:5px solid #ff9800}.b-yel{border-bottom:5px solid #fbc02d;}.b-grn{border-bottom:5px solid #2980b9}.b-pur{border-bottom:5px solid #9c27b0}
.act-glow { border-bottom: 5px solid #00e676 !important; color: #00e676 !important; background: #111 !important; box-shadow: inset 0 -4px 10px rgba(0,230,118,0.2) !important;}
.t-row{display:flex;align-items:center;justify-content:space-between;margin-bottom:12px}
.sq-btn{width:45px;height:45px;border-radius:12px;background:#242424;color:#00e5ff;border:none;font-size:18px;display:flex;align-items:center;justify-content:center;cursor:pointer}
.t-name{flex:1;background:#111;height:45px;margin:0 10px;border-radius:10px;display:flex;align-items:center;justify-content:center;font-size:14px;color:#00e676;font-weight:bold;}

.cir-row{display:flex;justify-content:space-between;align-items:center; gap:8px;}
.cir-btn{width:50px;height:50px;border-radius:50%;display:flex;align-items:center;justify-content:center;font-size:20px;color:#fff;border:none;background:#242424; cursor:pointer; transition:all 0.3s ease;}
.b-rnd-pur{border-bottom:3px solid #9c27b0; box-shadow: 0px 4px 8px -2px rgba(156,39,176,0.4);}
.b-rnd-org{border-bottom:3px solid #ff9800; box-shadow: 0px 4px 8px -2px rgba(255,152,0,0.4);}
.b-rnd-red{border-bottom:3px solid #f44336; box-shadow: 0px 4px 8px -2px rgba(244,67,54,0.4);}

/* એક્ટિવ (ગ્રીન પટ્ટી) બટન માટે */
.active-btn { border-bottom: 3px solid #00e676 !important; box-shadow: 0px 5px 12px -2px rgba(0,230,118,0.7) !important; color: #00e676 !important; background:#1a1a1a !important; }

/* ફોલ્ડર મોડલ (ઓવરલે) માટે CSS */
.modal-overlay { display:none; position:fixed; top:0; left:0; width:100%; height:100%; background:rgba(0,0,0,0.85); z-index:1000; overflow-y:auto; padding:20px; }
.modal-content { background:#1e1e1e; border-radius:15px; padding:15px; border:2px solid #00e676; box-shadow: 0px 0px 15px rgba(0,230,118,0.3); max-width:600px; margin:auto; margin-bottom:40px; }
.modal-header { display:flex; justify-content:space-between; align-items:center; margin-bottom:15px; border-bottom:1px solid #333; padding-bottom:10px; }
.modal-header h2 { color:#00e676; margin:0; font-size:18px; }
.close-btn { background:#f44336; color:#fff; border:none; padding:8px 15px; border-radius:8px; font-weight:bold; cursor:pointer; }
.track-list-grid { display:flex; flex-direction:column; gap:8px; }
.modal-trk-btn { padding:12px; background:#111; color:#00e5ff; border:1px solid #333; border-radius:8px; font-weight:bold; font-size:14px; text-align:left; cursor:pointer; transition:0.2s; }
.modal-trk-btn:active { background:#00e676; color:#000; }

.c-row{display:flex;justify-content:center;align-items:center;margin-bottom:12px; gap: 8px;}
.tile-btn{flex:1; padding:10px 0;border-radius:12px;display:flex;flex-direction:column;align-items:center;font-size:11px;font-weight:bold;color:#aaa;background:#242424;border:none;}
.vol-box{width:60px;height:45px;background:#111;border-radius:12px;display:flex;align-items:center;justify-content:center;font-size:20px;font-weight:bold;color:#fbc02d;}
.st-tabs { display:flex; justify-content:center; gap:10px; margin-bottom:15px; }
.st-tab { background:#333; color:#aaa; padding:10px 25px; border-radius:25px; font-weight:bold; font-size:13px; cursor:pointer;}
.st-tab.active { background:#ff9800; color:#fff; }
.s-card { background:#222; border-radius:12px; padding:15px; margin-bottom:12px; text-align:left; border-left:6px solid #555; position:relative; }
.sc-grn { border-left-color: #4caf50; } .sc-yel { border-left-color: #fbc02d; }
.sc-pur { border-left-color: #9c27b0; } .sc-org { border-left-color: #ff9800; } .sc-red { border-left-color: #f44336; }
.s-title { font-size: 15px; font-weight: bold; color: #ddd; display: flex; justify-content: space-between; align-items: center; margin-bottom: 15px; }
.switch { position: relative; display: inline-block; width: 46px; height: 24px; }
.switch input { opacity: 0; width: 0; height: 0; }
.slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #555; border-radius: 24px; transition: .3s; }
.slider:before { position: absolute; content: ""; height: 18px; width: 18px; left: 3px; bottom: 3px; background-color: white; border-radius: 50%; transition: .3s; }
input:checked + .slider { background-color: #4caf50; }
input:checked + .slider:before { transform: translateX(22px); }
.t-sel-row { display: flex; align-items: center; justify-content: space-between; margin-bottom: 8px; color:#aaa; font-size:13px; font-weight:bold; }
.sel-box { display: flex; gap: 5px; }
.sel-input { background: #333; color: #fff; border: 1px solid #444; border-radius: 6px; padding: 6px; font-weight: bold; font-size: 14px; outline:none; text-align:center; appearance: none; -webkit-appearance: none; }
.s-btn { background: #4caf50; color: #fff; font-weight: bold; border: none; padding: 15px; border-radius: 12px; width: 100%; font-size: 16px; cursor: pointer; margin-top: 10px; }
#toast{visibility:hidden;background:#fff;color:#000;padding:12px 25px;position:fixed;bottom:80px;left:50%;transform:translateX(-50%);border-radius:30px;z-index:99;font-weight:bold;}
.toast-show{visibility:visible!important;animation:fade 2s}
@keyframes fade{0%{opacity:0}20%{opacity:1}80%{opacity:1}100%{opacity:0}}
</style>
</head>
<body>
<div id="fldModal" class="modal-overlay">
  <div class="modal-content">
    <div class="modal-header">
      <h2>&#128194; SELECT TRACK</h2>
      <button class="close-btn" onclick="closeFolder()">✖ CLOSE</button>
    </div>
    <div class="track-list-grid">
      <script>
        const f4T = ["ORDA-SARADHAR","ORA AAVO-GURUJI","ORDA-KUNDAL","MHATMAY CHINTA","SUKH CHINTA MANI","MURTI CHINTA MANI","BLANCK","TERE DWAR apadyo","NVAL SNEHI nathji","Shriji daya karo","Aekalda kem rahevay","Prabhu tam vina","Tum mere swami","Kyare have dekhu","Dukhi divas ne rat","Mara nath mara nath","Aakhadiyo aklay","Dhun_Amrut swami","DHUN VED mandir","JAMO JAMO mara","CHESHTA","RAM & CHESHTA","Track 23","Track 24","Track 25","Track 26","Track 27","Track 28","Track 29","Track 30","Track 31","Track 32","Track 33","Track 34","Track 35","Track 36","Track 37","Track 38","Track 39","Track 40"];
        for(let i=0; i<40; i++) {
            document.write('<button class="modal-trk-btn" onclick="playSelectedTrack('+(i+1)+')">'+(i+1)+'. '+f4T[i]+'</button>');
        }
      </script>
    </div>
  </div>
</div>

<div class="nav-tabs">
  <button class="nt-btn act" id="tab-dash" onclick="switchView('dash')">CONTROL</button>
  <button class="nt-btn" id="tab-sett" onclick="switchView('sett')">SETTING</button>
</div>
<div class="c">
  <div id="view-dash">
    <div class="card">
      <div class="stat-hdr">&#128308;&#128591; RAMESHWAR &#128591;&#128308;</div>
      <div class="stat-row"> 
        <div id="clock" class="time-text">--:--</div>
        <img id="godImg" class="god-img" src="" alt="God">
        <div class="badge-col"><span id="netBadge" class="badge">LOCAL</span></div>
      </div>
      <div id="statusMsg" class="status-msg">WAITING...</div>
    </div>

    <div class="q-grid">
      <button id="btn-3" class="q-btn b-blu" onclick="cmd(3)">Mangla</button>
      <button id="btn-8" class="q-btn b-org" onclick="cmd(8)">Arti & Jay</button>
      <button id="btn-4" class="q-btn b-yel" onclick="cmd(4)">Today's Arti</button>
      <button id="btn-5" class="q-btn b-grn" onclick="cmd(5)">Today's Godi</button>
      <button id="btn-6" class="q-btn b-pur" onclick="cmd(6)">Dhun Morning</button>
      <button id="btn-7" class="q-btn b-pur" onclick="cmd(7)">Dhun Evening</button>
      <button id="btn-12" class="q-btn b-org" onclick="cmd(12)">H. Chalisa 1</button>
      <button id="btn-13" class="q-btn b-org" onclick="cmd(13)">H. Chalisa 2</button>
    </div>
      
    <div class="card">
      <div class="t-row">
        <button class="sq-btn b-blu" onclick="cmd(19)">&#9198;</button>
        <div class="t-name" id="trackName">-</div>
        <button class="sq-btn b-blu" onclick="cmd(18)">&#9197;</button>
      </div>

      <div class="cir-row" style="margin-top: 20px; margin-bottom: 10px;">
        <button id="btnFld" class="cir-btn b-rnd-pur" onclick="openFolder()">&#128193;</button>
        <button id="btnShuf" class="cir-btn b-rnd-org" onclick="cmd(21)">&#128256;</button>
        <button id="btnRep" class="cir-btn b-rnd-org" onclick="cmd(20)">&#128257;</button>
        <button id="btnMute" class="cir-btn b-rnd-pur" onclick="cmd(22)">&#128266;</button>
        <!-- સ્ટોપ બટન: 1st ક્લિક = STOP, 2nd ક્લિક = UN-STOP (મૂળ પોજીશન) -->
        <button id="btnStop" class="cir-btn b-rnd-red" onclick="cmd(2)">&#9209;</button>
      </div>
    </div>
    
    <div class="card">
      <div class="c-row" style="margin-bottom:0">
        <button class="tile-btn" style="border-bottom:4px solid #f44336;" onclick="cmd(17)">&#128171; RESTART</button>
        <button class="tile-btn" style="border-bottom:4px solid #2196f3;" onclick="cmd(10)">&#10134; VOL -</button>
        <div class="vol-box" id="volNum">--</div>
        <button class="tile-btn" style="border-bottom:4px solid #2196f3;" onclick="cmd(9)">&#10133; VOL +</button>
      </div>
    </div>  
 <div style='margin-top:25px;font-size:14px;color:#888;font-weight:bold;letter-spacing:1px;'>RAMESHWAR MAKKHAN EDITION PRO (SMARAN V2)</div> 
  </div> 
   <div id="view-sett" style="display:none;">
    <div style="color:#00e5ff; font-weight:bold; font-size:16px; margin-bottom:15px;">&#9881; SETTINGS PANEL</div>
    <div class="st-tabs">
      <div id="tab-mt" class="st-tab active" onclick="swT('mt')">MAIN TIMER</div>
      <div id="tab-ls" class="st-tab" onclick="swT('ls')">4 ACTIVE SLOTS</div>
    </div>

    <div id="sec-mt">
      <script>
        function drawOpts(type) {
            let h = "";
            if(type=='H') { for(let i=1;i<=12;i++) h+=`<option value="${i}">${i<10?'0'+i:i}</option>`; }
            if(type=='M') { for(let i=0;i<60;i++) h+=`<option value="${i}">${i<10?'0'+i:i}</option>`; }
            if(type=='A') { h=`<option value=" AM"> AM</option><option value=" PM"> PM</option>`; }
            return h;
        }
        function drawCard(id, title, color, hasEnd=true) {
            document.write(`
            <div class="s-card ${color}">
              <div class="s-title"><span style="${color==='sc-grn'?'color:#4caf50;':(color==='sc-yel'?'color:#fbc02d;':(color==='sc-pur'?'color:#9c27b0;':(color==='sc-red'?'color:#f44336;':'color:#ff9800;')))}">${title}</span> <label class="switch"><input type="checkbox" id="en_${id}"><span class="slider"></span></label></div>
              <div class="t-sel-row">
                <span>START</span>
                <div class="sel-box">
                  <select class="sel-input" id="${id}SH">${drawOpts('H')}</select> <span style="color:#fff">:</span>
                  <select class="sel-input" id="${id}SM">${drawOpts('M')}</select>
                  <select class="sel-input" id="${id}SA">${drawOpts('A')}</select>
                </div>
              </div>
              ${hasEnd ? `
              <div class="t-sel-row">
                <span>STOP</span>
                <div class="sel-box">
                  <select class="sel-input" id="${id}EH">${drawOpts('H')}</select> <span style="color:#fff">:</span>
                  <select class="sel-input" id="${id}EM">${drawOpts('M')}</select>
                  <select class="sel-input" id="${id}EA">${drawOpts('A')}</select>
                </div>
              </div>` : ''}
            </div>`);
        }
        drawCard('gd', 'Auto Godi', 'sc-grn');
        drawCard('sun', 'Sunday Godi', 'sc-grn', false);
        drawCard('md', 'Morning Dhun', 'sc-yel');
        drawCard('ed', 'Evening Dhun', 'sc-pur');
        drawCard('sd', 'Sunday Dhun', 'sc-pur');
        drawCard('hc1', 'H. Chalisa 1', 'sc-org');
        drawCard('hc2', 'H. Chalisa 2', 'sc-org');
      </script>
    </div>

    <div id="sec-ls" style="display:none;">
      <script>
        drawCard('u1', 'Active Slot 1', 'sc-red');
        drawCard('u2', 'Active Slot 2', 'sc-red');
        drawCard('u3', 'Active Slot 3', 'sc-red');
        drawCard('u4', 'Active Slot 4', 'sc-red');
      </script>
    </div>
    <button class="s-btn" onclick="saveSettings()">💾 SAVE & SYNC</button>
  </div>
</div>
<div id="toast"></div>

<script>
let isSending = false;
function toast(m) { let t=document.getElementById("toast"); t.innerText=m; t.className="toast-show"; setTimeout(()=>t.className="",2500); }
function switchView(v) { 
  document.getElementById('view-dash').style.display = v==='dash'?'block':'none'; 
  document.getElementById('view-sett').style.display = v==='sett'?'block':'none'; 
  document.getElementById('tab-dash').classList.toggle('act',v==='dash'); 
  document.getElementById('tab-sett').classList.toggle('act',v==='sett'); 
  if(v==='sett') loadSettingsIntoUI();
}
function swT(t) {
    document.getElementById('sec-mt').style.display = t==='mt'?'block':'none';
    document.getElementById('sec-ls').style.display = t==='ls'?'block':'none';
    document.getElementById('tab-mt').className = t==='mt' ? 'st-tab active' : 'st-tab';
    document.getElementById('tab-ls').className = t==='ls' ? 'st-tab active' : 'st-tab';
}

function openFolder() { document.getElementById('fldModal').style.display = 'block'; }
function closeFolder() { document.getElementById('fldModal').style.display = 'none'; }
function playSelectedTrack(trackNum) { cmd(1, 4, trackNum); closeFolder(); }

function cmd(a,f=0,t=0) {
  if(isSending) return; isSending = true;
  fetch(`/cmd?action=${a}&folder=${f}&track=${t}`).then(()=> { toast("Sent: "+a); isSending = false; }).catch(()=>isSending=false);
}

function setT(id, h24, m) {
    let ampm = h24 >= 12 ? ' PM' : ' AM';
    let h12 = h24 % 12; if(h12===0) h12 = 12;
    if(document.getElementById(id+'H')) document.getElementById(id+'H').value = h12;
    if(document.getElementById(id+'M')) document.getElementById(id+'M').value = m;
    if(document.getElementById(id+'A')) document.getElementById(id+'A').value = ampm;
}
function getT(id) {
    let h = parseInt(document.getElementById(id+'H').value);
    let m = parseInt(document.getElementById(id+'M').value);
    let a = document.getElementById(id+'A').value;
    if(a===' PM' && h<12) h+=12;
    if(a===' AM' && h===12) h=0;
    return {h, m};
}

function loadSettingsIntoUI() {
  fetch('/get_settings').then(r=>r.json()).then(s=>{
    document.getElementById("en_gd").checked = s.enGD; setT('gdS', s.gPH, s.gPM); setT('gdE', s.gPEH, s.gPEM);
    document.getElementById("en_sun").checked = s.enSunG; setT('sunS', s.sunGH, s.sunGM);
    document.getElementById("en_md").checked = s.enMDh; setT('mdS', s.mDhSH, s.mDhSM); setT('mdE', s.mDhEH, s.mDhEM);
    document.getElementById("en_ed").checked = s.enEDh; setT('edS', s.eDhSH, s.eDhSM); setT('edE', s.eDhEH, s.eDhEM);
    document.getElementById("en_sd").checked = s.enDS; setT('sdS', s.dhSH, s.dhSM); setT('sdE', s.dhEH, s.dhEM);
    document.getElementById("en_hc1").checked = s.enHC1; setT('hc1S', s.hc1SH, s.hc1SM); setT('hc1E', s.hc1EH, s.hc1EM);
    document.getElementById("en_hc2").checked = s.enHC2; setT('hc2S', s.hc2SH, s.hc2SM); setT('hc2E', s.hc2EH, s.hc2EM);
    document.getElementById("en_u1").checked = s.enUn1; setT('u1S', s.un1SH, s.un1SM); setT('u1E', s.un1EH, s.un1EM);
    document.getElementById("en_u2").checked = s.enUn2; setT('u2S', s.un2SH, s.un2SM); setT('u2E', s.un2EH, s.un2EM);
    document.getElementById("en_u3").checked = s.enUn3; setT('u3S', s.un3SH, s.un3SM); setT('u3E', s.un3EH, s.un3EM);
    document.getElementById("en_u4").checked = s.enUn4; setT('u4S', s.un4SH, s.un4SM); setT('u4E', s.un4EH, s.un4EM);
  });
}

function saveSettings() {
  let gdS=getT('gdS'), gdE=getT('gdE'), sunS=getT('sunS');
  let mdS=getT('mdS'), mdE=getT('mdE'), edS=getT('edS'), edE=getT('edE'), sdS=getT('sdS'), sdE=getT('sdE');
  let h1S=getT('hc1S'), h1E=getT('hc1E'), h2S=getT('hc2S'), h2E=getT('hc2E');
  let u1S=getT('u1S'), u1E=getT('u1E'), u2S=getT('u2S'), u2E=getT('u2E'), u3S=getT('u3S'), u3E=getT('u3E'), u4S=getT('u4S'), u4E=getT('u4E');

  let url = `/save_settings?enGD=${document.getElementById("en_gd").checked?1:0}&gPH=${gdS.h}&gPM=${gdS.m}&gPEH=${gdE.h}&gPEM=${gdE.m}` +
            `&enSunG=${document.getElementById("en_sun").checked?1:0}&sunGH=${sunS.h}&sunGM=${sunS.m}` +
            `&enMDh=${document.getElementById("en_md").checked?1:0}&mDhSH=${mdS.h}&mDhSM=${mdS.m}&mDhEH=${mdE.h}&mDhEM=${mdE.m}` +
            `&enEDh=${document.getElementById("en_ed").checked?1:0}&eDhSH=${edS.h}&eDhSM=${edS.m}&eDhEH=${edE.h}&eDhEM=${edE.m}` +
            `&enDS=${document.getElementById("en_sd").checked?1:0}&dhSH=${sdS.h}&dhSM=${sdS.m}&dhEH=${sdE.h}&dhEM=${sdE.m}` +
            `&enHC1=${document.getElementById("en_hc1").checked?1:0}&hc1SH=${h1S.h}&hc1SM=${h1S.m}&hc1EH=${h1E.h}&hc1EM=${h1E.m}` +
            `&enHC2=${document.getElementById("en_hc2").checked?1:0}&hc2SH=${h2S.h}&hc2SM=${h2S.m}&hc2EH=${h2E.h}&hc2EM=${h2E.m}` +
            `&enUn1=${document.getElementById("en_u1").checked?1:0}&un1SH=${u1S.h}&un1SM=${u1S.m}&un1EH=${u1E.h}&un1EM=${u1E.m}` +
            `&enUn2=${document.getElementById("en_u2").checked?1:0}&un2SH=${u2S.h}&un2SM=${u2S.m}&un2EH=${u2E.h}&un2EM=${u2E.m}` +
            `&enUn3=${document.getElementById("en_u3").checked?1:0}&un3SH=${u3S.h}&un3SM=${u3S.m}&un3EH=${u3E.h}&un3EM=${u3E.m}` +
            `&enUn4=${document.getElementById("en_u4").checked?1:0}&un4SH=${u4S.h}&un4SM=${u4S.m}&un4EH=${u4E.h}&un4EM=${u4E.m}`;
  fetch(url).then(()=>{ toast("Settings Saved!"); switchView('dash'); });
}

// 🔴 ડેસ્કટોપ આઈકન અને ફેવિકોન ડાયનેમિક સેટ કરવાનું લોજીક 🔴
fetch('/god_icon').then(r=>r.text()).then(src=>{ 
  if(src) { 
    document.getElementById('godImg').src = src; 
    let link = document.createElement('link'); link.rel = 'icon'; link.type = 'image/jpeg'; link.href = src; document.head.appendChild(link);
    let apple = document.createElement('link'); apple.rel = 'apple-touch-icon'; apple.href = src; document.head.appendChild(apple);
  } 
});

setInterval(() => {
  fetch('/status').then(r => r.json()).then(d => {
    document.getElementById("clock").innerText = d.time;
    document.getElementById("statusMsg").innerText = d.statusMsg;
    document.getElementById("trackName").innerText = d.statusMsg;
    document.getElementById("volNum").innerText = d.volume;
    let nb=document.getElementById('netBadge');
    if(d.online){nb.className='badge';nb.innerText='CLOUD';}else{nb.className='badge offline';nb.innerText='LOCAL';}
    
    // બટન સ્ટેટસ સિંકિંગ 
    let bs = document.getElementById("btnShuf"); if(d.isShuffle) bs.classList.add("active-btn"); else bs.classList.remove("active-btn");
    let br = document.getElementById("btnRep"); if(d.isRepeat) br.classList.add("active-btn"); else br.classList.remove("active-btn");
    let bm = document.getElementById("btnMute"); if(d.isMute) bm.classList.add("active-btn"); else bm.classList.remove("active-btn");
    let bstop = document.getElementById("btnStop"); if(!d.playing) bstop.classList.add("active-btn"); else bstop.classList.remove("active-btn");

    const activeBtns = [3, 8, 4, 5, 6, 7, 12, 13];
    activeBtns.forEach(id => {
      let b = document.getElementById("btn-" + id);
      if(b) {
        if(d.playing && d.activeCmd === id) { b.classList.add("act-glow"); } 
        else { b.classList.remove("act-glow"); }
      }
    });
  }).catch(e => console.log("Fetch Error", e));
}, 800);
</script>
</body></html>
)=====";

void saveS() {
  prefs.begin("smaran", false);
  // ALL SETTINGS SAVED PERMANENTLY HERE
  prefs.putInt("enGD", enGD); prefs.putInt("gPH", gPH); prefs.putInt("gPM", gPM); prefs.putInt("gPEH", gPEH); prefs.putInt("gPEM", gPEM);
  prefs.putInt("enSunG", enSunG); prefs.putInt("sunGH", sunGH); prefs.putInt("sunGM", sunGM);
  prefs.putInt("enMDh", enMDh); prefs.putInt("mDhSH", mDhSH); prefs.putInt("mDhSM", mDhSM); prefs.putInt("mDhEH", mDhEH); prefs.putInt("mDhEM", mDhEM);
  prefs.putInt("enEDh", enEDh); prefs.putInt("eDhSH", eDhSH); prefs.putInt("eDhSM", eDhSM); prefs.putInt("eDhEH", eDhEH); prefs.putInt("eDhEM", eDhEM);
  prefs.putInt("enDS", enDS); prefs.putInt("dhSH", dhSH); prefs.putInt("dhSM", dhSM); prefs.putInt("dhEH", dhEH); prefs.putInt("dhEM", dhEM);
  prefs.putInt("enHC1", enHC1); prefs.putInt("hc1SH", hc1SH); prefs.putInt("hc1SM", hc1SM); prefs.putInt("hc1EH", hc1EH); prefs.putInt("hc1EM", hc1EM);
  prefs.putInt("enHC2", enHC2); prefs.putInt("hc2SH", hc2SH); prefs.putInt("hc2SM", hc2SM); prefs.putInt("hc2EH", hc2EH); prefs.putInt("hc2EM", hc2EM);
  
  prefs.putInt("enUn1", enUn1); prefs.putInt("un1SH", un1SH); prefs.putInt("un1SM", un1SM); prefs.putInt("un1EH", un1EH); prefs.putInt("un1EM", un1EM);
  prefs.putInt("enUn2", enUn2); prefs.putInt("un2SH", un2SH); prefs.putInt("un2SM", un2SM); prefs.putInt("un2EH", un2EH); prefs.putInt("un2EM", un2EM);
  prefs.putInt("enUn3", enUn3); prefs.putInt("un3SH", un3SH); prefs.putInt("un3SM", un3SM); prefs.putInt("un3EH", un3EH); prefs.putInt("un3EM", un3EM);
  prefs.putInt("enUn4", enUn4); prefs.putInt("un4SH", un4SH); prefs.putInt("un4SM", un4SM); prefs.putInt("un4EH", un4EH); prefs.putInt("un4EM", un4EM);
  prefs.putInt("curVol", curVol);
  prefs.end();
}

void loadS() {
  prefs.begin("smaran", true);
  enGD = prefs.getInt("enGD", 1); gPH = prefs.getInt("gPH", 18); gPM = prefs.getInt("gPM", 50); gPEH = prefs.getInt("gPEH", 19); gPEM = prefs.getInt("gPEM", 5);
  enSunG = prefs.getInt("enSunG", 0); sunGH = prefs.getInt("sunGH", 18); sunGM = prefs.getInt("sunGM", 50);
  enMDh = prefs.getInt("enMDh", 1); mDhSH = prefs.getInt("mDhSH", 8); mDhSM = prefs.getInt("mDhSM", 30); mDhEH = prefs.getInt("mDhEH", 12); mDhEM = prefs.getInt("mDhEM", 0);
  enEDh = prefs.getInt("enEDh", 1); eDhSH = prefs.getInt("eDhSH", 16); eDhSM = prefs.getInt("eDhSM", 0); eDhEH = prefs.getInt("eDhEH", 18); eDhEM = prefs.getInt("eDhEM", 49);
  enDS = prefs.getInt("enDS", 1); dhSH = prefs.getInt("dhSH", 16); dhSM = prefs.getInt("dhSM", 0); dhEH = prefs.getInt("dhEH", 17); dhEM = prefs.getInt("dhEM", 30);
  enHC1 = prefs.getInt("enHC1", 0); hc1SH = prefs.getInt("hc1SH", 7); hc1SM = prefs.getInt("hc1SM", 0); hc1EH = prefs.getInt("hc1EH", 7); hc1EM = prefs.getInt("hc1EM", 15);
  enHC2 = prefs.getInt("enHC2", 0); hc2SH = prefs.getInt("hc2SH", 19); hc2SM = prefs.getInt("hc2SM", 0); hc2EH = prefs.getInt("hc2EH", 19); hc2EM = prefs.getInt("hc2EM", 15);

  enUn1 = prefs.getInt("enUn1", 1); un1SH = prefs.getInt("un1SH", 5); un1SM = prefs.getInt("un1SM", 0); un1EH = prefs.getInt("un1EH", 8); un1EM = prefs.getInt("un1EM", 0);
  enUn2 = prefs.getInt("enUn2", 1); un2SH = prefs.getInt("un2SH", 18); un2SM = prefs.getInt("un2SM", 0); un2EH = prefs.getInt("un2EH", 21); un2EM = prefs.getInt("un2EM", 0);
  enUn3 = prefs.getInt("enUn3", 0); un3SH = prefs.getInt("un3SH", 0); un3SM = prefs.getInt("un3SM", 0); un3EH = prefs.getInt("un3EH", 0); un3EM = prefs.getInt("un3EM", 0);
  enUn4 = prefs.getInt("enUn4", 0); un4SH = prefs.getInt("un4SH", 0); un4SM = prefs.getInt("un4SM", 0); un4EH = prefs.getInt("un4EH", 0); un4EM = prefs.getInt("un4EM", 0);
  curVol = prefs.getInt("curVol", 20);
  prefs.end();
}

void pushSettingsToCloud() {
  FirebaseJson json;
  json.set("gPH", gPH); json.set("gPM", gPM); json.set("gPEH", gPEH); json.set("gPEM", gPEM); json.set("enGD", enGD);
  json.set("sunGH", sunGH); json.set("sunGM", sunGM); json.set("enSunG", enSunG);
  json.set("mDhSH", mDhSH); json.set("mDhSM", mDhSM); json.set("mDhEH", mDhEH); json.set("mDhEM", mDhEM); json.set("enMDh", enMDh);
  json.set("eDhSH", eDhSH); json.set("eDhSM", eDhSM); json.set("eDhEH", eDhEH); json.set("eDhEM", eDhEM); json.set("enEDh", enEDh);
  json.set("dhSH", dhSH); json.set("dhSM", dhSM); json.set("dhEH", dhEH); json.set("dhEM", dhEM); json.set("enDS", enDS);
  json.set("hc1SH", hc1SH); json.set("hc1SM", hc1SM); json.set("hc1EH", hc1EH); json.set("hc1EM", hc1EM); json.set("enHC1", enHC1);
  json.set("hc2SH", hc2SH); json.set("hc2SM", hc2SM); json.set("hc2EH", hc2EH); json.set("hc2EM", hc2EM); json.set("enHC2", enHC2);
  
  json.set("un1SH", un1SH); json.set("un1SM", un1SM); json.set("un1EH", un1EH); json.set("un1EM", un1EM); json.set("enUn1", enUn1);
  json.set("un2SH", un2SH); json.set("un2SM", un2SM); json.set("un2EH", un2EH); json.set("un2EM", un2EM); json.set("enUn2", enUn2);
  json.set("un3SH", un3SH); json.set("un3SM", un3SM); json.set("un3EH", un3EH); json.set("un3EM", un3EM); json.set("enUn3", enUn3);
  json.set("un4SH", un4SH); json.set("un4SM", un4SM); json.set("un4EH", un4EH); json.set("un4EM", un4EM); json.set("enUn4", enUn4);
  Firebase.RTDB.setJSONAsync(&fbdoStat, "/smaran_v1/settings", &json);
}

void stopPlayer(){ if(dfOK){ myDFPlayer.stop(); } trkPly=0; curMsg="READY"; btnTriggered=0; activeCmd=0; }

void ply(int f, int t, String n){ 
    if(dfOK){ myDFPlayer.volume(curVol); myDFPlayer.playFolder(f,t); } 
    trkPly=1; curMsg=n; trkSt=millis(); curFld=f; curTrkNum=t; 
}
void plyDhun(int t, String n){ 
    if(dfOK){ myDFPlayer.volume(curVol); myDFPlayer.playFolder(3,t); } 
    trkPly=1; curMsg=n; trkSt=millis(); curFld=3; curTrkNum=t; 
}

// બટનના 4 એક્ટિવ સ્લોટ્સ ચેક કરવા માટેનું ફંક્શન
bool isUnlocked(int h, int m, int w) {
  long cM = h * 60 + m; 
  auto chk = [&](int sh, int sm, int eh, int em, bool en) {
    if (!en) return false; 
    long s = sh * 60 + sm; long e = eh * 60 + em; if(s == e) return false;
    return (s < e) ? (cM >= s && cM < e) : (s > e ? (cM >= s || cM < e) : false);
  };
  if(chk(un1SH,un1SM,un1EH,un1EM,enUn1)) return true; 
  if(chk(un2SH,un2SM,un2EH,un2EM,enUn2)) return true; 
  if(chk(un3SH,un3SM,un3EH,un3EM,enUn3)) return true; 
  if(chk(un4SH,un4SM,un4EH,un4EM,enUn4)) return true; 
  return false;
}

String getRdy(int h, int m, int w) {
  if ((h >= 7 && h < 18) || (h >= 21) || (h == 0 && m <= 30)) { return F("F1T9 ARTI Ready "); } 
  else if ((h == 5 && m >= 50) || (h == 6 && m <= 30)) { return F("Mangla    Ready "); } 
  else if ((h >= 18) && ((h < 20) || (h == 20 && m <= (w == 0 ? 45 : 30)))) { return "F1T" + String(w + 1) + " ARTI Ready "; }
  return F("FREE TIME READY ");
}

void executeCommand(int c, int folder = 0, int track = 0) {
  // 🟢 STOP/UN-STOP ટોગલ: c==2 સિવાય કોઈપણ નવું પ્લે-કમાન્ડ આવે તો "stopped" સ્ટેટ સાફ કરી દો 🟢
  if (c != 2) stoppedManually = false;

  if(c==1){ 
    stopPlayer(); btnTriggered=1; activeCmd=0; dhunManStop=true;
    if(folder == 4) { curF4 = track; ply(4, curF4, f4N[curF4-1]); } 
    else { ply(folder, track, "F" + String(folder) + " T" + String(track)); } 
  }
  else if(c==2){ 
    if(!stoppedManually){ 
      // 1st ક્લિક: STOP
      stopPlayer(); dhunManStop=true; f4Rep=0; f4Shuf=0; curMsg="MANUAL STOP"; stoppedManually=true; 
    } else { 
      // 2nd ક્લિક: UN-STOP — મૂળ પોજીશન (READY) માં પાછું, chkAuto() ફરી કબજો લેશે
      stoppedManually=false; dhunManStop=false; activeCmd=0; curMsg="READY"; 
    } 
  }
  else if(c==3){ stopPlayer(); btnTriggered=1; activeCmd=3; dhunManStop=true; ply(1, 8, "Mangla"); }
  else if(c==4){ stopPlayer(); btnTriggered=1; activeCmd=4; dhunManStop=true; ply(1, g_w+1, "T.Arti"); } 
  else if(c==5){ stopPlayer(); btnTriggered=1; activeCmd=5; dhunManStop=true; ply(2, g_w+1, "T.Godi"); } 
  else if(c==6){ stopPlayer(); btnTriggered=1; activeCmd=6; dhunManStop=true; plyDhun(1,"Dhun-Morn"); }
  else if(c==7){ stopPlayer(); btnTriggered=1; activeCmd=7; dhunManStop=true; plyDhun(2,"Dhun-Even"); }
  else if(c==8){ stopPlayer(); btnTriggered=1; activeCmd=8; dhunManStop=true; ply(1, 9, "Arti&Jay"); }
  else if(c==9){ if(curVol<30){ curVol++; if(dfOK){ myDFPlayer.volume(curVol); } saveS(); volShowTimer=millis(); } }
  else if(c==10){ if(curVol>0){ curVol--; if(dfOK){ myDFPlayer.volume(curVol); } saveS(); volShowTimer=millis(); } }
  else if(c==12){ stopPlayer(); btnTriggered=1; activeCmd=12; dhunManStop=true; ply(5, 1, "Chalisa 1"); }
  else if(c==13){ stopPlayer(); btnTriggered=1; activeCmd=13; dhunManStop=true; ply(5, 2, "Chalisa 2"); }
  else if(c==17){ delay(200); ESP.restart(); }
  else if(c==18){ curF4++; if(curF4>TOTAL_F4_TRACKS) curF4=1; btnTriggered=1; activeCmd=0; dhunManStop=true; ply(4,curF4,f4N[curF4-1]); }
  else if(c==19){ curF4--; if(curF4<1) curF4=TOTAL_F4_TRACKS; btnTriggered=1; activeCmd=0; dhunManStop=true; ply(4,curF4,f4N[curF4-1]); }
  else if(c==20){ f4Rep=!f4Rep; if(f4Rep) f4Shuf=0; }
  else if(c==21){ f4Shuf=!f4Shuf; if(f4Shuf) f4Rep=0; }
  else if(c==22){ if(isMut){ curVol=svVol?svVol:10; isMut=0; } else { svVol=curVol; curVol=0; isMut=1; } if(dfOK){ myDFPlayer.volume(curVol); } saveS(); volShowTimer=millis(); }
  else if(c==99){ loadS(); }
}

void chkAuto(){
  int h=g_h, m=g_m, w=g_w; bool isSun = (w == 0); 
  long curMin = h * 60 + m; bool inAnyDhun = false, inAnyHC = false;

  if (enMDh && curMin >= (mDhSH*60+mDhSM) && curMin < (mDhEH*60+mDhEM)) inAnyDhun = true;
  if (enEDh && !isSun && curMin >= (eDhSH*60+eDhSM) && curMin < (eDhEH*60+eDhEM)) inAnyDhun = true;
  if (enDS && isSun && curMin >= (dhSH*60+dhSM) && curMin < (dhEH*60+dhEM)) inAnyDhun = true;

  if (enHC1 && curMin >= (hc1SH*60+hc1SM) && curMin < (hc1EH*60+hc1EM)) inAnyHC = true;
  if (enHC2 && curMin >= (hc2SH*60+hc2SM) && curMin < (hc2EH*60+hc2EM)) inAnyHC = true;

  if (!inAnyDhun && !inAnyHC) dhunManStop = false; 

  if (inAnyDhun) {
      if (trkPly == 0 && !dhunManStop) { 
          activeCmd = (curMin < 720) ? 6 : 7;
          plyDhun((curMin < 720) ? 1 : 2, "Dhun"); 
      }
  }
  if (!inAnyDhun && trkPly && curFld == 3 && !btnTriggered) stopPlayer();

  if (inAnyHC) {
      if (trkPly == 0 && !dhunManStop) { 
          activeCmd = (curMin < 720) ? 12 : 13;
          ply(5, (curMin < 720) ? 1 : 2, "Chalisa"); 
      }
  }
  if (!inAnyHC && trkPly && curFld == 5 && !btnTriggered) stopPlayer();
  
  bool gW=(!isSun && h==gPH && m>=gPM && m<=(gPM+20)); bool sGW=(isSun && h==sunGH && m>=sunGM && m<=(sunGM+20)); 
  static bool autoPly=0; bool triggerGodi = false;
  if(gW && enGD && h==gPH && m==gPM) triggerGodi = true; 
  if(sGW && enSunG && h==sunGH && m==sunGM) triggerGodi = true; 

  if(!trkPly && (gW || sGW)){ 
      if(triggerGodi && !autoPly){ activeCmd = 5; ply(2,w+1,"Godi"); autoPly=1; } 
  } else { 
      if(!(gW || sGW)) autoPly=0; 
  } 
  if(h==gPEH && m==gPEM && w!=0 && trkPly && curFld==2 && !btnTriggered) stopPlayer();
}

String getLiveStatusJson() {
  String t = "--:--"; 
  int h = g_h, m = g_m, s = g_s, w = g_w;
  if(rtcOK) {
    int dh = h % 12; if(dh == 0) dh = 12;
    t = dW[w] + " " + String(dh) + ":" + (m < 10 ? "0" : "") + String(m) + ":" + (s < 10 ? "0" : "") + String(s) + (h >= 12 ? " PM" : " AM");
  }
  
  String liveMsg = curMsg;
  if (!dfOK) liveMsg = "DF ERROR! CHECK WIRING";
  else if (trkPly) {
      if (curFld == 4) liveMsg = String(curF4) + "/" + String(TOTAL_F4_TRACKS) + " " + String(f4N[curF4-1]);
      else if (curFld == 3) liveMsg = "Dhun Track " + String(curTrkNum);
      else if (curFld == 5) liveMsg = "Chalisa Track " + String(curTrkNum);
      else liveMsg = "F" + String(curFld) + " T" + String(curTrkNum) + " " + curMsg;
      
      long e = (millis() - trkSt) / 1000; int tm = e / 60, ts = e % 60;
      liveMsg += " [" + String(tm < 10 ? "0" : "") + String(tm) + ":" + String(ts < 10 ? "0" : "") + String(ts) + "]";
      liveMsg = (btnTriggered ? "MANUAL | " : "AUTO | ") + liveMsg;
  } else { 
      liveMsg = getRdy(h, m, w); 
  }

  String j = "{";
  j += "\"time\":\"" + t + "\",";
  j += "\"f4track\":" + String(curF4) + ",\"volume\":" + String(curVol) + ",";
  j += "\"activeCmd\":" + String(activeCmd) + ",";
  j += "\"statusMsg\":\"" + liveMsg + "\",\"playing\":" + String(trkPly ? 1 : 0) + ",";
  j += "\"isRepeat\":" + String(f4Rep ? 1 : 0) + ",\"isShuffle\":" + String(f4Shuf ? 1 : 0) + ",";
  j += "\"isMute\":" + String(isMut ? 1 : 0) + ",\"online\":1}";
  return j;
}

void handlePhysicalButton() {
  int h = g_h; int m = g_m; int dow = g_w; 
  
  // 🔴 4 એક્ટિવ સ્લોટ્સ ચેક કરશે. જો બહારનો સમય હશે તો બટન કામ નહીં કરે 🔴
  if (!isUnlocked(h, m, dow)) { return; }

  static unsigned long pressTime = 0; 
  static bool lastState = LOW; 
  bool currentState = digitalRead(btnPin); 
  unsigned long now = millis();

  if (lastState == LOW && currentState == HIGH) { 
      pressTime = now; 
  } 
  else if (lastState == HIGH && currentState == LOW) { 
      if (now - pressTime > 30) { 
          if ((h >= 8 && h < 18) || (h >= 21) || (h == 0 && m <= 30)) { executeCommand(8); } 
          else if ((h == 5 && m >= 50) || (h == 6) || (h == 7 && m <= 30)) { executeCommand(3); }
          else if ((h >= 18) && ((h < 20) || (h == 20 && m <= (dow == 0 ? 45 : 30)))) { executeCommand(4); }
      }
  }
  lastState = currentState;
}

void cloudTaskCode(void * parameter) {
  for(;;) {
    if (WiFi.status() == WL_CONNECTED && Firebase.ready()) {
      if (needCloudSync) { pushSettingsToCloud(); needCloudSync = false; }
      
      if (Firebase.RTDB.get(&fbdoCmd, "/smaran_v1/command")) {
         String dType = fbdoCmd.dataType(); 
         String rawData = (dType == "json") ? fbdoCmd.jsonString() : fbdoCmd.stringData();
         int cmd = 0, folder = 0, track = 0;
         if (dType == "json") {
#if ARDUINOJSON_VERSION_MAJOR >= 7
             JsonDocument doc;
#else
             DynamicJsonDocument doc(1024);
#endif
             DeserializationError err = deserializeJson(doc, rawData);
             if (!err && doc.containsKey("action")) { 
               cmd = doc["action"].as<String>().toInt(); 
               if(doc.containsKey("folder")) folder = doc["folder"].as<int>(); 
               if(doc.containsKey("track")) track = doc["track"].as<int>(); 
             }
         } else { cmd = rawData.toInt(); }
         
         if (cmd > 0) { 
           Firebase.RTDB.deleteNode(&fbdoCmd, "/smaran_v1/command"); 
           activeCloudCmd = cmd; activeCloudFolder = folder; activeCloudTrack = track;
           cloudCmdPending = true; 
         }
      }

      static unsigned long lastLivePush = 0;
      if (millis() - lastLivePush > 4000) {
         lastLivePush = millis();
         String tempJson;
         if(xSemaphoreTake(dataMutex, portMAX_DELAY)) {
             tempJson = sharedStatusJson; 
             xSemaphoreGive(dataMutex);
         }
         
         if(tempJson.length() > 10) {
           FirebaseJson fbJson; fbJson.setJsonData(tempJson);
           Firebase.RTDB.setJSONAsync(&fbdoStat, "/smaran_v1/status", &fbJson);
         }
      }
    }
    vTaskDelay(1500 / portTICK_PERIOD_MS); 
  }
}

void setup() {
  Serial.begin(115200); Serial.setDebugOutput(false); 
  dataMutex = xSemaphoreCreateMutex();

  Wire.begin(23, 22); Wire.setClock(400000); 

  pinMode(DF_BUSY_PIN, INPUT); pinMode(btnPin, INPUT_PULLDOWN); 
  pinMode(ledPin, OUTPUT); digitalWrite(ledPin, LOW);

  lcd.init(); lcd.backlight();
  lcd.clear(); lcd.print(F("RAMESHWAR ESP32")); lcd.setCursor(0,1); lcd.print(F("CRASH FIX PRO")); delay(1000);

  if(!rtc.begin()){ lcd.clear(); lcd.print(F("RTC FAIL!")); rtcOK = false; }
  else { if(rtc.lostPower()) rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); rtcOK = true; }

  if (rtcOK) { DateTime now = rtc.now(); g_h = now.hour(); g_m = now.minute(); g_w = now.dayOfTheWeek(); g_s = now.second(); }

  mySerial.begin(9600, SERIAL_8N1, ESP_RX, ESP_TX); delay(1500); 
  loadS(); 

  if(!myDFPlayer.begin(mySerial, false)) { 
    lcd.clear(); lcd.print(F("DF FAIL!")); delay(2000); dfOK = false; 
  } else { 
    dfOK = true; myDFPlayer.setTimeOut(500); 
    if(curVol<0||curVol>30) curVol=15; myDFPlayer.volume(curVol); 
  }

  WiFi.setSleep(false);
  WiFi.mode(WIFI_AP_STA); 
  IPAddress apIP(192, 168, 4, 8);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("RAMESHWAR_OFFLINE", "12345678");

  WiFi.begin(ssid, password);
  unsigned long startAtt = millis();
  while(WiFi.status() != WL_CONNECTED && millis() - startAtt < 8000) { delay(500); }

  if(WiFi.status() == WL_CONNECTED) {
    configTime(19800, 0, "pool.ntp.org", "time.nist.gov");
    fbConfig.api_key = API_KEY; fbConfig.database_url = DATABASE_URL; fbConfig.signer.tokens.legacy_token = FIREBASE_AUTH; 
    Firebase.begin(&fbConfig, &fbAuth); Firebase.reconnectWiFi(true);
    fbdoCmd.setBSSLBufferSize(2048, 1024); fbdoStat.setBSSLBufferSize(2048, 1024);
    Firebase.RTDB.deleteNode(&fbdoCmd, "/smaran_v1/command");
    needCloudSync = true;
  }

  server.on("/", HTTP_GET, [](){ server.sendHeader("Access-Control-Allow-Origin", "*"); server.sendHeader("Cache-Control","no-cache"); server.send(200, "text/html", offlineHTML); });
  server.on("/god_icon", HTTP_GET, [](){ server.sendHeader("Access-Control-Allow-Origin", "*"); server.send(200, "text/plain", iconBase64); });
  server.on("/status", HTTP_GET, [](){ 
    server.sendHeader("Access-Control-Allow-Origin", "*"); server.sendHeader("Cache-Control","no-cache"); 
    String temp; 
    if(xSemaphoreTake(dataMutex, portMAX_DELAY)) {
        temp = sharedStatusJson; 
        xSemaphoreGive(dataMutex);
    }
    server.send(200, "application/json", temp.length()>5 ? temp : "{}"); 
  });
  server.on("/cmd", HTTP_GET, [](){ server.sendHeader("Access-Control-Allow-Origin", "*"); if(server.hasArg("action")){ executeCommand(server.arg("action").toInt(), server.arg("folder").toInt(), server.arg("track").toInt()); needCloudSync = true; } server.send(200, "text/plain", "OK"); });
  
  server.on("/get_settings", HTTP_GET, [](){
    server.sendHeader("Access-Control-Allow-Origin", "*");
#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(1024);
#endif
    doc["enGD"]=enGD; doc["gPH"]=gPH; doc["gPM"]=gPM; doc["gPEH"]=gPEH; doc["gPEM"]=gPEM;
    doc["enSunG"]=enSunG; doc["sunGH"]=sunGH; doc["sunGM"]=sunGM;
    doc["enMDh"]=enMDh; doc["mDhSH"]=mDhSH; doc["mDhSM"]=mDhSM; doc["mDhEH"]=mDhEH; doc["mDhEM"]=mDhEM;
    doc["enEDh"]=enEDh; doc["eDhSH"]=eDhSH; doc["eDhSM"]=eDhSM; doc["eDhEH"]=eDhEH; doc["eDhEM"]=eDhEM;
    doc["enDS"]=enDS; doc["dhSH"]=dhSH; doc["dhSM"]=dhSM; doc["dhEH"]=dhEH; doc["dhEM"]=dhEM;
    doc["enHC1"]=enHC1; doc["hc1SH"]=hc1SH; doc["hc1SM"]=hc1SM; doc["hc1EH"]=hc1EH; doc["hc1EM"]=hc1EM;
    doc["enHC2"]=enHC2; doc["hc2SH"]=hc2SH; doc["hc2SM"]=hc2SM; doc["hc2EH"]=hc2EH; doc["hc2EM"]=hc2EM;
    
    doc["enUn1"]=enUn1; doc["un1SH"]=un1SH; doc["un1SM"]=un1SM; doc["un1EH"]=un1EH; doc["un1EM"]=un1EM;
    doc["enUn2"]=enUn2; doc["un2SH"]=un2SH; doc["un2SM"]=un2SM; doc["un2EH"]=un2EH; doc["un2EM"]=un2EM;
    doc["enUn3"]=enUn3; doc["un3SH"]=un3SH; doc["un3SM"]=un3SM; doc["un3EH"]=un3EH; doc["un3EM"]=un3EM;
    doc["enUn4"]=enUn4; doc["un4SH"]=un4SH; doc["un4SM"]=un4SM; doc["un4EH"]=un4EH; doc["un4EM"]=un4EM;
    
    String out; serializeJson(doc, out); server.send(200, "application/json", out);
  });

  server.on("/save_settings", HTTP_GET, [](){
    server.sendHeader("Access-Control-Allow-Origin", "*");
    if(server.hasArg("enGD")) enGD = server.arg("enGD").toInt(); if(server.hasArg("gPH")) gPH = server.arg("gPH").toInt(); if(server.hasArg("gPM")) gPM = server.arg("gPM").toInt(); if(server.hasArg("gPEH")) gPEH = server.arg("gPEH").toInt(); if(server.hasArg("gPEM")) gPEM = server.arg("gPEM").toInt();
    if(server.hasArg("enSunG")) enSunG = server.arg("enSunG").toInt(); if(server.hasArg("sunGH")) sunGH = server.arg("sunGH").toInt(); if(server.hasArg("sunGM")) sunGM = server.arg("sunGM").toInt();
    if(server.hasArg("enMDh")) enMDh = server.arg("enMDh").toInt(); if(server.hasArg("mDhSH")) mDhSH = server.arg("mDhSH").toInt(); if(server.hasArg("mDhSM")) mDhSM = server.arg("mDhSM").toInt(); if(server.hasArg("mDhEH")) mDhEH = server.arg("mDhEH").toInt(); if(server.hasArg("mDhEM")) mDhEM = server.arg("mDhEM").toInt();
    if(server.hasArg("enEDh")) enEDh = server.arg("enEDh").toInt(); if(server.hasArg("eDhSH")) eDhSH = server.arg("eDhSH").toInt(); if(server.hasArg("eDhSM")) eDhSM = server.arg("eDhSM").toInt(); if(server.hasArg("eDhEH")) eDhEH = server.arg("eDhEH").toInt(); if(server.hasArg("eDhEM")) eDhEM = server.arg("eDhEM").toInt();
    if(server.hasArg("enDS")) enDS = server.arg("enDS").toInt(); if(server.hasArg("dhSH")) dhSH = server.arg("dhSH").toInt(); if(server.hasArg("dhSM")) dhSM = server.arg("dhSM").toInt(); if(server.hasArg("dhEH")) dhEH = server.arg("dhEH").toInt(); if(server.hasArg("dhEM")) dhEM = server.arg("dhEM").toInt();
    if(server.hasArg("enHC1")) enHC1 = server.arg("enHC1").toInt(); if(server.hasArg("hc1SH")) hc1SH = server.arg("hc1SH").toInt(); if(server.hasArg("hc1SM")) hc1SM = server.arg("hc1SM").toInt(); if(server.hasArg("hc1EH")) hc1EH = server.arg("hc1EH").toInt(); if(server.hasArg("hc1EM")) hc1EM = server.arg("hc1EM").toInt();
    if(server.hasArg("enHC2")) enHC2 = server.arg("enHC2").toInt(); if(server.hasArg("hc2SH")) hc2SH = server.arg("hc2SH").toInt(); if(server.hasArg("hc2SM")) hc2SM = server.arg("hc2SM").toInt(); if(server.hasArg("hc2EH")) hc2EH = server.arg("hc2EH").toInt(); if(server.hasArg("hc2EM")) hc2EM = server.arg("hc2EM").toInt();
    
    if(server.hasArg("enUn1")) enUn1 = server.arg("enUn1").toInt(); if(server.hasArg("un1SH")) un1SH = server.arg("un1SH").toInt(); if(server.hasArg("un1SM")) un1SM = server.arg("un1SM").toInt(); if(server.hasArg("un1EH")) un1EH = server.arg("un1EH").toInt(); if(server.hasArg("un1EM")) un1EM = server.arg("un1EM").toInt();
    if(server.hasArg("enUn2")) enUn2 = server.arg("enUn2").toInt(); if(server.hasArg("un2SH")) un2SH = server.arg("un2SH").toInt(); if(server.hasArg("un2SM")) un2SM = server.arg("un2SM").toInt(); if(server.hasArg("un2EH")) un2EH = server.arg("un2EH").toInt(); if(server.hasArg("un2EM")) un2EM = server.arg("un2EM").toInt();
    if(server.hasArg("enUn3")) enUn3 = server.arg("enUn3").toInt(); if(server.hasArg("un3SH")) un3SH = server.arg("un3SH").toInt(); if(server.hasArg("un3SM")) un3SM = server.arg("un3SM").toInt(); if(server.hasArg("un3EH")) un3EH = server.arg("un3EH").toInt(); if(server.hasArg("un3EM")) un3EM = server.arg("un3EM").toInt();
    if(server.hasArg("enUn4")) enUn4 = server.arg("enUn4").toInt(); if(server.hasArg("un4SH")) un4SH = server.arg("un4SH").toInt(); if(server.hasArg("un4SM")) un4SM = server.arg("un4SM").toInt(); if(server.hasArg("un4EH")) un4EH = server.arg("un4EH").toInt(); if(server.hasArg("un4EM")) un4EM = server.arg("un4EM").toInt();
    
    saveS(); needCloudSync = true; server.send(200, "text/plain", "OK");
  });
  
  server.begin(); lcd.clear(); lcd.print(WiFi.localIP().toString()); delay(2000);

  // 🟢 સ્ટેબિલિટી ફિક્સ: CloudTask નો સ્ટેક 10000 → 20480 બાઈટ કર્યો.
  // Firebase/SSL (BearSSL) ને ઘણી સ્ટેક જગ્યા જોઈએ છે; 10000 બાઈટ ઓછા પડવાથી
  // "Stack canary watchpoint" / "Task watchdog" ક્રેશ થઈ મોડ્યુલ રિસ્ટાર્ટ થતું હતું. 🟢
  xTaskCreatePinnedToCore(cloudTaskCode, "CloudTask", 20480, NULL, 1, &CloudTask, 0);
}

void loop() {
  yield(); vTaskDelay(1); 
  server.handleClient(); 
  handlePhysicalButton();
  digitalWrite(ledPin, trkPly ? HIGH : LOW);
  
  if (cloudCmdPending) {
    executeCommand(activeCloudCmd, activeCloudFolder, activeCloudTrack);
    cloudCmdPending = false;
    needCloudSync = true;
  }

  static unsigned long lastRtcRead = 0;
  if (millis() - lastRtcRead >= 1000) {
      lastRtcRead = millis();
      if (rtcOK) {
          DateTime now = rtc.now();
          g_h = now.hour(); g_m = now.minute(); g_s = now.second(); g_w = now.dayOfTheWeek();
          chkAuto(); 
      }
  }

  if (WiFi.status() == WL_CONNECTED && (millis() - lastNtpSync > 21600000 || lastNtpSync == 0)) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 1000)) { 
      if (timeinfo.tm_year > 100) { 
        rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec)); 
        lastNtpSync = millis(); 
      } 
    }
  }

  if(trkPly && dfOK && (millis() - trkSt > 1000)){ 
     if(digitalRead(DF_BUSY_PIN) == HIGH){ 
        if(curFld==4){
            if(f4Rep){curF4++; if(curF4>TOTAL_F4_TRACKS)curF4=1; ply(4,curF4,f4N[curF4-1]);}
            else if(f4Shuf){curF4=random(1,TOTAL_F4_TRACKS+1); ply(4,curF4,f4N[curF4-1]);}
            else { stopPlayer(); }
        } else {
          stopPlayer();
        }
     }
  }

  static unsigned long lastJsonUpdate = 0;
  if (millis() - lastJsonUpdate > 2000) {
     lastJsonUpdate = millis();
     String tempJson = getLiveStatusJson();
     if(xSemaphoreTake(dataMutex, portMAX_DELAY)) {
         sharedStatusJson = tempJson;
         xSemaphoreGive(dataMutex);
     }
  }

  static unsigned long lastLcdUpdate=0;
  if(millis()-lastLcdUpdate>1000) {
     lastLcdUpdate = millis();
     if (rtcOK) {
         int h=g_h, m=g_m, w=g_w; int dh=h%12; if(dh==0) dh=12;
         bool isCloudOk = (WiFi.status() == WL_CONNECTED);
         String modeStr = isCloudOk ? "C+" : "O+"; modeStr += btnTriggered ? "M" : "A";      
         char l1buf[17]; snprintf(l1buf, sizeof(l1buf), "%s %02d:%02d%s %s", dW[w].c_str(), dh, m, (h>=12?" PM":" AM"), modeStr.c_str());
         String l1 = String(l1buf); while(l1.length()<16) l1+=" ";
         lcd.setCursor(0,0); lcd.print(l1);
         
         lcd.setCursor(0,1); String m1 = "";
         
         if (millis() - volShowTimer < 2000) {
             char vbuf[17]; snprintf(vbuf, sizeof(vbuf), "VOLUME: %02d      ", curVol);
             m1 = String(vbuf);
         } 
         else if (!dfOK) { m1 = "DF WIRING ERR! "; } 
         else if(trkPly && (millis() - trkSt > 0)) {
            long e = ((millis()-trkSt)/1000)+1; int tm=e/60, ts=e%60;
            String name = curMsg; 
            while(name.length() < 10) name += " "; 
            name = name.substring(0, 10);
            char b[17]; snprintf(b, sizeof(b), "%s %02d:%02d", name.c_str(), tm, ts); m1 = String(b);
         } else {
            // ટચ બટન માટેના લોક-અનલોક મેસેજને કાઢીને નોર્મલ મેસેજ કરી દીધો છે 
            m1=" RAMESHWAR READY";
         }
         while(m1.length()<16) m1+=" ";
         lcd.print(m1);
     }
  }
}
