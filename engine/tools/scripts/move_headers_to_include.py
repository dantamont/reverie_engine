# To run this, must ensure that correct python environmental variables are set
# PYTHONPATH must include  C:\Users\dante\Anaconda3\Lib and C:\Users\dante\Anaconda3\Lib\site-packages

import sys, os, re
from pathlib import Path

if __name__ == '__main__':
    
    if len(sys.argv) > 1:
        source_dir = sys.argv[1]
    else:
        source_dir = r"C:\Users\dante\Documents\Projects\grand-blue-engine\apps\reverie\src"

    total_num_matches = 0
    total_asjson_matches = 0;
    
    replacement_text_template = ""

    source_path = "C:/Users/dante/Documents/Projects/grand-blue-engine/apps/reverie/src/"

    for root, dir_names, filenames in os.walk(source_dir):
        # Get class names
        for filename in filenames:
            file_path = os.path.normpath(os.path.join(root, filename))
            dir_path = os.path.dirname(file_path)

            # Skip if not in correct folder
            if "third_party" in dir_path:
                continue

            if filename.startswith("G") and (filename.endswith(".h")):
                
                # Make directory if it does not exist
                new_dir = dir_path.replace("src", "include")
                os.makedirs(new_dir, exist_ok=True)
                
                # Move file
                new_path = file_path.replace("src", "include")
                os.rename(file_path, new_path)
                
                