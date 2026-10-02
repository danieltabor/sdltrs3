#!/usr/bin/env python3
import shlex
import os
import sys

emargs = os.getenv("EMARGS", default=None)
if emargs == None or len(emargs)==0:
	sys.exit(0)

seed = "canvas: canvasElement,"

target = sys.argv[1]
fin = open(target,"r")
html = fin.read()
fin.close()

insert = html.find(seed)
if insert < 0:
	print("Bad HTML")
	sys.exit(-1)

tab_size = 0
while html[insert-(tab_size+1)] != "\n":
	tab_size += 1
insert = insert-tab_size


s  = " "*tab_size
s += "arguments: ["
args = shlex.split(emargs)
for i in range(len(args)):
	if i:
		s+= ","
	s+="\"%s\"" % args[i]
s += "],\n"

html = html[:insert] + s + html[insert:]

fout = open(target,"w")
fout.write(html)
fout.close()
