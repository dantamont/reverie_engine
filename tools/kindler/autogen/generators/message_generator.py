# -*- coding: utf-8 -*-
"""File containing generator class"""

from generators.generator import Generator
from jinja2 import Environment, Template
import yaml
import os

class MessageGenerator(Generator):
    """Base class for auto-generating message classes"""

    _library_name = "ripple"
    _header_prefix = f"include/{_library_name}/network/messages"
    _source_prefix = f"{_library_name}/network/messages"
    _valid_member_descriptors = [
        "type", 
        "default_value",
        "description", 
        "deserialize_function"
        ]

    def __init__(self):
        """
        Parameters
        ----------
        name : str
            The name of the generator

        Returns
        -------
        """
        Generator.__init__(self, "Message Generator")

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
        self.gmessage_h_template = "gmessage_autogen/g_message.h.j2"
        self.gmessage_cpp_template = "gmessage_autogen/g_message.cpp.j2"
        self.gmessage_port_h_template = "gmessage_autogen/g_message_port.h.j2"
        self.gmessage_port_cpp_template = "gmessage_autogen/g_message_port.cpp.j2"
        self.gmessage_child_h_template = "gmessage_autogen/g_message_child.h.j2"
        self.gmessage_child_cpp_template = "gmessage_autogen/g_message_child.cpp.j2"
        self.gmessage_definitions_file = f"{self.data_path}/g_message_definitions.yaml"
        self.schema_version_file = f"{self.data_path}/g_schema_version.yaml"

        # Grab message definitions
        self.message_data = {}
        self.message_data["library_name"] = self._library_name
        with open(self.gmessage_definitions_file, "r") as yaml_stream:
            docs = yaml.safe_load_all(yaml_stream)
            my_docs = []
            for my_doc in docs:
                my_docs.append(my_doc)
            self.message_data["message_definitions"] = my_docs[0]

        # Aggregate message types
        self.message_data["message_types"] = []
        for message_type, message in self.message_data["message_definitions"]["messages"].items():
            self.message_data["message_types"].append(message_type)

    def update_schema_version(self):
        """Return whether or not to update schema version"""
        # Bump major version if message file modified
        if os.path.getmtime(self.gmessage_definitions_file) > os.path.getmtime(self.schema_version_file):
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

        # Render GMessage.h and GMessage.cpp
        g_message_h = self.render(self.gmessage_h_template, 
                                  f"{self._header_prefix}/GMessage.h",
                                  **self.message_data)
        g_message_cpp = self.render(self.gmessage_cpp_template, 
                                    f"{self._source_prefix}/GMessage.cpp",
                                    **self.message_data)        

        # Render GMessagePort.h and GMessagePort.cpp
        g_message_port_h = self.render(self.gmessage_port_h_template, 
                                       f"{self._header_prefix}/GMessagePort.h",
                                       **self.message_data)        
        g_message_port_cpp = self.render(self.gmessage_port_cpp_template, 
                                       f"{self._source_prefix}/GMessagePort.cpp",
                                       **self.message_data)  

        # Render child message types
        message_count = 0
        messages = self.message_data["message_definitions"]["messages"]
        for message_type, message in messages.items():
            # Validate the message
            message_copy = dict(message) # Necessary so as not to modify original message
            while message_copy.get("parent") is not None:
                # Check that no members are repeated, or improperly specified
                parent_type = message_copy["parent"]
                parent = dict(messages[parent_type]) # Don't want to modify original
                if "members" in message_copy:
                    for member_name, member_values in message_copy["members"].items():
                        if member_name in parent["members"]:
                            raise ValueError(f"Duplicate member name: '{member_name}'")
                        for descriptor_name, descriptor_value in member_values.items():
                            if not descriptor_name in self._valid_member_descriptors:
                                raise ValueError(f"Invalid descriptor: '{descriptor_name}'")
                message_copy = parent

            print(f"Generating files for {message_type}")

            # Set the required data for the template
            self.message_data["message_index"] = message_count
            self.message_data["parent_type"] = message.get("parent", "Message")
            self.message_data["message_type"] = message_type
            
            # Render the message header/source
            g_message_child_h = self.render(self.gmessage_child_h_template, 
                                  f"{self._header_prefix}/G{message_type}.h",
                                  **self.message_data)
            g_message_child_cpp = self.render(self.gmessage_child_cpp_template, 
                                  f"{self._source_prefix}/G{message_type}.cpp",
                                  **self.message_data)
            message_count += 1

        return self.generated_paths
    