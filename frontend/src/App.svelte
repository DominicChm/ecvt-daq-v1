<script lang="ts">
    import {
        Button,
        Collapse,
        Nav,
        Navbar,
        NavbarBrand,
        NavbarToggler,
        NavItem,
        NavLink,
        Spinner
    } from 'sveltestrap';
    import {connected, status} from "./stores";
    import Dash from "./Dash.svelte";
    import Router, {replace} from 'svelte-spa-router'
    import Runs from "./Runs.svelte";
    import RawData from "./RawData.svelte";
    import Stats from "./Stats.svelte";
    import {onMount} from "svelte";

    const NAVBAR_COLOR = "light";
    onMount(() => {
        replace("/runs")
    })
    const routes = {
        // '/': Dash,
        '/runs': Runs,
        '/raw': RawData,
        // '/stats': Stats,
    }

    let isOpen = false;

    function handleUpdate(event) {
        isOpen = event.detail.isOpen;
    }
</script>

<Navbar color={NAVBAR_COLOR} light expand="md">
    <NavbarBrand color={NAVBAR_COLOR} href="#/">ECVT DAQ</NavbarBrand>
    {#if (!$connected)}
        <div style="position: absolute; left: auto; right: auto; width: 100%; text-align: center; color: #0b5ed7">
            <strong>Connecting...</strong>
            <Spinner/>
        </div>
    {/if}

    <NavbarToggler on:click={() => (isOpen = !isOpen)}/>
    <Collapse {isOpen} navbar expand="md" on:update={handleUpdate}>
        <Nav class="ms-auto" navbar>
            <!--            <NavItem>-->
            <!--                <NavLink href="#/">Home</NavLink>-->
            <!--            </NavItem>-->
            <NavItem>
                <NavLink href="#/runs">Runs</NavLink>
            </NavItem>
            <NavItem>
                <NavLink href="#/raw">Raw Data</NavLink>
            </NavItem>
            <NavItem>
                {#if ($status.logging)}
                    <Button color="primary" href="/api/stop"
                            target="my_iframe">Stop Log
                    </Button>
                {:else }
                    <Button color="primary" href="/api/start"
                            target="my_iframe">Start
                        Log
                    </Button>

                {/if}
            </NavItem>

            <iframe id="iframe" name="my_iframe" style="display: none"></iframe>

            <!--            <NavItem>-->
            <!--                <NavLink href="#/stats">Stats</NavLink>-->
            <!--            </NavItem>-->

        </Nav>
    </Collapse>
</Navbar>

<main>
    <Router {routes}/>
</main>

