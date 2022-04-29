import './RTGraph.svelte.css';
/* src\RTGraph.svelte generated by Svelte v3.47.0 */
import {
	SvelteComponent,
	append,
	attr,
	binding_callbacks,
	detach,
	element,
	init,
	insert,
	listen,
	noop,
	run_all,
	safe_not_equal,
	set_data,
	set_input_value,
	set_style,
	space,
	text,
	to_number
} from "./_snowpack/pkg/svelte/internal.js";

import { onMount } from "./_snowpack/pkg/svelte.js";

function create_fragment(ctx) {
	let div5;
	let div4;
	let canvas_1;
	let t0;
	let div0;
	let pre0;
	let t1;
	let t2;
	let pre1;
	let t3;
	let t4;
	let div1;
	let t5;
	let div3;
	let div2;
	let pre2;
	let t7;
	let input0;
	let t8;
	let pre3;
	let t10;
	let input1;
	let t11;
	let pre4;
	let t12;
	let mounted;
	let dispose;

	return {
		c() {
			div5 = element("div");
			div4 = element("div");
			canvas_1 = element("canvas");
			t0 = space();
			div0 = element("div");
			pre0 = element("pre");
			t1 = text(/*ymax*/ ctx[1]);
			t2 = space();
			pre1 = element("pre");
			t3 = text(/*ymin*/ ctx[0]);
			t4 = space();
			div1 = element("div");
			t5 = space();
			div3 = element("div");
			div2 = element("div");
			pre2 = element("pre");
			pre2.textContent = "Max";
			t7 = space();
			input0 = element("input");
			t8 = space();
			pre3 = element("pre");
			pre3.textContent = "Min";
			t10 = space();
			input1 = element("input");
			t11 = space();
			pre4 = element("pre");
			t12 = text(/*key*/ ctx[2]);
			set_style(canvas_1, "width", "100%");
			set_style(canvas_1, "height", "100%");
			attr(canvas_1, "class", "svelte-j7l99t");
			set_style(pre0, "flex", "1");
			attr(pre0, "class", "svelte-j7l99t");
			attr(pre1, "class", "svelte-j7l99t");
			set_style(div0, "display", "flex");
			set_style(div0, "flex-direction", "column");
			set_style(div0, "background", "darkslategrey");
			attr(div1, "class", "sep svelte-j7l99t");
			attr(pre2, "class", "svelte-j7l99t");
			attr(input0, "type", "number");
			attr(input0, "class", "svelte-j7l99t");
			attr(pre3, "class", "svelte-j7l99t");
			attr(input1, "type", "number");
			attr(input1, "class", "svelte-j7l99t");
			attr(div4, "id", "root");
			attr(div4, "class", "svelte-j7l99t");
			set_style(pre4, "position", "absolute");
			set_style(pre4, "top", ".5rem");
			set_style(pre4, "left", ".5rem");
			set_style(pre4, "z-index", "100");
			set_style(pre4, "background", "black");
			attr(pre4, "class", "svelte-j7l99t");
			attr(div5, "class", "container svelte-j7l99t");
		},
		m(target, anchor) {
			insert(target, div5, anchor);
			append(div5, div4);
			append(div4, canvas_1);
			/*canvas_1_binding*/ ctx[8](canvas_1);
			append(div4, t0);
			append(div4, div0);
			append(div0, pre0);
			append(pre0, t1);
			append(div0, t2);
			append(div0, pre1);
			append(pre1, t3);
			append(div4, t4);
			append(div4, div1);
			append(div4, t5);
			append(div4, div3);
			append(div3, div2);
			append(div2, pre2);
			append(div2, t7);
			append(div2, input0);
			set_input_value(input0, /*ymax*/ ctx[1]);
			append(div2, t8);
			append(div2, pre3);
			append(div2, t10);
			append(div2, input1);
			set_input_value(input1, /*ymin*/ ctx[0]);
			append(div5, t11);
			append(div5, pre4);
			append(pre4, t12);

			if (!mounted) {
				dispose = [
					listen(input0, "input", /*input0_input_handler*/ ctx[9]),
					listen(input1, "input", /*input1_input_handler*/ ctx[10])
				];

				mounted = true;
			}
		},
		p(ctx, [dirty]) {
			if (dirty & /*ymax*/ 2) set_data(t1, /*ymax*/ ctx[1]);
			if (dirty & /*ymin*/ 1) set_data(t3, /*ymin*/ ctx[0]);

			if (dirty & /*ymax*/ 2 && to_number(input0.value) !== /*ymax*/ ctx[1]) {
				set_input_value(input0, /*ymax*/ ctx[1]);
			}

			if (dirty & /*ymin*/ 1 && to_number(input1.value) !== /*ymin*/ ctx[0]) {
				set_input_value(input1, /*ymin*/ ctx[0]);
			}

			if (dirty & /*key*/ 4) set_data(t12, /*key*/ ctx[2]);
		},
		i: noop,
		o: noop,
		d(detaching) {
			if (detaching) detach(div5);
			/*canvas_1_binding*/ ctx[8](null);
			mounted = false;
			run_all(dispose);
		}
	};
}

