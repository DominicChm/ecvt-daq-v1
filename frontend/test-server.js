const {WebSocketServer} = require('ws');
const ctypes = require("c-type-util");
const {cStruct} = require("c-type-util");

const wss = new WebSocketServer({port: 80, path: "/ws"});

wss.on('connection', function connection(ws) {
    ws.send(JSON.stringify({
        type: "header",
        data: 'Test, test2, test3, test4'
    }));

    let ctr = 0;
    setInterval(() => {
        ws.send(JSON.stringify({
            type: "frame",
            data: new Array(4).fill(ctr++)
        }));
    }, 1000);

    setInterval(() => {
        ws.send(JSON.stringify({
            type: "files",
            data:
        }));
    }, 1000);


});