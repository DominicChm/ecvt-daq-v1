const {WebSocketServer} = require('ws');
const ctypes = require("c-type-util");
const express = require("express");
const http = require("http");

const app = express();

app.use(express.static("test-dir"));
//initialize a simple http server
const server = http.createServer(app);

//initialize the WebSocket server instance
const wss = new WebSocketServer({server});

wss.on('connection', function connection(ws) {
    ws.send(JSON.stringify({type: "event", data: "runs"}));
    //setInterval(send_garbage, 1000);
});

server.listen(3000, () => console.log("Running on 3000"));