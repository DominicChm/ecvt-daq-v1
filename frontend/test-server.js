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
    let b = new ArrayBuffer(0);
    let size = 1000;
    let ctr = 0;

    console.log("Connection!");

    ws.send('something');

    setInterval(send_garbage, 100);

    function send_garbage() {
        while (b.byteLength < size)
            b = appendBuffers(b, create_garbage());

        ws.send(b.slice(0, size));
        b = b.slice(size);
    }

    function create_garbage() {
        return raw_ct.allocLE({
            startBytes: 0xAAAA,
            time: ctr++,
            // Engine
            engaged: false,
            eState: 1,
            eSpeed: 100 * Math.sin(ctr / 5),
            ePID: 3,
            eP: 4,
            eI: 5,
            eD: 6,

            // Primary
            pState: 1,
            pEnc: 2,
            pLC: 3,
            pPID: 4,

            // Secondary
            sState: 1,
            sEnc: 2,
            sLC: 3,
            sPID: 4,
        });
    }
});