import {derived, readable} from "svelte/store";
import {noop, subscribe} from "svelte/internal";

import.meta.hot;

interface FileEntry {
    type: "entry" | "deletion";
    file_base: string;
    name: string;
    description: string;
}

//@ts-ignore
console.log(__SNOWPACK_ENV__.MODE);

export const socket = readable<null | WebSocket>(null, (set) => {
    let ws: WebSocket;
    let destroyed = false;

    function onOpen() {
        set(ws)
    }

    function onClose() {
        console.log("Socket closed - Attempting reconnection...");
        ws.removeEventListener("open", onOpen);
        ws.removeEventListener("close", onClose)
        ws.removeEventListener("error", onError);
        set(null);

        if (!destroyed) setTimeout(connect, 500);
    }

    function onError(err: any) {
        console.error('Socket encountered error: ', err, 'Closing socket');
        ws.close();
    }

    function connect() {
        // ws = new
        ws = new WebSocket(`ws://${window.location.hostname}:${window.location.port}/ws`);
        // ws = new WebSocket(`ws://1.2.3.4/ws`);

        ws.addEventListener("open", onOpen);
        ws.addEventListener("close", onClose)
        ws.addEventListener("error", onError);
    }

    connect();

    return () => {
        console.log("Closing WS (in theory)");
        destroyed = true;
        ws.close();
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

export const status = derived(message, ($message: any, set) => {
    if ($message?.type !== "status") return;
    set($message.data);
}, {logging: false});

/* Combines header and data streams to create data objects for rendering */
export const frame = derived([raw_frame, header], ([$raw_frame, $header]) => {
    const parts = $raw_frame.split(',').map(v => v.trim()).filter(v => v);
    const entries = $header.map((k, i) => [k, parts[i]]);
    return Object.fromEntries(entries);
}, {});

export const connected = derived(socket, ($socket) => !!$socket);

/**
 * Store that contains an array of available run names.
 */
export const runs = derived([connected, message], (async ([$connected, $message]: any[], set: any) => {
    if (!$connected) return;
    if (!$message || $message.type !== "event" || $message.data !== "runs") return;
    console.log("RUNS")

    let res = await fetch(`r.jsonl`, {});
    const run_arr = (await res.text()).split("\n")
        .filter(v => v)
        .map(v => JSON.parse(v))
        .reduce((acc: Map<string, FileEntry>, v: FileEntry) => {
            switch (v.type) {
                case "entry":
                    acc.set(v.file_base, v);
                    break;
                case "deletion":
                    acc.delete(v.file_base);
                    break;
            }
            return acc;
        }, new Map());

    const sorted = Array.from(run_arr.values())
        .sort((a: any, b: any) => b.file_base - a.file_base);
    set(sorted);
}) as any);
// Make sure frames are always listening for data.
subscribe(frame, noop);



