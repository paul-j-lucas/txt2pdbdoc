# txt2pdbdoc

**txt2pdbdoc** is a Unix-based Text-to-Doc file conversion program.
(It also converts Doc files to plain text.)
A Doc file is a defacto standard file format
for text documents for PalmOS document reader applications.
My motivation for writing **txt2pdbdoc** was that such software at the time
was crufty and poorly documented.
In contrast,
**txt2pdbdoc** is well written and well documented.
**txt2pdbdoc** is based on the work of
Rick Bram, Florent Pillet, and Harold Bamford.

## Tip

If you'd like to be able to grab web pages
and have them converted to Doc automatically,
you can write a simple script that calls **lynx**(1)
(the text-based web browser)
with the `-source` option to dump the HTML into a file,
then run **html2pdbtxt**
followed by **txt2pdbdoc**.

## Installation

The git repository contains only the necessary source code.
Things like `configure` are _derived_ sources and
[should not be included in repositories](http://stackoverflow.com/a/18732931).
If you have `autoconf`, `automake`, and `m4` installed,
you can generate `configure` yourself by doing:

    autoreconf -fiv

Then follow the generic installation instructions given in `INSTALL`.
