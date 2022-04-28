<script>
    import {frame, header, runs} from "./stores";
    import {Button, Column, Table} from "sveltestrap";


</script>
<style>
    .column-float {

    }
</style>
<div>
    {#if ($runs)}
        <Table rows={$runs} let:row striped>
            <Column header="Run File" width="1fr">
                {row}
            </Column>
            <Column header="" width="0" >
                <Button download={"pee"}
                        href={`http://localhost:3000/runs/${row}`}>Download
                </Button>
            </Column>
        </Table>
    {/if}

</div>