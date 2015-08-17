Test File Format
================

Test files (ending in `.test`) must be a single line in the following format:

*command* `|` *options* `|` *doc_name* `|` *input* `|` *exit*

that is five fields separated by the pipe (`|`) character
(with optional whitespace)
where:

1. *command*  = command to execute (`txt2pdbdoc`)
2. *options*  = command-line options or blank for none
3. *doc_name* = name of file to dump
4. *input*    = name of file to dump
5. *exit*     = expected exit status code

Test scripts (ending in `.sh`) can alternatively be used
when additional commands need to be executed as part of the test.
Scripts are called with two command-line arguments:

1. The full path of the test output file.
2. The full path of the log file.

Test scripts must follow the normal Unix convention
of exiting with zero on success and non-zero on failure.
