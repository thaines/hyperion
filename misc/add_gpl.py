#! /usr/bin/env python

import os


# The message...
msg = "\n// This program is free software; you can redistribute it and/or\n// modify it under the terms of the GNU General Public License\n// as published by the Free Software Foundation; either version 2\n// of the License, or (at your option) any later version.\n\n// This program is distributed in the hope that it will be useful,\n// but WITHOUT ANY WARRANTY; without even the implied warranty of\n// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n// GNU General Public License for more details.\n\n// You should have received a copy of the GNU General Public License\n// along with this program; if not, write to the Free Software\n// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n\n"

# If found at start of line the message is inserted afterwards...
start = "// Copyright"


# Get list of all files tht need editting...
files = []
for dirpath,dirnames,filenames in os.walk('..'):
  if dirpath.startswith('../eos') or dirpath.startswith('../ares'):
    for fn in filenames:
      if fn.endswith('.cpp') or fn.endswith('.h'):
        files.append(os.path.join(dirpath,fn))

print 'Found', len(files), 'files'

for fn in files:
  # Copy file into memory...
  f = open(fn,'r')
  lines = f.readlines()
  f.close()

  # Rewrite file with change...
  insertCount = 0
  f = open(fn,'w')
  for line in lines:
    f.write(line)
    if line.startswith(start):
      f.write(msg)
      insertCount += 1
  f.close()

  print fn, insertCount