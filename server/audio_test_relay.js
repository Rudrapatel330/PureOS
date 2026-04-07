const net = require('net');
const WebSocket = require('ws');
const http = require('http');

const TCP_PORT = 7870;
const HTTP_PORT = 7871;
const WS_PORT = 7872;

// The one TCP client we are testing (PureOS)
let osClient = null;

// Forward web socket audio to the OS client
const wss = new WebSocket.Server({ port: WS_PORT });
wss.on('connection', (ws) => {
    console.log('[WS] Web client connected');
    ws.on('message', (m) => {
        try {
            const data = JSON.parse(m.toString());
            if (data.type === 'audio') {
                if (osClient) {
                    console.log(`[Relay] Forwarding audio to OS: ${data.data.length} b64 chars`);
                    const payload = JSON.stringify(data) + '\n';
                    osClient.write(payload);
                } else {
                    console.log(`[Relay] OS not connected, dropping audio (${data.data.length} chars)`);
                }
            } else if (data.type === 'chat') {
               console.log('[Relay] Chat message from web:', data.message);
               if (osClient) {
                   osClient.write(JSON.stringify(data) + '\n');
               }
            }
        } catch (e) {
            console.error('[WS] Parse error');
        }
    });
    ws.on('close', () => console.log('[WS] Web client disconnected'));
});

const tcpServer = net.createServer((socket) => {
    let buffer = ""; 
    osClient = socket;
    console.log(`[TCP] OS Client connected from ${socket.remoteAddress}`);

    socket.on('data', (chunk) => {
        buffer += chunk.toString();
        let lines = buffer.split('\n');
        buffer = lines.pop(); // Keep partial line in buffer

        lines.forEach(line => {
            if (!line.trim()) return;
            try {
                const data = JSON.parse(line);
                if (data.type === 'auth') {
                    console.log(`[TCP] OS identified as: ${data.username}`);
                    socket.write(JSON.stringify({ type: 'status', message: 'Test Relay Connected' }) + '\n');
                } else {
                    console.log(`[TCP] OS sent:`, data);
                }
            } catch (e) {
                console.log("[TCP] Parse error on line:", line);
            }
        });
    });
    socket.on('close', () => { 
        console.log('[TCP] OS Client disconnected');
        if (osClient === socket) osClient = null; 
    });
    socket.on('error', (err) => console.log('[TCP] Error:', err.message));
});
tcpServer.listen(TCP_PORT, '0.0.0.0');

