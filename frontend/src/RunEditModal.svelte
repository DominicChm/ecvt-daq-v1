<script lang="ts">
    import {runs} from "./stores";
    import {Button, Form, FormGroup, Input, Modal, ModalBody, ModalFooter, ModalHeader} from "sveltestrap";

    export let run;
    export let isOpen;

    const toggle = () => {
        isOpen = !isOpen;
        if (!isOpen) {
            //SUBMIT RUN
        }
    };

    $: header = run?.filename ? `Edit ${run.filename}` : "UNKNOWN RUN";
</script>

<Modal size="xl" {isOpen} body {toggle}>
    <ModalHeader {toggle}>{header}</ModalHeader>
    <ModalBody>
        <Form id="edit-form" action="/api/setmeta" method="get" target="my_iframe">
            <FormGroup floating label="Name">
                <Input placeholder="Enter a value" bind:value={run.name} name="name"/>
            </FormGroup>

            <FormGroup floating label="Description">
                <Input placeholder="Enter a value"
                       type="textarea" bind:value={run.description}
                       name="desc"/>
            </FormGroup>
            <input type="hidden" value={run.filename} name="file"/>
        </Form>
    </ModalBody>

    <iframe id="iframe" name="my_iframe"></iframe>

    <ModalFooter>
        <Button color="secondary" on:click={toggle}>Cancel</Button>
        <Button color="primary" on:click={toggle} type="submit"
                form="edit-form">Apply</Button>
    </ModalFooter>
</Modal>

