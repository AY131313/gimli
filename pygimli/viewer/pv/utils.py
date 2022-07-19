#!/usr/bin/env python
# -*- coding: utf-8 -*-
import numpy as np
import pygimli as pg

pv = pg.optImport('pyvista', requiredFor="properly visualize 3D data")

pgVTKCELLTypes = {
    pg.core.MESH_EDGE_CELL_RTTI:     3,
    pg.core.MESH_EDGE_RTTI:          3,
    pg.core.MESH_EDGE3_RTTI:         21,
    pg.core.MESH_EDGE3_CELL_RTTI:    21 ,
    pg.core.MESH_TRIANGLE_RTTI:      5  ,
    pg.core.MESH_TRIANGLE6_RTTI:     22 ,
    pg.core.MESH_QUADRANGLE_RTTI:    9  ,
    pg.core.MESH_QUADRANGLE8_RTTI:   23 ,
    pg.core.MESH_TETRAHEDRON_RTTI:   10 ,
    pg.core.MESH_TETRAHEDRON10_RTTI: 24 ,
    pg.core.MESH_TRIPRISM_RTTI:      13 ,
    pg.core.MESH_TRIPRISM15_RTTI:    13 ,
    pg.core.MESH_PYRAMID_RTTI:       14 ,
    pg.core.MESH_PYRAMID13_RTTI:     14 ,
    pg.core.MESH_HEXAHEDRON_RTTI:    12 ,
    pg.core.MESH_HEXAHEDRON20_RTTI:  25 ,
}

def pgMesh2pvMesh(mesh, data=None, label=None, boundaries=False):
    """
    pyGIMLi's mesh format is different from pyvista's needs,
    some preparation is necessary.

    Parameters
    ----------
    mesh: pg.Mesh
        Structure generated by pyGIMLi to display.
    data: iterable
        Parameter to distribute to cells/nodes.
    """
    if boundaries is True:
        mesh.createNeighbourInfos()
        b = mesh.createSubMesh(mesh.boundaries([b.id() for b in mesh.boundaries() if b.outside() or b.marker() != 0]))
        return pgMesh2pvMesh(b, data, label)
    
    if mesh.cellCount() > 0:
        ids = []
        for c in mesh.cells():
            if c.rtti() == pg.core.MESH_TETRAHEDRON10_RTTI:
                # gimli still work with old zienk. counting
                ids.extend([len(c.ids()), *(c.ids()[[0,1,2,3,4,7,5,6,9,8]])])
            else:
                ids.extend([len(c.ids()), *c.ids()])

        grid = pv.UnstructuredGrid(
            np.asarray(ids),
            np.asarray([pgVTKCELLTypes[c.rtti()] for c in mesh.cells()]).flatten(),
            np.asarray(mesh.positions()))

        grid.cell_data['Cell Marker'] = np.asarray(mesh.cellMarkers())

    elif mesh.boundaryCount() > 0:
        grid = pv.PolyData(np.asarray(mesh.positions()), 
                faces=np.hstack([[len(b.ids()), *b.ids()] 
                                        for b in mesh.boundaries()]))
        
        grid.cell_data['Boundary Marker'] = np.asarray(mesh.boundaryMarkers())

        # b = mesh.createSubMesh([b, for b in mesh.boundaries() if b.isoutsidee() or b.marker() != 0])

        # print(b)
        return pgMesh2pvMesh(b, data, label)
    
    if mesh.cellCount() > 0:
        grid = pv.UnstructuredGrid(
            np.asarray([[len(c.ids()), *c.ids()] for c in mesh.cells()]).flatten(),
            np.asarray([pgVTKCELLTypes[c.rtti()] for c in mesh.cells()]).flatten(),
            np.asarray(mesh.positions()))

        grid.cell_data['Cell marker'] = np.asarray(mesh.cellMarkers())

    elif mesh.boundaryCount() > 0:
        #pg._g('faces')
        grid = pv.PolyData(np.asarray(mesh.positions()), 
                faces=[[len(b.ids()), *b.ids()] for b in mesh.boundaries()])
        grid.cell_data['Boundary marker'] = np.asarray(mesh.boundaryMarkers())
        
    else:
        grid = pv.PolyData(np.asarray(mesh.positions()))


    # check for parameters inside the pg.Mesh
    for key, values in mesh.dataMap():
        if len(values) == mesh.cellCount():
            grid.cell_data[key] = np.asarray(values)
        elif len(values) == mesh.nodeCount():
            grid.point_data[key] = np.asarray(values)

    # check the given data as well
    try:
        if data is not None:
            if len(data) == mesh.cellCount():
                if label is None:
                    label = 'Cell data'
                grid.cell_data[label] = np.asarray(data)
            elif len(data) == mesh.nodeCount():
                if label is None:
                    label = 'Node data'
                grid.point_data[label] = np.asarray(data)
            else:
                pg.warn("Given data fits neither cell count nor node count:")
                pg.warn("{} vs. {} vs. {}".format(len(data), mesh.cellCount(),
                                                mesh.nodeCount()))
    except Exception as e:
        print(label)
        print(e)
        pg.error("fix pyvista mesh conversion")

    # print(grid.cell_data)
    # print(grid.point_data)

    if label is None:
        # last data that was added
        label = grid.array_names[-1]
    elif label not in grid.array_names:
        pg.warn("Given label '{}' was not found.".format(label))
        label = grid.array_names[-1]


    grid.set_active_scalars(label)

    return grid
