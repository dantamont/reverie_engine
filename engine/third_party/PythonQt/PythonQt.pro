# Specifies the name of the template to usue when generating the project
# Creates a Makefile for building targets in subdirectories. The subdirectories are specified using the SUBDIRS variable.
DEFINES += PYTHONQT_STATIC_LIB

TEMPLATE = subdirs

CONFIG += ordered
SUBDIRS = generator src extensions tests examples
