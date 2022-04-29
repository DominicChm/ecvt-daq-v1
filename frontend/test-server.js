const WebSocket = require('ws');
const express = require("express");
const http = require("http");

const app = express();
app.use(function(req, res, next) {
    res.header("Access-Control-Allow-Origin", "*"); // update to match the
    // domain you will make the request from
    res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    next();
});

app.use(express.static("test-dir"));

//initialize a simple http server
const server = http.createServer(app);

//initialize the WebSocket server instance
const wss = new WebSocket.Server({server, path: "/ws"});


wss.on("error", () => console.log("SAFBIA"));
wss.on('connection', function connection(ws) {
    console.log("CONN");
    ws.send(JSON.stringify({
        type: "header",
        data: 'Test, test2, test3, test4'
    }));

    ws.send(JSON.stringify({
        type: "event",
        data: 'runs'
    }));

    let ctr = 0;
    setInterval(() => {
        ws.send(JSON.stringify({
            type: "frame",
            data: new Array(4).fill(ctr++)
        }));
    }, 1000);
});

server.listen(3000, () => console.log("Listening!"));