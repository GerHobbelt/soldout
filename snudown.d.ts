interface Options {
    /**
     * Whether to add `rel="nofollow"` to all links.
     */
    nofollow?: boolean;
    /**
     * The `target` property of all links.
     */
    target?: string;
    /**
     * Whether to create a table of contents.
     *
     * Note that Reddit postprocesses the output instead of using this option to generate a TOC.
     */
    enableToc?: boolean;
    /**
     * Added to the `id` of each TOC link, i.e. `#PREFIXtoc_0`.
     */
    tocIdPrefix?: string;
}

/**
 * Render markdown `text` to an HTML string using the usertext renderer.
 */
export function markdown(text: string, options?: Options): string;

/**
 * Render markdown `text` to an HTML string using the wiki renderer.
 */
export function markdownWiki(text: string, options?: Options): string;
