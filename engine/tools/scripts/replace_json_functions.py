# To run this, must ensure that correct python environmental variables are set
# PYTHONPATH must include  C:\Users\dante\Anaconda3\Lib and C:\Users\dante\Anaconda3\Lib\site-packages

import sys, os, re

if __name__ == '__main__':
    
    if len(sys.argv) > 1:
        source_dir = sys.argv[1]
    else:
        source_dir = r"C:\Users\dante\Documents\Projects\grand-blue-engine\apps\reverie"

    total_num_matches = 0
    total_asjson_matches = 0;
    
    replacement_text_template = r"""    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(json& orJson, const CLASS_TYPE& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const json& korJson, CLASS_TYPE& orObject);
"""

    for root, dir_names, filenames in os.walk(source_dir):
#        print(root, dir_names, filenames)

        # Get class names
        for filename in filenames:
            class_names = list()

            file_path = os.path.normpath(os.path.join(root, filename))
            
            if filename.startswith("G") and filename.endswith(".h"):
                match_count = 0
                
#                print(f"Reading {filename}")
                # Read file contents
                text_file = open(file_path, 'r')
                file_text = text_file.read()
                text_file.close()
                
                # Perform a regex to get all class names
                cn_regex = re.compile("(?<=\sclass )(((?!\sclass )(.|\n))*)(?=asJson)")
                matches = cn_regex.findall(file_text)
                for m in matches:
                    class_name = re.search("\w+", m[0]).group()
                    class_names.append(class_name)
#                    print(f"Class {class_name} lives in {filename}")

                # Use multi-line regex to find text to replace
                find_json_regex = re.compile("[ ]*(\/{3}[\@\w ]+\s+(?:virtual)*[ ]+QJsonValue[ ]+asJson[\w \&\:\=\(\)]+;\s+\/{3}[\@\w ]+\s+(?:virtual)*[ ]+void[ ]+loadFromJson[\,\w \&\:\=\(\)]+;)")
                more_matches = find_json_regex.findall(file_text)
                
                find_asjson_regex = re.compile("(asJson[\w \&\:\=\(\)]+;)")
                asjson_matches = find_asjson_regex.findall(file_text)
                
                total_num_matches += len(more_matches)
                total_asjson_matches += len(asjson_matches)
#                print(f"Number of json block matches is {total_num_matches}")
#                print(f"Number of asJson matches is {total_asjson_matches}")
                
                # Replace using output = pattern.sub('replacement', fileContent)                
                for m in more_matches:
#                    print(m)
                    print(class_names[match_count])
                    replacement_text = replacement_text_template.replace("CLASS_TYPE", class_names[match_count])
                    print(replacement_text)
                    file_text = re.sub(find_json_regex, replacement_text, file_text, 1)
#                    file_text = file_text.replace(m, replacement_text, 1)
                    match_count += 1
                    
                print("----")
                # Write to file
                if len(more_matches) > 0:
#                    print(file_text)
                    with open(file_path, "w") as myfile:
                        myfile.write(file_text)

                
                
                