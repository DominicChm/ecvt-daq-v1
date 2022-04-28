import {derived, readable, writable} from "svelte/store";
import {noop, subscribe} from "svelte/internal";


export const socket = readable<null | WebSocket>(null, (set) => {
    let ws: WebSocket;
    let tout: number;

    function connect() {
        ws = new WebSocket(`ws://${window.location.hostname}:3000/ws`);
        //ws = new WebSocket(`ws://192.168.1.2:80/ws`);

        ws.onopen = () => set(ws);
        ws.onclose = function (e) {
            //console.log('Socket is closed. Reconnect will be attempted in 1 second.', e.reason);
            set(null);
            tout = setTimeout(connect, 500);
        };

        ws.onerror = function (err) {
            console.error('Socket encountered error: ', err, 'Closing socket');
            ws.close();
        };
    }

    connect();

    return () => {
        ws.close();
        ws.onclose = null;
        ws.onerror = null;
        clearTimeout(tout);
    }
});

/* Stores the last received JSON message */
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

/* Derives header data from message stream */
export const header = derived(message, ($message: any, set) => {
    if ($message?.type !== "header") return;
    set($message.data.trim().split(", "));
}, []);

/* Derives raw frame (CSV) data from message stream */
export const raw_frame = derived(message, ($message: any, set) => {
    if ($message?.type !== "frame") return;
    set($message.data);
}, "");

/* Combines header and data streams to create data objects for rendering */
export const frame = derived([raw_frame, header], ([$raw_frame, $header]) => {
    const entries = $header.map((k, i) => [k, $raw_frame[i]]);
    return Object.fromEntries(entries);
}, {});

export const connected = derived(socket, ($socket) => !!$socket);

export const runs = derived([connected], (async ([$connected]: any[], set: any) => {
    console.log("RUNS")
    if (!$connected) return;
    let res = await fetch("http://localhost:3000/runs.txt");
    const run_arr = (await res.text()).split(",")
        .map(v => v.trim())
        .filter(v => v);
    console.log(run_arr);
    set(run_arr);
}) as any);
// Make sure frames are always listening for data.
subscribe(frame, noop);



