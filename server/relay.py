import asyncio
import json
from aiohttp import web, WSCloseCode

TCP_PORT = 7860
HTTP_WS_PORT = 7862

# username -> (type, obj)
clients = {}

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
                    await broadcast(data['to'], data)
            except Exception as e:
                pass
    finally:
        if username and username in clients: del clients[username]
        writer.close()

# --- Unified HTTP & WebSocket Handler ---
async def handle_ws_route(request):
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
    </style></head><body>
    <div id="splash"><h1>KABUTAR</h1></div>
    <div id="header">
        <h2 id="title">PureOS_User</h2>
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
    const ws_proto = (location.protocol === 'https:') ? 'wss:' : 'ws:';
    const w = new WebSocket(ws_proto + "//" + location.host + "/ws");
    
    // Web Audio specific
    let audioCtx = null;
    let pcmNode = null;
    let isCalling = false;

    w.onopen=()=>{ 
        w.send(JSON.stringify({type:"auth",username:"Android_User"})); 
        a("System", "SECURE LINK ESTABLISHED", "other"); 
    };
    
    w.onmessage=(e)=>{ 
        const d=jp(e.data); 
        if(!d) return;

        if(d.type === "message" || d.type === "chat") {
            a(d.from, d.text || d.message, d.from==="Android_User"?"self":"other"); 
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

    function s(){ 
        const t=i.value.trim(); 
        if(!t)return; 
        w.send(JSON.stringify({type:"chat",from:"Android_User",to:"PureOS_User",message:t})); 
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
        a("System", "Calling...", "self");
        w.send(JSON.stringify({type:"call_request", to:"PureOS_User"}));
        document.getElementById("callBtn").innerText = "Calling...";
    }

    function acceptCall() {
        w.send(JSON.stringify({type:"call_accept", to:"PureOS_User"}));
        setupCallUI();
        startAudioCapture();
    }

    function endCall() {
        w.send(JSON.stringify({type:"call_end", to:"PureOS_User"}));
        resetCallUI();
        stopAudioCapture();
    }

    async function startAudioCapture() {
        if(!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)({sampleRate: 48000});
        if(audioCtx.state === 'suspended') await audioCtx.resume();

        try {
            const stream = await navigator.mediaDevices.getUserMedia({audio: true, video: false});
            const source = audioCtx.createMediaStreamSource(stream);
            
            pcmNode = audioCtx.createScriptProcessor(8192, 1, 1);
            pcmNode.onaudioprocess = function(e) {
                if (!isCalling) return;
                const inputData = e.inputBuffer.getChannelData(0);
                const pcm16 = new Int16Array(inputData.length);
                for (let i = 0; i < inputData.length; i++) {
                    let s = Math.max(-1, Math.min(1, inputData[i]));
                    pcm16[i] = s < 0 ? s * 0x8000 : s * 0x7FFF;
                }
                const u8 = new Uint8Array(pcm16.buffer);
                let binary = '';
                for (let i = 0; i < u8.byteLength; i++) {
                    binary += String.fromCharCode(u8[i]);
                }
                w.send(JSON.stringify({
                    type: "audio",
                    to: "PureOS_User",
                    data: btoa(binary)
                }));
            };
            source.connect(pcmNode);
            pcmNode.connect(audioCtx.destination);
            // Save stream to stop later
            pcmNode.stream = stream;
        } catch(err) {
            console.error("Mic error:", err);
            a("System", "Microphone access denied", "other");
            endCall();
        }
    }

    function stopAudioCapture() {
        if(pcmNode) {
            pcmNode.disconnect();
            if(pcmNode.stream) pcmNode.stream.getTracks().forEach(t => t.stop());
            pcmNode = null;
        }
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

            // Schedule chunks sequentially with a small jitter buffer
            const now = audioCtx.currentTime;
            // Add a 150ms buffer to absorb network jitter
            if (nextPlayTime < now) nextPlayTime = now + 0.15;
            source.start(nextPlayTime);
            nextPlayTime += audioBuffer.duration;
        } catch(e) {}
    }
    </script></body></html>"""
    return web.Response(text=html, content_type='text/html')

async def main():
    tcp_server = await asyncio.start_server(handle_tcp, '0.0.0.0', TCP_PORT)
    app = web.Application()
    app.add_routes([web.get('/', handle_index), web.get('/ws', handle_ws_route)])
    runner = web.AppRunner(app)
    await runner.setup()
    site = web.TCPSite(runner, '0.0.0.0', HTTP_WS_PORT)
    
    print(f"--- KABUTAR RELAY (VOICE ENABLED) READY ---")
    await asyncio.gather(tcp_server.serve_forever(), site.start())

if __name__ == "__main__":
    asyncio.run(main())
