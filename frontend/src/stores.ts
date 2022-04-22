import {derived, readable, writable} from "svelte/store";


export const socket = readable<null | WebSocket>(null, (set) => {
    let ws: WebSocket;

    function connect() {
        ws = new WebSocket(`ws://${window.location.hostname}:80/ws`);
        //ws = new WebSocket(`ws://192.168.1.2:80/ws`);

        ws.onopen = () => set(ws);
        ws.onclose = function (e) {
            //console.log('Socket is closed. Reconnect will be attempted in 1 second.', e.reason);
            set(null);
            setTimeout(connect, 500);
        };

        ws.onerror = function (err) {
            console.error('Socket encountered error: ', err, 'Closing socket');
            ws.close();
        };
    }

    connect();

    return () => {
        ws.close()
        ws.onclose = null;
        ws.onerror = null;
    }
});


export const message = derived(socket, ($socket, set) => {
    if (!$socket) return;

    console.log($socket);
    $socket.onmessage = ({data}) => {
        try {
            set(JSON.parse(data));
        } catch (e) {
            console.log(`MESSAGE ERROR: ${data}`);
            console.error(e);
        }
    }

    return () => {
        if ($socket)
            $socket.onmessage = null;
    }
}, null)

export const header = derived(message, ($message: any, set) => {
    if ($message?.type !== "header") return;
    set($message.header.trim().split(", "));
}, []);

export const data = derived(message, ($message: any, set) => {
    if ($message?.type !== "frame") return;
    console.log("Frame!");

}, null);

export const connected = derived(socket, ($socket) => !!$socket);
