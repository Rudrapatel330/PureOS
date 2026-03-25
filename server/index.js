const net = require('net');
const WebSocket = require('ws');
const http = require('http');

const TCP_PORT = 7860;
const WS_PORT = 7861;
const HTTP_PORT = 7862; 

const clients = new Map();

function broadcastToAllWS(data) {
    const payload = JSON.stringify(data);
    wss.clients.forEach(client => {
        if (client.readyState === WebSocket.OPEN) client.send(payload);
    });
}

function broadcast(target, data) {
    const client = clients.get(target);
    const payload = JSON.stringify(data);
    
    console.log(`[Relay] Routing ${data.from} -> ${target}: ${data.text}`);
    
    // Always sync back to all web clients
    broadcastToAllWS(data);

    if (client) {
        if (client.write) { 
            // ADD NEWLINE for TCP clients to help with framing
            client.write(payload + '\n'); 
            console.log(`   -> Delivered to TCP`);
        }
        else if (client.send) { 
            client.send(payload); 
            console.log(`   -> Delivered to WS`);
        }
    } else {
        console.log(`   !! Target ${target} OFFLINE !!`);
    }
}

const tcpServer = net.createServer((socket) => {
    let username = null;
    let buffer = ""; // TCP buffer to handle fragmentation

    socket.on('data', (chunk) => {
        buffer += chunk.toString();
        let lines = buffer.split('\n');
        buffer = lines.pop(); // Keep partial line in buffer

        lines.forEach(line => {
            if (!line.trim()) return;
            try {
                const data = JSON.parse(line);
                if (data.type === 'auth') {
                    username = data.username;
                    clients.set(username, socket);
                    console.log(`[TCP] ${username} connected`);
                    socket.write(JSON.stringify({ type: 'status', message: 'Connected' }) + '\n');
                } else if (data.type === 'chat') {
                    broadcast(data.to, { type: 'message', from: data.from, text: data.message });
                }
            } catch (e) {
                console.log("[TCP] Parse error on line:", line);
            }
        });
    });
    socket.on('close', () => { if (username) clients.delete(username); });
    socket.on('error', () => {});
});
tcpServer.listen(TCP_PORT, '0.0.0.0');

const wss = new WebSocket.Server({ port: WS_PORT });
wss.on('connection', (ws) => {
    let username = "Android_User";
    ws.on('message', (m) => {
        try {
            const d = JSON.parse(m.toString());
            if (d.type === 'auth') { username = d.username; clients.set(username, ws); }
            else if (d.type === 'chat') { broadcast(d.to, { type: 'message', from: d.from, text: d.message }); }
        } catch (e) {}
    });
    ws.on('close', () => { if (username) clients.delete(username); });
});

var html = '<!DOCTYPE html><html><head><title>PureChat Dashboard</title><style>' +
    'body { font-family: sans-serif; background: #1a1a2e; color: white; display: flex; flex-direction: column; height: 100vh; margin: 0; }' +
    '#chat { flex: 1; padding: 20px; overflow-y: auto; display: flex; flex-direction: column; gap: 10px; }' +
    '.msg { padding: 10px; border-radius: 10px; max-width: 70%; line-height: 1.4; border: 1px solid rgba(255,255,255,0.1); }' +
    '.self { background: #e94560; align-self: flex-end; } .other { background: #0f3460; align-self: flex-start; }' +
    '.status { font-size: 0.7em; opacity: 0.7; font-weight: bold; margin-bottom: 3px; } ' +
    '#input { padding: 20px; display: flex; gap: 10px; background: #16213e; }' +
    'input { flex: 1; padding: 12px; border: none; border-radius: 5px; background: #0f3460; color: white; outline: none; } ' +
    'button { padding: 10px 25px; background: #e94560; color: white; border: none; border-radius: 5px; cursor: pointer; font-weight: bold; }' +
    '</style></head><body><div id="chat"></div><div id="input"><input type="text" id="i" placeholder="Message to PureOS..." onkeydown="if(event.key===\'Enter\')s()"><button onclick="s()">Send</button></div>' +
    '<script>' +
    'const c=document.getElementById("chat"), i=document.getElementById("i"), w=new WebSocket("ws://"+window.location.hostname+":7861");' +
    'w.onopen=()=>{ w.send(JSON.stringify({type:"auth",username:"Android_User"})); a("System", "Ready to chat!", "other"); };' +
    'w.onmessage=(e)=>{ const d=JSON.parse(e.data); if(d.type==="message") a(d.from,d.text,d.from==="Android_User"?"self":"other"); };' +
    'function s(){ const t=i.value.trim(); if(!t)return; w.send(JSON.stringify({type:"chat",from:"Android_User",to:"PureOS_User",message:t})); i.value=""; }' +
    'function a(f,t,cl){ const d=document.createElement("div"); d.className="msg "+cl; d.innerHTML="<div class=\'status\'>"+f+"</div>"+t; c.appendChild(d); c.scrollTop=c.scrollHeight; }' +
    '</script></body></html>';

http.createServer(function(req, res) {
    res.writeHead(200, { 'Content-Type': 'text/html' });
    res.end(html);
}).listen(HTTP_PORT, '0.0.0.0', function() {
    console.log('--- RELAY SERVER READY ---');
    console.log('Dashboard: http://localhost:' + HTTP_PORT);
});
