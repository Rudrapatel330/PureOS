import asyncio
import json
from aiohttp import web, WSCloseCode

TCP_PORT = 7860
HTTP_WS_PORT = 7862

# username -> (type, obj)
clients = {}

async def broadcast_ws(data):
    payload = json.dumps(data, separators=(',', ':'))
    ws_clients = [c[1] for c in clients.values() if c[0] == 'ws']
    if ws_clients:
        await asyncio.gather(*[c.send_str(payload) for c in ws_clients if not c.closed], return_exceptions=True)

async def broadcast(target, data):
    print(f"[Relay] Routing {data['from']} -> {target}: {data['text']}")
    await broadcast_ws(data)
    if target in clients:
        ctype, cobj = clients[target]
        payload = json.dumps(data, separators=(',', ':'))
        if ctype == 'tcp':
            cobj.write((payload + '\n').encode())
            await cobj.drain()
            print(f"   -> Delivered to TCP")
        elif ctype == 'ws':
            if not cobj.closed:
                await cobj.send_str(payload)
                print(f"   -> Delivered to WS")

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
                elif data.get('type') == 'chat':
                    await broadcast(data.get('to'), {"type": "message", "from": data.get('from'), "text": data.get('message')})
            except Exception as e: pass
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
                elif data.get('type') == 'chat':
                    await broadcast(data.get('to'), {"type": "message", "from": data.get('from'), "text": data.get('message')})
    finally:
        if username and username in clients: del clients[username]
    return ws

async def handle_index(request):
    html = """<!DOCTYPE html><html><head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
    <title>KABUTAR PureChat</title>
    <style>
    @import url('https://fonts.googleapis.com/css2?family=Outfit:wght@300;600&display=swap');
    body { font-family: 'Outfit', sans-serif; background: #0f0c29; background: linear-gradient(135deg, #0f0c29, #302b63, #24243e); color: white; display: flex; flex-direction: column; height: 100vh; height: 100dvh; margin: 0; overflow: hidden; }
    
    /* Splash Screen */
    #splash { position: fixed; inset: 0; background: #0f0c29; display: flex; align-items: center; justify-content: center; z-index: 999; animation: fadeOut 0.8s ease 2s forwards; pointer-events: none; }
    #splash h1 { font-size: 15vw; font-weight: 600; letter-spacing: 2vw; background: linear-gradient(90deg, #ff2e63, #ffffff, #08d9d6); background-size: 200% auto; -webkit-background-clip: text; -webkit-text-fill-color: transparent; animation: popIn 1s cubic-bezier(0.175, 0.885, 0.32, 1.275) forwards, shine 3s linear infinite; opacity: 0; filter: drop-shadow(0 0 15px rgba(233, 69, 96, 0.5)); }
    
    @keyframes shine { to { background-position: 200% center; } }
    @keyframes popIn { 0% { transform: scale(0.5); opacity: 0; } 100% { transform: scale(1); opacity: 1; } }
    @keyframes fadeOut { 0% { opacity: 1; transform: scale(1); } 100% { opacity: 0; transform: scale(1.1); visibility: hidden; } }

    /* Main UI */
    #chat { flex: 1; padding: 20px; overflow-y: auto; display: flex; flex-direction: column; gap: 12px; scroll-behavior: smooth; }
    #chat::-webkit-scrollbar { width: 4px; }
    #chat::-webkit-scrollbar-thumb { background: rgba(255,255,255,0.1); border-radius: 10px; }
    
    .msg { padding: 12px 16px; border-radius: 18px; max-width: 80%; line-height: 1.5; font-size: 1rem; box-shadow: 0 4px 15px rgba(0,0,0,0.2); animation: slideIn 0.3s ease-out forwards; backdrop-filter: blur(8px); -webkit-backdrop-filter: blur(8px); border: 1px solid rgba(255,255,255,0.05); }
    .self { background: linear-gradient(135deg, #e94560, #ff2e63); align-self: flex-end; border-bottom-right-radius: 4px; }
    .other { background: rgba(255,255,255,0.1); align-self: flex-start; border-bottom-left-radius: 4px; }
    
    @keyframes slideIn { 0% { transform: translateX(10px); opacity: 0; } 100% { transform: translateX(0); opacity: 1; } }
    .status { font-size: 0.7rem; opacity: 0.5; font-weight: 600; margin-bottom: 4px; text-transform: uppercase; letter-spacing: 1px; } 
    
    #input-area { padding: 15px 20px; display: flex; gap: 10px; background: rgba(15, 21, 62, 0.98); border-top: 1px solid rgba(255,255,255,0.1); padding-bottom: calc(15px + env(safe-area-inset-bottom)); min-height: 50px; }
    input { flex: 1; padding: 12px 20px; border: none; border-radius: 25px; background: rgba(255,255,255,0.05); color: white; outline: none; font-size: 16px; border: 1px solid rgba(255,255,255,0.1); min-width: 0; }
    input:focus { border-color: #e94560; background: rgba(255,255,255,0.1); }
    button { padding: 0 25px; height: 45px; background: #e94560; color: white; border: none; border-radius: 25px; cursor: pointer; font-weight: 600; transition: 0.2s; flex-shrink: 0; }
    button:active { transform: scale(0.95); }
    </style></head><body>
    <div id="splash"><h1>KABUTAR</h1></div>
    <div id="chat"></div>
    <div id="input-area"><input type="text" id="i" placeholder="Message..." autocomplete="off" onkeydown="if(event.key==='Enter')s()"><button onclick="s()">Send</button></div>
    <script>
    const c=document.getElementById("chat"), i=document.getElementById("i");
    const ws_proto = (location.protocol === 'https:') ? 'wss:' : 'ws:';
    const w = new WebSocket(ws_proto + "//" + location.host + "/ws");
    w.onopen=()=>{ w.send(JSON.stringify({type:"auth",username:"Android_User"})); a("System", "SECURE LINK STABLISHED", "other"); };
    w.onmessage=(e)=>{ const d=jp(e.data); if(d && d.type==="message") a(d.from,d.text,d.from==="Android_User"?"self":"other"); };
    function s(){ const t=i.value.trim(); if(!t)return; w.send(JSON.stringify({type:"chat",from:"Android_User",to:"PureOS_User",message:t})); i.value=""; }
    function a(f,t,cl){ const d=document.createElement("div"); d.className="msg "+cl; d.innerHTML="<div class='status'>"+f+"</div>"+t; c.appendChild(d); c.scrollTop=c.scrollHeight; }
    function jp(s){ try { return JSON.parse(s); } catch(e){ return null; } }
    </script></body></html>"""
    return web.Response(text=html, content_type='text/html')

async def main():
    tcp_server = await asyncio.start_server(handle_tcp, '0.0.0.0', TCP_PORT)
    app = web.Application()
    app.add_routes([web.get('/', handle_index), web.get('/ws', handle_ws_route)])
    runner = web.AppRunner(app)
    await runner.setup()
    site = web.TCPSite(runner, '0.0.0.0', HTTP_WS_PORT)
    
    print(f"--- KABUTAR RELAY (UNIFIED) READY ---")
    await asyncio.gather(tcp_server.serve_forever(), site.start())

if __name__ == "__main__":
    asyncio.run(main())
