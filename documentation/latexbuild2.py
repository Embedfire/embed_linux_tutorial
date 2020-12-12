# -*- coding: utf-8 -*-

import datetime, os, sys

name = "i.MX Linux开发实战指南"
#输出PDF文档 名称格式 type = 1：手册  type = 0：教程
type = 0

time = datetime.datetime.now().year * 10000 + datetime.datetime.now().month * 100 + datetime.datetime.now().day



if type == 1:
    pdfname = '[野火] ' + name + str(time) + '.pdf'
else:
    pdfname = '[野火EmbedFire]《' + name + '》—' + str(time) + '.pdf'

print(pdfname)

os.rename('_build/latex/output.pdf', '_build/latex/' + pdfname)