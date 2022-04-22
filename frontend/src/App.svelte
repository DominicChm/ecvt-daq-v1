<script lang="ts">
    import {
        Collapse,
        Navbar,
        NavbarToggler,
        NavbarBrand,
        Nav,
        NavItem,
        NavLink,
        Dropdown,
        DropdownToggle,
        DropdownMenu,
        Spinner,
        DropdownItem
    } from 'sveltestrap';
    import {connected, header} from "./stores";
    import Realtime from "./Realtime.svelte";
    import Router from 'svelte-spa-router'
    import Runs from "./Runs.svelte";

    const routes = {
        '/': Realtime,
        '/runs': Runs,
    }

    let isOpen = false;

    function handleUpdate(event) {
        isOpen = event.detail.isOpen;
    }
</script>

<Navbar color="light" light expand="md">
    <NavbarBrand href="#/">ECVT DAQ</NavbarBrand>
    {#if (!$connected)}
        <div style="position: absolute; left: auto; right: auto; width: 100%; text-align: center; color: #0b5ed7">
            <strong>Connecting...</strong>
            <Spinner/>
        </div>
    {/if}

    <NavbarToggler on:click={() => (isOpen = !isOpen)}/>
    <Collapse {isOpen} navbar expand="md" on:update={handleUpdate}>
        <Nav class="ms-auto" navbar>
            <NavItem>
                <NavLink href="#/">Home</NavLink>
            </NavItem>
            <NavItem>
                <NavLink href="#/runs">Runs</NavLink>
            </NavItem>

            <!--            <Dropdown nav inNavbar>-->
            <!--                <DropdownToggle nav caret>Options</DropdownToggle>-->
            <!--                <DropdownMenu end>-->
            <!--                    <DropdownItem>Option 1</DropdownItem>-->
            <!--                    <DropdownItem>Option 2</DropdownItem>-->
            <!--                    <DropdownItem divider/>-->
            <!--                    <DropdownItem>Reset</DropdownItem>-->
            <!--                </DropdownMenu>-->
            <!--            </Dropdown>-->
        </Nav>
    </Collapse>
</Navbar>

<main>
    <Router {routes}/>
</main>

