# -*- coding: utf-8 -*-
"""File containing aggregator class"""

import os
from abc import ABCMeta, abstractmethod
import generators.generator
from jinja2 import Environment, FileSystemLoader, select_autoescape
import shutil # To remove directories
import yaml

class Aggregator:
    """Class for aggregating generators"""

    # Name of file where all auto-generated source file names are added
    _list_file_name = "file_list.txt"
    _list_header_file_name = "header_file_list.txt"

    # File path of generator data
    _generator_data_path = f"{os.path.dirname(os.path.abspath(__file__))}/data"

    # Path the file housing the schema version
    _schema_file_path = f"{_generator_data_path}/g_schema_version.yaml"

    # Header for schema file
    _schema_file_header = """# When modifying this file, bump the schema major version when:
#  - Modifying any existing messages
#  - Adding any messages to a place not at the end of the file
# Bump the schema minor version when:
#  - Adding any new messages to the end of the file
"""

    def __init__(self, generated_code_path: str):
        """
        Initialize the aggregator

        Parameters
        ----------
        generated_code_path: str
            The path at which to output all files
        """
        # Will start with an empty list of generators
        self.code_generators = []

        # Output path is needed for code generation
        self.generated_code_path = generated_code_path

        # Output path for generated schema number
        self._autogenerated_schema_path = f"{generated_code_path}/ag_schema_version.yaml"

        # Obtain current schema version
        self.get_schema_version()

        # Initialize Jinja2 stuff
        self.initialize_templates()

    def initialize_templates(self):
        """Initialize stuff related to Jinja2 templates"""
        template_path = f"{os.path.dirname(__file__)}/templates/"
        self.template_env = Environment(
            loader=FileSystemLoader(template_path),
            autoescape=select_autoescape()
        )

    def add_generator(self, generator):
        """Add a given generator to this aggregator"""
        generator.initialize(self.generated_code_path, 
                             self._generator_data_path,
                             self.template_env)
        self.code_generators.append(generator)

    @staticmethod
    def _read_schema_version(schema_file_path: str):
        """Obtain current schema version from a file"""
        schema_version  = [-1, -1, -1]
        with open(schema_file_path, "r") as yaml_stream:
            try:
                docs = yaml.safe_load_all(yaml_stream)
                # Only one doc in file, but load_all yields generator
                for my_yaml in docs:
                    schema_version[0] = my_yaml["schema_version"]["major"]
                    schema_version[1]  = my_yaml["schema_version"]["minor"]
                    schema_version[2]  = my_yaml["schema_version"]["patch"]
            except Exception as e:
                raise e
        return schema_version

    def get_schema_version(self):
        """Obtain current schema version"""
        self.schema_version = self._read_schema_version(self._schema_file_path)

    def _get_schema_version_yaml_string(self):
        """Obtain YAML string representing the schema version"""
        return f"#This file is autogenerated by {os.path.abspath(__file__)}. " +\
            f"\n# Do ~NOT~ modify!\n\n---\nschema_version:\n  " +\
            f"major: {self.schema_version[0]}\n  minor: {self.schema_version[1]}\n  patch: {self.schema_version[2]}\n..."
    
    def stale_schema(self):
        """Return true if code needs to be regenerated
        
        This is the case if either the schema number has changed, or if
        there is no record of the schema number. Note that generators
        may automatically bump the schema version via check_schema_updates
        """
        self.check_schema_updates()

        if os.path.exists(self._autogenerated_schema_path):
            # See if schema manually updated
            autogen_schema = self._read_schema_version(self._autogenerated_schema_path)
            major_changed = autogen_schema[0] < self.schema_version[0]
            minor_changed = autogen_schema[1] < self.schema_version[1]
            patch_changed = autogen_schema[2] < self.schema_version[2]
            schema_changed = major_changed or minor_changed or patch_changed
            
            if schema_changed:
                print(f"Schema changed from v{autogen_schema[0]}.{autogen_schema[1]}.{autogen_schema[2]} to v{self.schema_version[0]}.{self.schema_version[1]}.{self.schema_version[2]}")
            
            return schema_changed
        else:
            # No schema file exists, so auto-generation is stale
            return True

    def check_schema_updates(self):
        """Check if any generators have requested an updated schema"""
        bump_schema_version = [False, False, False]
        for generator in self.code_generators:
            bump_schema_version_generator = generator.update_schema_version()
            bump_schema_version = [this or that for this, that in zip(bump_schema_version, bump_schema_version_generator)]
    
        # Bump schema version if updated
        updated = any(bump_schema_version)
        if updated:
            self.schema_version = [x + int(y) for x, y in zip(self.schema_version, bump_schema_version)]
            schema_dict = {
            "schema_version": 
                { "major": self.schema_version[0], 
                  "minor": self.schema_version[1],
                  "patch": self.schema_version[2]}
            }
            with open(self._schema_file_path, "w") as file:
                file.write(self._schema_file_header)
            with open(self._schema_file_path, "a") as file:
                yaml.dump(schema_dict, file, explicit_start=True, explicit_end=True)
        return updated

    def regenerate(self):
        """Regenerate code, checking if schema version is updated"""
        # TODO: update schema version if has changed, or at least check for this
        if self.stale_schema():
            self.clear()
            self.generate()
        else:
            print("Schema up-to-date. No code generated")

    def clear(self):
        """Clears auto-generated files to prepare for a new run"""
        # Remove entire auto-gen directory
        if os.path.exists(self.generated_code_path):
            shutil.rmtree(self.generated_code_path)

    def generate(self):
        """
        Generates files for all generators
        """
        print(f"Starting code generation for {self.generated_code_path}")

        # Make sure that directory path exists
        os.makedirs(self.generated_code_path, exist_ok=True)

        # Perform code generation
        out_paths = []
        for generator in self.code_generators:
            new_paths = generator.generate()
            out_paths.extend(new_paths) # Aggregate all output files to add to file_list.txt

        out_headers = []
        for path in out_paths:
            # Check extension
            split_tup = os.path.splitext(path)
            ext = split_tup[1]
            if ext == ".h":
                out_headers.append(path)

        # Write paths of all generated files to file_list.txt
        out_paths_joined = ";".join(out_paths)
        file_list_path = f"{self.generated_code_path}/{self._list_file_name}"
        print(f"Output file_list: {file_list_path}")
        with open(file_list_path, "w") as list_file:
            list_file.write(out_paths_joined)

        # Write paths of all generated headers to header_list.txt
        out_headers_joined = ";".join(out_headers)
        header_list_path = f"{self.generated_code_path}/{self._list_header_file_name}"
        with open(header_list_path, "w") as list_file:
            list_file.write(out_headers_joined)

        # Write schema version to file
        with open(self._autogenerated_schema_path, "w") as generated_schema_file:
            generated_schema_file.write(self._get_schema_version_yaml_string())

        print("Auto-generation complete")