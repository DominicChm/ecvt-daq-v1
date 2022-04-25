<script>
    import {frame, header} from "./stores";
</script>

<div>
    {$header}
    {#if ($frame)}
        <pre>{JSON.stringify($frame, null, 2)}</pre>
    {/if}
</div>