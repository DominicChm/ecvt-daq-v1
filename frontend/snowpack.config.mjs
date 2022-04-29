/** @type {import("snowpack").SnowpackUserConfig } */
import proxy from "http2-proxy"
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
        {
            src: '/r/.*',
            dest: (req, res) => {
                return proxy.web(req, res, {
                    hostname: 'localhost',
                    port: 3000,
                });
            },
        },
        {
            src: '/r.txt',
            dest: (req, res) => {
                return proxy.web(req, res, {
                    hostname: 'localhost',
                    port: 3000,
                });
            },
        },
        {
            src: '/api/setmeta',
            dest: (req, res) => {
                return proxy.web(req, res, {
                    hostname: 'localhost',
                    port: 3000,
                });
            },
        },
        {
            src: '/ws',
            upgrade: (req, socket, head) => {
                const defaultWSHandler = (err, req, socket, head) => {
                    if (err) {
                        console.error('proxy error', err);
                        socket.destroy();
                    }
                };

                proxy.ws(
                    req,
                    socket,
                    head,
                    {
                        hostname: 'localhost',
                        port: 3000,
                    },
                    defaultWSHandler,
                );
            },
        },
    ],
    optimize: {
        bundle: true,
        minify: true,
        target: 'esnext',
        sourceMap: false,

    },
    packageOptions: {
        polyfillNode: false,
        //sourceMap: false,

    },
    devOptions: {
    },
    buildOptions: {
        out: "build",
        sourceMap: false,
    },
};