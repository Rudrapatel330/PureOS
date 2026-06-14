import asyncio
import json
from aiohttp import web, WSCloseCode

TCP_PORT = 7860
HTTP_WS_PORT = 7862

import urllib.parse
import traceback
import subprocess
import os

async def handle_ytsearch(request):
    query = request.query.get('q', '')
    print(f"[YT Search] Received query: '{query}'")
    if not query:
        return web.Response(text="")
        
    try:
        import yt_dlp
        ydl_opts = {
            'quiet': True,
            'extract_flat': 'in_playlist',
            'dump_single_json': True,
            'simulate': True
        }
        loop = asyncio.get_event_loop()
        def do_req():
            with yt_dlp.YoutubeDL(ydl_opts) as ydl:
                return ydl.extract_info(f"ytsearch15:{query}", download=False)
        info = await loop.run_in_executor(None, do_req)
        out = []
        for entry in info.get('entries', []):
            title = entry.get('title')
            videoId = entry.get('id')
            if title and videoId:
                title = title.replace('|', '-')
                out.append(f"{title}|{videoId}")
        return web.Response(text="\n".join(out))
    except Exception as e:
        traceback.print_exc()
        return web.Response(text="Error")

async def handle_ytplay(request):
    vid = request.query.get('id', '')
    print(f"[YT Play] Requested streaming for video ID: '{vid}'")
    if not vid:
        return web.Response(status=400)
    
    import uuid
    loop = asyncio.get_event_loop()
    def do_download():
        unique_id = uuid.uuid4().hex
        fname = f"temp_{vid}_{unique_id}.mp3"
        subprocess.run(["yt-dlp", "-x", "--audio-format", "mp3", "--audio-quality", "5", "-o", fname, f"https://www.youtube.com/watch?v={vid}"], check=False)
        if os.path.exists(fname):
            try:
                with open(fname, "rb") as f:
                    d = f.read()
                os.remove(fname)
                return d
            except Exception as e:
                print(f"Error reading/removing {fname}: {e}")
                return None
        return None
        
    data = await loop.run_in_executor(None, do_download)
    if data:
        return web.Response(body=data, content_type="audio/mpeg")
    return web.Response(status=500)

async def handle_ytthumb(request):
    vid = request.query.get('id', '')
    if not vid:
        return web.Response(status=400)
    
    url = f"https://i.ytimg.com/vi/{vid}/mqdefault.jpg"
    import requests
    loop = asyncio.get_event_loop()
    def do_fetch():
        try:
            res = requests.get(url, timeout=5)
            if res.status_code == 200:
                return res.content
            # Fallback to default
            res = requests.get(f"https://i.ytimg.com/vi/{vid}/default.jpg", timeout=5)
            if res.status_code == 200:
                return res.content
        except:
            pass
        return None
        
    data = await loop.run_in_executor(None, do_fetch)
    if data:
        return web.Response(body=data, content_type="image/jpeg")
    return web.Response(status=404)

async def handle_ytlyrics(request):
    q = request.query.get('q', '')
    if not q:
        return web.Response(status=400)
    
    url = f"https://lrclib.net/api/search?q={q}"
    import requests
    loop = asyncio.get_event_loop()
    def do_fetch():
        try:
            res = requests.get(url, timeout=5)
            if res.status_code == 200:
                data = res.json()
                if data and len(data) > 0:
                    return data[0].get('plainLyrics', '')
        except:
            pass
        return None
        
    data = await loop.run_in_executor(None, do_fetch)
    if data:
        return web.Response(text=data)
    return web.Response(status=404)

# username -> (type, obj)
clients = {}
os_phone_state = 'IDLE'

async def broadcast(target, data):
    dtype = data.get('type', '')
    if dtype != 'audio':
        print(f"[Relay] Routing {data.get('from')} -> {target} [{dtype}]")
    else:
        print(f"[Relay] AUDIO {data.get('from')} -> {target} ({len(data.get('data',''))} b64 chars)")
        
    payload = json.dumps(data, separators=(',', ':'))
    
    if target in clients:
        ctype, cobj = clients[target]
        try:
            if ctype == 'tcp':
                cobj.write((payload + '\n').encode())
                await cobj.drain()
            elif ctype == 'ws':
                if not cobj.closed:
                    await cobj.send_str(payload)
        except (ConnectionResetError, ConnectionError, OSError) as e:
            print(f"[Relay] Connection lost to {target}: {e}")
            if target in clients:
                del clients[target]

