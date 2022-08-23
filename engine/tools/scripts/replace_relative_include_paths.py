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
#        print(root, dir_names, filenames)
        # Get class names
        for filename in filenames:
            file_path = os.path.normpath(os.path.join(root, filename))
            dir_path = os.path.dirname(file_path)

            # Skip if not in correct folder
            if "third_party" in dir_path:
                continue

            relative_include_paths = list()
            absolute_include_paths = list()
            
            if filename.startswith("G") and (filename.endswith(".h") or filename.endswith(".cpp")):
                match_count = 0
                
                # Read file contents
                text_file = open(file_path, 'r')
                file_text = text_file.read()
                text_file.close()
                
                # Perform a regex to get all relative includes
                rel_regex = re.compile("((?:\.\.\/)+[\w\/]+.h)")
                matches = rel_regex.findall(file_text)
                
                for m in matches:
                    relative_include_paths.append(m)
                    absolute_path = os.path.normpath(os.path.join(dir_path, m))
                    absolute_include_paths.append(absolute_path.replace("\\","/").replace(source_path, ""))
                    # print(f"Relative path '{m}' lives in {filename}, which will converted to '{absolute_include_paths[-1]}")

                # Replace using output = pattern.sub('replacement', fileContent)                
                for path_to_replace in relative_include_paths:
                    file_text = re.sub(rel_regex, f'{absolute_include_paths[match_count]}', file_text, 1)
                    match_count += 1
                
                print(f"Match count: {match_count}, Text:\n {file_text[:300]}\n\n\n")

                # print("----")
                # Write to file
                if len(relative_include_paths) > 0:
#                    print(file_text)
                    with open(file_path, "w") as myfile:
                        myfile.write(file_text)

                
                
                