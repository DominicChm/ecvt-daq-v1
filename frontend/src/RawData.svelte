<script>
    import {frame, header} from "./stores";
    import {Column, Table} from "sveltestrap";

    $: table = Object.entries($frame)?.map(([key, value]) => ({key, value}));

</script>

<div>
    {#if (table)}
        <Table rows={table} let:row striped>
            <Column header="Key" width="8rem">
                {row.key}
            </Column>
            <Column header="Value" width="8rem">
                {row.value}
            </Column>
        </Table>
    {/if}

</div>