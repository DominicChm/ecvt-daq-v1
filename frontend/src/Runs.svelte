<script lang="ts">
    import {frame, header, runs} from "./stores";
    import {Button, Column, Container, Table} from "sveltestrap";
    import RunEditModal from "./RunEditModal.svelte";

    let editOpen = false;

    let editingRun;
    function editRun(run) {
        editingRun = run;
        editOpen = true;
    }
</script>
<style>
    .column-float {

    }
</style>
<div>
    {#if ($runs)}
        <Table rows={$runs} let:row striped>
            <Column header="Name" width="1fr">
                {row.name}
            </Column>
            <Column header="Desc" width="auto">
                {row.description}
            </Column>
            <Column header="Filename" width="1fr">
                {row.filename}
            </Column>
            <Column header="" width="0">
                <div style="display: flex; gap: .5rem">
                    <Button download={row.name} href={`/r/${row.filename}`}>Download</Button>
                    <Button on:click={() => editRun(row)}>Edit</Button>
                </div>
            </Column>
        </Table>
    {/if}

</div>

<RunEditModal bind:isOpen={editOpen} run={editingRun}/>