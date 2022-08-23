# -*- coding: utf-8 -*-
"""File containing generator class"""

from generators.generator import Generator
from jinja2 import Environment, Template
import yaml
import os

class EnumGenerator(Generator):
    """Class for generating enumerations"""

    _library_name = "enums"
    _header_prefix = f"include/{_library_name}"
    _source_prefix = f"{_library_name}"
    _valid_member_descriptors = ["type", "start_value"]

    def __init__(self):
        """
        Parameters
        ----------
        name : str
            The name of the generator

        Returns
        -------
        """
        Generator.__init__(self, "Enum Generator")

    def initialize(self, generated_code_path: str, data_path: str, template_env: Environment):
        """
        Initialize the generator with paths and template metadata

        Parameters
        ----------
        generated_code_path: str
            The path at which to output all files
        data_path: str
            The path in which generator data lives
        template_env: Environment
            The Jinja2 template environment used to obtain and render templates
        Returns
        -------
        list
            A list of strings containing the absolute paths of all generated files
        """

        Generator.initialize(self, generated_code_path, data_path, template_env)
        self.genum_h_template = "genum_autogen/g_enum.h.j2"
        self.genum_cpp_template = "genum_autogen/g_enum.cpp.j2"
        self.genum_definitions_file = f"{self.data_path}/g_enum_definitions.yaml"
        self.schema_version_file = f"{self.data_path}/g_schema_version.yaml"

        # Grab enum definitions
        self.enum_data = {}
        self.enum_data["library_name"] = self._library_name
        with open(self.genum_definitions_file, "r") as yaml_stream:
            docs = yaml.safe_load_all(yaml_stream)
            my_docs = []
            for my_doc in docs:
                my_docs.append(my_doc)
            self.enum_data["enum_definitions"] = my_docs[0]

        # Aggregate enum types
        self.enum_data["enum_types"] = []
        for enum_type, enum in self.enum_data["enum_definitions"]["enums"].items():
            self.enum_data["enum_types"].append(enum_type)

    def update_schema_version(self):
        """Return whether or not to update schema version"""
        # Bump major version if enum file modified
        if os.path.getmtime(self.genum_definitions_file) > os.path.getmtime(self.schema_version_file):
            return [True, False, False] # major, minor, patch
        else:
            return [False, False, False] # major, minor, patch

    def generate(self):
        """
        Generates files for this generator

        Parameters
        ----------

        Returns
        -------
        list
            A list of strings containing the absolute paths of all generated files
        """
        # Call base class method
        Generator.generate(self)

        # Render enum types
        enum_count = 0
        enums = self.enum_data["enum_definitions"]["enums"]
        for enum_type, enum in enums.items():
            # Validate the enum
            if "type" not in enum:
                enum["type"] = "Int32_t"

            if "is_flag" not in enum:
                enum["is_flag"] = False

            if "start_value" not in enum:
                enum["start_value"] = 0

            print(f"Generating files for {enum_type}")

            # Set the required data for the template
            self.enum_data["enum_index"] = enum_count
            self.enum_data["enum_type"] = enum_type
            
            # Render the enum header/source
            g_enum_child_h = self.render(self.genum_h_template, 
                                  f"{self._header_prefix}/G{enum_type}Enum.h",
                                  **self.enum_data)
            g_enum_child_cpp = self.render(self.genum_cpp_template, 
                                  f"{self._source_prefix}/G{enum_type}Enum.cpp",
                                  **self.enum_data)
            enum_count += 1

        return self.generated_paths
    