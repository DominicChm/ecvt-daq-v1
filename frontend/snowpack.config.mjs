/** @type {import("snowpack").SnowpackUserConfig } */
export default {
    mount: {
        public: {url: '/', static: true},
        src: {url: '/'},
    },
    plugins: [
        '@snowpack/plugin-svelte',
        [
            '@snowpack/plugin-typescript',
            {
                /* Yarn PnP workaround: see https://www.npmjs.com/package/@snowpack/plugin-typescript */
                ...(process.versions.pnp ? {tsc: 'yarn pnpify tsc'} : {}),
            },
        ],
    ],
    routes: [
        /* Enable an SPA Fallback in development: */
        //{"match": "routes", "src": ".*", "dest": "/index.html"},
    ],
    optimize: {
        bundle: true,
        minify: true,
        target: 'es2018',
        sourceMap: false,

    },
    packageOptions: {
        polyfillNode: true,
        sourceMap: false,

    },
    devOptions: {
    },
    buildOptions: {
        out: "../data",
        sourceMap: false,
    },
};