# EPUB Utilities

A collection of command-line utilities for creating EPUB documents from images or XHTML pages. These were originally Automator actions but have been rewritten as POSIX command-line programs for portability.

## Binder

Combines a set of XHTML documents into a single EPUB document with each XHTML file acting as a chapter. EPUB metadata is supplied through command line options and XHTML `<meta>` elements.

Chapter titles are taken from the XHTML `<title>` elements.

## Comic

Combines a set of images into a comic book. The images are assumed to be individual daily strips or full pages of a comic.

## Common Options

`--identifier`
: The URN identifying this document.

`--output`
: Specify the destination file.

`--force`
: Overwrite the output file unconditionally.

`--title`
: Specify the title of the generated EPUB document.

`--creator`
: The creator of the document's content.

`--file-as`
: How to collate the previous creator's name.

`--role`
: The role of the previous creator as a MARC relator.

`--collection`
: The name of the collection in which this work appears

`-issue`
: The position of this work inside the collection.

`--toc-stylesheet`
: The CSS stylesheet file to apply to the generated table of contents.

`--description`
: The description of the EPUB document, or the file containing the description when prefixed with an `@` symbol.

`--cover-mage`
: The image to use for the cover of the document.
