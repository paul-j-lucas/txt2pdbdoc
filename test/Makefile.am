##
#	txt2pdbdoc -- Text to Doc converter for Palm Pilots
#	test/Makefile.am
#
#	Copyright (C) 2015  Paul J. Lucas
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the Licence, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

AUTOMAKE_OPTIONS = 1.12			# needed for TEST_LOG_DRIVER

TESTS =	tests/txt2pdbdoc-b-d.test \
	tests/txt2pdbdoc-c-d.test \
	tests/txt2pdbdoc-c-t.test \
	tests/txt2pdbdoc-d-t.test \
	tests/txt2pdbdoc-D.test \
	tests/txt2pdbdoc-t.test

AM_TESTS_ENVIRONMENT = BUILD_SRC=$(top_builddir)/src; export BUILD_SRC ;
TEST_EXTENSIONS = .sh .test
SH_LOG_DRIVER = $(srcdir)/run_test.sh
TEST_LOG_DRIVER = $(srcdir)/run_test.sh

EXTRA_DIST = run_test.sh tests data expected
dist-hook:
	cd $(distdir)/tests && rm -f *.log *.trs

# vim:set noet sw=8 ts=8:
