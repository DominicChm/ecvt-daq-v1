<script>
    import {onMount} from "svelte";

    export let ymin = 0;
    export let ymax = 100;
    export let zoom = 100; //same unit as time
    export let currentTime = 0;
    export let xkey = "time";
    export let key;
    export let data;

    if (!data || !key) throw new Error("NO DATA OR KEY PASSED!");

    let canvas;


    onMount(() => {
        const ctx = canvas.getContext('2d');
        let frame = requestAnimationFrame(loop);
        console.log("SDAVBFHASG")

        function scalePoint(x, y) {
            let xmin = currentTime - zoom;
            let xmax = currentTime;
            return [canvas.width * (x - xmin) / (xmax - xmin), canvas.height * (1 - (y - ymin) / (ymax - ymin))]
        }

        function loop(t) {
            frame = requestAnimationFrame(loop);

            let rootPoint = data[data.length - 1];
            if (!rootPoint || rootPoint[xkey] == null || rootPoint[key] == null) return;
            ctx.strokeStyle = "limegreen";
            ctx.lineWidth = 2;

            ctx.clearRect(0, 0, canvas.width, canvas.height);

            ctx.beginPath();
            ctx.moveTo(...scalePoint(rootPoint[xkey], rootPoint[key]));
            for (let i = data.length - 2; i >= 0 && data[i][xkey] > currentTime - zoom; i--) {
                ctx.lineTo(...scalePoint(data[i][xkey], data[i][key]));
            }
            ctx.stroke();
        }

        function handleResize() {
            canvas.width = canvas.clientWidth;
            canvas.height = canvas.clientHeight;
        }

        window.addEventListener("resize", handleResize);
        handleResize();
        return () => {
            cancelAnimationFrame(frame);
            window.removeEventListener("resize", handleResize);
        };

    })


</script>

<style>
    .container {
        width: 100%;
        height: 100%;
        box-sizing: border-box;
        padding: .25rem;
        position: relative;
    }
    #root {
        display: flex;
        flex-direction: row;
        border: 1px solid white;
        /*background: red;*/
    }

    canvas {
        background: #222;
        min-width: 0;
    }

    pre {
        margin: 0 0;
        padding: .25rem;
    }

    input {
        width: auto;
        font-family: monospace;
    }

    .sep {
        border: .5px solid white;
    }
</style>

<div class="container">
    <div id="root">
        <canvas bind:this={canvas} style="width: 100%; height: 100%;"></canvas>
        <div style="display: flex; flex-direction: column; background: darkslategrey">
            <pre style="flex: 1">{ymax}</pre>
            <pre>{ymin}</pre>
        </div>
        <div class="sep"></div>
        <div>
            <div>
                <pre>Max</pre>
                <input type="number" bind:value={ymax}/>

                <pre>Min</pre>
                <input type="number" bind:value={ymin}/>

            </div>
        </div>
    </div>

    <pre style="position: absolute; top: .5rem; left: .5rem; z-index: 100; background: black">{key}</pre>

</div>