# --- TCP Handler (PureOS) ---
async def handle_tcp(reader, writer):
    global os_phone_state
    username = None
    try:
        while True:
            line = await reader.readline()
            if not line: break
            try:
                data = json.loads(line.decode().strip())
                if data.get('type') == 'auth':
                    username = data.get('username')
                    clients[username] = ('tcp', writer)
                    print(f"[TCP] {username} identified")
                    writer.write((json.dumps({"type": "status", "message": "Connected"}, separators=(',', ':')) + '\n').encode())
                    await writer.drain()
                elif 'to' in data:
                    # Forward general messages (chat, call_request, call_accept, audio, etc)
                    data['from'] = username
                    
                    if username == 'PureOS_Phone':
                        if data.get('type') in ('call_accept', 'call_request'):
                            os_phone_state = 'INCALL'
                        elif data.get('type') in ('call_reject', 'call_end'):
                            os_phone_state = 'IDLE'
                    elif data.get('to') == 'PureOS_Phone' and data.get('type') in ('call_end', 'call_reject'):
                        os_phone_state = 'IDLE'
                            
                    await broadcast(data['to'], data)
            except Exception as e:
                pass
    finally:
        if username and username in clients: del clients[username]
        writer.close()

# --- Unified HTTP & WebSocket Handler ---
async def handle_ws_route(request):
    global os_phone_state
    ws = web.WebSocketResponse()
    await ws.prepare(request)
    username = "Android_User"
    try:
        async for msg in ws:
            if msg.type == web.WSMsgType.TEXT:
                data = json.loads(msg.data)
                if data.get('type') == 'auth':
                    username = data.get('username')
                    clients[username] = ('ws', ws)
                    print(f"[WS] {username} identified")
                elif 'to' in data:
                    data['from'] = username
                    
                    if data.get('type') == 'call_request' and data.get('to') == 'PureOS_Phone':
                        if os_phone_state != 'IDLE':
                            reject = {
                                "type": "call_reject",
                                "from": "PureOS_Phone",
                                "to": username,
                                "message": "OS is busy on another call"
                            }
                            await broadcast(username, reject)
                            continue
                    elif data.get('to') == 'PureOS_Phone' and data.get('type') in ('call_end', 'call_reject'):
                        os_phone_state = 'IDLE'
                            
                    await broadcast(data['to'], data)
    finally:
        if username and username in clients: del clients[username]
    return ws