const html = `<!DOCTYPE html><html><head>
<title>Audio Sender Test</title>
<style>
  body { font-family: sans-serif; background: #222; color: #fff; padding: 20px; text-align: center; }
  .btn { padding: 15px 30px; font-size: 18px; cursor: pointer; background: #08d9d6; border: none; border-radius: 5px; margin: 10px; font-weight: bold; }
  #log { text-align: left; background: #111; padding: 10px; height: 300px; overflow-y: scroll; border: 1px solid #444; font-family: monospace; font-size: 12px; margin-top: 20px; }
</style>
</head><body>
<h2>Audio Sender Test (Web -> OS)</h2>
<p>This page captures your microphone and sends it to the OS test app.</p>
<div>
  <button id="startBtn" class="btn" onclick="startAudio()">1. Start Mic</button>
  <button id="sendBtn" class="btn" onclick="sendTestSignal()" disabled>2. Send 440Hz Beep</button>
</div>
<canvas id="viz" width="300" height="60" style="background:#000; border:1px solid #444; margin:10px;"></canvas>
<div id="status" style="margin:10px; color:#08d9d6; font-weight:bold;">Status: WS Connected</div>
<div id="log"></div>

<script>
const ws = new WebSocket("ws://" + location.hostname + ":${WS_PORT}");
const logEl = document.getElementById("log");
const statusEl = document.getElementById("status");
const vizCtx = document.getElementById("viz") ? document.getElementById("viz").getContext("2d") : null;
let audioCtx, pcmNode, beepInterval = null;

function log(msg) {
    logEl.innerHTML += "<div>" + new Date().toISOString().substring(11,23) + ": " + msg + "</div>";
    logEl.scrollTop = logEl.scrollHeight;
}

ws.onopen = () => { statusEl.innerText = "Status: WS Connected"; statusEl.style.color = "#08d9d6"; log("WebSocket connected"); };
ws.onclose = () => { statusEl.innerText = "Status: WS Disconnected"; statusEl.style.color = "#ff2e63"; log("WebSocket closed"); };

function toggleContinuousBeep() {
    if (beepInterval) {
        clearInterval(beepInterval);
        beepInterval = null;
        document.getElementById("beepBtn").innerText = "2. Toggle Loop Beep (OFF)";
        document.getElementById("beepBtn").style.background = "#08d9d6";
        log("Continuous beep stopped");
    } else {
        document.getElementById("beepBtn").innerText = "2. Toggle Loop Beep (ON)";
        document.getElementById("beepBtn").style.background = "#ff2e63";
        log("Continuous beep started (0.1s packets)");
        beepInterval = setInterval(sendTestSignal, 100);
    }
}

async function startAudio() {
    if(!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)({sampleRate: 48000});
    try {
        const stream = await navigator.mediaDevices.getUserMedia({audio: true});
        const source = audioCtx.createMediaStreamSource(stream);
        
        pcmNode = audioCtx.createScriptProcessor(4096, 1, 1);
        pcmNode.onaudioprocess = function(e) {
            const inputData = e.inputBuffer.getChannelData(0);
            
            // Draw visualizer
            if (vizCtx) {
                vizCtx.fillStyle = "#000"; vizCtx.fillRect(0,0,300,60);
                vizCtx.strokeStyle = "#08d9d6"; vizCtx.beginPath();
                let step = inputData.length / 300;
                for(let i=0; i<300; i++) {
                    let v = inputData[Math.floor(i*step)] * 30;
                    vizCtx.lineTo(i, 30 + v);
                }
                vizCtx.stroke();
            }

            const pcm16 = new Int16Array(inputData.length);
            let maxAmp = 0;
            for (let i = 0; i < inputData.length; i++) {
                let s = Math.max(-1, Math.min(1, inputData[i]));
                // Amplify significantly for weak mics
                s = s * 4.0;
                s = Math.max(-1, Math.min(1, s));
                pcm16[i] = s < 0 ? s * 0x8000 : s * 0x7FFF;
                if (Math.abs(pcm16[i]) > maxAmp) maxAmp = Math.abs(pcm16[i]);
            }
            
            if (maxAmp > 50) { // Only send non-silent audio
                const u8 = new Uint8Array(pcm16.buffer);
                let binary = '';
                for (let i = 0; i < u8.byteLength; i += 1024) {
                    binary += String.fromCharCode.apply(null, u8.slice(i, i + 1024));
                }
                ws.send(JSON.stringify({
                    type: "audio",
                    from: "TestSender",
                    to: "PureOS_Test",
                    data: btoa(binary)
                }));
            }
        };
        source.connect(pcmNode);
        pcmNode.connect(audioCtx.destination);
        document.getElementById("startBtn").innerText = "Mic Active";
        document.getElementById("startBtn").style.background = "#ff2e63";
        document.getElementById("startBtn").onclick = stopAudio;
        log("Microphone processing started");
    } catch(err) {
        log("Mic error: " + err.message);
    }
}

function stopAudio() {
    if(pcmNode) {
        pcmNode.disconnect();
        pcmNode = null;
    }
    document.getElementById("startBtn").innerText = "1. Start Mic";
    document.getElementById("startBtn").style.background = "#08d9d6";
    document.getElementById("startBtn").onclick = startAudio;
    log("Microphone stopped");
}

function sendTestSignal() {
    log("Sending 440Hz beep...");
    const samples = 4800; // 0.1s
    const pcm16 = new Int16Array(samples);
    for (let i = 0; i < samples; i++) {
        pcm16[i] = Math.sin((i / 48000) * 440 * 2 * Math.PI) * 0x7FFF;
    }
    const u8 = new Uint8Array(pcm16.buffer);
    let binary = '';
    for (let i = 0; i < u8.byteLength; i += 1024) {
        binary += String.fromCharCode.apply(null, u8.slice(i, i + 1024));
    }
    ws.send(JSON.stringify({
        type: "audio",
        from: "TestSender",
        to: "PureOS_Test",
        data: btoa(binary)
    }));
}
</script></body></html>`;

http.createServer(function(req, res) {
    res.writeHead(200, { 'Content-Type': 'text/html' });
    res.end(html);
}).listen(HTTP_PORT, '0.0.0.0', function() {
    console.log('--- AUDIO TEST RELAY READY ---');
    console.log('1. Run this server');
    console.log(`2. Open http://localhost:${HTTP_PORT} in your browser`);
    console.log(`3. Boot PureOS and launch the 'Audio Test' app`);
    console.log(`   (It will connect to TCP ${TCP_PORT})`);
});
