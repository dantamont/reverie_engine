# -*- coding: utf-8 -*-
"""
Created on Thu Mar 19 20:03:23 2020

@author: dante
"""
import json
import os

cwd = os.getcwd()
path = r"../../grand_blue/resources/fonts/fontawesome/metadata/icons.json"
with open(path) as jf:
    j_dict = json.load(jf)

out_dict = dict()
for k, v in j_dict.items():
    out_dict[k] = {
            "unicode":  bytes(f'\\u{v["unicode"]}', "ascii").decode('unicode-escape'), 
            "styles": v["styles"],
            "label": v["label"]}

out_path = r"../../grand_blue/resources/fonts/fontawesome/unicode_info.json"
with open(out_path, "w") as of:
    json.dump(out_dict, of)