async def handle_index(request):
    html = """<!DOCTYPE html><html><head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
    <title>KABUTAR PureChat & Call</title>
    <style>
    @import url('https://fonts.googleapis.com/css2?family=Outfit:wght@300;600&display=swap');
    body { font-family: 'Outfit', sans-serif; background: #0f0c29; background: linear-gradient(135deg, #0f0c29, #302b63, #24243e); color: white; display: flex; flex-direction: column; height: 100vh; height: 100dvh; margin: 0; overflow: hidden; }
    
    #splash { position: fixed; inset: 0; background: #0f0c29; display: flex; align-items: center; justify-content: center; z-index: 999; animation: fadeOut 0.8s ease 1s forwards; pointer-events: none; }
    #splash h1 { font-size: 15vw; font-weight: 600; background: linear-gradient(90deg, #ff2e63, #ffffff, #08d9d6); -webkit-background-clip: text; -webkit-text-fill-color: transparent; animation: popIn 1s cubic-bezier(0.175, 0.885, 0.32, 1.275) forwards; opacity: 0; }
    
    @keyframes popIn { 0% { transform: scale(0.5); opacity: 0; } 100% { transform: scale(1); opacity: 1; } }
    @keyframes fadeOut { 0% { opacity: 1; transform: scale(1); } 100% { opacity: 0; transform: scale(1.1); visibility: hidden; } }

    #header { padding: 15px; background: rgba(15, 21, 62, 0.98); border-bottom: 1px solid rgba(255,255,255,0.1); display: flex; justify-content: space-between; align-items: center; }
    #header h2 { margin: 0; font-size: 1.2rem; }
    .btn { padding: 8px 15px; border-radius: 20px; border: none; font-weight: 600; cursor: pointer; color: white; transition: 0.2s; }
    #callBtn { background: #08d9d6; color: #0f0c29; }
    #endBtn { background: #ff2e63; display: none; }

    #chat { flex: 1; padding: 20px; overflow-y: auto; display: flex; flex-direction: column; gap: 12px; }
    #chat::-webkit-scrollbar { width: 4px; }
    #chat::-webkit-scrollbar-thumb { background: rgba(255,255,255,0.1); border-radius: 10px; }
    
    .msg { padding: 12px 16px; border-radius: 18px; max-width: 80%; line-height: 1.5; font-size: 1rem; box-shadow: 0 4px 15px rgba(0,0,0,0.2); backdrop-filter: blur(8px); -webkit-backdrop-filter: blur(8px); border: 1px solid rgba(255,255,255,0.05); }
    .self { background: linear-gradient(135deg, #e94560, #ff2e63); align-self: flex-end; border-bottom-right-radius: 4px; }
    .other { background: rgba(255,255,255,0.1); align-self: flex-start; border-bottom-left-radius: 4px; }
    .status { font-size: 0.7rem; opacity: 0.5; font-weight: 600; margin-bottom: 4px; text-transform: uppercase; } 
    
    #input-area { padding: 15px 20px; display: flex; gap: 10px; background: rgba(15, 21, 62, 0.98); border-top: 1px solid rgba(255,255,255,0.1); }
    input { flex: 1; padding: 12px 20px; border: none; border-radius: 25px; background: rgba(255,255,255,0.05); color: white; outline: none; font-size: 16px; border: 1px solid rgba(255,255,255,0.1); min-width: 0; }
    input:focus { border-color: #e94560; }
    #sendBtn { background: #e94560; }
    #login-overlay { position: fixed; inset: 0; background: #0f0c29; display: flex; flex-direction: column; align-items: center; justify-content: center; z-index: 1000; }
    #login-overlay h2 { font-size: 2rem; margin-bottom: 20px; }
    #login-overlay input { font-size: 1.2rem; padding: 15px; margin-bottom: 20px; text-align: center; border: none; border-radius: 10px; color: black; }
    #login-overlay .btn { font-size: 1.2rem; padding: 10px 30px; background: #08d9d6; color: #0f0c29; border: none; border-radius: 10px; cursor: pointer; font-weight: bold; }
    </style></head><body>
    <div id="login-overlay">
        <h2>Enter Phone Number</h2>
        <input type="text" id="phoneInput" placeholder="e.g. 555-1234">
        <button class="btn" onclick="doLogin()">Login</button>
    </div>
    <div id="splash"><h1>KABUTAR</h1></div>
    <div id="header">
        <h2 id="title">User</h2>
        <div>
            <button id="callBtn" class="btn" onclick="startCall()">Call</button>
            <button id="endBtn" class="btn" onclick="endCall()">End Call</button>
        </div>
    </div>
    <div id="chat"></div>
    <div id="input-area">
        <input type="text" id="i" placeholder="Message..." autocomplete="off" onkeydown="if(event.key==='Enter')s()">
        <button id="sendBtn" class="btn" onclick="s()">Send</button>
    </div>

    <script>
    const c=document.getElementById("chat"), i=document.getElementById("i");
    let w = null;
    let my_username = "Unknown";

    // Web Audio specific
    let audioCtx = null;
    let micStream = null;
    let micSource = null;
    let micWorklet = null;
    let micFallback = null;
    let isCalling = false;
    
    document.getElementById("header").style.display = "none";
    document.getElementById("chat").style.display = "none";
    document.getElementById("input-area").style.display = "none";

    let saved = localStorage.getItem("kabutar_phone");
    if (saved) document.getElementById("phoneInput").value = saved;

    function doLogin() {
        let p = document.getElementById("phoneInput").value.trim();
        if(!p) return;
        localStorage.setItem("kabutar_phone", p);
        my_username = p;
        
        try {
            if(!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)({sampleRate: 48000});
            if(audioCtx.state === 'suspended') audioCtx.resume();
        } catch(e) {}
        
        document.getElementById("login-overlay").style.display = "none";
        document.getElementById("header").style.display = "flex";
        document.getElementById("chat").style.display = "flex";
        document.getElementById("input-area").style.display = "flex";
        document.getElementById("title").innerText = my_username;

        const ws_proto = (location.protocol === 'https:') ? 'wss:' : 'ws:';
        w = new WebSocket(ws_proto + "//" + location.host + "/ws");

    w.onopen=()=>{ 
        w.send(JSON.stringify({type:"auth",username:my_username})); 
        a("System", "SECURE LINK ESTABLISHED (" + my_username + ")", "other"); 
    };
    
    w.onmessage=(e)=>{ 
        const d=jp(e.data); 
        if(!d) return;

        if(d.type === "message" || d.type === "chat") {
            a(d.from, d.text || d.message, d.from===my_username?"self":"other"); 
        } else if(d.type === "call_request") {
            a("System", "Incoming call from " + d.from, "other");
            document.getElementById("callBtn").innerText = "Accept";
            document.getElementById("callBtn").onclick = () => acceptCall();
        } else if(d.type === "call_accept") {
            a("System", "Call connected", "other");
            setupCallUI();
            startAudioCapture();
        } else if(d.type === "call_end" || d.type === "call_reject") {
            a("System", "Call ended", "other");
            resetCallUI();
            stopAudioCapture();
        } else if(d.type === "audio") {
            if(isCalling) playAudioData(d.data);
        }
    };
    } // End doLogin()

    function s(){ 
        const t=i.value.trim(); 
        if(!t)return; 
        w.send(JSON.stringify({type:"chat",from:my_username,to:"PureOS_User",message:t})); 
        i.value=""; 
        a("Me", t, "self");
    }

    function a(f,t,cl){ 
        const d=document.createElement("div"); 
        d.className="msg "+cl; 
        d.innerHTML="<div class='status'>"+f+"</div>"+t; 
        c.appendChild(d); 
        c.scrollTop=c.scrollHeight; 
    }

    function jp(s){ try { return JSON.parse(s); } catch(e){ return null; } }

    // Helper: convert Float32 PCM to base64-encoded Int16 PCM
    function floatToPcm16Base64(floatData) {
        const pcm16 = new Int16Array(floatData.length);
        for (let j = 0; j < floatData.length; j++) {
            let s = Math.max(-1, Math.min(1, floatData[j]));
            pcm16[j] = s < 0 ? s * 0x8000 : s * 0x7FFF;
        }
        const u8 = new Uint8Array(pcm16.buffer);
        let binary = '';
        for (let j = 0; j < u8.byteLength; j++) {
            binary += String.fromCharCode(u8[j]);
        }
        return btoa(binary);
    }

    // Audio Calling Logic
    function setupCallUI() {
        document.getElementById("callBtn").style.display = "none";
        document.getElementById("endBtn").style.display = "block";
        isCalling = true;
    }

    function resetCallUI() {
        document.getElementById("callBtn").style.display = "block";
        document.getElementById("callBtn").innerText = "Call";
        document.getElementById("callBtn").onclick = () => startCall();
        document.getElementById("endBtn").style.display = "none";
        isCalling = false;
    }

    function startCall() {
        try {
            if(!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)({sampleRate: 48000});
            if(audioCtx.state === 'suspended') audioCtx.resume();
        } catch(e) {}
        
        a("System", "Calling...", "self");
        w.send(JSON.stringify({type:"call_request", to:"PureOS_Phone"}));
        document.getElementById("callBtn").innerText = "Calling...";
    }

    function acceptCall() {
        try {
            if(!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)({sampleRate: 48000});
            if(audioCtx.state === 'suspended') audioCtx.resume();
        } catch(e) {}
        
        w.send(JSON.stringify({type:"call_accept", to:"PureOS_Phone"}));
        setupCallUI();
        startAudioCapture();
    }

    function endCall() {
        w.send(JSON.stringify({type:"call_end", to:"PureOS_Phone"}));
        resetCallUI();
        stopAudioCapture();
    }

    async function startAudioCapture() {
        if(!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)({sampleRate: 48000});
        if(audioCtx.state === 'suspended') await audioCtx.resume();

        try {
            micStream = await navigator.mediaDevices.getUserMedia({
                audio: {
                    echoCancellation: true,
                    noiseSuppression: true,
                    autoGainControl: true
                },
                video: false
            });
            micSource = audioCtx.createMediaStreamSource(micStream);

            // Try AudioWorklet first (runs on dedicated audio thread — no main-thread glitches)
            let useWorklet = false;
            if (audioCtx.audioWorklet) {
                try {
                    const workletCode = `
                        class MicProcessor extends AudioWorkletProcessor {
                            constructor() { super(); this._buf = []; }
                            process(inputs) {
                                const ch = inputs[0] && inputs[0][0];
                                if (!ch) return true;
                                for (let i = 0; i < ch.length; i++) this._buf.push(ch[i]);
                                while (this._buf.length >= 1024) {
                                    this.port.postMessage(new Float32Array(this._buf.splice(0, 1024)));
                                }
                                return true;
                            }
                        }
                        registerProcessor('mic-processor', MicProcessor);
                    `;
                    const blob = new Blob([workletCode], {type: 'application/javascript'});
                    const url = URL.createObjectURL(blob);
                    await audioCtx.audioWorklet.addModule(url);
                    URL.revokeObjectURL(url);

                    micWorklet = new AudioWorkletNode(audioCtx, 'mic-processor');
                    micWorklet.port.onmessage = (ev) => {
                        if (!isCalling) return;
                        w.send(JSON.stringify({
                            type: "audio", to: "PureOS_Phone",
                            data: floatToPcm16Base64(ev.data)
                        }));
                    };
                    micSource.connect(micWorklet);
                    // Do NOT connect to destination — that would echo mic back to speakers
                    useWorklet = true;
                    console.log("Mic: using AudioWorklet (1024 samples)");
                } catch(we) {
                    console.warn("AudioWorklet failed, falling back:", we);
                }
            }

            // Fallback: ScriptProcessor with 1024 buffer
            if (!useWorklet) {
                micFallback = audioCtx.createScriptProcessor(1024, 1, 1);
                micFallback.onaudioprocess = function(e) {
                    if (!isCalling) return;
                    const inputData = e.inputBuffer.getChannelData(0);
                    w.send(JSON.stringify({
                        type: "audio", to: "PureOS_Phone",
                        data: floatToPcm16Base64(inputData)
                    }));
                };
                micSource.connect(micFallback);
                // ScriptProcessor needs a destination to fire, but use silent gain to prevent echo
                const silentGain = audioCtx.createGain();
                silentGain.gain.value = 0;
                micFallback.connect(silentGain);
                silentGain.connect(audioCtx.destination);
                console.log("Mic: using ScriptProcessor fallback (1024 samples)");
            }
        } catch(err) {
            console.error("Mic error:", err);
            a("System", "Microphone access denied", "other");
            endCall();
        }
    }

    function stopAudioCapture() {
        if(micWorklet) { micWorklet.disconnect(); micWorklet = null; }
        if(micFallback) { micFallback.disconnect(); micFallback = null; }
        if(micSource) { micSource.disconnect(); micSource = null; }
        if(micStream) { micStream.getTracks().forEach(t => t.stop()); micStream = null; }
        nextPlayTime = 0;
    }

    let nextPlayTime = 0;

    function playAudioData(base64Data) {
        if (!audioCtx || !isCalling) return;
        try {
            const binaryStr = atob(base64Data);
            const len = binaryStr.length;
            const bytes = new Uint8Array(len);
            for (let i = 0; i < len; i++) {
                bytes[i] = binaryStr.charCodeAt(i);
            }
            const pcm16 = new Int16Array(bytes.buffer);
            const float32 = new Float32Array(pcm16.length);
            for (let i = 0; i < pcm16.length; i++) {
                float32[i] = pcm16[i] / 0x8000;
            }
            const audioBuffer = audioCtx.createBuffer(1, float32.length, 48000);
            audioBuffer.getChannelData(0).set(float32);
            const source = audioCtx.createBufferSource();
            source.buffer = audioBuffer;
            source.connect(audioCtx.destination);

            const now = audioCtx.currentTime;
            // Tighter jitter buffer: 150ms reset gap
            if (nextPlayTime < now) nextPlayTime = now + 0.15;
            // Max-drift guard: don't schedule more than 500ms ahead
            if (nextPlayTime > now + 0.5) nextPlayTime = now + 0.15;
            source.start(nextPlayTime);
            nextPlayTime += audioBuffer.duration;
        } catch(e) {}
    }
    </script></body></html>"""
    return web.Response(text=html, content_type='text/html')

async def main():
    tcp_server = await asyncio.start_server(handle_tcp, '0.0.0.0', TCP_PORT)
    app = web.Application()
    app.add_routes([
        web.get('/', handle_index), 
        web.get('/ws', handle_ws_route),
        web.get('/ytsearch', handle_ytsearch),
        web.get('/ytplay', handle_ytplay),
        web.get('/ytthumb', handle_ytthumb),
        web.get('/ytlyrics', handle_ytlyrics)
    ])
    runner = web.AppRunner(app)
    await runner.setup()
    site = web.TCPSite(runner, '0.0.0.0', HTTP_WS_PORT)
    
    print(f"--- KABUTAR RELAY (VOICE ENABLED) READY ---")
    await asyncio.gather(tcp_server.serve_forever(), site.start())

if __name__ == "__main__":
    asyncio.run(main())
