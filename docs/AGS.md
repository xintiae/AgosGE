# The AGS file format

## Introduction

The AGS file format is a similar concept to the UAsset format in Unreal Engine 4; all the required assets for a game (like Models or Textures) are unified in one format.

## Specification

### Sections

The contents of an AGS file are split up into __sections__. A section begins with one line which simply contains the name of the sections (see [types of sections](###Types-of-Sections)) wrapped inside hashtags. The data follows in the next line.

Example of some sections:

```
#type#
model

#vertices#
-1.000000 0.000000 0.000000
...
```

### Types of Sections

- type: Specifies which asset is being stored. See [Types of Assets](#Types-of-assets-and-their-formats)
- name (optional): A possible name of the asset which will be displayed in the engine. If there is none, then the name of the (imported) file will be used.

#### Sections of a 3D-Model asset

- vertices: A list of all vertices (Note: indices are determined by the order of the list)
- vertices_normals: A list of the normal vectors of their corresponding vertex 
- vertex_texture: A list of the XY-coordinates of a texture to their corresponding vertex.

#### Sections of a Texture

TODO

### Types of assets and their formats

Below is a list of the data types which can be saved in an AGS file. After that, the value of the *type* keyword and the currently supported formats for importing is specified.

- 3D-Models - model (.obj)
- Textures - tex (.png)