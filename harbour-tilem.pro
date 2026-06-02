TEMPLATE = subdirs
SUBDIRS = tilem-core src

CONFIG += ordered

# Make sure tilem-core is built before src
src.depends = tilem-core
