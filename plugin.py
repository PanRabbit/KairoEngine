bl_info = {
    "name": "Kario Engine Tools",
    "author": "PanRabbit",
}

import bpy
import os
import json
import math

from mathutils import Euler, Matrix, Vector

from bpy_extras.io_utils import ImportHelper
from bpy.props import StringProperty
from bpy.types import Operator

# Engine Dir
engine_dir = "/home/holo/Documents/CodeProjects/KairoEngine/"
level = "/home/holo/Documents/CodeProjects/KairoEngine/levels/day.json"

# Convertion matrices
M = Matrix.Rotation(math.radians(90.0), 3, 'X')


def convertRot(deg_xyz):
    rx, ry, rz = (math.radians(a) for a in deg_xyz)
    Rx = Matrix.Rotation(rx, 3, 'X')
    Ry = Matrix.Rotation(ry, 3, 'Y')
    Rz = Matrix.Rotation(rz, 3, 'Z')
    R_engine = Rx @ Ry @ Rz        # matches game_object.h 58-63
    R_final  = M @ R_engine        # convert to Z-up
    return R_final.to_euler('XYZ')

def convertLoc(loc):
    x, y, z = loc
    finalLoc = (x, -z, y)
    return finalLoc

# class OT_JSONParseOperator(Operator, ImportHelper):
class OT_JSONParseOperator(Operator): # ImportHelper opens the file browser
    """Load KairoEngine Level JSON"""
    bl_idname = "json.read"
    bl_label = "Read Level File"
    bl_options = {'REGISTER'}
    
    filter_glob: StringProperty(
        default = "*.json",
        options = {"HIDDEN"},
    )

    def execute(self, context):
        with open(level, 'r') as f: # self.filepath
            d = json.load(f)
            
        # game objects
        for obj in d["GameObjects"].values():
            bpy.ops.object.select_all(action='DESELECT')   # clear selection first
            bpy.ops.wm.obj_import(filepath=engine_dir + obj["modelPath"])
            imported = list(context.selected_objects)      # these are the new ones
            for imp in imported:
                
                imp.location = convertLoc(obj["location"])
        
                imp.rotation_mode = 'XYZ'
                imp.rotation_euler = convertRot(obj["rotation"])

                imp.scale = obj["scale"]
        
        # sun
        bpy.ops.object.select_all(action='DESELECT')

        bpy.ops.object.light_add(
            type='SUN',
            location=(0.0, -2.0, 0.0),
        )

        sun = context.object
        sun.data.color  = tuple(d["SunColor"][:3])
        sun.data.energy = 1.0

        sun_dir_blender = M @ Vector(d["SunDirection"])
        sun.rotation_euler = sun_dir_blender.to_track_quat('-Z', 'Y').to_euler()
        
        # spotlights
        for spot in d["SpotLights"].values():
            bpy.ops.object.select_all(action='DESELECT')

            bpy.ops.object.light_add(
                type='SPOT',
                location=convertLoc(spot["position"]),
            )

            light = context.object
            light.data.color  = tuple(spot["color"][:3])
            light.data.energy = spot["intensity"] * 10  # 10x to match visually from engine


            outer = spot["outerCutOff"]
            inner = spot["cutOff"]
            light.data.spot_size  = math.radians(outer)
            light.data.spot_blend = 1.0 - (inner / outer) # Engine uses inner/outer angles, blender uses ratio

            # Range cutoff (EEVEE only, Cycles ignores this)
            light.data.use_custom_distance = True
            light.data.cutoff_distance     = spot["radius"]

            # engine stores direction in Y-up, convert to Z-up, then aim -Z along it
            dir_blender = M @ Vector(spot["direction"])
            light.rotation_euler = dir_blender.to_track_quat('-Z', 'Y').to_euler()
        
        # pointlights
        for pointLight in d["PointLights"].values():
            bpy.ops.object.select_all(action='DESELECT')

            bpy.ops.object.light_add(
                type='POINT',
                location=convertLoc(pointLight["position"]),
                radius=pointLight["radius"],
            )

            light = context.object   # the newly added light is the active object
            light.data.color  = tuple(pointLight["color"][:3])
            light.data.energy = pointLight["intensity"] * 10 # 10x to match visually from engine
        
        # This is what shows up in the Info area and the system console
        self.report({'INFO'}, "Loaded!")
        return {'FINISHED'}



class PT_KairoPanel(bpy.types.Panel):
    bl_label = "Level Stuff"
    bl_idname = "READ_PT_PANEL"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = "KairoEngine"

    def draw(self, context):
        layout = self.layout
        layout.operator(OT_JSONParseOperator.bl_idname, icon='INFO')


classes = (OT_JSONParseOperator, PT_KairoPanel)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)


if __name__ == "__main__":
    register()