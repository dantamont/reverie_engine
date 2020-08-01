# Specifies the name of the template to usue when generating the project
# Creates a Makefile for building targets in subdirectories. The subdirectories are specified using the SUBDIRS variable.
TEMPLATE = subdirs

# User-added
CONFIG(release, debug|release){
DEFINES += "_ITERATOR_DEBUG_LEVEL=0"
}
CONFIG-=debug
CONFIG+=release

CONFIG += ordered
SUBDIRS = generator src extensions tests examples
