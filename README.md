# EPUB Utilities

A collection of command-line utilities for creating EPUB documents from images or XHTML pages. These were originally Automator actions but have been rewritten as POSIX command-line programs for portability.

## Common Options

<dl>

<dt><tt>--identifier</tt><dt><dd>The URN identifying this document.</dd>

<dt><tt>--output</tt><dt><dd>Specify the destination file.</dd>

<dt><tt>--force</tt><dt><dd>Overwrite the output file unconditionally.</dd>

<dt><tt>--title</tt><dt><dd>Specify the title of the generated EPUB document.</dd>

<dt><tt>--creator</tt><dt><dd>The creator of the document's content.</dd>

<dt><tt>--file-as</tt><dt><dd>How to collate the previous creator's name.</dd>

<dt><tt>--role</tt><dt><dd>The role of the previous creator as a MARC relator.</dd>

<dt><tt>--collection</tt><dt><dd>The name of the collection in which this work appears</dd>

<dt><tt>-issue</tt><dt><dd>The position of this work inside the collection.</dd>

<dt><tt>--toc-stylesheet</tt><dt><dd>The CSS stylesheet file to apply to the generated table of contents.</dd>

<dt><tt>--description</tt><dt><dd>The description of the EPUB document, or the file containing the description when prefixed with an `@` symbol.</dd>

<dt><tt>--cover-mage</tt><dt><dd>The image to use for the cover of the document.</dd>

</dl>

## Binder

Combines a set of XHTML documents into a single EPUB document with each XHTML file acting as a chapter. EPUB metadata is supplied through command line options and XHTML `<meta>` elements.

Chapter titles are taken from the XHTML `<title>` elements.

### Special options:

<dl>
<dt><tt>--basedir</tt></dt><dd>Specifies the directory in which the source files are found. Default: the current directory</dd>
<dt><tt>--omit-toc</tt></dt><dd>Omit the table of contents from the spine.  The table of contents will still be generated but it won't be part of the reading flow.</dd>
</dl>

### Special arguments:

If an argument begins with `@` it is the name of the file containing the list of files to be used.  A lone `@` uses standard in as the file list source.

File names are relative to the base directory.  The destination file name will be the same as the source file name unless the name contains a colon, in which case everything before the colon is the source file name and everything after is the destination name.  Note that this does **not** update links within the content.

## Comic

Combines a set of images into a comic book. The images are assumed to be individual daily strips or full pages of a comic.

### Special options:

<dl>
<dt><tt>--page-width</tt></dt><dd>The width of the page in pixels. Default: 1536</dd>
<dt><tt>--page-height</tt></dt><dd>The height of the page in pixels. Default: 2048</dd>
<dt><tt>--page-size</tt></dt><dd>Alternate way of specifying the page width and height as <i>W</i><tt>x</tt><i>H</i>.</dd>
<dt><tt>--pack-frames</tt></dt><dd>Remove the space between multiple images on a single page.</dd>
<dt><tt>--spread-frames</tt></dt><dd>Put the most space between multiple images on a single page.</dd>
<dt><tt>--link</tt></dt><dd>Link rather than copy the images into the EPUB folder.</dd>
<dt><tt>--upscale</tt></dt><dd>Allow narrow image scaling to be greater than 100%.</dd>
</dl>

### Special arguments:

If an argument begins with `@` it is the name of the file containing the list of files to be used.  A lone `@` uses standard in as the file list source.

