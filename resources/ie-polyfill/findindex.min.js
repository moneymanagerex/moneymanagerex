/**
 * Minified by jsDelivr using Terser v3.14.1.
 * Original file: /npm/findindex_polyfill_mdn@1.0.0/findIndex.js
 *
 * Do NOT use SRI with dynamically generated files! More information: https://www.jsdelivr.com/using-sri-with-dynamic-files
 */
Array.prototype.findIndex||Object.defineProperty(Array.prototype,"findIndex",{value:function(r){if(null==this)throw new TypeError('"this" is null or not defined');var e=Object(this),t=e.length>>>0;if("function"!=typeof r)throw new TypeError("predicate must be a function");for(var n=arguments[1],i=0;i<t;){var o=e[i];if(r.call(n,o,i,e))return i;i++}return-1},configurable:!0,writable:!0});
//# sourceMappingURL=/sm/62b4860885465cb39fdbbcf8106b7ea29b0e5502fd16963e1e824fee1ecb4a13.map