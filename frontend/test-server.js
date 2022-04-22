const {WebSocketServer} = require('ws');
const ctypes = require("c-type-util");
const {cStruct} = require("c-type-util");

const wss = new WebSocketServer({port: 80, path: "/ws"});

console.log("Started");
const raw_ct = ctypes.cStruct({
    startBytes: ctypes.uint16,
    time: ctypes.uint32,
    // Engine
    engaged: ctypes.boolean,
    eState: ctypes.int8,
    eSpeed: ctypes.int16,
    ePID: ctypes.int16,
    eP: ctypes.int16,
    eI: ctypes.int16,
    eD: ctypes.int16,
    // Primary
    pState: ctypes.int8,
    pEnc: ctypes.int32,
    pLC: ctypes.int16,
    pPID: ctypes.int16,
    // Secondary
    sState: ctypes.int8,
    sEnc: ctypes.int32,
    sLC: ctypes.int16,
    sPID: ctypes.int16,
});

function appendBuffers(buffer1, buffer2) {
    var tmp = new Uint8Array(buffer1.byteLength + buffer2.byteLength);
    tmp.set(new Uint8Array(buffer1), 0);
    tmp.set(new Uint8Array(buffer2), buffer1.byteLength);
    return tmp.buffer;
}


wss.on('connection', function connection(ws) {
    ws.send(JSON.stringify({type: "header", header: 'Test, test2, test3, test4'}));

});