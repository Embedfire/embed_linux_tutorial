# -*- coding: utf-8 -*-


#***************************************************************#
#                       需要替换的内容
#***************************************************************#
new_str = []
old_str = []

old_str.append("\\documentclass[letterpaper,10pt,english]{sphinxmanual}\n")
new_str.append("\\documentclass[12pt,english,oneside,AutoFakeBold]{sphinxmanual}\n")
#生成的tex文档中需要替换的行，使用list.append方法将需要替换的行内容
#成对添加到new_str = []和old_str = []

#*******************************end*****************************#

#***************************************************************#
#                       不生成序号的章节
#***************************************************************#
chapter_str = ['关于本项目','关于野火','修订历史','如何参与项目','常见问题','版权说明']

#*******************************end*****************************#

chapter_num = 0
f = open("_build/latex/output.tex_bak", "r", encoding="utf-8") 
line = f.readline()
f_new = open("_build/latex/output.tex", "w", encoding="utf-8") 

while line:
    if line in old_str:
        line_num = old_str.index(line)
        line = new_str[line_num]
        print("change line: ",line)
    else :
        if line[1:8] == "chapter": 
            if  line[line.find('{')+1:line.find('}')] in chapter_str:
                print("origin line: ",line)
                line = line[:line.find('{')] + '*' + line[line.find('{'):] + '\\addcontentsline{toc}{chapter}' + line[line.find('{'):line.find('}')+1] + '\n'
                print("change line: ",line,'\n')
                section = 1
            else:
                section = 0
                chapter_num = chapter_num + 1
                print("origin line: ",line)
                line =  line[:line.find('{')+1] + '第' + str(chapter_num) + '章 \quad ' + line[line.find('{')+1:]
                print("change line: ",line,'\n')            

        if line[1:8] == "section": 
            if  section == 1:
                print("origin line: ",line)
                line = line[:line.find('{')] + '*' + line[line.find('{'):] + '\\addcontentsline{toc}{section}' + line[line.find('{'):line.find('}')+1] + '\n'
                print("change line: ",line,'\n')                
        

    f_new.write(line)
    line = f.readline()

f.close()
f_new.close()

