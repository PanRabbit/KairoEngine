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
level = "/home/holo/Documents/CodeProjects/KairoEngine/levels/Alley.json"

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


def createMaterial(material_path, material_name):
        # load material definition file
    with open(material_path, 'r') as f:
        material_data = json.load(f)    

    # extract material data with fallbacks
    useDiffuseMap    = material_data.get("useDiffuseMap", False)
    diffuseMapPath   = material_data.get("diffuseMapPath", "")
    diffuseColor     = material_data.get("diffuseColor", [1, 1, 1])

    useAlphaMap      = material_data.get("useAlphaMap", False) # UNIMPLEMENTED
    alphaMapPath     = material_data.get("alphaMapPath", "") # UNIMPLEMENTED

    useNormalMap     = material_data.get("useNormalMap", False)
    normalMapPath    = material_data.get("normalMapPath", "")

    useChecker       = material_data.get("useChecker", False) # UNIMPLEMENTED
    checkerSize      = material_data.get("checkerSize", 1.0) # UNIMPLEMENTED
    secondaryColor   = material_data.get("secondaryColor", [0, 0, 0]) # UNIMPLEMENTED

    useRoughnessMap  = material_data.get("useRoughnessMap", False)
    roughnessMapPath = material_data.get("roughnessMapPath", "")
    roughness        = material_data.get("roughness", 1.0)

    useMetallicMap   = material_data.get("useMetallicMap", False)
    metallicMapPath  = material_data.get("metallicMapPath", "")
    metallic         = material_data.get("metallic", 0.0)

    useAO            = material_data.get("useAO", False) # UNIMPLEMENTED
    aoMapPath        = material_data.get("aoMapPath", "") # UNIMPLEMENTED
    ao               = material_data.get("ao", 1.0) # UNIMPLEMENTED

    transparent      = material_data.get("transparent", False) # UNIMPLEMENTED

    coordOffset      = material_data.get("coordOffset", [0, 0]) # UNIMPLEMENTED
    coordScale       = material_data.get("coordScale", [1, 1]) # UNIMPLEMENTED

    # create material
    mat = bpy.data.materials.new(name=material_name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes["Principled BSDF"]
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links

    bsdf.inputs["Base Color"].default_value = (*diffuseColor, 1.0)
    bsdf.inputs["Roughness"].default_value = roughness
    bsdf.inputs["Metallic"].default_value = metallic


    # create texture node
    def add_tex(map_path, non_color=False):
            tex = nodes.new("ShaderNodeTexImage")
            tex.image = bpy.data.images.load(engine_dir + map_path)
            if non_color:
                tex.image.colorspace_settings.name = "Non-Color"
            return tex

    if useDiffuseMap:
        tex = add_tex(diffuseMapPath)
        links.new(tex.outputs["Color"], bsdf.inputs["Base Color"])
    if useRoughnessMap:
        tex = add_tex(roughnessMapPath, non_color=True)
        links.new(tex.outputs["Color"], bsdf.inputs["Roughness"])
    if useMetallicMap:
        tex = add_tex(metallicMapPath, non_color=True)
        links.new(tex.outputs["Color"], bsdf.inputs["Metallic"])
    if useNormalMap:
        tex = add_tex(normalMapPath, non_color=True)
        nmap = nodes.new("ShaderNodeNormalMap")
        links.new(tex.outputs["Color"], nmap.inputs["Color"])
        links.new(nmap.outputs["Normal"], bsdf.inputs["Normal"])

    return mat



def blenderObjectName(name): # Remove blender's extra .00x suffix, not names that already end in .001
    if name and "." in name:
        head, tail = name.rsplit(".", 1)
        if tail.isdigit():
            return head
    return name or ""

def lookupSlot(name, material_slots):
    if name in material_slots:
        return material_slots[name]
    base = blenderObjectName(name)
    if base != name and base in material_slots:
        return material_slots[base]
    return None

def assignMaterials(obj, material_slots):
    # Faces use polygon.material_index into mesh.materials.
    # Only replace slots this mesh already has — never append the whole level list.
    if obj.type != "MESH":
        return
    mesh = obj.data

    matched = False
    for i, slot in enumerate(obj.material_slots):
        mat = lookupSlot(slot.name, material_slots)
        if mat is not None:
            mesh.materials[i] = mat
            matched = True

    if matched:
        return

    # OBJ with no usemtl (cube.obj) and a single kairo material
    if len(mesh.materials) == 0 and len(material_slots) == 1:
        mesh.materials.append(next(iter(material_slots.values())))



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
        

        created_materials = {}

        # ==========================
        # IMPORT MODELS
        # ==========================
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

            # ==========================
            # IMPORT MATERIALS
            # ==========================

            material_slots = {}
            for slot_name, material_name in obj["materials"].items():
                if material_name not in created_materials:
                    material_path = engine_dir + "materials/" + material_name + ".json"
                    created_materials[material_name] = createMaterial(material_path, material_name)

                material_slots[slot_name] = created_materials[material_name]

            for imp in imported:
                assignMaterials(imp, material_slots)

        
        # ==========================
        # IMPORT LIGHTS
        # ==========================

        # ========== SUN ==========
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
        
        # ========== SPOTLIGHTS ==========
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
        
        # ========== POINTLIGHTS ==========
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