# """
# Blender script to export the scene as a Collada (.dae) file.
# The output path is specified on the command line of blender after the double-dash ('--').
#
# Example usage:
#   /path/to/blender --factory-startup /path/to/scene.blend -P /path/to/ExportCollada.py -- /path/to/scene.dae
#
# """
import bpy
import os
import sys

try:
    output_path = sys.argv[sys.argv.index("--") + 1]
except (ValueError, IndexError):
    print("export.py: error: missing output path argument", file=sys.stderr)
    sys.exit(1)

bpy.ops.wm.collada_export(filepath=output_path)