import sys, os

if __name__ == '__main__':

    source_dir = sys.argv[1]

    for root, dir_names, filenames in os.walk(source_dir):
        # print(root, dir_names, filenames)

        for filename in filenames:
            file_path = os.path.normpath(os.path.join(root, filename))
            
            if "Gb" in file_path:
                new_path = file_path.replace("Gb", "G")
                os.rename(file_path, new_path) 
                print(file_path + " renamed to " + new_path)
    # for count, filename in enumerate(os.listdir(source_dir)): 
    #     print(filename)
        # dst ="Hostel" + str(count) + ".jpg"
        # src ='xyz'+ filename 
        # dst ='xyz'+ dst 
            
        # rename all the files 
        # os.rename(src, dst) 