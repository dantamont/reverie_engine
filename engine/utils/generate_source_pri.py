# -*- coding: utf-8 -*-
"""
Generates the .pri file for loading in all source code present in project 
repo

Created on Thu Aug  1 19:28:10 2019

@author: dante

See: https://www.toptal.com/qt/vital-guide-qmake
"""

import os
import sys

def create_file_list_str(name: str, file_paths: str):
    file_paths.sort()
    file_list_str = ""
    file_list_str += "{} += ".format(name)
    
    for i, fp in enumerate(file_paths):
        file_list_str += fp
        if i != len(file_paths) - 1:
            file_list_str += " \\"
        file_list_str += " \n \t"
    
    file_list_str += "\n"
    return file_list_str

project_dir = sys.argv[1]
#project_dir = r"C:\Users\dante\Documents\Projects\grand-blue-engine\grand_blue"

extensions = ["h", "cpp", "ui", "qrc"]
file_header = "# ----------------------------------------------------\n" + \
         "# This file was generated by generate_source_pri.py\n" + \
         "# ----------------------------------------------------\n"
    
headers = []
sources = []
forms = []
resources = []

print("Generating source for project directory: " + project_dir)
for dir_name, subdir_list, file_list in os.walk(project_dir):
    common_prefix = os.path.commonprefix([dir_name, project_dir])
    rel_dir_path = os.path.relpath(dir_name, common_prefix).replace("\\", "/")

    if "qt_generated" in dir_name:
        continue

    print("Found directory: {}".format(rel_dir_path))
    for file_name in file_list:
        extension = file_name.split(".")[-1]
        full_name = rel_dir_path + "/" + file_name
        
        if extension not in extensions:
            continue
        elif "eigen/" in rel_dir_path and "eigen/Eigen" not in rel_dir_path:
            # Avoid extra eigen directories
            continue
        print("\t{}".format(file_name))

        if extension == "h":
            headers.append(full_name)
        elif extension == "cpp":
            sources.append(full_name)
        elif extension == "qrc":
            resources.append(full_name)
        elif extension == "ui":
            forms.append(full_name)
            
out_pri_file = os.path.join(project_dir, "grand_blue.pri").replace("\\", "/")
header_str = create_file_list_str("HEADERS", headers)
sources_str = create_file_list_str("SOURCES", sources)
forms_str = create_file_list_str("FORMS", forms)
resources_str = create_file_list_str("RESOURCES", resources)
# include_path_str = 'INCLUDEPATH += "../grand_blue"'

with open(out_pri_file, "w") as f:
    f.write(file_header)
    f.write("\n")
    
    f.write(header_str)
    f.write(sources_str)
    f.write(forms_str)
    f.write(resources_str)
	
    # f.write(include_path_str)
    
print("Done generating " + out_pri_file)