<script>
    import {frame, header, runs} from "./stores";
</script>

<div>
    {$header}
    {#if ($frame)}
        <pre>{JSON.stringify($frame, null, 2)}</pre>
        <pre>{$runs}</pre>
    {/if}
</div>