function instance($$self, $$props, $$invalidate) {
	let { ymin = 0 } = $$props;
	let { ymax = 100 } = $$props;
	let { zoom = 100 } = $$props;
	let { currentTime = 0 } = $$props;
	let { xkey = "time" } = $$props;
	let { key } = $$props;
	let { data } = $$props;
	if (!data || !key) throw new Error("NO DATA OR KEY PASSED!");
	let canvas;

	onMount(() => {
		const ctx = canvas.getContext('2d');
		let frame = requestAnimationFrame(loop);
		console.log("SDAVBFHASG");

		function scalePoint(x, y) {
			let xmin = currentTime - zoom;
			let xmax = currentTime;

			return [
				canvas.width * (x - xmin) / (xmax - xmin),
				canvas.height * (1 - (y - ymin) / (ymax - ymin))
			];
		}

		function loop(t) {
			frame = requestAnimationFrame(loop);
			let rootPoint = data[data.length - 1];
			if (!rootPoint || rootPoint[xkey] == null || rootPoint[key] == null) return;
			ctx.strokeStyle = "limegreen";
			ctx.lineWidth = 2;
			ctx.clearRect(0, 0, canvas.width, canvas.height);
			ctx.beginPath();
			ctx.moveTo(...scalePoint(rootPoint[xkey], rootPoint[key]));

			for (let i = data.length - 2; i >= 0 && data[i][xkey] > currentTime - zoom; i--) {
				ctx.lineTo(...scalePoint(data[i][xkey], data[i][key]));
			}

			ctx.stroke();
		}

		function handleResize() {
			$$invalidate(3, canvas.width = canvas.clientWidth, canvas);
			$$invalidate(3, canvas.height = canvas.clientHeight, canvas);
		}

		window.addEventListener("resize", handleResize);
		handleResize();

		return () => {
			cancelAnimationFrame(frame);
			window.removeEventListener("resize", handleResize);
		};
	});

	function canvas_1_binding($$value) {
		binding_callbacks[$$value ? 'unshift' : 'push'](() => {
			canvas = $$value;
			$$invalidate(3, canvas);
		});
	}

	function input0_input_handler() {
		ymax = to_number(this.value);
		$$invalidate(1, ymax);
	}

	function input1_input_handler() {
		ymin = to_number(this.value);
		$$invalidate(0, ymin);
	}

	$$self.$$set = $$props => {
		if ('ymin' in $$props) $$invalidate(0, ymin = $$props.ymin);
		if ('ymax' in $$props) $$invalidate(1, ymax = $$props.ymax);
		if ('zoom' in $$props) $$invalidate(4, zoom = $$props.zoom);
		if ('currentTime' in $$props) $$invalidate(5, currentTime = $$props.currentTime);
		if ('xkey' in $$props) $$invalidate(6, xkey = $$props.xkey);
		if ('key' in $$props) $$invalidate(2, key = $$props.key);
		if ('data' in $$props) $$invalidate(7, data = $$props.data);
	};

	return [
		ymin,
		ymax,
		key,
		canvas,
		zoom,
		currentTime,
		xkey,
		data,
		canvas_1_binding,
		input0_input_handler,
		input1_input_handler
	];
}

class RTGraph extends SvelteComponent {
	constructor(options) {
		super();

		init(this, options, instance, create_fragment, safe_not_equal, {
			ymin: 0,
			ymax: 1,
			zoom: 4,
			currentTime: 5,
			xkey: 6,
			key: 2,
			data: 7
		});
	}
}

export default RTGraph;